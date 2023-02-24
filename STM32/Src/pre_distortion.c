#include "pre_distortion.h"
#include "fft.h"
#include "lcd.h"
#include "rf_unit.h"
#include "trx_manager.h"

static float32_t DPD_distortion_gain_points[DPD_POINTS] = {0};
static float32_t DPD_distortion_gain_points_best[DPD_POINTS] = {0};
static float32_t DPD_best_imd_sum = 0;
static float32_t DPD_best_imd3 = 0;
static float32_t DPD_best_imd5 = 0;
static float32_t DPD_max_rms = 0;
static bool DPD_need_calibration = false;
static bool DPD_start_imd_printed = false;
static float32_t DPD_prev_cycle_imd_sum_max = 0;
static float32_t DPD_curr_cycle_imd_sum_max = 0;

static bool DPD_OLD_TWO_SIGNAL_TUNE = false;
static bool DPD_OLD_Full_Duplex = false;
static uint8_t DPD_OLD_TUNE_MAX_POWER = false;

static void DPD_printDbmStatus(float32_t imd3, float32_t imd5);
static void DPD_getDistortionForSample(float32_t *i, float32_t *q);

void DPD_Init() {
	for (uint32_t index = 0; index < DPD_POINTS; index++) {
		DPD_distortion_gain_points[index] = 1.0f;
		DPD_distortion_gain_points_best[index] = 1.0f;
	}
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
		DPD_best_imd3 = 0;
		DPD_best_imd5 = 0;

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

	println("IMD3: ", FFT_Current_TX_IMD3, " IMD5: ", FFT_Current_TX_IMD5);

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

	static uint32_t current_point = 0;
	static bool current_direction = true; // false - left, true - right
	static float32_t prev_imd_sum = 0;
	static int32_t error_count = 0;

	float32_t current_imd_sum = FFT_Current_TX_IMD3 + FFT_Current_TX_IMD5;
	if (!DPD_start_imd_printed) {
		DPD_start_imd_printed = true;
		DPD_printDbmStatus(FFT_Current_TX_IMD3, FFT_Current_TX_IMD5);
	}

	if (prev_imd_sum > current_imd_sum) {
		error_count++;
	} else {
		if (error_count > -5) {
			error_count--;
		}

		if (DPD_best_imd_sum < current_imd_sum) {
			DPD_distortion_gain_points_best[current_point] = DPD_distortion_gain_points[current_point];
			DPD_best_imd_sum = current_imd_sum;
			DPD_best_imd3 = FFT_Current_TX_IMD3;
			DPD_best_imd5 = FFT_Current_TX_IMD5;
		}
		DPD_best_imd_sum = DPD_best_imd_sum * 0.999f + current_imd_sum * 0.001f;
	}
	prev_imd_sum = current_imd_sum;

	// switch point on error, after cycle switch direction
	if (error_count >= 5) {
		DPD_distortion_gain_points[current_point] = DPD_distortion_gain_points_best[current_point]; // revert changes

		current_point++;
		error_count = 0;

		DPD_printDbmStatus(DPD_best_imd3, DPD_best_imd5);

		if (current_point >= DPD_POINTS) {
			current_point = 0;
			current_direction = !current_direction;

			if (current_direction == true) { // end cycle
				if (DPD_prev_cycle_imd_sum_max > DPD_curr_cycle_imd_sum_max) {
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
						println(index, ": ", DPD_distortion_gain_points[index]);
					}
				}
				DPD_prev_cycle_imd_sum_max = DPD_curr_cycle_imd_sum_max;
				DPD_curr_cycle_imd_sum_max = 0;
			}
		}

		return;
	}

	DPD_distortion_gain_points[current_point] += (current_direction ? 1.0f : -1.0f) * DPD_CORRECTION_GAIN_STEP;

	if (DPD_curr_cycle_imd_sum_max < current_imd_sum) {
		DPD_curr_cycle_imd_sum_max = current_imd_sum;
	}

	print("DPD point: ", current_point, " dir: ", (current_direction ? "+" : "-"), " err: ", error_count, " gain: ", DPD_distortion_gain_points[current_point]);
	println(" BEST: ", DPD_best_imd_sum, " IMD3: ", FFT_Current_TX_IMD3, " IMD5: ", FFT_Current_TX_IMD5);
}

static void DPD_printDbmStatus(float32_t imd3, float32_t imd5) {
	char ctmp[32] = {0};
	sprintf(ctmp, "IMD3: %.1f IMD5: %.1f", imd3, imd5);
	LCD_showTooltip(ctmp);
}

static void DPD_getDistortionForSample(float32_t *i, float32_t *q) {
	float32_t rms = sqrtf(*i * *i + *q * *q);
	if (rms > DPD_max_rms) {
		DPD_max_rms = rms;
	}

	float32_t max_rms_in_table = DPD_max_rms;

	uint32_t point_left = 0;
	uint32_t point_right = 1;

	// find nearest points
	for (uint32_t index = 1; index < DPD_POINTS; index++) {
		float32_t rms_on_point_prev = (index - 1) * max_rms_in_table / (float32_t)(DPD_POINTS - 1);
		float32_t rms_on_point = index * max_rms_in_table / (float32_t)(DPD_POINTS - 1);

		if (rms >= rms_on_point_prev && rms <= rms_on_point) {
			point_right = index;
		}
	}
	point_left = point_right - 1;

	// calculate point
	float32_t rms_left = point_left * max_rms_in_table / (float32_t)(DPD_POINTS - 1);
	float32_t rms_right = point_right * max_rms_in_table / (float32_t)(DPD_POINTS - 1);
	float32_t rms_diff = rms_right - rms_left;
	float32_t left_diff = rms - rms_left;
	float32_t right_diff = rms_right - rms;
	float32_t left_percent = right_diff / rms_diff;
	float32_t right_percent = left_diff / rms_diff;

	// get gain correction
	float32_t gain_correction = DPD_distortion_gain_points[point_left] * left_percent + DPD_distortion_gain_points[point_right] * right_percent;

	// debug each 1000 sample
	/*static uint32_t debug_counter = 0;
	debug_counter++;
	if(debug_counter > 1000) {
	  debug_counter = 0;
	  println("RMS: ", rms, " PL: ", point_left, " PR: ", point_right, " RL: ", rms_left, " RR: ", rms_right, " G: ", gain_correction);
	}*/

	// apply distortion
	*i = *i * gain_correction;
	*q = *q * gain_correction;
}
