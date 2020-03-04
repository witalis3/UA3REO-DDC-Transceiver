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
#include "audio_filters.h"
#include "usbd_audio_if.h"
#include "cw_decoder.h"
#include "peripheral.h"

volatile bool TRX_ptt_hard = false;
volatile bool TRX_ptt_cat = false;
volatile bool TRX_old_ptt_cat = false;
volatile bool TRX_key_serial = false;
volatile bool TRX_old_key_serial = false;
volatile bool TRX_key_dot_hard = false;
volatile bool TRX_key_dash_hard = false;
volatile uint_fast16_t TRX_Key_Timeout_est = 0;
volatile bool TRX_IQ_swap = false;
volatile bool TRX_Squelched = false;
volatile bool TRX_Tune = false;
volatile bool TRX_Inited = false;
volatile int_fast16_t TRX_RX_dBm = -100;
volatile bool TRX_ADC_OTR = false;
volatile bool TRX_DAC_OTR = false;
volatile uint_fast8_t TRX_Time_InActive = 0; //секунд бездействия, используется для спящего режима
volatile uint_fast8_t TRX_Fan_Timeout = 0;   //секунд, сколько ещё осталось крутить вентилятор
volatile int16_t TRX_ADC_MINAMPLITUDE = 0;
volatile int16_t TRX_ADC_MAXAMPLITUDE = 0;
volatile bool TRX_SNMP_Synced = false;
volatile int_fast16_t TRX_SHIFT = 0;
volatile float32_t TRX_MAX_TX_Amplitude = MAX_TX_AMPLITUDE;
volatile float32_t TRX_SWR_forward = 0;
volatile float32_t TRX_SWR_backward = 0;
volatile float32_t TRX_SWR = 0;
static uint_fast8_t autogain_wait_reaction = 0;   //таймер ожидания реакции от смены режимов ATT/PRE
static uint_fast8_t autogain_stage = 0;			 //этап отработки актокорректировщика усиления
static uint32_t KEYER_symbol_start_time = 0; //время старта символа автоматического ключа
static bool KEYER_symbol_status = false;	 //статус (сигнал или период) символа автоматического ключа

static uint_fast8_t TRX_TXRXMode = 0; //0 - undef, 1 - rx, 2 - tx, 3 - txrx
static void TRX_Start_RX(void);
static void TRX_Start_TX(void);
static void TRX_Start_TXRX(void);

bool TRX_on_TX(void)
{
	if (TRX_ptt_hard || TRX_ptt_cat || TRX_Tune || TRX_getMode(CurrentVFO()) == TRX_MODE_LOOPBACK || TRX_Key_Timeout_est > 0)
		return true;
	return false;
}

void TRX_Init()
{
	CWDecoder_Init();
	TRX_Start_RX();
	uint_fast8_t saved_mode = CurrentVFO()->Mode;
	TRX_setFrequency(CurrentVFO()->Freq, CurrentVFO());
	TRX_setMode(saved_mode, CurrentVFO());
	sendToDebug_strln("[OK] TRX inited");
}

void TRX_Restart_Mode()
{
	uint_fast8_t mode = TRX_getMode(CurrentVFO());
	if (TRX_on_TX())
	{
		if (mode == TRX_MODE_LOOPBACK || mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U)
			TRX_Start_TXRX();
		else
		{
			if (TRX.CLAR)
			{
				TRX.current_vfo = !TRX.current_vfo;
				LCD_UpdateQuery.FreqInfo = true;
				LCD_UpdateQuery.TopButtons = true;
			}
			TRX_Start_TX();
		}
	}
	else
	{
		if (TRX.CLAR)
		{
			TRX.current_vfo = !TRX.current_vfo;
			LCD_UpdateQuery.FreqInfo = true;
			LCD_UpdateQuery.TopButtons = true;
		}
		TRX_Start_RX();
	}
	FFT_Reset();
}

static void TRX_Start_RX()
{
	if (TRX_TXRXMode == 1)
		return;
	//sendToDebug_str("RX MODE\r\n");
	PERIPH_RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	Processor_NeedRXBuffer = true;
	WM8731_Buffer_underrun = false;
	WM8731_DMA_state = true;
	WM8731_RX_mode();
	WM8731_start_i2s_and_dma();
	TRX_TXRXMode = 1;
}

static void TRX_Start_TX()
{
	if (TRX_TXRXMode == 2)
		return;
	//sendToDebug_str("TX MODE\r\n");
	PERIPH_RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	HAL_Delay(10); //задерка перед подачей ВЧ сигнала, чтобы успели сработать реле
	WM8731_TX_mode();
	WM8731_start_i2s_and_dma();
	TRX_TXRXMode = 2;
}

static void TRX_Start_TXRX()
{
	if (TRX_TXRXMode == 3)
		return;
	//sendToDebug_str("TXRX MODE\r\n");
	PERIPH_RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	WM8731_TXRX_mode();
	WM8731_start_i2s_and_dma();
	TRX_TXRXMode = 3;
}

void TRX_ptt_change(void)
{
	if (TRX_Tune)
		return;
	bool TRX_new_ptt_hard = !HAL_GPIO_ReadPin(PTT_IN_GPIO_Port, PTT_IN_Pin);
	if (TRX_ptt_hard != TRX_new_ptt_hard)
	{
		TRX_ptt_hard = TRX_new_ptt_hard;
		TRX_ptt_cat = false;
		LCD_UpdateQuery.StatusInfoGUI = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
	if (TRX_ptt_cat != TRX_old_ptt_cat)
	{
		TRX_Time_InActive = 0;
		TRX_old_ptt_cat = TRX_ptt_cat;
		LCD_UpdateQuery.StatusInfoGUI = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
}

void TRX_key_change(void)
{
	if (TRX_Tune)
		return;
	if (CurrentVFO()->Mode != TRX_MODE_CW_L && CurrentVFO()->Mode != TRX_MODE_CW_U)
		return;
	bool TRX_new_key_dot_hard = !HAL_GPIO_ReadPin(KEY_IN_DOT_GPIO_Port, KEY_IN_DOT_Pin);
	if (TRX_key_dot_hard != TRX_new_key_dot_hard)
	{
		TRX_key_dot_hard = TRX_new_key_dot_hard;
		if (TRX_key_dot_hard == true)
			TRX_Key_Timeout_est = TRX.Key_timeout;
		if (TRX.Key_timeout == 0)
			TRX_ptt_cat = TRX_key_dot_hard;
		KEYER_symbol_start_time = 0;
		KEYER_symbol_status = false;
		LCD_UpdateQuery.StatusInfoGUI = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
	bool TRX_new_key_dash_hard = !HAL_GPIO_ReadPin(KEY_IN_DASH_GPIO_Port, KEY_IN_DASH_Pin);
	if (TRX_key_dash_hard != TRX_new_key_dash_hard)
	{
		TRX_key_dash_hard = TRX_new_key_dash_hard;
		if (TRX_key_dash_hard == true)
			TRX_Key_Timeout_est = TRX.Key_timeout;
		if (TRX.Key_timeout == 0)
			TRX_ptt_cat = TRX_key_dash_hard;
		KEYER_symbol_start_time = 0;
		KEYER_symbol_status = false;
		LCD_UpdateQuery.StatusInfoGUI = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
	if (TRX_key_serial != TRX_old_key_serial)
	{
		TRX_Time_InActive = 0;
		TRX_old_key_serial = TRX_key_serial;
		if (TRX_key_serial == true)
			TRX_Key_Timeout_est = TRX.Key_timeout;
		if (TRX.Key_timeout == 0)
			TRX_ptt_cat = TRX_key_serial;
		LCD_UpdateQuery.StatusInfoGUI = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
}

void TRX_setFrequency(int32_t _freq, VFO *vfo)
{
	if (_freq < 1)
		return;
	if (_freq >= MAX_FREQ_HZ)
		_freq = MAX_FREQ_HZ;

	vfo->Freq = _freq;
	int_fast8_t bandFromFreq = getBandFromFreq(_freq, false);
	if (bandFromFreq >= 0)
		TRX.TRX_Saved_freq[bandFromFreq] = _freq;
	if (TRX.BandMapEnabled)
	{
		uint_fast8_t mode_from_bandmap = getModeFromFreq(vfo->Freq);
		if (TRX_getMode(vfo) != mode_from_bandmap)
		{
			TRX_setMode(mode_from_bandmap, vfo);
			LCD_UpdateQuery.TopButtons = true;
		}
	}
	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(vfo->Freq);
	FPGA_NeedSendParams = true;
	//NeedFFTInputBuffer = true;
}

uint32_t TRX_getFrequency(VFO *vfo)
{
	return vfo->Freq;
}

void TRX_setMode(uint_fast8_t _mode, VFO *vfo)
{
	if (vfo->Mode == TRX_MODE_LOOPBACK || _mode == TRX_MODE_LOOPBACK)
		LCD_UpdateQuery.StatusInfoGUI = true;
	vfo->Mode = _mode;
	if (vfo->Mode == TRX_MODE_LOOPBACK)
		TRX_Start_TXRX();

	switch (_mode)
	{
	case TRX_MODE_AM:
		vfo->LPF_Filter_Width = TRX.AM_LPF_Filter;
		vfo->HPF_Filter_Width = 0;
		break;
	case TRX_MODE_LSB:
	case TRX_MODE_USB:
	case TRX_MODE_DIGI_L:
	case TRX_MODE_DIGI_U:
		vfo->LPF_Filter_Width = TRX.SSB_LPF_Filter;
		vfo->HPF_Filter_Width = TRX.SSB_HPF_Filter;
		break;
	case TRX_MODE_CW_L:
	case TRX_MODE_CW_U:
		vfo->LPF_Filter_Width = TRX.CW_LPF_Filter;
		vfo->HPF_Filter_Width = TRX.CW_HPF_Filter;
		break;
	case TRX_MODE_NFM:
		vfo->LPF_Filter_Width = TRX.FM_LPF_Filter;
		vfo->HPF_Filter_Width = 0;
		break;
	case TRX_MODE_WFM:
		vfo->LPF_Filter_Width = 0;
		vfo->HPF_Filter_Width = 0;
		break;
	}
	ReinitAudioFilters();
	NeedSaveSettings = true;
}

uint_fast8_t TRX_getMode(VFO *vfo)
{
	return vfo->Mode;
}

void TRX_DoAutoGain(void)
{
	//Process AutoGain feature
	if (TRX.AutoGain)
	{
		TRX.LPF = true;
		TRX.BPF = true;
		switch (autogain_stage)
		{
		case 0: //этап 1 - включаем ДПФ, ЛПФ, Аттенюатор, выключаем предусилитель (-12dB)
			TRX.LNA = false;
			TRX.ATT = true;
			LCD_UpdateQuery.TopButtons = true;
			autogain_stage++;
			autogain_wait_reaction = 0;
			//sendToDebug_str("AUTOGAIN LPF+BPF+ATT\r\n");
			break;
		case 1:																		 //сменили состояние, обрабатываем результаты
			if ((TRX_ADC_MAXAMPLITUDE * db2rateV(-CALIBRATE.att_db)) <= AUTOGAIN_MAX_AMPLITUDE) //если можем выключить АТТ - переходим на следующий этап (+12dB)
				autogain_wait_reaction++;
			else
				autogain_wait_reaction = 0;
			if (autogain_wait_reaction >= AUTOGAIN_CORRECTOR_WAITSTEP)
			{
				autogain_stage++;
				autogain_wait_reaction = 0;
			}
			break;
		case 2: //этап 1 - включаем ДПФ, ЛПФ, выключаем Аттенюатор, выключаем предусилитель (+0dB)
			TRX.LNA = false;
			TRX.ATT = false;
			LCD_UpdateQuery.TopButtons = true;
			autogain_stage++;
			autogain_wait_reaction = 0;
			//sendToDebug_str("AUTOGAIN LPF+BPF\r\n");
			break;
		case 3: //сменили состояние, обрабатываем результаты
			if (TRX_ADC_MAXAMPLITUDE > AUTOGAIN_MAX_AMPLITUDE)
				autogain_stage -= 3;																			//слишком большое усиление, возвращаемся на этап назад
			if ((TRX_ADC_MAXAMPLITUDE * db2rateV(CALIBRATE.lna_gain_db) / db2rateV(-CALIBRATE.att_db)) <= AUTOGAIN_MAX_AMPLITUDE) //если можем включить АТТ+PREAMP - переходим на следующий этап (+20dB-12dB)
				autogain_wait_reaction++;
			else
				autogain_wait_reaction = 0;
			if (autogain_wait_reaction >= AUTOGAIN_CORRECTOR_WAITSTEP)
			{
				autogain_stage++;
				autogain_wait_reaction = 0;
			}
			break;
		case 4: //этап 2 - включаем ДПФ, ЛПФ, Аттенюатор, Предусилитель (+8dB)
			TRX.LNA = true;
			TRX.ATT = true;
			LCD_UpdateQuery.TopButtons = true;
			autogain_stage++;
			autogain_wait_reaction = 0;
			//sendToDebug_str("AUTOGAIN LPF+BPF+PREAMP+ATT\r\n");
			break;
		case 5: //сменили состояние, обрабатываем результаты
			if (TRX_ADC_MAXAMPLITUDE > AUTOGAIN_MAX_AMPLITUDE)
				autogain_stage -= 3;												 //слишком большое усиление, возвращаемся на этап назад
			if ((TRX_ADC_MAXAMPLITUDE * db2rateV(-CALIBRATE.att_db)) <= AUTOGAIN_MAX_AMPLITUDE) //если можем выключить АТТ - переходим на следующий этап (+12dB)
				autogain_wait_reaction++;
			else
				autogain_wait_reaction = 0;
			if (autogain_wait_reaction >= AUTOGAIN_CORRECTOR_WAITSTEP)
			{
				autogain_stage++;
				autogain_wait_reaction = 0;
			}
			break;
		case 6: //этап 3 - включаем ДПФ, ЛПФ, Предусилитель, выключаем Аттенюатор (+20dB)
			TRX.LNA = true;
			TRX.ATT = false;
			LCD_UpdateQuery.TopButtons = true;
			autogain_stage++;
			autogain_wait_reaction = 0;
			//sendToDebug_str("AUTOGAIN LPF+BPF+PREAMP\r\n");
			break;
		case 7: //сменили состояние, обрабатываем результаты
			if (TRX_ADC_MAXAMPLITUDE > AUTOGAIN_MAX_AMPLITUDE)
				autogain_stage -= 3; //слишком большое усиление, возвращаемся на этап назад
			break;
		default:
			autogain_stage = 0;
			break;
		}
	}
}

void TRX_DBMCalculate(void)
{
	float32_t s_meter_power = Processor_RX_Power_value * CALIBRATE.smeter_calibration; // калибровка S-метра АЦП
	Processor_RX_Power_value = 0;
	float32_t s_meter_rate = s_meter_power / (float32_t)FPGA_BUS_FULL_SCALE_POW;
	float32_t ADC_Vrms_Value = ADC_VREF * s_meter_rate;				   //получаем значение пик-пик напряжения на входе АЦП в вольтах
	float32_t ADC_RF_IN_Value = (ADC_Vrms_Value / ADC_RF_TRANS_RATIO); //Получаем напряжение на антенном входе с учётом трансформатора
	if (ADC_RF_IN_Value < 0.0000001f)
		ADC_RF_IN_Value = 0.0000001f;
	TRX_RX_dBm = 10 * log10f_fast((ADC_RF_IN_Value * ADC_RF_IN_Value) / (50.0f * 0.001f)); //получаем значение мощности в dBm для сопротивления 50ом
}

float32_t TRX_GetALC(void)
{
	float32_t res = Processor_TX_MAX_amplitude_OUT / Processor_selected_RFpower_amplitude;
	if (res > 0.99f && res < 1.0f)
		res = 1.0f;
	if (res < 0.01f)
		res = 0.0f;
	return res;
}

float32_t TRX_GenerateCWSignal(float32_t power)
{
	if (!TRX.CW_KEYER)
		return power;

	uint32_t dot_length_ms = 1200 / TRX.CW_KEYER_WPM;
	uint32_t dash_length_ms = dot_length_ms * 3;
	uint32_t sim_space_length_ms = dot_length_ms;
	uint32_t curTime = HAL_GetTick();

	if (TRX_key_dot_hard && (KEYER_symbol_start_time + dot_length_ms) > curTime) //зажата точка
	{
		if (KEYER_symbol_status)
			return power;
		else
			return 0.0f;
	}
	else if (TRX_key_dash_hard && (KEYER_symbol_start_time + dash_length_ms) > curTime) //зажато тире
	{
		if (KEYER_symbol_status)
			return power;
		else
			return 0.0f;
	}
	else if ((KEYER_symbol_start_time + sim_space_length_ms) < curTime)
	{
		KEYER_symbol_start_time = curTime;
		KEYER_symbol_status = !KEYER_symbol_status;
	}
	return power;
}
