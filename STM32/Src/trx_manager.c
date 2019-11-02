#include "stm32f4xx_hal.h"
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
volatile bool TRX_key_hard = false;
volatile uint16_t TRX_Key_Timeout_est = 0;
volatile bool TRX_IQ_swap = false;
volatile bool TRX_Squelched = false;
volatile bool TRX_Tune = false;
volatile bool TRX_Inited = false;
volatile int16_t TRX_RX_dBm = -100;
volatile bool TRX_ADC_OTR = false;
volatile bool TRX_DAC_OTR = false;
volatile uint8_t TRX_Time_InActive = 0; //секунд бездействия, используется для спящего режима
volatile uint8_t TRX_Fan_Timeout = 0; //секунд, сколько ещё осталось крутить вентилятор
volatile int16_t TRX_ADC_MINAMPLITUDE = 0;
volatile int16_t TRX_ADC_MAXAMPLITUDE = 0;
volatile bool TRX_SNMP_Synced = false;
volatile int16_t TRX_SHIFT = 0;

static uint8_t autogain_wait_reaction = 0; //таймер ожидания реакции от смены режимов ATT/PRE
static uint8_t autogain_stage = 0; //этап отработки актокорректировщика усиления

const char *MODE_DESCR[TRX_MODE_COUNT] = {
	"LSB",
	"USB",
	"IQ",
	"CW_L",
	"CW_U",
	"DIGL",
	"DIGU",
	"NOTX",
	"NFM",
	"WFM",
	"AM",
	"LOOP"
};

static void TRX_Start_RX(void);
static void TRX_Start_TX(void);
static void TRX_Start_TXRX(void);

bool TRX_on_TX(void)
{
	if (TRX_ptt_hard || TRX_ptt_cat || TRX_Tune || TRX_getMode(CurrentVFO()) == TRX_MODE_LOOPBACK || TRX_Key_Timeout_est > 0) return true;
	return false;
}

void TRX_Init()
{
	CWDecoder_Init();
	TRX_Start_RX();
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

void TRX_Restart_Mode()
{
	uint8_t mode = TRX_getMode(CurrentVFO());
	if (TRX_on_TX())
	{
		if (mode == TRX_MODE_LOOPBACK || mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U)
			TRX_Start_TXRX();
		else
		{
			if(TRX.CLAR)
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
		if(TRX.CLAR)
		{
			TRX.current_vfo = !TRX.current_vfo;
			LCD_UpdateQuery.FreqInfo = true;
			LCD_UpdateQuery.TopButtons = true;
		}
		TRX_Start_RX();
	}
}

static void TRX_Start_RX()
{
	sendToDebug_str("RX MODE\r\n");
	PERIPH_RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	Processor_NeedRXBuffer = true;
	WM8731_Buffer_underrun = false;
	WM8731_DMA_state = true;
	WM8731_RX_mode();
	WM8731_start_i2s_and_dma();
}

static void TRX_Start_TX()
{
	sendToDebug_str("TX MODE\r\n");
	PERIPH_RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	HAL_Delay(10); //задерка перед подачей ВЧ сигнала, чтобы успели сработать реле
	WM8731_TX_mode();
	WM8731_start_i2s_and_dma();
}

static void TRX_Start_TXRX()
{
	sendToDebug_str("LOOP MODE\r\n");
	PERIPH_RF_UNIT_UpdateState(false);
	WM8731_CleanBuffer();
	WM8731_TXRX_mode();
	WM8731_start_i2s_and_dma();
}

void TRX_ptt_change(void)
{
	if (TRX_Tune) return;
	bool TRX_new_ptt_hard = !HAL_GPIO_ReadPin(PTT_IN_GPIO_Port, PTT_IN_Pin);
	if (TRX_ptt_hard != TRX_new_ptt_hard)
	{
		TRX_Time_InActive = 0;
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
	if (TRX_Tune) return;
	bool TRX_new_ptt_hard = !HAL_GPIO_ReadPin(KEY_IN_DOT_GPIO_Port, KEY_IN_DOT_Pin);
	if (TRX_key_hard != TRX_new_ptt_hard)
	{
		TRX_Time_InActive = 0;
		TRX_key_hard = TRX_new_ptt_hard;
		if (TRX_key_hard == true) TRX_Key_Timeout_est = TRX.Key_timeout;
		if (TRX.Key_timeout == 0) TRX_ptt_cat = TRX_key_hard;
		LCD_UpdateQuery.StatusInfoGUI = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
	if (TRX_key_serial != TRX_old_key_serial)
	{
		TRX_Time_InActive = 0;
		TRX_old_key_serial = TRX_key_serial;
		if (TRX_key_serial == true) TRX_Key_Timeout_est = TRX.Key_timeout;
		if (TRX.Key_timeout == 0) TRX_ptt_cat = TRX_key_serial;
		LCD_UpdateQuery.StatusInfoGUI = true;
		FPGA_NeedSendParams = true;
		TRX_Restart_Mode();
	}
}

void TRX_setFrequency(int32_t _freq, VFO* vfo)
{
	if (_freq < 1) return;
	if (_freq >= MAX_FREQ_HZ) _freq = MAX_FREQ_HZ;

	vfo->Freq = _freq;
	if (getBandFromFreq(_freq) >= 0) TRX.saved_freq[getBandFromFreq(_freq)] = _freq;
	if (TRX.BandMapEnabled)
	{
		uint8_t mode_from_bandmap = getModeFromFreq(vfo->Freq);
		if (TRX_getMode(vfo) != mode_from_bandmap)
		{
			TRX_setMode(mode_from_bandmap, vfo);
			LCD_UpdateQuery.TopButtons = true;
		}
	}
	FPGA_NeedSendParams = true;
	NeedFFTInputBuffer = true;
}

uint32_t TRX_getFrequency(VFO* vfo)
{
	return vfo->Freq;
}

void TRX_setMode(uint8_t _mode, VFO* vfo)
{
	vfo->Mode = _mode;
	if (vfo->Mode == TRX_MODE_LOOPBACK) TRX_Start_TXRX();
	switch (_mode)
	{
	case TRX_MODE_LSB:
	case TRX_MODE_USB:
	case TRX_MODE_DIGI_L:
	case TRX_MODE_DIGI_U:
	case TRX_MODE_AM:
		vfo->Filter_Width = TRX.SSB_Filter;
		break;
	case TRX_MODE_CW_L:
	case TRX_MODE_CW_U:
		vfo->Filter_Width = TRX.CW_Filter;
		break;
	case TRX_MODE_NFM:
		vfo->Filter_Width = TRX.FM_Filter;
		break;
	case TRX_MODE_WFM:
		vfo->Filter_Width = 0;
		break;
	}
	ReinitAudioFilters();
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

uint8_t TRX_getMode(VFO* vfo)
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
			TRX.Preamp = false;
			TRX.ATT = true;
			LCD_UpdateQuery.TopButtons = true;
			autogain_stage++;
			autogain_wait_reaction = 0;
			//sendToDebug_str("AUTOGAIN LPF+BPF+ATT\r\n");
			break;
		case 1: //сменили состояние, обрабатываем результаты
			if ((TRX_ADC_MAXAMPLITUDE*db2rateV(ATT_DB)) <= AUTOGAIN_MAX_AMPLITUDE) //если можем выключить АТТ - переходим на следующий этап (+12dB)
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
			TRX.Preamp = false;
			TRX.ATT = false;
			LCD_UpdateQuery.TopButtons = true;
			autogain_stage++;
			autogain_wait_reaction = 0;
			//sendToDebug_str("AUTOGAIN LPF+BPF\r\n");
			break;
		case 3: //сменили состояние, обрабатываем результаты
			if (TRX_ADC_MAXAMPLITUDE > AUTOGAIN_MAX_AMPLITUDE) autogain_stage -= 3; //слишком большое усиление, возвращаемся на этап назад
			if ((TRX_ADC_MAXAMPLITUDE*db2rateV(PREAMP_GAIN_DB) / db2rateV(ATT_DB)) <= AUTOGAIN_MAX_AMPLITUDE) //если можем включить АТТ+PREAMP - переходим на следующий этап (+20dB-12dB)
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
			TRX.Preamp = true;
			TRX.ATT = true;
			LCD_UpdateQuery.TopButtons = true;
			autogain_stage++;
			autogain_wait_reaction = 0;
			//sendToDebug_str("AUTOGAIN LPF+BPF+PREAMP+ATT\r\n");
			break;
		case 5: //сменили состояние, обрабатываем результаты
			if (TRX_ADC_MAXAMPLITUDE > AUTOGAIN_MAX_AMPLITUDE) autogain_stage -= 3; //слишком большое усиление, возвращаемся на этап назад
			if ((TRX_ADC_MAXAMPLITUDE*db2rateV(ATT_DB)) <= AUTOGAIN_MAX_AMPLITUDE) //если можем выключить АТТ - переходим на следующий этап (+12dB)
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
			TRX.Preamp = true;
			TRX.ATT = false;
			LCD_UpdateQuery.TopButtons = true;
			autogain_stage++;
			autogain_wait_reaction = 0;
			//sendToDebug_str("AUTOGAIN LPF+BPF+PREAMP\r\n");
			break;
		case 7: //сменили состояние, обрабатываем результаты
			if (TRX_ADC_MAXAMPLITUDE > AUTOGAIN_MAX_AMPLITUDE) autogain_stage -= 3; //слишком большое усиление, возвращаемся на этап назад
			break;
		default:
			autogain_stage = 0;
			break;
		}
	}
}

void TRX_DBMCalculate(void)
{
	float32_t Audio_Vpp_value = (Processor_RX_Audio_Samples_MAX_value / (float32_t)TRX.RF_Gain) - (Processor_RX_Audio_Samples_MIN_value / (float32_t)TRX.RF_Gain); //получаем разницу между максимальным и минимальным значением в аудио-семплах
	for (int i = 0; i < (FPGA_BUS_BITS - ADC_BITS); i++) Audio_Vpp_value = Audio_Vpp_value / 2.0f; //приводим разрядность аудио к разрядности АЦП
	float32_t ADC_Vpp_Value = Audio_Vpp_value * ADC_VREF / ((float32_t)powf(2.0f, ADC_BITS) - 1.0f); //получаем значение пик-пик напряжения на входе АЦП в вольтах
	float32_t ADC_Vrms_Value = ADC_Vpp_Value * 0.3535f; // Получаем действующее (RMS) напряжение на аходе АЦП
	float32_t ADC_RF_IN_Value = (ADC_Vrms_Value / ADC_RF_TRANS_RATIO)*ADC_RF_INPUT_VALUE_CALIBRATION; //Получаем напряжение на антенном входе с учётом трансформатора и калибровки
	if (ADC_RF_IN_Value < 0.0000001f) ADC_RF_IN_Value = 0.0000001f;
	TRX_RX_dBm = 10 * log10f_fast((ADC_RF_IN_Value*ADC_RF_IN_Value) / (50.0f*0.001f)); //получаем значение мощности в dBm для сопротивления 50ом
	Processor_RX_Audio_Samples_MAX_value = 0;
	Processor_RX_Audio_Samples_MIN_value = 0;
}
