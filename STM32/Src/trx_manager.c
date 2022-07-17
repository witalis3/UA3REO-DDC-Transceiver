#include "hardware.h"
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
#include "cw.h"
#include "hardware.h"

volatile bool TRX_ptt_hard = false;
volatile bool TRX_ptt_soft = false;
volatile bool TRX_old_ptt_soft = false;
volatile bool TRX_RX1_IQ_swap = false;
volatile bool TRX_RX2_IQ_swap = false;
volatile bool TRX_TX_IQ_swap = false;
volatile bool TRX_Tune = false;
volatile bool TRX_Inited = false;
volatile float32_t TRX_RX1_dBm = -100.0f;
volatile float32_t TRX_RX2_dBm = -100.0f;
volatile bool TRX_ADC_OTR = false;
volatile bool TRX_DAC_OTR = false;
volatile bool TRX_MIC_BELOW_NOISEGATE = false;
volatile int16_t TRX_ADC_MINAMPLITUDE = 0;
volatile int16_t TRX_ADC_MAXAMPLITUDE = 0;
volatile int32_t TRX_VCXO_ERROR = 0;
volatile uint32_t TRX_SNTP_Synced = 0; // time of the last time synchronization
volatile int_fast16_t TRX_RIT = 0;
volatile int_fast16_t TRX_XIT = 0;
volatile float32_t TRX_MAX_TX_Amplitude = 1.0f;
volatile float32_t TRX_PWR_Forward = 0;
volatile float32_t TRX_PWR_Backward = 0;
volatile float32_t TRX_SWR = 0;
volatile float32_t TRX_PWR_Forward_SMOOTHED = 0;
volatile float32_t TRX_PWR_Backward_SMOOTHED = 0;
volatile float32_t TRX_SWR_SMOOTHED = 0;
char TRX_SWR_SMOOTHED_STR[8] = "1.0";
volatile float32_t TRX_VLT_forward = 0;	 // Tisho
volatile float32_t TRX_VLT_backward = 0; // Tisho
volatile float32_t TRX_ALC_OUT = 0;
volatile float32_t TRX_ALC_IN = 0;
volatile bool TRX_SWR_PROTECTOR = false;
volatile bool TRX_DAC_DIV0 = false;
volatile bool TRX_DAC_DIV1 = false;
volatile bool TRX_DAC_HP1 = false;
volatile bool TRX_DAC_HP2 = false;
volatile bool TRX_DAC_X4 = false;
volatile bool TRX_DCDC_Freq = false;
volatile bool TRX_DAC_DRV_A0 = true;
volatile bool TRX_DAC_DRV_A1 = true;
volatile float32_t TRX_STM32_VREF = 3.3f;		  // voltage on STM32
volatile float32_t TRX_STM32_TEMPERATURE = 30.0f; // STM32 temperature
volatile float32_t TRX_IQ_phase_error = 0.0f;
volatile bool TRX_Temporary_Stop_BandMap = false;
volatile bool TRX_Mute = false;
volatile bool TRX_AFAmp_Mute = false;
volatile uint32_t TRX_Temporary_Mute_StartTime = 0;
uint32_t TRX_freq_phrase = 0;
uint32_t TRX_freq_phrase2 = 0;
uint32_t TRX_freq_phrase_tx = 0;
volatile float32_t TRX_RF_Temperature = 0.0f;
volatile bool TRX_ScanMode = false;
bool TRX_phase_restarted = false;
uint32_t TRX_TX_StartTime = 0;
uint32_t TRX_TX_EndTime = 0;
uint32_t TRX_Inactive_Time = 0;
uint32_t TRX_DXCluster_UpdateTime = 0;
volatile float32_t TRX_PWR_Voltage = 12.0f;
volatile float32_t TRX_PWR_Current = 0.0f;
volatile float32_t TRX_RF_Current = 0.0f;
volatile float32_t TRX_VBAT_Voltage = 0.0f;
volatile uint_fast16_t CW_Key_Timeout_est = 0;
uint32_t dbg_FPGA_samples = 0;

static uint_fast8_t TRX_TXRXMode = 0; // 0 - undef, 1 - rx, 2 - tx, 3 - txrx
static bool TRX_SPLIT_Applied = false;
static bool TRX_ANT_swap_applyed = false;
static void TRX_Start_RX(void);
static void TRX_Start_TX(void);
static void TRX_Start_TXRX(void);

void TRX_Init()
{
	TRX_Start_RX();
	WM8731_TXRX_mode();
	WM8731_start_i2s_and_dma();
	uint_fast8_t saved_mode = CurrentVFO->Mode;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	TRX_setMode(saved_mode, CurrentVFO);
	HRDW_Init();
}

void TRX_Restart_Mode()
{
	ADCDAC_OVR_StatusLatency = 0;
	uint_fast8_t mode = CurrentVFO->Mode;

	// Switch mode
	if (TRX_on_TX)
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

	// SPLIT
	if (TRX.SPLIT_Enabled && !TRX_SPLIT_Applied)
	{
		TRX_SPLIT_Applied = true;

		TRX.selected_vfo = !TRX.selected_vfo;
		if (!TRX.selected_vfo)
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
		TRX.ANT_selected = TRX.BANDS_SAVED_SETTINGS[band].ANT_selected;

		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.TopButtons = true;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}
	
	//Ant swap for mode 1RX/2TX and others
	if(TRX.ANT_mode && !TRX_ANT_swap_applyed) {
		TRX_ANT_swap_applyed = true;
		TRX.ANT_selected = !TRX.ANT_selected;
	}

	if (TRX.XIT_Enabled)
		LCD_UpdateQuery.FreqInfoRedraw = true;
}

static void TRX_Start_RX()
{
	if (TRX_TXRXMode == 1)
		return;
	println("RX MODE");
	TRX_phase_restarted = false;
	RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	Processor_NeedRXBuffer = false;
	WM8731_Buffer_underrun = false;
	WM8731_DMA_state = true;
	TRX_SPLIT_Applied = false;
	TRX_ANT_swap_applyed = false;
	TRX_TXRXMode = 1;

	// clean TX buffer
	dma_memset((void *)&FPGA_Audio_SendBuffer_Q[0], 0x00, sizeof(FPGA_Audio_SendBuffer_Q));
	dma_memset((void *)&FPGA_Audio_SendBuffer_I[0], 0x00, sizeof(FPGA_Audio_SendBuffer_I));

	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	NeedReinitReverber = true;
	NeedFFTReinit = true;
	
	//FPGA_NeedRestart_TX = true;
}

static void TRX_Start_TX()
{
	if (TRX_TXRXMode == 2)
		return;
	println("TX MODE");
	RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	TRX_TX_StartTime = HAL_GetTick();
	TRX_SPLIT_Applied = false;
	TRX_ANT_swap_applyed = false;
	TRX_TXRXMode = 2;

	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	NeedReinitReverber = true;
	NeedFFTReinit = true;
}

static void TRX_Start_TXRX()
{
	if (TRX_TXRXMode == 3)
		return;
	println("TXRX MODE");
	RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	TRX_TX_StartTime = HAL_GetTick();
	TRX_SPLIT_Applied = false;
	TRX_ANT_swap_applyed = false;
	TRX_TXRXMode = 3;

	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	NeedReinitReverber = true;
	NeedFFTReinit = true;
	
	//FPGA_NeedRestart_TX = true;
}

void TRX_ptt_change(void)
{
	TRX_Inactive_Time = 0;
	if (TRX_Tune)
		TRX_Tune = false;

	bool notx = TRX_TX_Disabled(CurrentVFO->Freq);
	if (notx)
	{
		TRX_ptt_soft = false;
		TRX_ptt_hard = false;
		return;
	}

	if (CurrentVFO->Mode == TRX_MODE_CW && TRX.CW_PTT_Type == KEY_PTT) // cw ptt type
	{
		TRX_ptt_soft = false;
		CW_key_serial = false;
		return;
	}

	bool TRX_new_ptt_hard = !HAL_GPIO_ReadPin(PTT_IN_GPIO_Port, PTT_IN_Pin);
	if (TRX_ptt_hard != TRX_new_ptt_hard)
	{
		if (TRX.Auto_Input_Switch)
		{
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY || CurrentVFO->Mode == TRX_MODE_IQ)
				TRX.InputType_DIGI = TRX_INPUT_MIC;
			else
				TRX.InputType_MAIN = TRX_INPUT_MIC;
		}

		TRX_SWR_PROTECTOR = false;
		TRX_ptt_hard = TRX_new_ptt_hard;
		TRX_ptt_soft = false;
		CW_key_serial = false;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
	if (TRX_ptt_soft != TRX_old_ptt_soft)
	{
		if (TRX.Auto_Input_Switch)
		{
			if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY || CurrentVFO->Mode == TRX_MODE_IQ)
				TRX.InputType_DIGI = TRX_INPUT_USB;
			else
				TRX.InputType_MAIN = TRX_INPUT_USB;
		}

		TRX_SWR_PROTECTOR = false;
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
	switch (band)
	{
	case BANDID_2200m:
		if (CALIBRATE.NOTX_2200m)
			notx = true;
		break;
	case BANDID_160m:
		if (CALIBRATE.NOTX_160m)
			notx = true;
		break;
	case BANDID_80m:
		if (CALIBRATE.NOTX_80m)
			notx = true;
		break;
	case BANDID_60m:
		if (CALIBRATE.NOTX_60m)
			notx = true;
		break;
	case BANDID_40m:
		if (CALIBRATE.NOTX_40m)
			notx = true;
		break;
	case BANDID_30m:
		if (CALIBRATE.NOTX_30m)
			notx = true;
		break;
	case BANDID_20m:
		if (CALIBRATE.NOTX_20m)
			notx = true;
		break;
	case BANDID_17m:
		if (CALIBRATE.NOTX_17m)
			notx = true;
		break;
	case BANDID_15m:
		if (CALIBRATE.NOTX_15m)
			notx = true;
		break;
	case BANDID_12m:
		if (CALIBRATE.NOTX_12m)
			notx = true;
		break;
	case BANDID_CB:
		if (CALIBRATE.NOTX_CB)
			notx = true;
		break;
	case BANDID_10m:
		if (CALIBRATE.NOTX_10m)
			notx = true;
		break;
	case BANDID_6m:
		if (CALIBRATE.NOTX_6m)
			notx = true;
		break;
	case BANDID_4m:
		notx = false;
		break;
	case BANDID_2m:
	case BANDID_Marine:
		if (CALIBRATE.NOTX_2m)
			notx = true;
		break;
	case BANDID_70cm:
		if (CALIBRATE.NOTX_70cm)
			notx = true;
		break;
	default:
		if (CALIBRATE.NOTX_NOTHAM)
			notx = true;
		break;
	}
	return notx;
}

void TRX_setFrequency(uint64_t _freq, VFO *vfo)
{
	if (_freq < 1)
		return;

	bool transverter_enabled = false;
	if (TRX.Transverter_23cm || TRX.Transverter_13cm || TRX.Transverter_6cm || TRX.Transverter_3cm)
		transverter_enabled = true;

	if (!transverter_enabled && _freq >= MAX_RX_FREQ_HZ)
		_freq = MAX_RX_FREQ_HZ;

	int64_t freq_diff = _freq - vfo->Freq;
	vfo->Freq = _freq;

	// set DC-DC Sync freq
	uint32_t dcdc_offset_0 = abs((int32_t)DCDC_FREQ_0 / 2 - (int32_t)_freq % (int32_t)DCDC_FREQ_0);
	uint32_t dcdc_offset_1 = abs((int32_t)DCDC_FREQ_1 / 2 - (int32_t)_freq % (int32_t)DCDC_FREQ_1);
	if (dcdc_offset_0 > dcdc_offset_1)
		TRX_DCDC_Freq = 1;
	else
		TRX_DCDC_Freq = 0;

	// get settings and fpga freq phrase
	int64_t vfoa_freq = CurrentVFO->Freq + TRX_RIT;
	if (TRX.Transverter_70cm && getBandFromFreq(vfoa_freq, true) == BANDID_70cm)
		vfoa_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfoa_freq - BANDS[BANDID_70cm].startFreq);
	if (TRX.Transverter_23cm && getBandFromFreq(vfoa_freq, true) == BANDID_23cm)
		vfoa_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfoa_freq - BANDS[BANDID_23cm].startFreq);
	if (TRX.Transverter_13cm && getBandFromFreq(vfoa_freq, true) == BANDID_13cm)
		vfoa_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfoa_freq - BANDS[BANDID_13cm].startFreq);
	if (TRX.Transverter_6cm && getBandFromFreq(vfoa_freq, true) == BANDID_6cm)
		vfoa_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfoa_freq - BANDS[BANDID_6cm].startFreq);
	if (TRX.Transverter_3cm && getBandFromFreq(vfoa_freq, true) == BANDID_3cm)
		vfoa_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfoa_freq - BANDS[BANDID_3cm].startFreq);
	TRX_freq_phrase = getRXPhraseFromFrequency(vfoa_freq, 1);

	int64_t vfob_freq = SecondaryVFO->Freq + TRX_RIT;
	if (TRX.Transverter_70cm && getBandFromFreq(vfob_freq, true) == BANDID_70cm)
		vfob_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfob_freq - BANDS[BANDID_70cm].startFreq);
	if (TRX.Transverter_23cm && getBandFromFreq(vfob_freq, true) == BANDID_23cm)
		vfob_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfob_freq - BANDS[BANDID_23cm].startFreq);
	if (TRX.Transverter_13cm && getBandFromFreq(vfob_freq, true) == BANDID_13cm)
		vfob_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfob_freq - BANDS[BANDID_13cm].startFreq);
	if (TRX.Transverter_6cm && getBandFromFreq(vfob_freq, true) == BANDID_6cm)
		vfob_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfob_freq - BANDS[BANDID_6cm].startFreq);
	if (TRX.Transverter_3cm && getBandFromFreq(vfob_freq, true) == BANDID_3cm)
		vfob_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfob_freq - BANDS[BANDID_3cm].startFreq);

	TRX_freq_phrase2 = getRXPhraseFromFrequency(vfob_freq, 2);

	int64_t vfo_tx_freq = CurrentVFO->Freq + TRX_XIT;
	if (TRX.Transverter_70cm && getBandFromFreq(vfo_tx_freq, true) == BANDID_70cm)
		vfo_tx_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfo_tx_freq - BANDS[BANDID_70cm].startFreq);
	if (TRX.Transverter_23cm && getBandFromFreq(vfo_tx_freq, true) == BANDID_23cm)
		vfo_tx_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfo_tx_freq - BANDS[BANDID_23cm].startFreq);
	if (TRX.Transverter_13cm && getBandFromFreq(vfo_tx_freq, true) == BANDID_13cm)
		vfo_tx_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfo_tx_freq - BANDS[BANDID_13cm].startFreq);
	if (TRX.Transverter_6cm && getBandFromFreq(vfo_tx_freq, true) == BANDID_6cm)
		vfo_tx_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfo_tx_freq - BANDS[BANDID_6cm].startFreq);
	if (TRX.Transverter_3cm && getBandFromFreq(vfo_tx_freq, true) == BANDID_3cm)
		vfo_tx_freq = (TRX.Transverter_Offset_Mhz * 1000000) + (vfo_tx_freq - BANDS[BANDID_3cm].startFreq);
	TRX_freq_phrase_tx = getTXPhraseFromFrequency(vfo_tx_freq);
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(vfo_tx_freq);

	FPGA_NeedSendParams = true;

	// services
	if (SYSMENU_spectrum_opened || SYSMENU_swr_opened)
		return;

	// get band
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

	// SPLIT freq secondary VFO sync
	if (TRX.SPLIT_Enabled && vfo == CurrentVFO)
	{
		TRX_setFrequency(SecondaryVFO->Freq + freq_diff, SecondaryVFO);
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
		vfo->HPF_RX_Filter_Width = 0;
		vfo->HPF_TX_Filter_Width = 0;
		break;
	case TRX_MODE_LSB:
	case TRX_MODE_USB:
		vfo->LPF_RX_Filter_Width = TRX.SSB_LPF_RX_Filter;
		vfo->LPF_TX_Filter_Width = TRX.SSB_LPF_TX_Filter;
		vfo->HPF_RX_Filter_Width = TRX.SSB_HPF_RX_Filter;
		vfo->HPF_TX_Filter_Width = TRX.SSB_HPF_TX_Filter;
		break;
	case TRX_MODE_DIGI_L:
	case TRX_MODE_DIGI_U:
	case TRX_MODE_RTTY:
		vfo->LPF_RX_Filter_Width = TRX.DIGI_LPF_Filter;
		vfo->LPF_TX_Filter_Width = TRX.DIGI_LPF_Filter;
		vfo->HPF_RX_Filter_Width = 0;
		vfo->HPF_TX_Filter_Width = 0;
		break;
	case TRX_MODE_CW:
		vfo->LPF_RX_Filter_Width = TRX.CW_LPF_Filter;
		vfo->LPF_TX_Filter_Width = TRX.CW_LPF_Filter;
		vfo->HPF_RX_Filter_Width = 0;
		vfo->HPF_TX_Filter_Width = 0;
		LCD_UpdateQuery.TextBar = true;
		break;
	case TRX_MODE_NFM:
		vfo->LPF_RX_Filter_Width = TRX.FM_LPF_RX_Filter;
		vfo->LPF_TX_Filter_Width = TRX.FM_LPF_TX_Filter;
		vfo->HPF_RX_Filter_Width = 0;
		vfo->HPF_TX_Filter_Width = 0;
		break;
	case TRX_MODE_WFM:
		vfo->LPF_RX_Filter_Width = 16000;
		vfo->LPF_TX_Filter_Width = 16000;
		vfo->HPF_RX_Filter_Width = 0;
		vfo->HPF_TX_Filter_Width = 0;
		break;
	}

	// FFT Zoom change
	if (TRX.FFT_Zoom != TRX.FFT_ZoomCW)
	{
		if (old_mode == TRX_MODE_CW && _mode != TRX_MODE_CW)
			NeedFFTReinit = true;
		if (old_mode != TRX_MODE_CW && _mode == TRX_MODE_CW)
			NeedFFTReinit = true;
	}

	// FM Samplerate change
	if (TRX.SAMPLERATE_MAIN != TRX.SAMPLERATE_FM && old_mode != TRX_MODE_WFM && old_mode != TRX_MODE_NFM && (_mode == TRX_MODE_WFM || _mode == TRX_MODE_NFM))
		NeedFFTReinit = true;
	if (TRX.SAMPLERATE_MAIN != TRX.SAMPLERATE_FM && (old_mode == TRX_MODE_WFM || old_mode == TRX_MODE_NFM) && _mode != TRX_MODE_WFM && _mode != TRX_MODE_NFM)
		NeedFFTReinit = true;

	if (old_mode != _mode) {
		TRX_TemporaryMute();
		NeedReinitAudioFiltersClean = true;
	}
	
	WM8731_TXRX_mode();
	
	NeedReinitNotch = true;
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

	// Process AutoGain feature
	if (TRX.AutoGain && !TRX_on_TX)
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
			// save settings
			int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
			TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
			TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
			TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
		}
	}
}

void TRX_TemporaryMute(void)
{
	WM8731_Mute();
	TRX_Temporary_Mute_StartTime = HAL_GetTick();
}

// process frequency scanner
void TRX_ProcessScanMode(void)
{
	static bool oldState = false;
	static uint32_t StateChangeTime = 0;
	bool goSweep = false;

	if (CurrentVFO->Mode == TRX_MODE_WFM || CurrentVFO->Mode == TRX_MODE_NFM)
	{
		if (oldState != DFM_RX1.squelched)
		{
			oldState = DFM_RX1.squelched;
			StateChangeTime = HAL_GetTick();
		}

		if (DFM_RX1.squelched && ((HAL_GetTick() - StateChangeTime) > SCANNER_NOSIGNAL_TIME))
			goSweep = true;
		if (!DFM_RX1.squelched && ((HAL_GetTick() - StateChangeTime) > SCANNER_SIGNAL_TIME_FM))
			goSweep = true;
	}
	else
	{
		if (oldState != VAD_RX1_Muting)
		{
			oldState = VAD_RX1_Muting;
			StateChangeTime = HAL_GetTick();
		}

		if (VAD_RX1_Muting && ((HAL_GetTick() - StateChangeTime) > SCANNER_NOSIGNAL_TIME))
			goSweep = true;
		if (!VAD_RX1_Muting && ((HAL_GetTick() - StateChangeTime) > SCANNER_SIGNAL_TIME_OTHER))
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
