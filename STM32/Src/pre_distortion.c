#include "pre_distortion.h"
#include "atu.h"
#include "fft.h"
#include "lcd.h"
#include "rf_unit.h"
#include "trx_manager.h"

static float32_t DPD_distortion_gain_points[DPD_POINTS] = {0};
static float32_t DPD_distortion_gain_points_best[DPD_POINTS] = {0};
static float32_t DPD_distortion_gain_points_locked[DPD_POINTS] = {0};

static const float32_t DPD_distortion_gain_points_lock_stage0[DPD_POINTS] = {1, 0, 0, 0, 1, 0, 0, 0, 1}; // 0 - skip, 1 - in progress, 2 - finished
static const float32_t DPD_distortion_gain_points_lock_stage1[DPD_POINTS] = {2, 0, 0, 0, 1, 0, 0, 0, 2};
static const float32_t DPD_distortion_gain_points_lock_stage2[DPD_POINTS] = {2, 0, 1, 0, 2, 0, 1, 0, 2};
static const float32_t DPD_distortion_gain_points_lock_stage3[DPD_POINTS] = {2, 1, 2, 1, 2, 0, 2, 0, 2};
static const float32_t DPD_distortion_gain_points_lock_stage4[DPD_POINTS] = {2, 2, 2, 2, 2, 1, 2, 1, 2};
static const float32_t DPD_distortion_gain_points_lock_stage5[DPD_POINTS] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

static float32_t DPD_best_imd_sum = 0;
static float32_t DPD_max_rms = 0;
static bool DPD_need_calibration = false;
static bool DPD_start_imd_printed = false;
static float32_t DPD_prev_cycle_imd_sum_max = 0;
static float32_t DPD_curr_cycle_imd_sum_max = 0;
static uint8_t DPD_calibration_stage = 0;
static uint32_t DPD_current_point = 0;
static bool DPD_current_direction = true; // false - left, true - right

static bool DPD_OLD_TWO_SIGNAL_TUNE = false;
static bool DPD_OLD_Full_Duplex = false;
static uint8_t DPD_OLD_TUNE_MAX_POWER = false;

static float32_t DPD_SPLINE_Xbuff[DPD_POINTS];
static float32_t DPD_SPLINE_Ybuff[DPD_POINTS];
static float32_t DPD_SPLINE_tempbuf[2 * DPD_POINTS - 1];
static float32_t DPD_SPLINE_coef[3 * (DPD_POINTS - 1)];
static arm_spline_instance_f32 DPD_SPLINE_Instance;
static bool DPD_SPLINE_NeedReinit = true;

static void DPD_checkCurrentPointAvailable();
static void DPD_printDbmStatus(float32_t imd3, float32_t imd5);
static void DPD_getDistortionForSample(float32_t *i, float32_t *q);

void DPD_Init() {
	DPD_calibration_stage = 0;
	DPD_current_point = 0;
	DPD_current_direction = true;

	for (uint32_t index = 0; index < DPD_POINTS; index++) {
		DPD_distortion_gain_points[index] = 1.0f + index * 0.0000001f;
		DPD_distortion_gain_points_best[index] = DPD_distortion_gain_points[index];
		memcpy(DPD_distortion_gain_points_locked, DPD_distortion_gain_points_lock_stage0, sizeof(DPD_distortion_gain_points_locked));
	}

	DPD_SPLINE_NeedReinit = true;
}

void DPD_ProcessPredistortion(float32_t *buffer_i, float32_t *buffer_q, uint32_t size) {
	if (!TRX.Digital_Pre_Distortion) {
		return;
	}

	for (uint32_t index = 0; index < size; index++) {
		DPD_getDistortionForSample(&buffer_i[index], &buffer_q[index]);
	}
}

void DPD_StartCalibration() {
	if (!DPD_need_calibration) { // start
		DPD_prev_cycle_imd_sum_max = 0;
		DPD_curr_cycle_imd_sum_max = 0;
		DPD_max_rms = 0;
		DPD_best_imd_sum = 0;

		DPD_OLD_TWO_SIGNAL_TUNE = TRX.TWO_SIGNAL_TUNE;
		DPD_OLD_Full_Duplex = TRX.Full_Duplex;
		DPD_OLD_TUNE_MAX_POWER = CALIBRATE.TUNE_MAX_POWER;

		TRX.TWO_SIGNAL_TUNE = true;
		TRX.Full_Duplex = true;
		CALIBRATE.TUNE_MAX_POWER = CALIBRATE.MAX_RF_POWER_ON_METER;
		TRX.FFT_Averaging = FFT_MAX_MEANS;

		DPD_Init();

		if (!TRX_Tune) {
			BUTTONHANDLER_TUNE(0);
		}

		DPD_start_imd_printed = false;
	} else { // stop
		TRX.TWO_SIGNAL_TUNE = DPD_OLD_TWO_SIGNAL_TUNE;
		TRX.Full_Duplex = DPD_OLD_Full_Duplex;
		CALIBRATE.TUNE_MAX_POWER = DPD_OLD_TUNE_MAX_POWER;

		DPD_Init();

		if (TRX_Tune) {
			BUTTONHANDLER_TUNE(0);
		}
	}

	DPD_need_calibration = !DPD_need_calibration;
}

void DPD_ProcessCalibration() {
	if (!ATU_TunePowerStabilized) {
		return;
	}

	static uint32_t DPD_last_dbm_show_time = 0;
	if (HAL_GetTick() - DPD_last_dbm_show_time > 1000) {
		DPD_last_dbm_show_time = HAL_GetTick();

		DPD_printDbmStatus(FFT_Current_TX_IMD3, FFT_Current_TX_IMD5);
	}

	if (!DPD_need_calibration) {
		return;
	}
	if (!TRX.Digital_Pre_Distortion) {
		return;
	}

	static float32_t prev_imd_sum = 0;
	static int32_t error_count = 0;

	float32_t current_imd_sum = FFT_Current_TX_IMD3 + FFT_Current_TX_IMD5 / 2.0f + FFT_Current_TX_IMD7 / 4.0f + FFT_Current_TX_IMD9 / 8.0f;

	if (!DPD_start_imd_printed) {
		DPD_start_imd_printed = true;
		DPD_printDbmStatus(FFT_Current_TX_IMD3, FFT_Current_TX_IMD5);
	}

	DPD_checkCurrentPointAvailable();

	if (prev_imd_sum > current_imd_sum) {
		error_count++;
	} else {
		if (error_count > -DPD_MAX_ERRORS) {
			error_count--;
		}

		if (DPD_best_imd_sum < current_imd_sum) {
			DPD_distortion_gain_points_best[DPD_current_point] = DPD_distortion_gain_points[DPD_current_point];
			DPD_best_imd_sum = current_imd_sum;
		}
	}
	prev_imd_sum = current_imd_sum;

	// switch point on error, after cycle switch direction
	if (error_count >= DPD_MAX_ERRORS) {
		DPD_distortion_gain_points[DPD_current_point] = DPD_distortion_gain_points_best[DPD_current_point]; // revert changes
		DPD_SPLINE_NeedReinit = true;

		DPD_current_point++;
		error_count = 0;

		while (DPD_distortion_gain_points_locked[DPD_current_point] != 1 && DPD_current_point < DPD_POINTS) {
			DPD_current_point++;
		}

		if (DPD_current_point >= DPD_POINTS) { // end of points
			DPD_current_point = 0;
			DPD_checkCurrentPointAvailable();
			DPD_current_direction = !DPD_current_direction;

			if (DPD_current_direction == true) { // end cycle
				println("DPD end cycle");

				DPD_calibration_stage++;
				float32_t spline_result = 0;
				float32_t rms_on_point = 0;

				if (DPD_calibration_stage == 1) {
					memcpy(DPD_distortion_gain_points_locked, DPD_distortion_gain_points_lock_stage1, sizeof(DPD_distortion_gain_points_locked));
					DPD_SPLINE_NeedReinit = true;
				}

				if (DPD_calibration_stage == 2) {
					memcpy(DPD_distortion_gain_points_locked, DPD_distortion_gain_points_lock_stage2, sizeof(DPD_distortion_gain_points_locked));
					DPD_SPLINE_NeedReinit = true;

					DPD_distortion_gain_points_best[2] = (DPD_distortion_gain_points[0] + DPD_distortion_gain_points[4]) / 2.0f;
					DPD_distortion_gain_points_best[6] = (DPD_distortion_gain_points[4] + DPD_distortion_gain_points[8]) / 2.0f;
				}

				if (DPD_calibration_stage == 3) {
					memcpy(DPD_distortion_gain_points_locked, DPD_distortion_gain_points_lock_stage3, sizeof(DPD_distortion_gain_points_locked));
					DPD_SPLINE_NeedReinit = true;

					DPD_distortion_gain_points_best[1] = (DPD_distortion_gain_points[0] + DPD_distortion_gain_points[2]) / 2.0f;
					DPD_distortion_gain_points_best[3] = (DPD_distortion_gain_points[2] + DPD_distortion_gain_points[4]) / 2.0f;
				}

				if (DPD_calibration_stage == 4) {
					memcpy(DPD_distortion_gain_points_locked, DPD_distortion_gain_points_lock_stage4, sizeof(DPD_distortion_gain_points_locked));
					DPD_SPLINE_NeedReinit = true;

					DPD_distortion_gain_points_best[5] = (DPD_distortion_gain_points[4] + DPD_distortion_gain_points[6]) / 2.0f;
					DPD_distortion_gain_points_best[7] = (DPD_distortion_gain_points[6] + DPD_distortion_gain_points[8]) / 2.0f;
				}

				if (DPD_calibration_stage == 5) {
					memcpy(DPD_distortion_gain_points_locked, DPD_distortion_gain_points_lock_stage5, sizeof(DPD_distortion_gain_points_locked));
					DPD_SPLINE_NeedReinit = true;
				}

				// stage 6 - fine repeat 5

				DPD_current_point = 0;
				DPD_checkCurrentPointAvailable();
				memcpy(DPD_distortion_gain_points, DPD_distortion_gain_points_best, sizeof(DPD_distortion_gain_points));
				DPD_SPLINE_NeedReinit = true;

				if (DPD_calibration_stage == 7) {
					DPD_need_calibration = false;

					if (TRX_Tune) {
						BUTTONHANDLER_TUNE(0);
					}

					TRX.TWO_SIGNAL_TUNE = DPD_OLD_TWO_SIGNAL_TUNE;
					TRX.Full_Duplex = DPD_OLD_Full_Duplex;
					CALIBRATE.TUNE_MAX_POWER = DPD_OLD_TUNE_MAX_POWER;

					LCD_showTooltip("DPD Calibr complete");

					for (uint32_t index = 0; index < DPD_POINTS; index++) {
						DPD_distortion_gain_points[index] = DPD_distortion_gain_points_best[index];
						DPD_SPLINE_NeedReinit = true;
						println(index, ": ", DPD_distortion_gain_points[index]);
					}
				}

				DPD_prev_cycle_imd_sum_max = DPD_curr_cycle_imd_sum_max;
				DPD_curr_cycle_imd_sum_max = 0;
			}
		}

		return;
	}

	DPD_checkCurrentPointAvailable();
	if (DPD_current_point == DPD_POINTS - 1) { // only plus direction for last point
		DPD_distortion_gain_points[DPD_current_point] += 1.0f * DPD_CORRECTION_GAIN_STEP;
	} else {
		DPD_distortion_gain_points[DPD_current_point] += (DPD_current_direction ? 1.0f : -1.0f) * DPD_CORRECTION_GAIN_STEP;
	}
	DPD_SPLINE_NeedReinit = true;

	if (DPD_curr_cycle_imd_sum_max < current_imd_sum) {
		DPD_curr_cycle_imd_sum_max = current_imd_sum;
	}

	print("DPD stage: ", DPD_calibration_stage, " point: ", DPD_current_point, " dir: ", (DPD_current_direction ? "+" : "-"), " err: ", error_count);
	println(" gain: ", DPD_distortion_gain_points[DPD_current_point], " BEST: ", DPD_best_imd_sum, " PREV: ", DPD_prev_cycle_imd_sum_max, " CUR: ", DPD_curr_cycle_imd_sum_max);
}

static void DPD_checkCurrentPointAvailable() {
	while (DPD_distortion_gain_points_locked[DPD_current_point] != 1 && DPD_current_point < DPD_POINTS) {
		DPD_current_point++;
	}
	if (DPD_current_point >= DPD_POINTS) {
		DPD_current_point = 0;

		while (DPD_distortion_gain_points_locked[DPD_current_point] != 1 && DPD_current_point < DPD_POINTS) {
			DPD_current_point++;
		}
	}
}

static void DPD_printDbmStatus(float32_t imd3, float32_t imd5) {
	char ctmp[32] = {0};
	sprintf(ctmp, "IMD3: %.1f IMD5: %.1f", imd3, imd5);
	LCD_showTooltip(ctmp);
}

static void DPD_getDistortionForSample(float32_t *i, float32_t *q) {
	float32_t rms = sqrtf(*i * *i + *q * *q);
	if (rms > DPD_max_rms && DPD_need_calibration) {
		DPD_max_rms = rms;
		DPD_SPLINE_NeedReinit = true;
	}

	if (rms > DPD_max_rms) { // overflow over table
		float32_t gain_correction = DPD_distortion_gain_points[DPD_POINTS - 1];
		*i = *i * gain_correction;
		*q = *q * gain_correction;
		return;
	}

	// Spline reinit
	if (DPD_SPLINE_NeedReinit) {
		uint8_t point_count = 0;

		for (uint8_t i = 0; i < DPD_POINTS; i++) {
			if (DPD_distortion_gain_points_locked[i] == 0) {
				continue;
			}

			float32_t y = DPD_distortion_gain_points[i];

			if (point_count > 0 && y == DPD_SPLINE_Ybuff[point_count - 1]) { // skip equal
				continue;
			}

			DPD_SPLINE_Xbuff[point_count] = (float32_t)i * DPD_max_rms / (float32_t)(DPD_POINTS - 1) + 0.0001f;
			DPD_SPLINE_Ybuff[point_count] = y;
			point_count++;
		}

		if (point_count > 2) { // minimal points count
			// ARM_SPLINE_NATURAL / ARM_SPLINE_PARABOLIC_RUNOUT
			arm_spline_init_f32(&DPD_SPLINE_Instance, ARM_SPLINE_NATURAL, DPD_SPLINE_Xbuff, DPD_SPLINE_Ybuff, point_count, DPD_SPLINE_coef, DPD_SPLINE_tempbuf);
		}

		DPD_SPLINE_NeedReinit = false;
	}

	if (DPD_SPLINE_Instance.n_x < 3) { // no point for inperpolation
		return;
	}

	// get cubic spline data
	float32_t gain_correction;
	arm_spline_f32(&DPD_SPLINE_Instance, &rms, &gain_correction, 1);

	// debug each 1000 sample
	/* static uint32_t debug_counter = 0;
	debug_counter++;
	if(debug_counter > 1000) {
	  debug_counter = 0;
	  println("RMS: ", rms, " G: ", gain_correction);
	} */

	// apply distortion
	*i = *i * gain_correction;
	*q = *q * gain_correction;
}
