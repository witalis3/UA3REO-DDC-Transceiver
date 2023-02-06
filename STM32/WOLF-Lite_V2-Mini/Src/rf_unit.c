#include "rf_unit.h"
#include "audio_filters.h"
#include "front_unit.h"
#include "functions.h"
#include "lcd.h"
#include "main.h"
#include "settings.h"
#include "stm32h7xx_hal.h"
#include "system_menu.h"
#include "trx_manager.h"

bool ATU_TunePowerStabilized = false;

void RF_UNIT_ATU_Invalidate(void) {}

static uint8_t getBPFByFreq(uint32_t freq) {
	if (!TRX.RF_Filters) {
		return 255;
	}

	if (freq >= CALIBRATE.RFU_BPF_0_START && freq < CALIBRATE.RFU_BPF_0_END) {
		return 1;
	}
	if (freq >= CALIBRATE.RFU_BPF_1_START && freq < CALIBRATE.RFU_BPF_1_END) {
		return 8;
	}
	if (freq >= CALIBRATE.RFU_BPF_2_START && freq < CALIBRATE.RFU_BPF_2_END) {
		return 7;
	}
	if (freq >= CALIBRATE.RFU_BPF_3_START && freq < CALIBRATE.RFU_BPF_3_END) {
		return 5;
	}
	if (freq >= CALIBRATE.RFU_BPF_4_START && freq < CALIBRATE.RFU_BPF_4_END) {
		return 6;
	}
	if (freq >= CALIBRATE.RFU_BPF_5_START && freq < CALIBRATE.RFU_BPF_5_END) {
		return 3;
	}
	if (freq >= CALIBRATE.RFU_BPF_6_START && freq < CALIBRATE.RFU_BPF_6_END) {
		return 4;
	}
	if (freq >= CALIBRATE.RFU_HPF_START) {
		return 2;
	}

	return 255;
}

void RF_UNIT_UpdateState(bool clean) // pass values to RF-UNIT
{
	uint8_t bpf = getBPFByFreq(CurrentVFO->Freq);
	// uint8_t bpf_second = getBPFByFreq(SecondaryVFO->Freq);

	uint8_t band_out = 0;
	int8_t band = getBandFromFreq(CurrentVFO->RealRXFreq, true);

	// Transverters
	if (TRX.Transverter_3cm && band == BANDID_3cm) { // 3cm
		band_out = CALIBRATE.EXT_TRANSV_3cm;
	}
	if (TRX.Transverter_6cm && band == BANDID_6cm) { // 6cm
		band_out = CALIBRATE.EXT_TRANSV_6cm;
	}
	if (TRX.Transverter_13cm && band == BANDID_13cm) { // 13cm
		band_out = CALIBRATE.EXT_TRANSV_13cm;
	}
	if (TRX.Transverter_23cm && band == BANDID_23cm) { // 23cm
		band_out = CALIBRATE.EXT_TRANSV_23cm;
	}
	if (TRX.Transverter_70cm && band == BANDID_70cm) { // 70cm
		band_out = CALIBRATE.EXT_TRANSV_70cm;
	}

	if (!TRX.Transverter_70cm && band == BANDID_70cm) { // 70cm
		band_out = CALIBRATE.EXT_70cm;
	}
	if (band < BANDID_70cm) { // 2m
		band_out = CALIBRATE.EXT_2m;
	}
	if (band < BANDID_2m) { // FM
		band_out = CALIBRATE.EXT_FM;
	}
	if (band < BANDID_FM) { // 4m
		band_out = CALIBRATE.EXT_4m;
	}
	if (band < BANDID_4m) { // 6m
		band_out = CALIBRATE.EXT_6m;
	}
	if (band < BANDID_6m) { // 10m
		band_out = CALIBRATE.EXT_10m;
	}
	if (band < BANDID_10m) { // CB
		band_out = CALIBRATE.EXT_CB;
	}
	if (band < BANDID_CB) { // 12m
		band_out = CALIBRATE.EXT_12m;
	}
	if (band < BANDID_12m) { // 15m
		band_out = CALIBRATE.EXT_15m;
	}
	if (band < BANDID_15m) { // 17m
		band_out = CALIBRATE.EXT_17m;
	}
	if (band < BANDID_17m) { // 20m
		band_out = CALIBRATE.EXT_20m;
	}
	if (band < BANDID_20m) { // 30m
		band_out = CALIBRATE.EXT_30m;
	}
	if (band < BANDID_30m) { // 40m
		band_out = CALIBRATE.EXT_40m;
	}
	if (band < BANDID_40m) { // 60m
		band_out = CALIBRATE.EXT_60m;
	}
	if (band < BANDID_60m) { // 80m
		band_out = CALIBRATE.EXT_80m;
	}
	if (band < BANDID_80m) { // 160m
		band_out = CALIBRATE.EXT_160m;
	}
	if (band < BANDID_160m) { // 2200m
		band_out = CALIBRATE.EXT_2200m;
	}

	// RF Unit
	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
	MINI_DELAY
	for (uint8_t registerNumber = 0; registerNumber < 8; registerNumber++) {
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
		MINI_DELAY
		if (!clean) {
			// U10-D7 xxx
			//			if (registerNumber == 0 && (bpf == 5 || bpf == 7)) {
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET);
			//			}
			//			// U10-D6 xxx
			//			if (registerNumber == 1 && (bpf == 5 || bpf == 6)) {
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET);
			//			}
			//			// U10-D5 xxx
			//			if (registerNumber == 2 && att_val_4) {
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET);
			//			}
			// U10-D4 TX_OUT
			if (registerNumber == 3 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U10-D3 BAND-A
			if (registerNumber == 4 && bitRead(band_out, 0)) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U10-D2 BAND-B
			if (registerNumber == 5 && bitRead(band_out, 1)) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U10-D1 BAND-C
			if (registerNumber == 6 && bitRead(band_out, 2)) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
			// U10-D0 BAND-D
			if (registerNumber == 7 && bitRead(band_out, 3)) {
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			}
		}
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_SET);
	}
	MINI_DELAY
	HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET);
	MINI_DELAY
	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RFUNIT_OE_GPIO_Port, RFUNIT_OE_Pin, GPIO_PIN_RESET);
}

void RF_UNIT_ProcessSensors(void) {
#define B16_RANGE 65535.0f
#define B14_RANGE 16383.0f

	// PWR Voltage
	float32_t PWR_Voltage = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_3) * TRX_STM32_VREF / B16_RANGE;
	PWR_Voltage = PWR_Voltage * (CALIBRATE.PWR_VLT_Calibration) / 100.0f;
	if (fabsf(PWR_Voltage - TRX_PWR_Voltage) > 0.3f) {
		TRX_PWR_Voltage = TRX_PWR_Voltage * 0.99f + PWR_Voltage * 0.01f;
	}
	if (fabsf(PWR_Voltage - TRX_PWR_Voltage) > 1.0f) {
		TRX_PWR_Voltage = PWR_Voltage;
	}

	//	float32_t PWR_Current_Voltage = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3) * TRX_STM32_VREF / B16_RANGE;
	//	float32_t PWR_Current = (PWR_Current_Voltage - CALIBRATE.PWR_CUR_Calibration) / 0.100f; // 0.066 - ACS712-30, 0.100 - ACS712-20
	//	if (fabsf(PWR_Current - TRX_PWR_Current) > 0.1f) {
	//		TRX_PWR_Current = TRX_PWR_Current * 0.95f + PWR_Current * 0.05f;
	//	}
	//	if (fabsf(PWR_Current - TRX_PWR_Current) > 1.0f) {
	//		TRX_PWR_Current = PWR_Current;
	//	}

	//	 println(TRX_PWR_Voltage, " ", PWR_Voltage, " ", TRX_PWR_Current, " ", TRX_STM32_VREF);

	TRX_VBAT_Voltage = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_3)) * TRX_STM32_VREF / B14_RANGE; // why 14bit?

	// SWR
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / B16_RANGE;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / B16_RANGE;
	// println("-", VBAT_Voltage, " ", forward, " ", backward);

	// forward = forward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
	if (forward < 0.05f) // do not measure less than 50mV
	{
		TRX_VLT_forward = 0.0f;
		TRX_VLT_backward = 0.0f;
		TRX_PWR_Forward = 0.0f;
		TRX_PWR_Backward = 0.0f;
		TRX_SWR = 1.0f;
	} else {
		forward += 0.21f; // drop on diode

		// Transformation ratio of the SWR meter
		if (CurrentVFO->Freq >= 80000000) {
			forward = forward * CALIBRATE.SWR_FWD_Calibration_VHF;
		} else if (CurrentVFO->Freq >= 40000000) {
			forward = forward * CALIBRATE.SWR_FWD_Calibration_6M;
		} else {
			forward = forward * CALIBRATE.SWR_FWD_Calibration_HF;
		}

		// backward = backward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
		if (backward >= 0.05f) // do not measure less than 50mV
		{
			backward += 0.21f; // drop on diode

			// Transformation ratio of the SWR meter
			if (CurrentVFO->Freq >= 80000000) {
				backward = backward * CALIBRATE.SWR_BWD_Calibration_VHF;
			} else if (CurrentVFO->Freq >= 40000000) {
				backward = backward * CALIBRATE.SWR_BWD_Calibration_6M;
			} else {
				backward = backward * CALIBRATE.SWR_BWD_Calibration_HF;
			}
		} else {
			backward = 0.001f;
		}

		// smooth process
		TRX_VLT_forward = TRX_VLT_forward + (forward - TRX_VLT_forward) / 2;
		TRX_VLT_backward = TRX_VLT_backward + (backward - TRX_VLT_backward) / 2;
		if ((TRX_VLT_forward - TRX_VLT_backward) > 0.0f) {
			TRX_SWR = (TRX_VLT_forward + TRX_VLT_backward) / (TRX_VLT_forward - TRX_VLT_backward);
		} else {
			TRX_SWR = 1.0f;
		}

		if (TRX_VLT_backward > TRX_VLT_forward) {
			TRX_SWR = 9.9f;
		}
		if (TRX_SWR > 9.9f) {
			TRX_SWR = 9.9f;
		}

		TRX_PWR_Forward = (TRX_VLT_forward * TRX_VLT_forward) / 50.0f;
		if (TRX_PWR_Forward < 0.0f) {
			TRX_PWR_Forward = 0.0f;
		}
		TRX_PWR_Backward = (TRX_VLT_backward * TRX_VLT_backward) / 50.0f;

		if (TRX_PWR_Backward < 0.0f) {
			TRX_PWR_Backward = 0.0f;
		}

		if (TRX_PWR_Forward < TRX_PWR_Backward) {
			TRX_PWR_Backward = TRX_PWR_Forward;
		}
	}

#define smooth_stick_time 500
	static uint32_t forw_smooth_time = 0;
	if (HAL_GetTick() - forw_smooth_time > smooth_stick_time) {
		TRX_PWR_Forward_SMOOTHED = TRX_PWR_Forward_SMOOTHED * 0.99f + TRX_PWR_Forward * 0.01f;
	}
	if (TRX_PWR_Forward > TRX_PWR_Forward_SMOOTHED) {
		TRX_PWR_Forward_SMOOTHED = TRX_PWR_Forward;
		forw_smooth_time = HAL_GetTick();
	}
	TRX_PWR_Backward_SMOOTHED = TRX_PWR_Backward_SMOOTHED * 0.99f + TRX_PWR_Backward * 0.01f;
	TRX_SWR_SMOOTHED = TRX_SWR_SMOOTHED * 0.98f + TRX_SWR * 0.02f;

	// println("FWD: ", forward, " (", TRX_PWR_Forward, " / ", TRX_PWR_Forward_SMOOTHED, ") BKW: ", backward, " (", TRX_PWR_Backward, " / ",
	// TRX_PWR_Backward_SMOOTHED, ") SWR: ", TRX_SWR, " (", TRX_SWR_SMOOTHED, ")");

	// TANGENT
	float32_t SW1_Voltage = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1) * TRX_STM32_VREF / B16_RANGE * 1000.0f;
	float32_t SW2_Voltage = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2) * TRX_STM32_VREF / B16_RANGE * 1000.0f;
	// println(SW1_Voltage, " ", SW2_Voltage);

	// Yaesu MH-48
	for (uint16_t tb = 0; tb < (sizeof(PERIPH_FrontPanel_TANGENT_MH48) / sizeof(PERIPH_FrontPanel_Button)); tb++) {
		if ((SW2_Voltage < 500.0f || SW2_Voltage > 3100.0f) && PERIPH_FrontPanel_TANGENT_MH48[tb].channel == 1) {
			FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], SW1_Voltage);
		}
		if (SW1_Voltage > 2800.0f & PERIPH_FrontPanel_TANGENT_MH48[tb].channel == 2) {
			FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], SW2_Voltage);
		}
	}
}
