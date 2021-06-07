#include "stm32h7xx_hal.h"
#include "main.h"
#include "trx_manager.h"
#include "functions.h"
#include "lcd.h"
#include "fpga.h"
#include "settings.h"
#include "wm8731.h"
#include "fpga.h"
#include "bands.h"
#include "agc.h"
#include "audio_filters.h"
#include "usbd_audio_if.h"
#include "cw_decoder.h"
#include "front_unit.h"
#include "rf_unit.h"
#include "system_menu.h"
#include "vad.h"
#include "swr_analyzer.h"

volatile bool TRX_ptt_hard = false;
volatile bool TRX_ptt_soft = false;
volatile bool TRX_old_ptt_soft = false;
volatile bool TRX_key_serial = false;
volatile bool TRX_old_key_serial = false;
volatile bool TRX_key_dot_hard = false;
volatile bool TRX_key_dash_hard = false;
volatile uint_fast16_t TRX_Key_Timeout_est = 0;
volatile bool TRX_RX1_IQ_swap = false;
volatile bool TRX_RX2_IQ_swap = false;
volatile bool TRX_TX_IQ_swap = false;
volatile bool TRX_Tune = false;
volatile bool TRX_Inited = false;
volatile float32_t TRX_RX_dBm = -100.0f;
volatile bool TRX_ADC_OTR = false;
volatile bool TRX_DAC_OTR = false;
volatile int16_t TRX_ADC_MINAMPLITUDE = 0;
volatile int16_t TRX_ADC_MAXAMPLITUDE = 0;
volatile int32_t TRX_VCXO_ERROR = 0;
volatile uint16_t TRX_Volume = 0;
volatile uint32_t TRX_SNTP_Synced = 0; // time of the last time synchronization
volatile int_fast16_t TRX_SHIFT = 0;
volatile float32_t TRX_MAX_TX_Amplitude = MAX_TX_AMPLITUDE;
volatile float32_t TRX_PWR_Forward = 0;
volatile float32_t TRX_PWR_Backward = 0;
volatile float32_t TRX_SWR = 0;
volatile float32_t TRX_VLT_forward = 0;	 //Tisho
volatile float32_t TRX_VLT_backward = 0; //Tisho
volatile float32_t TRX_ALC_OUT = 0;
volatile float32_t TRX_ALC_IN = 0;
volatile bool TRX_DAC_DIV0 = false;
volatile bool TRX_DAC_DIV1 = false;
volatile bool TRX_DAC_HP1 = false;
volatile bool TRX_DAC_HP2 = false;
volatile bool TRX_DAC_X4 = false;
volatile bool TRX_DCDC_Freq = false;
volatile bool TRX_DAC_DRV_A0 = true;
volatile bool TRX_DAC_DRV_A1 = true;
static uint32_t KEYER_symbol_start_time = 0;	  // start time of the automatic key character
static uint_fast8_t KEYER_symbol_status = 0;	  // status (signal or period) of the automatic key symbol
volatile float32_t TRX_STM32_VREF = 3.3f;		  // voltage on STM32
volatile float32_t TRX_STM32_TEMPERATURE = 30.0f; // STM32 temperature
volatile float32_t TRX_IQ_phase_error = 0.0f;
volatile bool TRX_Temporary_Stop_BandMap = false;
volatile bool TRX_Mute = false;
volatile uint32_t TRX_Temporary_Mute_StartTime = 0;
uint32_t TRX_freq_phrase = 0;
uint32_t TRX_freq_phrase2 = 0;
uint32_t TRX_freq_phrase_tx = 0;
volatile float32_t TRX_RF_Temperature = 0.0f;
volatile bool TRX_ScanMode = false;
bool TRX_phase_restarted = false;
uint32_t TRX_TX_StartTime = 0;
uint32_t TRX_DXCluster_UpdateTime = 0;

static uint_fast8_t TRX_TXRXMode = 0; //0 - undef, 1 - rx, 2 - tx, 3 - txrx
static void TRX_Start_RX(void);
static void TRX_Start_TX(void);
static void TRX_Start_TXRX(void);

bool TRX_on_TX(void)
{
	if (TRX_ptt_hard || TRX_ptt_soft || TRX_Tune || CurrentVFO->Mode == TRX_MODE_LOOPBACK || TRX_Key_Timeout_est > 0)
		return true;
	return false;
}

void TRX_Init()
{
	TRX_Start_RX();
	WM8731_TXRX_mode();
	WM8731_start_i2s_and_dma();
	uint_fast8_t saved_mode = CurrentVFO->Mode;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	TRX_setMode(saved_mode, CurrentVFO);
	HAL_ADCEx_InjectedStart(&hadc1); //ADC RF-UNIT'а
	HAL_ADCEx_InjectedStart(&hadc3); //ADC CPU temperature
}

void TRX_Restart_Mode()
{
	ADCDAC_OVR_StatusLatency = 0;
	uint_fast8_t mode = CurrentVFO->Mode;
	//CLAR
	if (TRX.CLAR)
	{
		TRX.selected_vfo = !TRX.selected_vfo;
		if(!TRX.selected_vfo)
		{
			CurrentVFO = &TRX.VFO_A;
			SecondaryVFO = &TRX.VFO_B;
		}
		else
		{
			CurrentVFO = &TRX.VFO_B;
			SecondaryVFO = &TRX.VFO_A;
		}
		TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
		TRX_setMode(CurrentVFO->Mode, CurrentVFO);

		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		TRX.ANT = TRX.BANDS_SAVED_SETTINGS[band].ANT;

		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.TopButtons = true;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}
	//
	if (TRX_on_TX())
	{
		if (mode == TRX_MODE_LOOPBACK || mode == TRX_MODE_CW)
			TRX_Start_TXRX();
		else
			TRX_Start_TX();
	}
	else
	{
		TRX_Start_RX();
	}
	NeedReinitReverber = true;
	NeedFFTReinit = true;
}

static void TRX_Start_RX()
{
	if (TRX_TXRXMode == 1)
		return;
	println("RX MODE");
	FPGA_NeedRestart = true;
	TRX_phase_restarted = false;
	RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	Processor_NeedRXBuffer = false;
	WM8731_Buffer_underrun = false;
	WM8731_DMA_state = true;
	TRX_TXRXMode = 1;
	//clean TX buffer
	dma_memset((void *)&FPGA_Audio_SendBuffer_Q[0], 0x00, sizeof(FPGA_Audio_SendBuffer_Q));
	dma_memset((void *)&FPGA_Audio_SendBuffer_I[0], 0x00, sizeof(FPGA_Audio_SendBuffer_I));
}

static void TRX_Start_TX()
{
	if (TRX_TXRXMode == 2)
		return;
	println("TX MODE");
	RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	TRX_TX_StartTime = HAL_GetTick();
	TRX_TXRXMode = 2;
}

static void TRX_Start_TXRX()
{
	if (TRX_TXRXMode == 3)
		return;
	println("TXRX MODE");
	RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	TRX_TXRXMode = 3;
}

void TRX_ptt_change(void)
{
	if (TRX_Tune)
		TRX_Tune = false;
	
	bool notx = TRX_TX_Disabled(CurrentVFO->Freq);
	if(notx)
	{
		TRX_ptt_soft = false;
		TRX_ptt_hard = false;
		return;
	}
	
	bool TRX_new_ptt_hard = !HAL_GPIO_ReadPin(PTT_IN_GPIO_Port, PTT_IN_Pin);
	if (TRX_ptt_hard != TRX_new_ptt_hard)
	{
		TRX_ptt_hard = TRX_new_ptt_hard;
		TRX_ptt_soft = false;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
	if (TRX_ptt_soft != TRX_old_ptt_soft)
	{
		TRX_old_ptt_soft = TRX_ptt_soft;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
}

bool TRX_TX_Disabled(uint32_t freq)
{
	bool notx = false;
	int8_t band = getBandFromFreq(freq, false);
	switch(band)
	{
		case BANDID_2200m:
			if(CALIBRATE.NOTX_2200m)
				notx = true;
			break;
		case BANDID_160m:
			if(CALIBRATE.NOTX_160m)
				notx = true;
			break;
		case BANDID_80m:
			if(CALIBRATE.NOTX_80m)
				notx = true;
			break;
		case BANDID_60m:
			if(CALIBRATE.NOTX_60m)
				notx = true;
			break;
		case BANDID_40m:
			if(CALIBRATE.NOTX_40m)
				notx = true;
			break;
		case BANDID_30m:
			if(CALIBRATE.NOTX_30m)
				notx = true;
			break;
		case BANDID_20m:
			if(CALIBRATE.NOTX_20m)
				notx = true;
			break;
		case BANDID_17m:
			if(CALIBRATE.NOTX_17m)
				notx = true;
			break;
		case BANDID_15m:
			if(CALIBRATE.NOTX_15m)
				notx = true;
			break;
		case BANDID_12m:
			if(CALIBRATE.NOTX_12m)
				notx = true;
			break;
		case BANDID_CB:
			if(CALIBRATE.NOTX_CB)
				notx = true;
			break;
		case BANDID_10m:
			if(CALIBRATE.NOTX_10m)
				notx = true;
			break;
		case BANDID_6m:
			if(CALIBRATE.NOTX_6m)
				notx = true;
			break;
		case BANDID_2m:
		case BANDID_Marine:
			if(CALIBRATE.NOTX_2m)
				notx = true;
			break;
		case BANDID_70cm:
			if(CALIBRATE.NOTX_70cm)
				notx = true;
			break;
		default:
			if(CALIBRATE.NOTX_NOTHAM)
				notx = true;
			break;
	}
	return notx;
}

void TRX_key_change(void)
{
	if (TRX_Tune)
		return;
	if (CurrentVFO->Mode != TRX_MODE_CW)
		return;
	bool TRX_new_key_dot_hard = !HAL_GPIO_ReadPin(KEY_IN_DOT_GPIO_Port, KEY_IN_DOT_Pin);
	if (TRX_key_dot_hard != TRX_new_key_dot_hard)
	{
		TRX_key_dot_hard = TRX_new_key_dot_hard;
		if (TRX_key_dot_hard == true && (KEYER_symbol_status == 0 || !TRX.CW_KEYER))
		{
			TRX_Key_Timeout_est = TRX.CW_Key_timeout;
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
			FPGA_NeedSendParams = true;
			TRX_Restart_Mode();
		}
	}
	bool TRX_new_key_dash_hard = !HAL_GPIO_ReadPin(KEY_IN_DASH_GPIO_Port, KEY_IN_DASH_Pin);
	if (TRX_key_dash_hard != TRX_new_key_dash_hard)
	{
		TRX_key_dash_hard = TRX_new_key_dash_hard;
		if (TRX_key_dash_hard == true && (KEYER_symbol_status == 0 || !TRX.CW_KEYER))
		{
			TRX_Key_Timeout_est = TRX.CW_Key_timeout;
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
			FPGA_NeedSendParams = true;
			TRX_Restart_Mode();
		}
	}
	if (TRX_key_serial != TRX_old_key_serial)
	{
		TRX_old_key_serial = TRX_key_serial;
		if (TRX_key_serial == true)
			TRX_Key_Timeout_est = TRX.CW_Key_timeout;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
}

void TRX_setFrequency(uint32_t _freq, VFO *vfo)
{
	if (_freq < 1)
		return;
	if (_freq >= MAX_RX_FREQ_HZ)
		_freq = MAX_RX_FREQ_HZ;

	vfo->Freq = _freq;
	
	//set DC-DC Sync freq
	uint32_t dcdc_offset_0 = abs((int32_t)DCDC_FREQ_0 / 2 - (int32_t)_freq % (int32_t)DCDC_FREQ_0);
	uint32_t dcdc_offset_1 = abs((int32_t)DCDC_FREQ_1 / 2 - (int32_t)_freq % (int32_t)DCDC_FREQ_1);
	if (dcdc_offset_0 > dcdc_offset_1)
		TRX_DCDC_Freq = 1;
	else
		TRX_DCDC_Freq = 0;

	//get settings and fpga freq phrase
	TRX_freq_phrase = getRXPhraseFromFrequency((int32_t)CurrentVFO->Freq + TRX_SHIFT, 1);
	TRX_freq_phrase2 = getRXPhraseFromFrequency((int32_t)SecondaryVFO->Freq + TRX_SHIFT, 2);
	TRX_freq_phrase_tx = getTXPhraseFromFrequency((int32_t)CurrentVFO->Freq);
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(vfo->Freq);
	FPGA_NeedSendParams = true;
	
	//services
	if (SYSMENU_spectrum_opened || SYSMENU_swr_opened)
		return;
	
	//get band
	int_fast8_t bandFromFreq = getBandFromFreq(_freq, true);
	if (bandFromFreq >= 0)
	{
		TRX.BANDS_SAVED_SETTINGS[bandFromFreq].Freq = _freq;
	}
	if (TRX.BandMapEnabled && !TRX_Temporary_Stop_BandMap)
	{
		uint_fast8_t mode_from_bandmap = getModeFromFreq(vfo->Freq);
		if (vfo->Mode != mode_from_bandmap)
		{
			TRX_setMode(mode_from_bandmap, vfo);
			TRX.BANDS_SAVED_SETTINGS[bandFromFreq].Mode = mode_from_bandmap;
			LCD_UpdateQuery.TopButtons = true;
		}
	}
}

void TRX_setTXFrequencyFloat(float64_t _freq, VFO *vfo)
{
	TRX_setFrequency((uint32_t)_freq, vfo);
	TRX_freq_phrase_tx = getTXPhraseFromFrequency(_freq);
	FPGA_NeedSendParams = true;
}

void TRX_setMode(uint_fast8_t _mode, VFO *vfo)
{
	uint_fast8_t old_mode = vfo->Mode;
	vfo->Mode = _mode;
	if (vfo->Mode == TRX_MODE_LOOPBACK)
		TRX_Start_TXRX();

	switch (_mode)
	{
	case TRX_MODE_AM:
	case TRX_MODE_SAM:
		vfo->LPF_RX_Filter_Width = TRX.AM_LPF_RX_Filter;
		vfo->LPF_TX_Filter_Width = TRX.AM_LPF_TX_Filter;
		vfo->HPF_Filter_Width = 0;
		break;
	case TRX_MODE_LSB:
	case TRX_MODE_USB:
	case TRX_MODE_DIGI_L:
	case TRX_MODE_DIGI_U:
		vfo->LPF_RX_Filter_Width = TRX.SSB_LPF_RX_Filter;
		vfo->LPF_TX_Filter_Width = TRX.SSB_LPF_TX_Filter;
		vfo->HPF_Filter_Width = TRX.SSB_HPF_Filter;
		break;
	case TRX_MODE_CW:
		vfo->LPF_RX_Filter_Width = TRX.CW_LPF_Filter;
		vfo->LPF_TX_Filter_Width = TRX.CW_LPF_Filter;
		vfo->HPF_Filter_Width = 0;
		LCD_UpdateQuery.TextBar = true;
		break;
	case TRX_MODE_NFM:
		vfo->LPF_RX_Filter_Width = TRX.FM_LPF_RX_Filter;
		vfo->LPF_TX_Filter_Width = TRX.FM_LPF_TX_Filter;
		vfo->HPF_Filter_Width = 0;
		break;
	case TRX_MODE_WFM:
		vfo->LPF_RX_Filter_Width = 16000;
		vfo->LPF_TX_Filter_Width = 16000;
		vfo->HPF_Filter_Width = 0;
		break;
	}
	
	//FFT Zoom change
	if (TRX.FFT_Zoom != TRX.FFT_ZoomCW)
	{
		if (old_mode == TRX_MODE_CW && _mode != TRX_MODE_CW)
			NeedFFTReinit = true;
		if (old_mode != TRX_MODE_CW && _mode == TRX_MODE_CW)
			NeedFFTReinit = true;
	}

	//FM Samplerate change
	if (TRX.SAMPLERATE_MAIN != TRX.SAMPLERATE_FM && old_mode != TRX_MODE_WFM && old_mode != TRX_MODE_NFM && (_mode == TRX_MODE_WFM || _mode == TRX_MODE_NFM))
		NeedFFTReinit = true;
	if (TRX.SAMPLERATE_MAIN != TRX.SAMPLERATE_FM && (old_mode == TRX_MODE_WFM || old_mode == TRX_MODE_NFM) && _mode != TRX_MODE_WFM && _mode != TRX_MODE_NFM)
		NeedFFTReinit = true;

	WM8731_TXRX_mode();
	if (old_mode != _mode)
		NeedReinitAudioFiltersClean = true;
	NeedReinitAudioFilters = true;
	NeedSaveSettings = true;
	LCD_UpdateQuery.StatusInfoBar = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedWTFRedraw = true;
}

void TRX_DoAutoGain(void)
{
	uint8_t skip_cycles = 0;
	if (skip_cycles > 0)
	{
		skip_cycles--;
		return;
	}

	//Process AutoGain feature
	if (TRX.AutoGain && !TRX_on_TX())
	{
		if (!TRX.ATT)
		{
			TRX.ATT = true;
			LCD_UpdateQuery.TopButtons = true;
		}

		int32_t max_amplitude = abs(TRX_ADC_MAXAMPLITUDE);
		if (abs(TRX_ADC_MINAMPLITUDE) > max_amplitude)
			max_amplitude = abs(TRX_ADC_MINAMPLITUDE);

		float32_t new_att_val = TRX.ATT_DB;
		if (max_amplitude > (AUTOGAINER_TAGET + AUTOGAINER_HYSTERESIS) && new_att_val < 31.5f)
			new_att_val += 0.5f;
		else if (max_amplitude < (AUTOGAINER_TAGET - AUTOGAINER_HYSTERESIS) && new_att_val > 0.0f)
			new_att_val -= 0.5f;

		if (new_att_val == 0.0f && max_amplitude < (AUTOGAINER_TAGET - AUTOGAINER_HYSTERESIS) && !TRX.ADC_Driver)
		{
			TRX.ADC_Driver = true;
			LCD_UpdateQuery.TopButtons = true;
			skip_cycles = 5;
		}
		else if (new_att_val == 0.0f && max_amplitude < (AUTOGAINER_TAGET - AUTOGAINER_HYSTERESIS) && !TRX.ADC_PGA)
		{
			TRX.ADC_PGA = true;
			LCD_UpdateQuery.TopButtons = true;
			skip_cycles = 5;
		}

		if (new_att_val != TRX.ATT_DB)
		{
			TRX.ATT_DB = new_att_val;
			LCD_UpdateQuery.TopButtons = true;
			//save settings
			int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
			TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
			TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
			TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
		}
	}
}

void TRX_DBMCalculate(void)
{
	if (Processor_RX_Power_value == 0)
	{
		TRX_RX_dBm = -150.0f;
		return;
	}

	float32_t adc_volts = Processor_RX_Power_value * (TRX.ADC_PGA ? (ADC_RANGE_PGA / 2.0f) : (ADC_RANGE / 2.0f));
	if (TRX.ADC_Driver)
		adc_volts *= db2rateV(-ADC_DRIVER_GAIN_DB);
	TRX_RX_dBm = 10.0f * log10f_fast((adc_volts * adc_volts / ADC_INPUT_IMPEDANCE) / 0.001f);
	if(CurrentVFO->Freq < 70000000)
		TRX_RX_dBm += CALIBRATE.smeter_calibration_hf;
	else
		TRX_RX_dBm += CALIBRATE.smeter_calibration_vhf;

	if(TRX_RX_dBm < -150.0f)
		TRX_RX_dBm = -150.0f;
	Processor_RX_Power_value = 0;
}

float32_t current_cw_power = 0.0f;
static float32_t TRX_generateRiseSignal(float32_t power)
{
	if (current_cw_power < power)
		current_cw_power += power * 0.007f;
	if (current_cw_power > power)
		current_cw_power = power;
	return current_cw_power;
}
static float32_t TRX_generateFallSignal(float32_t power)
{
	if (current_cw_power > 0.0f)
		current_cw_power -= power * 0.007f;
	if (current_cw_power < 0.0f)
		current_cw_power = 0.0f;
	return current_cw_power;
}

float32_t TRX_GenerateCWSignal(float32_t power)
{
	if (!TRX.CW_KEYER)
	{
		if (!TRX_key_serial && !TRX_ptt_hard && !TRX_key_dot_hard && !TRX_key_dash_hard)
			return TRX_generateFallSignal(power);
		return TRX_generateRiseSignal(power);
	}

	uint32_t dot_length_ms = 1200 / TRX.CW_KEYER_WPM;
	uint32_t dash_length_ms = dot_length_ms * 3;
	uint32_t sim_space_length_ms = dot_length_ms;
	uint32_t curTime = HAL_GetTick();
	//dot
	if (KEYER_symbol_status == 0 && TRX_key_dot_hard)
	{
		KEYER_symbol_start_time = curTime;
		KEYER_symbol_status = 1;
	}
	if (KEYER_symbol_status == 1 && (KEYER_symbol_start_time + dot_length_ms) > curTime)
	{
		TRX_Key_Timeout_est = TRX.CW_Key_timeout;
		return TRX_generateRiseSignal(power);
	}
	if (KEYER_symbol_status == 1 && (KEYER_symbol_start_time + dot_length_ms) < curTime)
	{
		KEYER_symbol_start_time = curTime;
		KEYER_symbol_status = 3;
	}

	//dash
	if (KEYER_symbol_status == 0 && TRX_key_dash_hard)
	{
		KEYER_symbol_start_time = curTime;
		KEYER_symbol_status = 2;
	}
	if (KEYER_symbol_status == 2 && (KEYER_symbol_start_time + dash_length_ms) > curTime)
	{
		TRX_Key_Timeout_est = TRX.CW_Key_timeout;
		return TRX_generateRiseSignal(power);
	}
	if (KEYER_symbol_status == 2 && (KEYER_symbol_start_time + dash_length_ms) < curTime)
	{
		KEYER_symbol_start_time = curTime;
		KEYER_symbol_status = 3;
	}

	//space
	if (KEYER_symbol_status == 3 && (KEYER_symbol_start_time + sim_space_length_ms) > curTime)
	{
		TRX_Key_Timeout_est = TRX.CW_Key_timeout;
		return TRX_generateFallSignal(power);
	}
	if (KEYER_symbol_status == 3 && (KEYER_symbol_start_time + sim_space_length_ms) < curTime)
	{
		KEYER_symbol_status = 0;
	}

	return TRX_generateFallSignal(power);
}

float32_t TRX_getSTM32H743Temperature(void)
{
	uint16_t TS_CAL1 = *((uint16_t *)0x1FF1E820); // TS_CAL1 Temperature sensor raw data acquired value at 30 °C, VDDA=3.3 V //-V566
	uint16_t TS_CAL2 = *((uint16_t *)0x1FF1E840); // TS_CAL2 Temperature sensor raw data acquired value at 110 °C, VDDA=3.3 V //-V566
	uint32_t TS_DATA = HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_1);
	float32_t result = ((110.0f - 30.0f) / ((float32_t)TS_CAL2 - (float32_t)TS_CAL1)) * ((float32_t)TS_DATA - (float32_t)TS_CAL1) + 30; //from reference
	return result;
}

float32_t TRX_getSTM32H743vref(void)
{
	uint16_t VREFINT_CAL = *VREFINT_CAL_ADDR; // VREFIN_CAL Raw data acquired at temperature of 30 °C, VDDA = 3.3 V //-V566
	uint32_t VREFINT_DATA = HAL_ADCEx_InjectedGetValue(&hadc3, ADC_INJECTED_RANK_2);
	float32_t result = 3.3f * (float32_t)VREFINT_CAL / (float32_t)VREFINT_DATA; //from reference
	return result;
}

void TRX_TemporaryMute(void)
{
	WM8731_Mute();
	TRX_Temporary_Mute_StartTime = HAL_GetTick();
}

//process frequency scanner
void TRX_ProcessScanMode(void)
{
	static bool oldState = false;
	static uint32_t StateChangeTime = 0;
	bool goSweep = false;

	if (CurrentVFO->Mode == TRX_MODE_WFM || CurrentVFO->Mode == TRX_MODE_NFM)
	{
		if (oldState != DFM_RX1_Squelched)
		{
			oldState = DFM_RX1_Squelched;
			StateChangeTime = HAL_GetTick();
		}

		if (DFM_RX1_Squelched && ((HAL_GetTick() - StateChangeTime) > SCANNER_NOSIGNAL_TIME))
			goSweep = true;
		if (!DFM_RX1_Squelched && ((HAL_GetTick() - StateChangeTime) > SCANNER_SIGNAL_TIME_FM))
			goSweep = true;
	}
	else
	{
		if (oldState != VAD_Muting)
		{
			oldState = VAD_Muting;
			StateChangeTime = HAL_GetTick();
		}

		if (VAD_Muting && ((HAL_GetTick() - StateChangeTime) > SCANNER_NOSIGNAL_TIME))
			goSweep = true;
		if (!VAD_Muting && ((HAL_GetTick() - StateChangeTime) > SCANNER_SIGNAL_TIME_OTHER))
			goSweep = true;
	}

	if (goSweep)
	{
		int8_t band = getBandFromFreq(CurrentVFO->Freq, false);
		for (uint8_t region_id = 0; region_id < BANDS[band].regionsCount; region_id++)
		{
			if ((BANDS[band].regions[region_id].startFreq <= CurrentVFO->Freq) && (BANDS[band].regions[region_id].endFreq > CurrentVFO->Freq))
			{
				uint32_t step = SCANNER_FREQ_STEP_OTHER;
				if (CurrentVFO->Mode == TRX_MODE_WFM)
					step = SCANNER_FREQ_STEP_WFM;
				if (CurrentVFO->Mode == TRX_MODE_NFM)
					step = SCANNER_FREQ_STEP_NFM;

				uint32_t new_freq = (CurrentVFO->Freq + step) / step * step;
				if (new_freq >= BANDS[band].regions[region_id].endFreq)
					new_freq = BANDS[band].regions[region_id].startFreq;

				TRX_setFrequency(new_freq, CurrentVFO);
				LCD_UpdateQuery.FreqInfo = true;
				StateChangeTime = HAL_GetTick();
				break;
			}
		}
	}
}

static uint32_t setFreqSlowly_target = 0;
static bool setFreqSlowly_processing = 0;
void TRX_setFrequencySlowly(uint32_t target_freq)
{
	setFreqSlowly_target = target_freq;
	setFreqSlowly_processing = true;
}

void TRX_setFrequencySlowly_Process(void)
{
	if (!setFreqSlowly_processing)
		return;
	int32_t diff = CurrentVFO->Freq - setFreqSlowly_target;
	if (diff > TRX_SLOW_SETFREQ_MIN_STEPSIZE || diff < -TRX_SLOW_SETFREQ_MIN_STEPSIZE)
	{
		TRX_setFrequency(CurrentVFO->Freq - diff / 4, CurrentVFO);
		LCD_UpdateQuery.FreqInfo = true;
	}
	else
	{
		TRX_setFrequency(setFreqSlowly_target, CurrentVFO);
		LCD_UpdateQuery.FreqInfo = true;
		setFreqSlowly_processing = false;
	}
}
