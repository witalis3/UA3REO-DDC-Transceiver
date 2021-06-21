#include "stm32h7xx_hal.h"
#include "main.h"
#include "rf_unit.h"
#include "lcd.h"
#include "trx_manager.h"
#include "settings.h"
#include "system_menu.h"
#include "functions.h"
#include "audio_filters.h"

static bool ATU_Finished = false;
static bool ATU_InProcess = false;
static float32_t ATU_MinSWR = 1.0;
static uint8_t ATU_MinSWR_I = 0;
static uint8_t ATU_MinSWR_C = 0;
static bool ATU_MinSWR_T = false;
static uint8_t ATU_Stage = 0;
bool ATU_TunePowerStabilized = false;

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
	if(CALIBRATE.RF_unit_type == RF_UNIT_BIG)
	{
		if (freq >= CALIBRATE.RFU_BPF_7_START && freq < CALIBRATE.RFU_BPF_7_END)
			return 7;
		if (freq >= CALIBRATE.RFU_BPF_8_START && freq < CALIBRATE.RFU_BPF_8_END)
			return 8;
		if (freq >= CALIBRATE.RFU_BPF_9_START && freq < CALIBRATE.RFU_BPF_9_END)
			return 9;
	}
	if(CALIBRATE.RF_unit_type == RF_UNIT_QRP)
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
}

static void RF_UNIT_ProcessATU(void)
{
	if (!TRX.ATU_Enabled)
	{
		ATU_Finished = true;
		return;
	}
	if (!TRX_Tune)
		return;
	if (TRX_PWR_Forward < 2.0f)
		return;
	//if (TRX_SWR > 2.0f && !ATU_InProcess)
		//ATU_Finished = false;
	if (ATU_Finished)
		return;
	if (!ATU_TunePowerStabilized)
		return;
	if (TRX_SWR <= NORMAL_SWR)
	{
		ATU_Finished = true;
		return;
	}
	
	#define delay_stages 1
	static uint8_t delay_stages_count = 0;
	if(delay_stages_count < delay_stages)
	{
		delay_stages_count++;
		return;
	}
	else
	{
		delay_stages_count = 0;
	}
	
	float32_t TRX_PWR = TRX_PWR_Forward - TRX_PWR_Backward;
	
	static float32_t ATU_MinSWR_Slider = 9.9f;
	static float32_t ATU_MinSWR_prev = 9.9f;
	static float32_t ATU_MinSWR_prev_prev = 9.9f;
		
	if(!ATU_Finished && !ATU_InProcess)
	{
		TRX.ATU_I = 0;
		TRX.ATU_C = 0;
		TRX.ATU_T = 0;
		ATU_MinSWR = 99.9f;
		ATU_MinSWR_prev = ATU_MinSWR;
		ATU_MinSWR_prev_prev = ATU_MinSWR;
		ATU_MinSWR_I = 0;
		ATU_MinSWR_C = 0;
		ATU_MinSWR_T = false;
		ATU_Stage = 0;
		ATU_InProcess = true;
	}
	else
	{
		//best result
		if(ATU_MinSWR > TRX_SWR)
		{
			ATU_MinSWR = TRX_SWR;
			ATU_MinSWR_I = TRX.ATU_I;
			ATU_MinSWR_C = TRX.ATU_C;
			ATU_MinSWR_T = TRX.ATU_T;
		}
		//wrong way?
		bool wrong_way = false;
		if(ATU_MinSWR_prev_prev < ATU_MinSWR_prev && ATU_MinSWR_prev < TRX_SWR)
			wrong_way = true;
		ATU_MinSWR_prev_prev = ATU_MinSWR_prev;
		ATU_MinSWR_prev = TRX_PWR;
		
		//debug
		if(ATU_Stage == 0)
			print("INDS ");
		if(ATU_Stage == 1)
			print("CAPS ");
		if(ATU_Stage == 2)
			print("CAPS T ");
		if(ATU_Stage == 3)
			print("I-1 ");
		if(ATU_Stage == 4)
			print("I+1 ");
		if(wrong_way) 
			print("Wr Way ");
		println("I: ", TRX.ATU_I, " C: ", TRX.ATU_C, " T: ", (uint8_t)TRX.ATU_T, " SWR: ", TRX_SWR, " PWR: ", TRX_PWR);
		//iteration block
		uint8_t MAX_ATU_POS = 0;
		if(CALIBRATE.RF_unit_type == RF_UNIT_BIG)
			MAX_ATU_POS = B8(00011111); //5x5 tuner
		
		if(ATU_Stage == 0) //iterate inds
		{
			if(TRX.ATU_I < MAX_ATU_POS && !wrong_way)
			{
				TRX.ATU_I++;
			}
			else
			{
				ATU_Stage = 1;
				TRX.ATU_I = ATU_MinSWR_I;
				TRX.ATU_C = 1;
				ATU_MinSWR_prev_prev = ATU_MinSWR_prev = 9.9f;
			}
		}
		else if(ATU_Stage == 1) //iterate caps
		{
			if(TRX.ATU_C < MAX_ATU_POS)
			{
				TRX.ATU_C++;
			}
			else
			{
				ATU_Stage = 2;
				TRX.ATU_C = 0;
				TRX.ATU_T = true;
				ATU_MinSWR_prev_prev = ATU_MinSWR_prev = 9.9f;
			}
		}
		else if(ATU_Stage == 2) //iterate caps with other T
		{
			if(TRX.ATU_C < MAX_ATU_POS)
			{
				TRX.ATU_C++;
			}
			else
			{
				TRX.ATU_C = 0;
				TRX.ATU_T = ATU_MinSWR_T;
				ATU_MinSWR_Slider = ATU_MinSWR;
				if(TRX.ATU_I > 0)
				{
					TRX.ATU_I = ATU_MinSWR_I - 1;
					ATU_Stage = 3;
				}
				else
					ATU_Stage = 4;
				ATU_MinSWR_prev_prev = ATU_MinSWR_prev = 9.09f;
			}
		}
		else if(ATU_Stage == 3) //iterate caps with i-1
		{
			if(TRX.ATU_C < MAX_ATU_POS && !wrong_way)
			{
				TRX.ATU_C++;
			}
			else
			{
				//slide more?
				if(ATU_MinSWR < ATU_MinSWR_Slider && TRX.ATU_I > 0)
				{
					ATU_MinSWR_Slider = ATU_MinSWR;
					TRX.ATU_I = ATU_MinSWR_I - 1;
					TRX.ATU_C = 0;
				}
				else //go slide right
				{
					TRX.ATU_C = 0;
					ATU_MinSWR_Slider = ATU_MinSWR;
					if(TRX.ATU_I < MAX_ATU_POS)
					{
						TRX.ATU_I = ATU_MinSWR_I + 1;
						ATU_Stage = 4;
					}
					else
						ATU_Stage = 5;
				}
				ATU_MinSWR_prev_prev = ATU_MinSWR_prev = 9.9f;
			}
		}
		else if(ATU_Stage == 4) //iterate caps with i+1
		{
			if(TRX.ATU_C < MAX_ATU_POS && !wrong_way)
			{
				TRX.ATU_C++;
			}
			else
			{
				//slide more?
				if(ATU_MinSWR < ATU_MinSWR_Slider && TRX.ATU_I < MAX_ATU_POS)
				{
					ATU_MinSWR_Slider = ATU_MinSWR;
					TRX.ATU_I = ATU_MinSWR_I + 1;
					TRX.ATU_C = 0;
				}
				else //enough
				{
					ATU_Stage = 5;
				}
				ATU_MinSWR_prev_prev = ATU_MinSWR_prev = 0.0f;
			}
		}
		
		if(ATU_Stage == 5) //finish tune
		{
			ATU_InProcess = false;
			ATU_Finished = true;
			TRX.ATU_I = ATU_MinSWR_I;
			TRX.ATU_C = ATU_MinSWR_C;
			TRX.ATU_T = ATU_MinSWR_T;
			println("ATU best I: ", TRX.ATU_I, " C: ", TRX.ATU_C, " T: ", (uint8_t)TRX.ATU_T, " SWR: ", ATU_MinSWR, " PWR: ", TRX_PWR);
		}
	}
}

void RF_UNIT_UpdateState(bool clean) // pass values to RF-UNIT
{
	bool dualrx_lpf_disabled = false;
	bool dualrx_bpf_disabled = false;
	if(CALIBRATE.RF_unit_type == RF_UNIT_QRP)
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

	uint8_t band_out = 0;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band == BANDID_2200m || band == 1 || band == 2) //2200m
		band_out = CALIBRATE.EXT_2200m;
	if (band == BANDID_160m || band == 4) //160m
		band_out = CALIBRATE.EXT_160m;
	if (band == BANDID_80m || band == 5 || band == 7 || band == 8) //80m
		band_out = CALIBRATE.EXT_80m;
	if (band == BANDID_60m || band == 9 || band == 11) //60m
		band_out = CALIBRATE.EXT_60m;
	if (band == BANDID_40m || band == 13) //40m
		band_out = CALIBRATE.EXT_40m;
	if (band == BANDID_30m || band == 14 || band == 16) //30m
		band_out = CALIBRATE.EXT_30m;
	if (band == BANDID_20m || band == 17 || band == 19) //20m
		band_out = CALIBRATE.EXT_20m;
	if (band == BANDID_17m || band == 20 || band == 22) //17m
		band_out = CALIBRATE.EXT_17m;
	if (band == BANDID_15m || band == 24) //15m
		band_out = CALIBRATE.EXT_15m;
	if (band == BANDID_12m || band == 26) //12m
		band_out = CALIBRATE.EXT_12m;
	if (band == BANDID_CB) //CB
		band_out = CALIBRATE.EXT_CB;
	if (band == BANDID_10m) //10m
		band_out = CALIBRATE.EXT_10m;
	if (band == BANDID_6m) //6m
		band_out = CALIBRATE.EXT_6m;
	if (band == BANDID_FM || band == 31) //FM
		band_out = CALIBRATE.EXT_FM;
	if (band == BANDID_2m || band == BANDID_Marine) //2m
		band_out = CALIBRATE.EXT_2m;
	if (band == BANDID_70cm) //70cm
		band_out = CALIBRATE.EXT_70cm;

	//QRP Version RF Unit
	if(CALIBRATE.RF_unit_type == RF_UNIT_QRP)
	{
		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); //latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 24; registerNumber++)
		{
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); //data
			MINI_DELAY
			if (!clean)
			{
				//U7-QH LPF_ON
				if (registerNumber == 0 && TRX.RF_Filters && (CurrentVFO->Freq <= CALIBRATE.RFU_LPF_END) && !dualrx_lpf_disabled)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-QG LNA_ON
				if (registerNumber == 1 && !TRX_on_TX() && TRX.LNA)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-QF ATT_ON_0.5
				if (registerNumber == 2 && TRX.ATT && att_val_05)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-QE ATT_ON_1
				if (registerNumber == 3 && TRX.ATT && att_val_1)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-QD ATT_ON_2
				if (registerNumber == 4 && TRX.ATT && att_val_2)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-QC ATT_ON_4
				if (registerNumber == 5 && TRX.ATT && att_val_4)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-QB ATT_ON_8
				if (registerNumber == 6 && TRX.ATT && att_val_8)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-QA ATT_ON_16
				if (registerNumber == 7 && TRX.ATT && att_val_16)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);

				//U1-QH NOT USED
				//if (registerNumber == 8)
				//U1-QG BPF_2_A0
				if (registerNumber == 9 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 1 || bpf == 2))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-QF BPF_2_A1
				if (registerNumber == 10 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 4 || bpf == 2))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-QE BPF_2_!EN
				if (registerNumber == 11 && (!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 1 && bpf != 2 && bpf != 3 && bpf != 4)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-QD BPF_1_A0
				if (registerNumber == 12 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 5 || bpf == 6))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-QC BPF_1_A1
				if (registerNumber == 13 && TRX.RF_Filters && !dualrx_bpf_disabled && (bpf == 0 || bpf == 6))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-QB BPF_1_!EN
				if (registerNumber == 14 && (!TRX.RF_Filters || dualrx_bpf_disabled || (bpf != 0 && bpf != 5 && bpf != 6 && bpf != 7)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-QA BPF_ON
				if (registerNumber == 15 && TRX.RF_Filters && !dualrx_bpf_disabled && bpf != 255)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);

				//U3-QH BAND_OUT_0
				if (registerNumber == 16 && bitRead(band_out, 0))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-QG PTT_OUT
				if (registerNumber == 17 && TRX_on_TX() && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-QF BAND_OUT_2
				if (registerNumber == 18 && bitRead(band_out, 2))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-QE BAND_OUT_3
				if (registerNumber == 19 && bitRead(band_out, 3))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-QD TUNE_OUT
				if (registerNumber == 20 && TRX_Tune)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-QC BAND_OUT_1
				if (registerNumber == 21 && bitRead(band_out, 1))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-QB FAN_OUT
				if (registerNumber == 22) //FAN
				{
					static bool fan_status = false;
					static bool fan_pwm = false;
					if (fan_status && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) // Temperature at which the fan stops
						fan_status = false;
					if (!fan_status && TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						fan_status = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) // Temperature at which the fan starts at full power
						fan_pwm = false;

					if (fan_status)
					{
						if (fan_pwm) //PWM
						{
							const uint8_t on_ticks = 1;
							const uint8_t off_ticks = 1;
							static bool pwm_status = false; //true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status)
								HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks))
							{
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						}
						else
							HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
					}
				}
				//U3-QA ANT1_TX_OUT
				if (registerNumber == 23 && !TRX.ANT && TRX_on_TX()) //ANT1
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				if (registerNumber == 23 && TRX.ANT && !TRX_on_TX()) //ANT2
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
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

	//BIG Version RF Unit
	if(CALIBRATE.RF_unit_type == RF_UNIT_BIG)
	{
		if(TRX_Tune)
			RF_UNIT_ProcessATU();
		static const uint8_t MAX_ATU_POS = B8(00011111); //5x5 tuner
		
		HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); //latch
		MINI_DELAY
		for (uint8_t registerNumber = 0; registerNumber < 40; registerNumber++)
		{
			HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); // data block
			MINI_DELAY
			HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); //data
			MINI_DELAY
			if (!clean)
			{
				//U1-7 HF-VHF-SELECT
				if (registerNumber == 0 && CurrentVFO->Freq >= 70000000)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-6 ATT_ON_1
				if (registerNumber == 1 && !(TRX.ATT && att_val_1))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-5 ATT_ON_0.5
				if (registerNumber == 2 && !(TRX.ATT && att_val_05))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-4 ATT_ON_16
				if (registerNumber == 3 && !(TRX.ATT && att_val_16))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-3 ATT_ON_2
				if (registerNumber == 4 && !(TRX.ATT && att_val_2))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-2 ATT_ON_4
				if (registerNumber == 5 && !(TRX.ATT && att_val_4))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-1 ATT_ON_8
				if (registerNumber == 6 && !(TRX.ATT && att_val_8))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U1-0 LNA_ON
				if (registerNumber == 7 && !(!TRX_on_TX() && TRX.LNA))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				
				//U3-7 TUN_C_5
				if (registerNumber == 8 && bitRead(TRX.ATU_C, 4))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-6 TUN_C_4
				if (registerNumber == 9 && bitRead(TRX.ATU_C, 3))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-5 BPF_1
				if (registerNumber == 10 && (bpf == 1 || (!TRX_on_TX() && TRX.Dual_RX && bpf_second == 1)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-4 BPF_2
				if (registerNumber == 11 && (bpf == 2 || (!TRX_on_TX() && TRX.Dual_RX && bpf_second == 2)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-3 TX_PTT_OUT
				if (registerNumber == 12 && TRX_on_TX() && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-2 TUN_C_1
				if (registerNumber == 13 && bitRead(TRX.ATU_C, 0))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-1 TUN_C_2
				if (registerNumber == 14 && bitRead(TRX.ATU_C, 1))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U3-0 TUN_C_3
				if (registerNumber == 15 && bitRead(TRX.ATU_C, 2))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				
				//U2-7 TUN_T
				if (registerNumber == 16 && TRX.ATU_T)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U2-6 TUN_I_5
				if (registerNumber == 17 && bitRead(TRX.ATU_I, 4))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U2-5 UNUSED
				//if (registerNumber == 18 &&
				//U2-4 VHF_AMP_BIAS_ON
				if (registerNumber == 19 && TRX_on_TX() && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq >= 70000000)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U2-3 TUN_I_1
				if (registerNumber == 20 && bitRead(TRX.ATU_I, 0))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U2-2 TUN_I_2
				if (registerNumber == 21 && bitRead(TRX.ATU_I, 1))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U2-1 TUN_I_3
				if (registerNumber == 22 && bitRead(TRX.ATU_I, 2))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U2-0 TUN_I_4
				if (registerNumber == 23 && bitRead(TRX.ATU_I, 3))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				
				//U7-7 BPF_6
				if (registerNumber == 24 && (bpf == 6 || (!TRX_on_TX() && TRX.Dual_RX && bpf_second == 6)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-6 BPF_5
				if (registerNumber == 25 && (bpf == 5 || (!TRX_on_TX() && TRX.Dual_RX && bpf_second == 5)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-5 BPF_4
				if (registerNumber == 26 && (bpf == 4 || (!TRX_on_TX() && TRX.Dual_RX && bpf_second == 4)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-4 BPF_3
				if (registerNumber == 27 && (bpf == 3 || (!TRX_on_TX() && TRX.Dual_RX && bpf_second == 3)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-3 BPF_7
				if (registerNumber == 28 && (bpf == 7 || (!TRX_on_TX() && TRX.Dual_RX && bpf_second == 7)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-2 BPF_8
				if (registerNumber == 29 && (bpf == 8 || (!TRX_on_TX() && TRX.Dual_RX && bpf_second == 8)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-1 BPF_9
				if (registerNumber == 30 && (bpf == 9 || (!TRX_on_TX() && TRX.Dual_RX && bpf_second == 9)))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U7-0 HF_AMP_BIAS_ON
				if (registerNumber == 31 && TRX_on_TX() && CurrentVFO->Mode != TRX_MODE_LOOPBACK && CurrentVFO->Freq < 70000000)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				
				//U11-7 ANT1-2_OUT
				if (registerNumber == 32 && TRX.ANT)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U11-6 FAN_OUT
				if (registerNumber == 33)
				{
					static bool fan_status = false;
					static bool fan_pwm = false;
					if (fan_status && TRX_RF_Temperature <= CALIBRATE.FAN_MEDIUM_STOP) // Temperature at which the fan stops
						fan_status = false;
					if (!fan_status && TRX_RF_Temperature >= CALIBRATE.FAN_MEDIUM_START) // Temperature at which the fan starts at half power
					{
						fan_status = true;
						fan_pwm = true;
					}
					if (TRX_RF_Temperature >= CALIBRATE.FAN_FULL_START) // Temperature at which the fan starts at full power
						fan_pwm = false;

					if (fan_status)
					{
						if (fan_pwm) //PWM
						{
							const uint8_t on_ticks = 1;
							const uint8_t off_ticks = 1;
							static bool pwm_status = false; //true - on false - off
							static uint8_t pwm_ticks = 0;
							pwm_ticks++;
							if (pwm_status)
								HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
							if ((pwm_status && pwm_ticks == on_ticks) || (!pwm_status && pwm_ticks == off_ticks))
							{
								pwm_status = !pwm_status;
								pwm_ticks = 0;
							}
						}
						else
							HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
					}
				}
				//U11-5 BAND_OUT_3
				if (registerNumber == 34 && bitRead(band_out, 3))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U11-4 TX_PTT_OUT
				if (registerNumber == 35 && TRX_on_TX() && CurrentVFO->Mode != TRX_MODE_LOOPBACK)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U11-3 BAND_OUT_1
				if (registerNumber == 36 && bitRead(band_out, 1))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U11-2 TUNE_OUT
				if (registerNumber == 37 && TRX_Tune)
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U11-1 BAND_OUT_2
				if (registerNumber == 38 && bitRead(band_out, 2))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				//U11-0 BAND_OUT_0
				if (registerNumber == 39 && bitRead(band_out, 0))
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
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
	//THERMAL

	float32_t rf_thermal = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3)) * TRX_STM32_VREF / 16383.0f;

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
	float32_t TRX_RF_Temperature_new = (power_left * (1.0f - part_point)) + (power_right * (part_point));
	if (TRX_RF_Temperature_new < 0.0f)
		TRX_RF_Temperature_new = 0.0f;
	if (fabsf(TRX_RF_Temperature_new - TRX_RF_Temperature) > 0.5f) //hysteresis
		TRX_RF_Temperature = TRX_RF_Temperature_new;

	//SWR
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / 16383.0f;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / 16383.0f;
	TRX_ALC_IN = (float32_t)HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_4) * TRX_STM32_VREF / 16383.0f;
	//	static float32_t TRX_VLT_forward = 0.0f;		//Tisho
	//	static float32_t TRX_VLT_backward = 0.0f;		//Tisho

#if (defined(SWR_AD8307_LOG)) //If it is used the Log amp. AD8307

	float32_t P_FW_dBm, P_BW_dBm;
	float32_t V_FW_Scaled, V_BW_Scaled;
	//float32_t NewSWR;

	TRX_VLT_forward = TRX_VLT_forward + (forward - TRX_VLT_forward) / 4;
	TRX_VLT_backward = TRX_VLT_backward + (backward - TRX_VLT_backward) / 4;

	//Calculate the Forward values
	P_FW_dBm = ((TRX_VLT_forward * 1000) - CALIBRATE.FW_AD8307_OFFS) / (CALIBRATE.FW_AD8307_SLP);
	V_FW_Scaled = pow(10, (double)((P_FW_dBm - 10) / 20));	   //Calculate in voltage (Vp - 50ohm terminated)
	TRX_PWR_Forward = pow(10, (double)((P_FW_dBm - 30) / 10)); //Calculate in W

	//Calculate the Backward values
	P_BW_dBm = ((TRX_VLT_backward * 1000) - CALIBRATE.BW_AD8307_OFFS) / (CALIBRATE.BW_AD8307_SLP);
	V_BW_Scaled = pow(10, (double)((P_BW_dBm - 10) / 20));		//Calculate in voltage (Vp - 50ohm terminated)
	TRX_PWR_Backward = pow(10, (double)((P_BW_dBm - 30) / 10)); //Calculate in W

	TRX_SWR = (V_FW_Scaled + V_BW_Scaled) / (V_FW_Scaled - V_BW_Scaled); //Calculate SWR

	//TRX_SWR = TRX_SWR + (NewSWR - TRX_SWR) / 2;

	if (TRX_SWR > 10.0f)
		TRX_SWR = 10.0f;
	if (TRX_SWR < 0.0f)
		TRX_SWR = 0.0f;

#else //if it is used the standard measure (diode rectifier)
	forward = forward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
	if (forward < 0.1f)							  // do not measure less than 100mV
	{
		TRX_VLT_forward = 0.0f;
		TRX_VLT_backward = 0.0f;
		TRX_PWR_Forward = 0.0f;
		TRX_PWR_Backward = 0.0f;
		TRX_SWR = 1.0f;
	}
	else
	{
		forward += 0.21f;								   // drop on diode
		
		// Transformation ratio of the SWR meter
		if (CurrentVFO->Freq >= 80000000)
			forward = forward * CALIBRATE.SWR_FWD_Calibration_VHF;
		else if (CurrentVFO->Freq >= 40000000)
			forward = forward * CALIBRATE.SWR_FWD_Calibration_6M;
		else
			forward = forward * CALIBRATE.SWR_FWD_Calibration_HF;

		backward = backward / (510.0f / (0.0f + 510.0f)); // adjust the voltage based on the voltage divider (0 ohm and 510 ohm)
		if (backward >= 0.1f)								// do not measure less than 100mV
		{
			backward += 0.21f;									 // drop on diode
			
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
}

//Tisho
//used to controll the calibration of the FW and BW power measurments
void RF_UNIT_MeasureVoltage(void)
{
	float32_t forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / 16383.0f;
	float32_t backward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / 16383.0f;
	//use the TRX_VLT_forward and TRX_VLT_backward global variables
	//for the raw ADC input voltages
	//in the TDM_Voltages() the other stuff will be calculated localy

	static float32_t VLT_forward = 0.0f;
	static float32_t VLT_backward = 0.0f;
	//	TRX_VLT_forward = (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2)) * TRX_STM32_VREF / 16383.0f;
	//	TRX_VLT_backward= (float32_t)(HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1)) * TRX_STM32_VREF / 16383.0f;
	VLT_forward = VLT_forward + (forward - VLT_forward) / 10;
	VLT_backward = VLT_backward + (backward - VLT_backward) / 10;

	TRX_VLT_forward = VLT_forward;
	TRX_VLT_backward = VLT_backward;
}
