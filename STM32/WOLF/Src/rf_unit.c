#include "stm32h7xx_hal.h"
#include "main.h"
#include "rf_unit.h"
#include "lcd.h"
#include "trx_manager.h"
#include "settings.h"
#include "system_menu.h"
#include "functions.h"
#include "audio_filters.h"
#include "front_unit.h"

static bool ATU_Finished = false;
static bool ATU_InProcess = false;
static bool ATU_BestValsProbed = false;
static float32_t ATU_MinSWR = 1.0;
static uint8_t ATU_MinSWR_I = 0;
static uint8_t ATU_MinSWR_C = 0;
static bool ATU_MinSWR_T = false;
static uint8_t ATU_Stage = 0;
bool ATU_TunePowerStabilized = false;
bool FAN_Active = false;
static bool FAN_Active_old = false;

#define SENS_TABLE_COUNT 24
static const int16_t KTY81_120_sensTable[SENS_TABLE_COUNT][2] = { // table of sensor characteristics
	{-55, 490},
	{-50, 515},
	{-40, 567},
	{-30, 624},
	{-20, 684},
	{-10, 747},
	{0, 815},
	{10, 886},
	{20, 961},
	{25, 1000},
	{30, 1040},
	{40, 1122},
	{50, 1209},
	{60, 1299},
	{70, 1392},
	{80, 1490},
	{90, 1591},
	{100, 1696},
	{110, 1805},
	{120, 1915},
	{125, 1970},
	{130, 2023},
	{140, 2124},
	{150, 2211}};

static uint8_t getBPFByFreq(uint32_t freq)
{
	if (freq >= CALIBRATE.RFU_BPF_0_START && freq < CALIBRATE.RFU_BPF_0_END)
		return 0;
	if (freq >= CALIBRATE.RFU_BPF_1_START && freq < CALIBRATE.RFU_BPF_1_END)
		return 1;
	if (freq >= CALIBRATE.RFU_BPF_2_START && freq < CALIBRATE.RFU_BPF_2_END)
		return 2;
	if (freq >= CALIBRATE.RFU_BPF_3_START && freq < CALIBRATE.RFU_BPF_3_END)
		return 3;
	if (freq >= CALIBRATE.RFU_BPF_4_START && freq < CALIBRATE.RFU_BPF_4_END)
		return 4;
	if (freq >= CALIBRATE.RFU_BPF_5_START && freq < CALIBRATE.RFU_BPF_5_END)
		return 5;
	if (freq >= CALIBRATE.RFU_BPF_6_START && freq < CALIBRATE.RFU_BPF_6_END)
		return 6;
	if (CALIBRATE.RF_unit_type == RF_UNIT_BIG || CALIBRATE.RF_unit_type == RF_UNIT_SPLIT || CALIBRATE.RF_unit_type == RF_UNIT_WF_100D)
	{
		if (freq >= CALIBRATE.RFU_BPF_7_START && freq < CALIBRATE.RFU_BPF_7_END)
			return 7;
	}
	if (CALIBRATE.RF_unit_type == RF_UNIT_BIG || CALIBRATE.RF_unit_type == RF_UNIT_SPLIT)
	{
		if (freq >= CALIBRATE.RFU_BPF_8_START && freq < CALIBRATE.RFU_BPF_8_END)
			return 8;
	}
	if (CALIBRATE.RF_unit_type == RF_UNIT_QRP || CALIBRATE.RF_unit_type == RF_UNIT_RU4PN)
	{
		if (freq >= CALIBRATE.RFU_HPF_START)
			return 7;
	}
	return 255;
}

void RF_UNIT_ATU_Invalidate(void)
{
	ATU_Finished = false;
	ATU_InProcess = false;
	ATU_TunePowerStabilized = false;
	ATU_BestValsProbed = false;
}

static void RF_UNIT_ProcessATU(void)
{
	if (!TRX.ATU_Enabled || !TRX.TUNER_Enabled)
	{
		ATU_Finished = true;
		return;
	}
	
	if (TRX.TWO_SIGNAL_TUNE)
	{
		ATU_Finished = true;
		return;
	}

	if (!TRX_Tune)
		return;

	if (ATU_Finished)
		return;

	if (!ATU_Finished && TRX.RF_Power == 0)
	{
		ATU_Finished = true;
		return;
	}

	if (!ATU_TunePowerStabilized)
	{
		return;
	}

	if (!ATU_BestValsProbed)
	{
		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		if (band >= 0)
		{
			TRX.ATU_I = TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_I;
			TRX.ATU_C = TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_C;
			TRX.ATU_T = TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_T;
		}
		LCD_UpdateQuery.StatusInfoBar = true;
	}

#define delay_stages_saved (CALIBRATE.ATU_AVERAGING * 2)
#define delay_stages_tune CALIBRATE.ATU_AVERAGING
	static float32_t TRX_SWR_val = 1.0f;
	static uint8_t delay_stages_count = 0;
	if ((!ATU_BestValsProbed && delay_stages_count < delay_stages_saved) || (ATU_BestValsProbed && delay_stages_count < delay_stages_tune))
	{
		if (delay_stages_count == 0)
			TRX_SWR_val = TRX_SWR;

		// if(TRX_SWR < TRX_SWR_val)
		// TRX_SWR_val = TRX_SWR;
		TRX_SWR_val = TRX_SWR_val * 0.5f + TRX_SWR * 0.5f;

		delay_stages_count++;
		return;
	}
	else
	{
		delay_stages_count = 0;
	}

	char buff[64] = {0};
	// float32_t TRX_PWR = TRX_PWR_Forward - TRX_PWR_Backward;
	float32_t TRX_PWR = TRX_PWR_Forward;

	if (TRX_PWR >= 5.0f)
		if ((!ATU_BestValsProbed && TRX_SWR_val <= NORMAL_SWR_SAVED) || (ATU_BestValsProbed && TRX_SWR_val <= NORMAL_SWR_TUNE))
		{
			println("Normal SWR, stop!");
			sprintf(buff, "Best SWR: %.1f", TRX_SWR_val);
			LCD_showTooltip(buff);
			ATU_Finished = true;
			delay_stages_count = 0;
			FRONTPANEL_BUTTONHANDLER_TUNE(0);

			int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
			if (band >= 0)
			{
				TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_I = TRX.ATU_I;
				TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_C = TRX.ATU_C;
				TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_T = TRX.ATU_T;
			}
			return;
		}

	if (!ATU_BestValsProbed)
		ATU_BestValsProbed = true;

	static float32_t ATU_MinSWR_Slider = 9.9f;
	static float32_t ATU_MinSWR_prev_1 = 9.9f;
	static float32_t ATU_MinSWR_prev_2 = 9.9f;
	static float32_t ATU_MinSWR_prev_3 = 9.9f;

	if (!ATU_Finished && !ATU_InProcess)
	{
		TRX.ATU_I = 0;
		TRX.ATU_C = 0;
		TRX.ATU_T = 0;
		ATU_MinSWR = 99.9f;
		ATU_MinSWR_prev_1 = ATU_MinSWR;
		ATU_MinSWR_prev_2 = ATU_MinSWR;
		ATU_MinSWR_prev_3 = ATU_MinSWR;
		ATU_MinSWR_I = 0;
		ATU_MinSWR_C = 0;
		ATU_MinSWR_T = false;
		ATU_Stage = 0;
		ATU_InProcess = true;
		LCD_UpdateQuery.StatusInfoBar = true;
	}
	else
	{
		// best result
		if (ATU_MinSWR > TRX_SWR_val && TRX_PWR >= (float32_t)CALIBRATE.TUNE_MAX_POWER / 2.0f)
		{
			ATU_MinSWR = TRX_SWR_val;
			ATU_MinSWR_I = TRX.ATU_I;
			ATU_MinSWR_C = TRX.ATU_C;
			ATU_MinSWR_T = TRX.ATU_T;
		}
		// wrong way?
		bool wrong_way = false;
		if (ATU_MinSWR_prev_3 < ATU_MinSWR_prev_2 && ATU_MinSWR_prev_2 < ATU_MinSWR_prev_1 && ATU_MinSWR_prev_1 < TRX_SWR_val)
			if (fabsf(ATU_MinSWR_prev_3 - ATU_MinSWR) > 1.0f)
				wrong_way = true;
		// println("PREV PREV: ", ATU_MinSWR_prev_prev, " PREV: ", ATU_MinSWR_prev, " NOW: ", TRX_SWR);
		ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2;
		ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1;
		ATU_MinSWR_prev_1 = TRX_SWR_val;

		// debug
		if (ATU_Stage == 0)
			print("INDS ");
		if (ATU_Stage == 1)
			print("CAPS ");
		if (ATU_Stage == 2)
			print("CAPS T ");
		if (ATU_Stage == 3)
			print("I-1 ");
		if (ATU_Stage == 4)
			print("I+1 ");
		if (wrong_way)
			print("Wr Way ");
		println("Stage: ", ATU_Stage, " I: ", TRX.ATU_I, " C: ", TRX.ATU_C, " T: ", (uint8_t)TRX.ATU_T, " SWR: ", TRX_SWR_val, " PWR: ", TRX_PWR);
		// iteration block

		if (ATU_Stage == 0) // iterate inds
		{
			if (TRX.ATU_I < ATU_MAXPOS && !wrong_way)
			{
				TRX.ATU_I++;
			}
			else
			{
				ATU_Stage = 1;
				TRX.ATU_I = ATU_MinSWR_I;
				TRX.ATU_C = 1;
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		}
		else if (ATU_Stage == 1) // iterate caps
		{
			if (TRX.ATU_C < ATU_MAXPOS && !wrong_way)
			{
				TRX.ATU_C++;
			}
			else
			{
				ATU_Stage = 2;
				TRX.ATU_C = 0;
				TRX.ATU_T = true;
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		}
		else if (ATU_Stage == 2) // iterate caps with other T
		{
			if (TRX.ATU_C < ATU_MAXPOS && !wrong_way)
			{
				TRX.ATU_C++;
			}
			else
			{
				TRX.ATU_C = 0;
				TRX.ATU_T = ATU_MinSWR_T;
				ATU_MinSWR_Slider = ATU_MinSWR;
				if (TRX.ATU_I > 0)
				{
					TRX.ATU_I = ATU_MinSWR_I - 1;
					ATU_Stage = 3;
				}
				else
					ATU_Stage = 4;
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		}
		else if (ATU_Stage == 3) // iterate caps with i-1
		{
			if (TRX.ATU_C < ATU_MAXPOS && !wrong_way)
			{
				TRX.ATU_C++;
			}
			else
			{
				// slide more?
				if (ATU_MinSWR < ATU_MinSWR_Slider && TRX.ATU_I > 0)
				{
					ATU_MinSWR_Slider = ATU_MinSWR;
					TRX.ATU_I = ATU_MinSWR_I - 1;
					TRX.ATU_C = 0;
				}
				else // go slide right
				{
					TRX.ATU_C = 0;
					ATU_MinSWR_Slider = ATU_MinSWR;
					if (TRX.ATU_I < ATU_MAXPOS)
					{
						TRX.ATU_I = ATU_MinSWR_I + 1;
						ATU_Stage = 4;
					}
					else
						ATU_Stage = 5;
				}
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		}
		else if (ATU_Stage == 4) // iterate caps with i+1
		{
			if (TRX.ATU_C < ATU_MAXPOS && !wrong_way)
			{
				TRX.ATU_C++;
			}
			else
			{
				// slide more?
				if (ATU_MinSWR < ATU_MinSWR_Slider && TRX.ATU_I < ATU_MAXPOS)
				{
					ATU_MinSWR_Slider = ATU_MinSWR;
					TRX.ATU_I = ATU_MinSWR_I + 1;
					TRX.ATU_C = 0;
				}
				else // enough
				{
					ATU_Stage = 5;
				}
				ATU_MinSWR_prev_3 = ATU_MinSWR_prev_2 = ATU_MinSWR_prev_1 = 9.9f;
			}
		}

		if (ATU_Stage == 5) // finish tune
		{
			ATU_InProcess = false;
			ATU_Finished = true;
			TRX.ATU_I = ATU_MinSWR_I;
			TRX.ATU_C = ATU_MinSWR_C;
			TRX.ATU_T = ATU_MinSWR_T;
			println("ATU best I: ", TRX.ATU_I, " C: ", TRX.ATU_C, " T: ", (uint8_t)TRX.ATU_T, " SWR: ", ATU_MinSWR, " PWR: ", TRX_PWR);
			sprintf(buff, "Best SWR: %.1f", ATU_MinSWR);
			LCD_showTooltip(buff);
			delay_stages_count = 0;

			int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
			if (band >= 0)
			{
				TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_I = TRX.ATU_I;
				TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_C = TRX.ATU_C;
				TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_T = TRX.ATU_T;
			}

			FRONTPANEL_BUTTONHANDLER_TUNE(0);
		}

		LCD_UpdateQuery.StatusInfoBar = true;
	}
}

void RF_UNIT_UpdateState(bool clean) // pass values to RF-UNIT
{
	bool dualrx_lpf_disabled = false;
	bool dualrx_bpf_disabled = false;
	if (CALIBRATE.RF_unit_type == RF_UNIT_QRP || CALIBRATE.RF_unit_type == RF_UNIT_RU4PN || CALIBRATE.RF_unit_type == RF_UNIT_WF_100D)
	{
		if (TRX.Dual_RX && SecondaryVFO->Freq > CALIBRATE.RFU_LPF_END)
			dualrx_lpf_disabled = true;
		if (TRX.Dual_RX && getBPFByFreq(CurrentVFO->Freq) != getBPFByFreq(SecondaryVFO->Freq))
			dualrx_bpf_disabled = true;
	}

	float32_t att_val = TRX.ATT_DB;
	bool att_val_16 = false, att_val_8 = false, att_val_4 = false, att_val_2 = false, att_val_1 = false, att_val_05 = false;
	if (att_val >= 16.0f)
	{
		att_val_16 = true;
		att_val -= 16.0f;
	}
	if (att_val >= 8.0f)
	{
		att_val_8 = true;
		att_val -= 8.0f;
	}
	if (att_val >= 4.0f)
	{
		att_val_4 = true;
		att_val -= 4.0f;
	}
	if (att_val >= 2.0f)
	{
		att_val_2 = true;
		att_val -= 2.0f;
	}
	if (att_val >= 1.0f)
	{
		att_val_1 = true;
		att_val -= 1.0f;
	}
	if (att_val >= 0.5f)
	{
		att_val_05 = true;
		att_val -= 0.5f;
	}

	uint8_t bpf = getBPFByFreq(CurrentVFO->Freq);
	uint8_t bpf_second = getBPFByFreq(SecondaryVFO->Freq);

	bool turn_on_tx_lpf = true;
	if (((HAL_GetTick() - TRX_TX_EndTime) > TX_LPF_TIMEOUT || TRX_TX_EndTime == 0) && !TRX_on_TX)
		turn_on_tx_lpf = false;

	uint8_t band_out = 0;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band == BANDID_2200m || band == 1 || band == 2) // 2200m
		band_out = CALIBRATE.EXT_2200m;
	if (band == BANDID_160m || band == 4) // 160m
		band_out = CALIBRATE.EXT_160m;
	if (band == BANDID_80m || band == 5 || band == 7 || band == 8) // 80m
		band_out = CALIBRATE.EXT_80m;
	if (band == BANDID_60m || band == 9 || band == 11) // 60m
		band_out = CALIBRATE.EXT_60m;
	if (band == BANDID_40m || band == 13) // 40m
		band_out = CALIBRATE.EXT_40m;
	if (band == BANDID_30m || band == 14 || band == 16) // 30m
		band_out = CALIBRATE.EXT_30m;
	if (band == BANDID_20m || band == 17 || band == 19) // 20m
		band_out = CALIBRATE.EXT_20m;
	if (band == BANDID_17m || band == 20 || band == 22) // 17m
		band_out = CALIBRATE.EXT_17m;
	if (band == BANDID_15m || band == 24) // 15m
		band_out = CALIBRATE.EXT_15m;
	if (band == BANDID_12m || band == 26) // 12m
		band_out = CALIBRATE.EXT_12m;
	if (band == BANDID_CB) // CB
		band_out = CALIBRATE.EXT_CB;
	if (band == BANDID_10m) // 10m
		band_out = CALIBRATE.EXT_10m;
	if (band == BANDID_6m) // 6m
		band_out = CALIBRATE.EXT_6m;
	if (band == BANDID_FM || band == 31) // FM
		band_out = CALIBRATE.EXT_FM;
	if (band == BANDID_2m || band == BANDID_Marine) // 2m
		band_out = CALIBRATE.EXT_2m;
	if (band == BANDID_70cm) // 70cm
		band_out = CALIBRATE.EXT_70cm;

	// Transverters
	if (TRX.Transverter_70cm && band == BANDID_70cm) // 70cm
		band_out = CALIBRATE.EXT_TRANSV_70cm;
	if (TRX.Transverter_23cm && band == BANDID_23cm) // 23cm
		band_out = CALIBRATE.EXT_TRANSV_23cm;
	if (TRX.Transverter_13cm && band == BANDID_13cm) // 13cm
		band_out = CALIBRATE.EXT_TRANSV_13cm;
	if (TRX.Transverter_6cm && band == BANDID_6cm) // 6cm
		band_out = CALIBRATE.EXT_TRANSV_6cm;
	if (TRX.Transverter_3cm && band == BANDID_3cm) // 3cm
		band_out = CALIBRATE.EXT_TRANSV_3cm;
	
	// Skip shift register updating while transmit
	if (CALIBRATE.RF_unit_type == RF_UNIT_BIG || 
		CALIBRATE.RF_unit_type == RF_UNIT_RU4PN || 
		CALIBRATE.RF_unit_type == RF_UNIT_WF_100D || 
		CALIBRATE.RF_unit_type == RF_UNIT_SPLIT) 
	{
		static bool shift_registers_lock = false;
		static uint8_t shift_registers_lock_counter = 0;
		
		if (TRX_on_TX && !TRX_Tune && !shift_registers_lock) {
			shift_registers_lock_counter++;
			if(shift_registers_lock_counter >= 3) // 30ms lock delay
				shift_registers_lock = true;
		}
		
		if (!TRX_on_TX) {
			shift_registers_lock = false;
			shift_registers_lock_counter = 0;
		}
		
		if(shift_registers_lock) {
			return;
		}
	}

	// QRP Version RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_QRP)
	{
		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 24; registerNumber++)
		{
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
			MINI_DELAY
			if (!clean)
			{
				// U7-QH LPF_ON
				if (registerNumber == 0 && TRX.RF_Filters && (CurrentVFO->Freq <= CALIBRATE.RFU_LPF_END) && !dualrx_lpf_disabled)
					SET_DATA_PIN;
				// U7-QG LNA_ON
				if (registerNumber == 1 && !TRX_on_TX && TRX.LNA)
					SET_DATA_PIN;
				// U7-QF ATT_ON_0.5
				if (registerNumber == 2 && TRX.ATT && att_val_05)
					SET_DATA_PIN;
				// U7-QE ATT_ON_1
				if (registerNumber == 3 && TRX.ATT && att_val_1)
					SET_DATA_PIN;
				// U7-QD ATT_ON_2
				if (registerNumber == 4 && TRX.ATT && att_val_2)
					SET_DATA_PIN;
				// U7-QC ATT_ON_4
				if (registerNumber == 5 && TRX.ATT && att_val_4)
					SET_DATA_PIN;
				// U7-QB ATT_ON_8
				if (registerNumber == 6 && TRX.ATT && att_val_8)
					SET_DATA_PIN;
				// U7-QA ATT_ON_16
				if (registerNumber == 7 && TRX.ATT && att_val_16)
					SET_DATA_PIN;

				// U1-QH NOT USED
				// if (registerNumber == 8)
				// U1-QG BPF_2_A0
				if (registerNumber == 9 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 2 || bpf == 5 || bpf == 6)) // 1,2 - bpf2; 5,6 - bpf1
					SET_DATA_PIN;
				// U1-QF BPF_2_A1
				if (registerNumber == 10 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 4 || bpf == 2 || bpf == 0 || bpf == 6)) // 4,2 - bpf2; 0,6 - bpf1
					SET_DATA_PIN;
				// U1-QE BPF_2_!EN
				if (registerNumber == 11 && (!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 1 && bpf != 2 && bpf != 3 && bpf != 4))) // 1,2,3,4 - bpf2
					SET_DATA_PIN;
				// U1-QD BPF_1_A0
				if (registerNumber == 12 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 2 || bpf == 5 || bpf == 6)) // 1,2 - bpf2; 5,6 - bpf1
					SET_DATA_PIN;
				// U1-QC BPF_1_A1
				if (registerNumber == 13 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 4 || bpf == 2 || bpf == 0 || bpf == 6)) // 4,2 - bpf2; 0,6 - bpf1
					SET_DATA_PIN;
				// U1-QB BPF_1_!EN
				if (registerNumber == 14 && (!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 0 && bpf != 5 && bpf != 6 && bpf != 7))) // 5,6,7,0 - bpf1
					SET_DATA_PIN;
				// U1-QA BPF_ON
				if (registerNumber == 15 && TRX.RF_Filters && !dualrx_bpf_disabled && bpf != 255)
					SET_DATA_PIN;

				// U3-QH BAND_OUT_0
				if (registerNumber == 16 && bitRead(band_out, 0))
					SET_DATA_PIN;
				// U3-QG PTT_OUT
				if (registerNumber == 17 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					SET_DATA_PIN;
				// U3-QF BAND_OUT_2
				if (registerNumber == 18 && bitRead(band_out, 2))
					SET_DATA_PIN;
				// U3-QE BAND_OUT_3
				if (registerNumber == 19 && bitRead(band_out, 3))
					SET_DATA_PIN;
				// U3-QD TUNE_OUT
				if (registerNumber == 20 && TRX_Tune)
					SET_DATA_PIN;
				// U3-QC BAND_OUT_1
				if (registerNumber == 21 && bitRead(band_out, 1))
					SET_DATA_PIN;
				// U3-QB FAN_OUT
				if (registerNumber == 22) // FAN
				{
					static bool fan_pwm = false;
					if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) // Temperature at which the fan stops
						FAN_Active = false;
					if (!FAN_Active && TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						FAN_Active = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) // Temperature at which the fan starts at full power
						fan_pwm = false;

					if (FAN_Active != FAN_Active_old)
					{
						FAN_Active_old = FAN_Active;
						LCD_UpdateQuery.StatusInfoGUI = true;
					}

					if (FAN_Active)
					{
						if (fan_pwm) // PWM
						{
							const uint8_t on_ticks = 1;
							const uint8_t off_ticks = 1;
							static bool pwm_status = false; // true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status)
								SET_DATA_PIN;
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks))
							{
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						}
						else
							SET_DATA_PIN;
					}
				}
				// U3-QA ANT1_TX_OUT
				if (registerNumber == 23 && !TRX.ANT_selected && TRX_on_TX) // ANT1
					SET_DATA_PIN;
				if (registerNumber == 23 && TRX.ANT_selected && !TRX_on_TX) // ANT2
					SET_DATA_PIN;
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

	// BIG Version RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_BIG)
	{
		if (TRX_Tune && CurrentVFO->Freq <= 70000000)
			RF_UNIT_ProcessATU();

		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 40; registerNumber++)
		{
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
			MINI_DELAY
			if (!clean)
			{
				// U1-7 HF-VHF-SELECT
				if (registerNumber == 0 && CurrentVFO->Freq >= 70000000)
					SET_DATA_PIN;
				// U1-6 ATT_ON_1
				if (registerNumber == 1 && !(TRX.ATT && att_val_1))
					SET_DATA_PIN;
				// U1-5 ATT_ON_0.5
				if (registerNumber == 2 && !(TRX.ATT && att_val_05))
					SET_DATA_PIN;
				// U1-4 ATT_ON_16
				if (registerNumber == 3 && !(TRX.ATT && att_val_16))
					SET_DATA_PIN;
				// U1-3 ATT_ON_2
				if (registerNumber == 4 && !(TRX.ATT && att_val_2))
					SET_DATA_PIN;
				// U1-2 ATT_ON_4
				if (registerNumber == 5 && !(TRX.ATT && att_val_4))
					SET_DATA_PIN;
				// U1-1 ATT_ON_8
				if (registerNumber == 6 && !(TRX.ATT && att_val_8))
					SET_DATA_PIN;
				// U1-0 LNA_ON
				if (registerNumber == 7 && !(!TRX_on_TX && TRX.LNA))
					SET_DATA_PIN;

				// U3-7 TUN_C_5
				if (registerNumber == 8 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4))
					SET_DATA_PIN;
				// U3-6 TUN_C_4
				if (registerNumber == 9 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3))
					SET_DATA_PIN;
				// U3-5 BPF_1
				if (registerNumber == 10 && (bpf == 0 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 0)))
					SET_DATA_PIN;
				// U3-4 BPF_2
				if (registerNumber == 11 && (bpf == 1 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 1)))
					SET_DATA_PIN;
				// U3-3 TX_PTT_OUT
				if (registerNumber == 12 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					SET_DATA_PIN;
				// U3-2 TUN_C_1
				if (registerNumber == 13 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0))
					SET_DATA_PIN;
				// U3-1 TUN_C_2
				if (registerNumber == 14 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1))
					SET_DATA_PIN;
				// U3-0 TUN_C_3
				if (registerNumber == 15 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2))
					SET_DATA_PIN;

				// U2-7 TUN_T
				if (registerNumber == 16 && TRX.TUNER_Enabled && TRX.ATU_T)
					SET_DATA_PIN;
				// U2-6 TUN_I_5
				if (registerNumber == 17 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4))
					SET_DATA_PIN;
				// U2-5 UNUSED
				// if (registerNumber == 18 &&
				// U2-4 VHF_AMP_BIAS_ON
				if (registerNumber == 19 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq >= 70000000)
					SET_DATA_PIN;
				// U2-3 TUN_I_1
				if (registerNumber == 20 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0))
					SET_DATA_PIN;
				// U2-2 TUN_I_2
				if (registerNumber == 21 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1))
					SET_DATA_PIN;
				// U2-1 TUN_I_3
				if (registerNumber == 22 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2))
					SET_DATA_PIN;
				// U2-0 TUN_I_4
				if (registerNumber == 23 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3))
					SET_DATA_PIN;

				// U7-7 BPF_6
				if (registerNumber == 24 && (bpf == 5 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 5)))
					SET_DATA_PIN;
				// U7-6 BPF_5
				if (registerNumber == 25 && (bpf == 4 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 4)))
					SET_DATA_PIN;
				// U7-5 BPF_4
				if (registerNumber == 26 && (bpf == 3 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 3)))
					SET_DATA_PIN;
				// U7-4 BPF_3
				if (registerNumber == 27 && (bpf == 2 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 2)))
					SET_DATA_PIN;
				// U7-3 BPF_7
				if (registerNumber == 28 && (bpf == 6 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 6)))
					SET_DATA_PIN;
				// U7-2 BPF_8
				if (registerNumber == 29 && (bpf == 7 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 7)))
					SET_DATA_PIN;
				// U7-1 BPF_9
				if (registerNumber == 30 && (bpf == 8 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 8)))
					SET_DATA_PIN;
				// U7-0 HF_AMP_BIAS_ON
				if (registerNumber == 31 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq < 70000000)
					SET_DATA_PIN;

				// U11-7 ANT1-2_OUT
				if (registerNumber == 32 && TRX.ANT_selected)
					SET_DATA_PIN;
				// U11-6 FAN_OUT
				if (registerNumber == 33)
				{
					static bool fan_pwm = false;
					if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) // Temperature at which the fan stops
						FAN_Active = false;
					if (!FAN_Active && TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						FAN_Active = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) // Temperature at which the fan starts at full power
						fan_pwm = false;

					if (FAN_Active != FAN_Active_old)
					{
						FAN_Active_old = FAN_Active;
						LCD_UpdateQuery.StatusInfoGUI = true;
					}

					if (FAN_Active)
					{
						if (fan_pwm) // PWM
						{
							const uint8_t on_ticks = 1;
							const uint8_t off_ticks = 1;
							static bool pwm_status = false; // true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status)
								SET_DATA_PIN;
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks))
							{
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						}
						else
							SET_DATA_PIN;
					}
				}
				// U11-5 BAND_OUT_3
				if (registerNumber == 34 && bitRead(band_out, 3))
					SET_DATA_PIN;
				// U11-4 TX_PTT_OUT
				if (registerNumber == 35 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					SET_DATA_PIN;
				// U11-3 BAND_OUT_1
				if (registerNumber == 36 && bitRead(band_out, 1))
					SET_DATA_PIN;
				// U11-2 TUNE_OUT
				if (registerNumber == 37 && TRX_Tune)
					SET_DATA_PIN;
				// U11-1 BAND_OUT_2
				if (registerNumber == 38 && bitRead(band_out, 2))
					SET_DATA_PIN;
				// U11-0 BAND_OUT_0
				if (registerNumber == 39 && bitRead(band_out, 0))
					SET_DATA_PIN;
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

	// SPLIT Version RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_SPLIT)
	{
		if (TRX_Tune && CurrentVFO->Freq <= 70000000)
			RF_UNIT_ProcessATU();

		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 48; registerNumber++)
		{
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
			MINI_DELAY
			if (!clean)
			{
				// U5-7 ANT1-2_OUT
				if (registerNumber == 0 && TRX.ANT_selected)
					SET_DATA_PIN;
				// U5-6 TUN_I_4
				if (registerNumber == 1 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3))
					SET_DATA_PIN;
				// U5-5 TUN_I_1
				if (registerNumber == 2 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0))
					SET_DATA_PIN;
				// U5-4 TUN_I_2
				if (registerNumber == 3 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1))
					SET_DATA_PIN;
				// U5-3 TUN_C_4
				if (registerNumber == 4 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3))
					SET_DATA_PIN;
				// U5-2 TUN_C_1
				if (registerNumber == 5 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0))
					SET_DATA_PIN;
				// U5-1 TUN_C_2
				if (registerNumber == 6 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1))
					SET_DATA_PIN;
				// U5-0 FAN_OUT
				if (registerNumber == 7)
				{
					static bool fan_pwm = false;
					if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) // Temperature at which the fan stops
						FAN_Active = false;
					if (!FAN_Active && TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						FAN_Active = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) // Temperature at which the fan starts at full power
						fan_pwm = false;

					if (FAN_Active != FAN_Active_old)
					{
						FAN_Active_old = FAN_Active;
						LCD_UpdateQuery.StatusInfoGUI = true;
					}

					if (FAN_Active)
					{
						if (fan_pwm) // PWM
						{
							const uint8_t on_ticks = 1;
							const uint8_t off_ticks = 1;
							static bool pwm_status = false; // true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status)
								SET_DATA_PIN;
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks))
							{
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						}
						else
							SET_DATA_PIN;
					}
				}

				// U1-7 TUN_C_5
				if (registerNumber == 8 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4))
					SET_DATA_PIN;
				// U1-6 TUN_C_6
				if (registerNumber == 9 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 5))
					SET_DATA_PIN;
				// U1-5 TUN_C_3
				if (registerNumber == 10 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2))
					SET_DATA_PIN;
				// U1-4 TUN_I_7
				if (registerNumber == 11 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 6))
					SET_DATA_PIN;
				// U1-3 TUN_T
				if (registerNumber == 12 && TRX.TUNER_Enabled && TRX.ATU_T)
					SET_DATA_PIN;
				// U2-2 TUN_I_5
				if (registerNumber == 13 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4))
					SET_DATA_PIN;
				// U2-1 TUN_I_6
				if (registerNumber == 14 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 5))
					SET_DATA_PIN;
				// U2-0 TUN_I_3
				if (registerNumber == 15 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2))
					SET_DATA_PIN;

				// U3-7 BPF_7
				if (registerNumber == 16 && (bpf == 6 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 6)))
					SET_DATA_PIN;
				// U3-6 BPF_6
				if (registerNumber == 17 && (bpf == 5 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 5)))
					SET_DATA_PIN;
				// U3-5 BPF_5
				if (registerNumber == 18 && (bpf == 4 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 4)))
					SET_DATA_PIN;
				// U3-4 BPF_4
				if (registerNumber == 19 && (bpf == 3 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 3)))
					SET_DATA_PIN;
				// U3-3 BPF_3
				if (registerNumber == 20 && (bpf == 2 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 2)))
					SET_DATA_PIN;
				// U3-2 BPF_2
				if (registerNumber == 21 && (bpf == 1 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 1)))
					SET_DATA_PIN;
				// U3-1 BPF_1
				if (registerNumber == 22 && (bpf == 0 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 0)))
					SET_DATA_PIN;
				// U3-0 TUN_C_7
				if (registerNumber == 23 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 6))
					SET_DATA_PIN;

				// U2-7 TX_PTT_OUT
				if (registerNumber == 24 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					SET_DATA_PIN;
				// U2-6 EXT_2
				// if (registerNumber == 25 &&
				// U2-5 EXT_1
				// if (registerNumber == 26 &&
				// U2-4 HF_AMP_BIAS_ON
				if (registerNumber == 27 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq < 70000000)
					SET_DATA_PIN;
				// U2-3 VHF_AMP_BIAS_ON
				if (registerNumber == 28 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq >= 70000000)
					SET_DATA_PIN;
				// U2-2 HF-VHF-SELECT
				if (registerNumber == 29 && CurrentVFO->Freq >= 70000000)
					SET_DATA_PIN;
				// U2-1 BPF_9
				if (registerNumber == 30 && (bpf == 8 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 8)))
					SET_DATA_PIN;
				// U2-0 BPF_8
				if (registerNumber == 31 && (bpf == 7 || (!TRX_on_TX && TRX.Dual_RX && bpf_second == 7)))
					SET_DATA_PIN;

				// U7-7 ATT_ON_0.5
				if (registerNumber == 32 && !(TRX.ATT && att_val_05))
					SET_DATA_PIN;
				// U7-6 ATT_ON_1
				if (registerNumber == 33 && !(TRX.ATT && att_val_1))
					SET_DATA_PIN;
				// U7-5 ATT_ON_2
				if (registerNumber == 34 && !(TRX.ATT && att_val_2))
					SET_DATA_PIN;
				// U7-4 ATT_ON_4
				if (registerNumber == 35 && !(TRX.ATT && att_val_4))
					SET_DATA_PIN;
				// U7-3 EXT_6
				// if (registerNumber == 36 &&
				// U7-2 EXT_5
				// if (registerNumber == 37 &&
				// U7-1 EXT_4
				// if (registerNumber == 38 &&
				// U7-0 EXT_3
				// if (registerNumber == 39 &&

				// U11-7 LNA_ON
				if (registerNumber == 40 && !(!TRX_on_TX && TRX.LNA))
					SET_DATA_PIN;
				// U11-6 BAND_OUT_0
				if (registerNumber == 41 && bitRead(band_out, 0))
					SET_DATA_PIN;
				// U11-5 BAND_OUT_3
				if (registerNumber == 42 && bitRead(band_out, 3))
					SET_DATA_PIN;
				// U11-4 BAND_OUT_2
				if (registerNumber == 43 && bitRead(band_out, 2))
					SET_DATA_PIN;
				// U11-3 BAND_OUT_1
				if (registerNumber == 44 && bitRead(band_out, 1))
					SET_DATA_PIN;
				// U11-2 TUNE_OUT
				if (registerNumber == 45 && TRX_Tune)
					SET_DATA_PIN;
				// U11-1 ATT_ON_8
				if (registerNumber == 46 && !(TRX.ATT && att_val_8))
					SET_DATA_PIN;
				// U11-0 ATT_ON_16
				if (registerNumber == 47 && !(TRX.ATT && att_val_16))
					SET_DATA_PIN;
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

	// RU4PN Version RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_RU4PN)
	{
		if (TRX_Tune && CurrentVFO->Freq <= 70000000)
			RF_UNIT_ProcessATU();

		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 48; registerNumber++)
		{
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
			MINI_DELAY
			if (!clean)
			{
				// U2-7 TUN_C_3
				if (registerNumber == 0 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2))
					SET_DATA_PIN;
				// U2-6 HF-VHF-SELECT
				if (registerNumber == 1 && CurrentVFO->Freq >= 70000000)
					SET_DATA_PIN;
				// U2-5 HF_AMP_BIAS_ON
				if (registerNumber == 2 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq < 70000000)
					SET_DATA_PIN;
				// U2-4 VHF_AMP_BIAS_ON
				if (registerNumber == 3 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq >= 70000000)
					SET_DATA_PIN;
				// U2-3 TUN_C_1
				if (registerNumber == 4 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0))
					SET_DATA_PIN;
				// U2-2 TUN_I_1
				if (registerNumber == 5 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0))
					SET_DATA_PIN;
				// U2-1 TUN_C_2
				if (registerNumber == 6 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1))
					SET_DATA_PIN;
				// U2-0 TUN_I_2
				if (registerNumber == 7 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1))
					SET_DATA_PIN;

				// U3-7 HF-VHF-SELECT
				if (registerNumber == 8 && CurrentVFO->Freq >= 70000000)
					SET_DATA_PIN;
				// U3-6 NOT USED
				// if (registerNumber == 9)
				// U3-5 NOT USED
				// if (registerNumber == 10)
				// U3-4 NOT USED
				// if (registerNumber == 11)
				// U3-3 TUN_I_3
				if (registerNumber == 12 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2))
					SET_DATA_PIN;
				// U3-2 TUN_C_4
				if (registerNumber == 13 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3))
					SET_DATA_PIN;
				// U3-1 TUN_I_4
				if (registerNumber == 14 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3))
					SET_DATA_PIN;
				// U3-0 TUN_C_5
				if (registerNumber == 15 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4))
					SET_DATA_PIN;

				// U11-7 ANT1-2_OUT
				if (registerNumber == 16 && TRX.ANT_selected)
					SET_DATA_PIN;
				// U11-6 FAN_OUT
				if (registerNumber == 17)
				{
					static bool fan_pwm = false;
					if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) // Temperature at which the fan stops
						FAN_Active = false;
					if (!FAN_Active && TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						FAN_Active = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) // Temperature at which the fan starts at full power
						fan_pwm = false;

					if (FAN_Active != FAN_Active_old)
					{
						FAN_Active_old = FAN_Active;
						LCD_UpdateQuery.StatusInfoGUI = true;
					}

					if (FAN_Active)
					{
						if (fan_pwm) // PWM
						{
							const uint8_t on_ticks = 1;
							const uint8_t off_ticks = 1;
							static bool pwm_status = false; // true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status)
								SET_DATA_PIN;
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks))
							{
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						}
						else
							SET_DATA_PIN;
					}
				}
				// U11-5 BAND_OUT_3
				if (registerNumber == 18 && bitRead(band_out, 3))
					SET_DATA_PIN;
				// U11-4 BAND_OUT_1
				if (registerNumber == 19 && bitRead(band_out, 1))
					SET_DATA_PIN;
				// U11-3 TUNE_OUT
				if (registerNumber == 20 && TRX_Tune)
					SET_DATA_PIN;
				// U11-2 BAND_OUT_2
				if (registerNumber == 21 && bitRead(band_out, 2))
					SET_DATA_PIN;
				// U11-1 BAND_OUT_0
				if (registerNumber == 22 && bitRead(band_out, 0))
					SET_DATA_PIN;
				// U11-0 TX_PTT_OUT
				if (registerNumber == 23 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					SET_DATA_PIN;

				// U1-7 LPF_ON
				if (registerNumber == 24 && (TRX.RF_Filters && (CurrentVFO->Freq <= CALIBRATE.RFU_LPF_END) && !dualrx_lpf_disabled))
					SET_DATA_PIN;
				// U1-6 ATT_ON_1
				if (registerNumber == 25 && !(TRX.ATT && att_val_1))
					SET_DATA_PIN;
				// U1-5 ATT_ON_0.5
				if (registerNumber == 26 && !(TRX.ATT && att_val_05))
					SET_DATA_PIN;
				// U1-4 ATT_ON_16
				if (registerNumber == 27 && !(TRX.ATT && att_val_16))
					SET_DATA_PIN;
				// U1-3 ATT_ON_2
				if (registerNumber == 28 && !(TRX.ATT && att_val_2))
					SET_DATA_PIN;
				// U1-2 ATT_ON_4
				if (registerNumber == 29 && !(TRX.ATT && att_val_4))
					SET_DATA_PIN;
				// U1-1 ATT_ON_8
				if (registerNumber == 30 && !(TRX.ATT && att_val_8))
					SET_DATA_PIN;
				// U1-0 LNA_ON
				if (registerNumber == 31 && !(!TRX_on_TX && TRX.LNA))
					SET_DATA_PIN;

				// U7-7 LPF_7
				if (registerNumber == 32 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Freq > 31000000 && CurrentVFO->Freq <= 60000000)
					SET_DATA_PIN;
				// U7-6 LPF_6
				if (registerNumber == 33 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Freq > 22000000 && CurrentVFO->Freq <= 31000000)
					SET_DATA_PIN;
				// U7-5 LPF_5
				if (registerNumber == 34 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Freq > 15000000 && CurrentVFO->Freq <= 22000000)
					SET_DATA_PIN;
				// U7-4 LPF_4
				if (registerNumber == 35 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Freq > 7500000 && CurrentVFO->Freq <= 15000000)
					SET_DATA_PIN;
				// U7-3 TUN_I_5
				if (registerNumber == 36 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4))
					SET_DATA_PIN;
				// U7-2 LPF_1
				if (registerNumber == 37 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Freq <= 2500000)
					SET_DATA_PIN;
				// U7-1 LPF_2
				if (registerNumber == 38 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Freq > 2500000 && CurrentVFO->Freq <= 4500000)
					SET_DATA_PIN;
				// U7-0 LPF_3
				if (registerNumber == 39 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Freq > 4500000 && CurrentVFO->Freq <= 7500000)
					SET_DATA_PIN;

				// U21-7 TUN_T
				if (registerNumber == 40 && TRX.TUNER_Enabled && TRX.ATU_T)
					SET_DATA_PIN;
				// U21-6 BPF_2_!EN
				if (registerNumber == 41 && !(!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 1 && bpf != 2 && bpf != 3 && bpf != 4))) // 1,2,3,4 - bpf2
					SET_DATA_PIN;
				// U21-5 BPF_2_A1
				if (registerNumber == 42 && !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 4 || bpf == 2 || bpf == 0 || bpf == 6))) // 4,2 - bpf2; 0,6 - bpf1
					SET_DATA_PIN;
				// U21-4 BPF_2_A0
				if (registerNumber == 43 && !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 2 || bpf == 5 || bpf == 6))) // 1,2 - bpf2; 5,6 - bpf1
					SET_DATA_PIN;
				// U21-3 BPF_1_A0
				if (registerNumber == 44 && !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 2 || bpf == 5 || bpf == 6))) // 1,2 - bpf2; 5,6 - bpf1
					SET_DATA_PIN;
				// U21-2 BPF_1_A1
				if (registerNumber == 45 && !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 4 || bpf == 2 || bpf == 0 || bpf == 6))) // 4,2 - bpf2; 0,6 - bpf1
					SET_DATA_PIN;
				// U21-1 BPF_1_!EN
				if (registerNumber == 46 && !(!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 0 && bpf != 5 && bpf != 6 && bpf != 7))) // 5,6,7,0 - bpf1
					SET_DATA_PIN;
				// U21-0 BPF_ON
				if (registerNumber == 47 && !(TRX.RF_Filters && !dualrx_bpf_disabled && bpf != 255))
					SET_DATA_PIN;
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

	// WF-100D RF Unit ///////////////////////////////////////////////////////////////////////
	if (CALIBRATE.RF_unit_type == RF_UNIT_WF_100D)
	{
		if (TRX_Tune && CurrentVFO->Freq <= 70000000)
			RF_UNIT_ProcessATU();

		uint8_t lpf_index = 7; //6m
		if (CurrentVFO->Freq <= 2000000) //160m
			lpf_index = 1;
		if (CurrentVFO->Freq > 2000000 && CurrentVFO->Freq <= 5000000) //80m
			lpf_index = 2;
		if (CurrentVFO->Freq > 5000000 && CurrentVFO->Freq <= 9000000) //40m
			lpf_index = 3;
		if (CurrentVFO->Freq > 9000000 && CurrentVFO->Freq <= 16000000) //30m,20m
			lpf_index = 4;
		if (CurrentVFO->Freq > 16000000 && CurrentVFO->Freq <= 22000000) //17m,15m
			lpf_index = 5;
		if (CurrentVFO->Freq > 22000000 && CurrentVFO->Freq <= 30000000) //12m,CB,10m
			lpf_index = 6;

		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); // latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 56; registerNumber++)
		{
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); // data
			MINI_DELAY
			if (!clean)
			{
				// U1-7 -
				// if (registerNumber == 0
				// U1-6 FAN_OUT
				if (registerNumber == 1)
				{
					static bool fan_pwm = false;
					if (FAN_Active && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) // Temperature at which the fan stops
						FAN_Active = false;
					if (!FAN_Active && TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						FAN_Active = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) // Temperature at which the fan starts at full power
						fan_pwm = false;

					if (FAN_Active != FAN_Active_old)
					{
						FAN_Active_old = FAN_Active;
						LCD_UpdateQuery.StatusInfoGUI = true;
					}

					if (FAN_Active)
					{
						if (fan_pwm) // PWM
						{
							const uint8_t on_ticks = 1;
							const uint8_t off_ticks = 1;
							static bool pwm_status = false; // true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status)
								SET_DATA_PIN;
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks))
							{
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						}
						else
							SET_DATA_PIN;
					}
				}
				// U1-5 -
				// if (registerNumber == 2
				// U1-4 -
				// if (registerNumber == 3
				// U1-3 TX_PTT_OUT
				if (registerNumber == 4 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					SET_DATA_PIN;
				// U1-2 HF_AMP_BIAS_ON
				if (registerNumber == 5 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq < 70000000)
					SET_DATA_PIN;
				// U1-1 HF-VHF-SELECT
				if (registerNumber == 6 && CurrentVFO->Freq >= 70000000)
					SET_DATA_PIN;
				// U1-0 VHF_AMP_BIAS_ON
				if (registerNumber == 7 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq >= 70000000)
					SET_DATA_PIN;

				// U2-7 TUN_I_7
				if (registerNumber == 8 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 6))
					SET_DATA_PIN;
				// U2-6 TUN_I_6
				if (registerNumber == 9 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 5))
					SET_DATA_PIN;
				// U2-5 TUN_I_5
				if (registerNumber == 10 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 4))
					SET_DATA_PIN;
				// U2-4 ANT1-2_OUT
				if (registerNumber == 11 && !TRX.ANT_selected)
					SET_DATA_PIN;
				// U2-3 TUN_I_4
				if (registerNumber == 12 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 3))
					SET_DATA_PIN;
				// U2-2 TUN_I_3
				if (registerNumber == 13 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 2))
					SET_DATA_PIN;
				// U2-1 TUN_I_2
				if (registerNumber == 14 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 1))
					SET_DATA_PIN;
				// U2-0 TUN_I_1
				if (registerNumber == 15 && TRX.TUNER_Enabled && bitRead(TRX.ATU_I, 0))
					SET_DATA_PIN;

				// U3-7 TUN_C_7
				if (registerNumber == 16 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 6))
					SET_DATA_PIN;
				// U3-6 TUN_C_6
				if (registerNumber == 17 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 5))
					SET_DATA_PIN;
				// U3-5 TUN_C_5
				if (registerNumber == 18 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 4))
					SET_DATA_PIN;
				// U3-4 TUN_C_4
				if (registerNumber == 19 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 3))
					SET_DATA_PIN;
				// U3-3 TUN_C_3
				if (registerNumber == 20 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 2))
					SET_DATA_PIN;
				// U3-2 TUN_C_2
				if (registerNumber == 21 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 1))
					SET_DATA_PIN;
				// U3-1 TUN_C_1
				if (registerNumber == 22 && TRX.TUNER_Enabled && bitRead(TRX.ATU_C, 0))
					SET_DATA_PIN;
				// U3-0 TUN_T
				if (registerNumber == 23 && TRX.TUNER_Enabled && TRX.ATU_T)
					SET_DATA_PIN;

				// U23-7 -
				// if (registerNumber == 24
				// U23-6 ATT_ON_16
				if (registerNumber == 25 && !(TRX.ATT && att_val_16))
					SET_DATA_PIN;
				// U23-5 ATT_ON_0.5
				if (registerNumber == 26 && !(TRX.ATT && att_val_05))
					SET_DATA_PIN;
				// U23-4 ATT_ON_1
				if (registerNumber == 27 && !(TRX.ATT && att_val_1))
					SET_DATA_PIN;
				// U23-3 ATT_ON_2
				if (registerNumber == 28 && !(TRX.ATT && att_val_2))
					SET_DATA_PIN;
				// U23-2 ATT_ON_4
				if (registerNumber == 29 && !(TRX.ATT && att_val_4))
					SET_DATA_PIN;
				// U23-1 ATT_ON_8
				if (registerNumber == 30 && !(TRX.ATT && att_val_8))
					SET_DATA_PIN;
				// U23-0 -
				// if (registerNumber == 31

				// U24-7 LPF_5
				if (registerNumber == 32 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 5)
					SET_DATA_PIN;
				// U24-6 LPF_6
				if (registerNumber == 33 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 6)
					SET_DATA_PIN;
				// U24-5 LPF_7
				if (registerNumber == 34 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 7)
					SET_DATA_PIN;
				// U24-4 HF_AMP_BIAS_ON
				if (registerNumber == 35 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq < 70000000)
					SET_DATA_PIN;
				// U24-3 LPF_1
				if (registerNumber == 36 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 1)
					SET_DATA_PIN;
				// U24-2 LPF_2
				if (registerNumber == 37 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 2)
					SET_DATA_PIN;
				// U24-1 LPF_3
				if (registerNumber == 38 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 3)
					SET_DATA_PIN;
				// U24-0 LPF_4
				if (registerNumber == 39 && (turn_on_tx_lpf || TRX_on_TX) && CurrentVFO->Mode != TRX_MODE_LOOPBACK && lpf_index == 4)
					SET_DATA_PIN;

				// U31-H U3 BPF_1_A1
				if (registerNumber == 40 && !dualrx_bpf_disabled && TRX.RF_Filters && ((CurrentVFO->Freq >= CALIBRATE.RFU_HPF_START && bpf == 255) || bpf == 7)) // hpf 145(7)
					SET_DATA_PIN;
				// U31-G U7 BPF_3_A1
				if (registerNumber == 41 && !dualrx_bpf_disabled && TRX.RF_Filters && (bpf == 1 || bpf == 0)) // 2.5-4(1) 1.6-2.5(0)
					SET_DATA_PIN;
				// U31-F U7 BPF_3_A0
				if (registerNumber == 42 && !dualrx_bpf_disabled && TRX.RF_Filters && (bpf == 3 || bpf == 0)) // 7-12(3) 1.6-2.5(0)
					SET_DATA_PIN;
				// U31-E U3 BPF_1_A0
				if (registerNumber == 43 && !dualrx_bpf_disabled && TRX.RF_Filters && ((CurrentVFO->Freq <= CALIBRATE.RFU_LPF_END && bpf == 255) || bpf == 7)) // lpf 145(7)
					SET_DATA_PIN;
				// U31-D U5 BPF_2_A0
				if (registerNumber == 44 && !dualrx_bpf_disabled && TRX.RF_Filters && (bpf == 6 || bpf == 4)) // 21.5-30(6) 12-14.5(4)
					SET_DATA_PIN;
				// U31-C U5 BPF_2_A1
				if (registerNumber == 45 && !dualrx_bpf_disabled && TRX.RF_Filters && (bpf == 5 || bpf == 4)) // 14.5-21.5(5) 12-14.5(4)
					SET_DATA_PIN;
				// U31-B U5 BPF_2_EN
				if (registerNumber == 46 && !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 5 || bpf == 6 || bpf == 4))) // net5?, 21.5-30(6), 14.5-21.5(5), 12-14.5(4)
					SET_DATA_PIN;
				// U31-A U7 BPF_3_EN
				if (registerNumber == 47 && !(TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 3 || bpf == 0 || bpf == 2))) // 6-7.3(2), 7-12(3), 2.5-4(1), 1.6-2.5(0)
					SET_DATA_PIN;

				// U32-H -
				// if (registerNumber == 48)
				// U32-G -
				// if (registerNumber == 49)
				// U32-F -
				// if (registerNumber == 50)
				// U32-E -
				// if (registerNumber == 51)
				// U32-D Net_RX/TX
				if (registerNumber == 52 && TRX_on_TX && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					SET_DATA_PIN;
				// U32-C - LNA_ON
				if (registerNumber == 53 && (!TRX_on_TX && TRX.LNA))
					SET_DATA_PIN;
				// U32-B -
				// if (registerNumber == 54)
				// U32-A U3 BPF_1_EN
				if (registerNumber == 55 && !(TRX.RF_Filters && ((CurrentVFO->Freq >= CALIBRATE.RFU_HPF_START && bpf == 255) || (CurrentVFO->Freq <= CALIBRATE.RFU_LPF_END && bpf == 255) || bpf == 7 || dualrx_bpf_disabled))) // bypass, lpf, hpf, 145(7)
					SET_DATA_PIN;
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
}

void RF_UNIT_ProcessSensors(void)
{
#define B16_RANGE 65535.0f
#define B14_RANGE 16383.0f

	// THERMAL
	float32_t rf_thermal = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3)) * TRX_STM32_VREF / B16_RANGE;
	
	float32_t therm_resistance = -2000.0f * rf_thermal / (-3.3f + rf_thermal);
	uint_fast8_t point_left = 0;
	uint_fast8_t point_right = SENS_TABLE_COUNT - 1;
	for (uint_fast8_t i = 0; i < SENS_TABLE_COUNT; i++)
		if (KTY81_120_sensTable[i][1] < therm_resistance)
			point_left = i;
	for (uint_fast8_t i = (SENS_TABLE_COUNT - 1); i > 0; i--)
		if (KTY81_120_sensTable[i][1] >= therm_resistance)
			point_right = i;
	float32_t power_left = (float32_t)KTY81_120_sensTable[point_left][0];
	float32_t power_right = (float32_t)KTY81_120_sensTable[point_right][0];
	float32_t part_point_left = therm_resistance - KTY81_120_sensTable[point_left][1];
	float32_t part_point_right = KTY81_120_sensTable[point_right][1] - therm_resistance;
	float32_t part_point = part_point_left / (part_point_left + part_point_right);
	float32_t TRX_RF_Temperature_measured = (power_left * (1.0f - part_point)) + (power_right * (part_point));
	
	if (TRX_RF_Temperature_measured < -100.0f)
		TRX_RF_Temperature_measured = 75.0f;
	if (TRX_RF_Temperature_measured < 0.0f)
		TRX_RF_Temperature_measured = 0.0f;
	
	static float32_t TRX_RF_Temperature_averaged = 20.0f;
	TRX_RF_Temperature_averaged = TRX_RF_Temperature_averaged * 0.995f + TRX_RF_Temperature_measured * 0.005f;

	if (fabsf(TRX_RF_Temperature_averaged - TRX_RF_Temperature) >= 1.0f) // hysteresis
		TRX_RF_Temperature = TRX_RF_Temperature_averaged;

	//VBAT
	TRX_VBAT_Voltage = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_3)) * TRX_STM32_VREF / B14_RANGE;
	
	// SWR
	TRX_ALC_IN = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_4) * TRX_STM32_VREF / B16_RANGE;
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / B16_RANGE;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / B16_RANGE;
	// println("FWD: ", forward, " BKW: ", backward);
	// static float32_t TRX_VLT_forward = 0.0f;		//Tisho
	// static float32_t TRX_VLT_backward = 0.0f;		//Tisho

#if (defined(SWR_AD8307_LOG)) // If it is used the Log amp. AD8307
	float32_t P_FW_dBm, P_BW_dBm;
	float32_t V_FW_Scaled, V_BW_Scaled;
	// float32_t NewSWR;

	TRX_VLT_forward = TRX_VLT_forward + (forward - TRX_VLT_forward) / 4;
	TRX_VLT_backward = TRX_VLT_backward + (backward - TRX_VLT_backward) / 4;

	// Calculate the Forward values
	P_FW_dBm = ((TRX_VLT_forward * 1000) - CALIBRATE.FW_AD8307_OFFS) / (CALIBRATE.FW_AD8307_SLP);
	V_FW_Scaled = pow(10, (double)((P_FW_dBm - 10) / 20));	   // Calculate in voltage (Vp - 50ohm terminated)
	TRX_PWR_Forward = pow(10, (double)((P_FW_dBm - 30) / 10)); // Calculate in W

	// Calculate the Backward values
	P_BW_dBm = ((TRX_VLT_backward * 1000) - CALIBRATE.BW_AD8307_OFFS) / (CALIBRATE.BW_AD8307_SLP);
	V_BW_Scaled = pow(10, (double)((P_BW_dBm - 10) / 20));		// Calculate in voltage (Vp - 50ohm terminated)
	TRX_PWR_Backward = pow(10, (double)((P_BW_dBm - 30) / 10)); // Calculate in W

	TRX_SWR = (V_FW_Scaled + V_BW_Scaled) / (V_FW_Scaled - V_BW_Scaled); // Calculate SWR

	// TRX_SWR = TRX_SWR + (NewSWR - TRX_SWR) / 2;

	if (TRX_SWR > 10.0f)
		TRX_SWR = 10.0f;
	if (TRX_SWR < 0.0f)
		TRX_SWR = 0.0f;

#else // if it is used the standard measure (diode rectifier)
	// forward = forward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
	if (forward < 0.05f) // do not measure less than 100mV
	{
		TRX_VLT_forward = 0.0f;
		TRX_VLT_backward = 0.0f;
		TRX_PWR_Forward = 0.0f;
		TRX_PWR_Backward = 0.0f;
		TRX_SWR = 1.0f;
	}
	else
	{
		forward += 0.21f; // drop on diode

		// Transformation ratio of the SWR meter
		if (CurrentVFO->Freq >= 80000000)
			forward = forward * CALIBRATE.SWR_FWD_Calibration_VHF;
		else if (CurrentVFO->Freq >= 40000000)
			forward = forward * CALIBRATE.SWR_FWD_Calibration_6M;
		else
			forward = forward * CALIBRATE.SWR_FWD_Calibration_HF;

		// backward = backward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
		if (backward >= 0.05f) // do not measure less than 100mV
		{
			backward += 0.21f; // drop on diode

			// Transformation ratio of the SWR meter
			if (CurrentVFO->Freq >= 80000000)
				backward = backward * CALIBRATE.SWR_REF_Calibration_VHF;
			else if (CurrentVFO->Freq >= 40000000)
				backward = backward * CALIBRATE.SWR_REF_Calibration_6M;
			else
				backward = backward * CALIBRATE.SWR_REF_Calibration_HF;
		}
		else
			backward = 0.001f;

		// smooth process
		TRX_VLT_forward = TRX_VLT_forward + (forward - TRX_VLT_forward) / 2;
		TRX_VLT_backward = TRX_VLT_backward + (backward - TRX_VLT_backward) / 2;
		if ((TRX_VLT_forward - TRX_VLT_backward) > 0.0f)
			TRX_SWR = (TRX_VLT_forward + TRX_VLT_backward) / (TRX_VLT_forward - TRX_VLT_backward);
		else
			TRX_SWR = 1.0f;

		if (TRX_VLT_backward > TRX_VLT_forward)
			TRX_SWR = 9.9f;
		if (TRX_SWR > 9.9f)
			TRX_SWR = 9.9f;

		TRX_PWR_Forward = (TRX_VLT_forward * TRX_VLT_forward) / 50.0f;
		if (TRX_PWR_Forward < 0.0f)
			TRX_PWR_Forward = 0.0f;
		TRX_PWR_Backward = (TRX_VLT_backward * TRX_VLT_backward) / 50.0f;

		if (TRX_PWR_Backward < 0.0f)
			TRX_PWR_Backward = 0.0f;

		if (TRX_PWR_Forward < TRX_PWR_Backward)
			TRX_PWR_Backward = TRX_PWR_Forward;
	}
#endif

#define smooth_stick_time 100
	static uint32_t forw_smooth_time = 0;
	if ((HAL_GetTick() - forw_smooth_time) > smooth_stick_time)
	{
		TRX_PWR_Forward_SMOOTHED = TRX_PWR_Forward_SMOOTHED * 0.99f + TRX_PWR_Forward * 0.01f;
		//TRX_PWR_Backward_SMOOTHED = TRX_PWR_Backward_SMOOTHED * 0.99f + TRX_PWR_Backward * 0.01f;
	}
	
	if (TRX_PWR_Forward > TRX_PWR_Forward_SMOOTHED)
	{
		TRX_PWR_Forward_SMOOTHED = TRX_PWR_Forward;
		//TRX_PWR_Backward_SMOOTHED = TRX_PWR_Backward;
		forw_smooth_time = HAL_GetTick();
	}
	
	TRX_PWR_Backward_SMOOTHED = TRX_PWR_Backward_SMOOTHED * 0.99f + TRX_PWR_Backward * 0.01f;
	TRX_SWR_SMOOTHED = TRX_SWR_SMOOTHED * 0.98f + TRX_SWR * 0.02f;
	
	sprintf(TRX_SWR_SMOOTHED_STR, "%.1f", (double)TRX_SWR_SMOOTHED);
}

// Tisho
// used to controll the calibration of the FW and BW power measurments
void RF_UNIT_MeasureVoltage(void)
{
#define B16_RANGE 65535.0f
	//#define B14_RANGE 16383.0f

	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / B16_RANGE;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / B16_RANGE;
	// use the TRX_VLT_forward and TRX_VLT_backward global variables
	// for the raw ADC input voltages
	// in the TDM_Voltages() the other stuff will be calculated localy

	static float32_t VLT_forward = 0.0f;
	static float32_t VLT_backward = 0.0f;
	VLT_forward = VLT_forward + (forward - VLT_forward) / 10;
	VLT_backward = VLT_backward + (backward - VLT_backward) / 10;

	TRX_VLT_forward = VLT_forward;
	TRX_VLT_backward = VLT_backward;
}
