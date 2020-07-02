#include "stm32h7xx_hal.h"
#include "main.h"
#include "peripheral.h"
#include "lcd.h"
#include "trx_manager.h"
#include "agc.h"
#include "settings.h"
#include "system_menu.h"
#include "functions.h"
#include "audio_filters.h"

static bool PERIPH_SPI_busy = false;
volatile bool PERIPH_SPI_process = false;
static bool FRONTPanel_MCP3008_1_Enabled = true;
static bool FRONTPanel_MCP3008_2_Enabled = true;

static int32_t ENCODER_slowler = 0;
static uint32_t ENCODER_AValDeb = 0;
static uint32_t ENCODER2_AValDeb = 0;
static bool ENCODER2_SWLast = true;

volatile PERIPH_FrontPanel_Type PERIPH_FrontPanel = {0};

static void PERIPH_ENCODER_Rotated(int8_t direction);
static void PERIPH_ENCODER2_Rotated(int8_t direction);
static uint16_t PERIPH_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN);

void PERIPH_ENCODER_checkRotate(void)
{
	static uint8_t ENClastClkVal = 0;
	static bool ENCfirst = true;
	uint8_t ENCODER_DTVal = HAL_GPIO_ReadPin(ENC_DT_GPIO_Port, ENC_DT_Pin);
	uint8_t ENCODER_CLKVal = HAL_GPIO_ReadPin(ENC_CLK_GPIO_Port, ENC_CLK_Pin);

	if (ENCfirst)
	{
		ENClastClkVal = ENCODER_CLKVal;
		ENCfirst = false;
	}
	if ((HAL_GetTick() - ENCODER_AValDeb) < CALIBRATE.ENCODER_DEBOUNCE)
		return;

	if(ENClastClkVal != ENCODER_CLKVal)
	{
		if (!CALIBRATE.ENCODER_ON_FALLING || ENCODER_CLKVal == 0)
		{
			if (ENCODER_DTVal != ENCODER_CLKVal)
			{ // Если вывод A изменился первым - вращение по часовой стрелке
				ENCODER_slowler--;
				if (ENCODER_slowler <= -CALIBRATE.ENCODER_SLOW_RATE)
				{
					PERIPH_ENCODER_Rotated(CALIBRATE.ENCODER_INVERT ? 1 : -1);
					ENCODER_slowler = 0;
				}
			}
			else
			{ // иначе B изменил свое состояние первым - вращение против часовой стрелки
				ENCODER_slowler++;
				if (ENCODER_slowler >= CALIBRATE.ENCODER_SLOW_RATE)
				{
					PERIPH_ENCODER_Rotated(CALIBRATE.ENCODER_INVERT ? -1 : 1);
					ENCODER_slowler = 0;
				}
			}
		}
		ENCODER_AValDeb = HAL_GetTick();
		ENClastClkVal = ENCODER_CLKVal;
	}
}

void PERIPH_ENCODER2_checkRotate(void)
{
	uint8_t ENCODER2_DTVal = HAL_GPIO_ReadPin(ENC2_DT_GPIO_Port, ENC2_DT_Pin);
	uint8_t ENCODER2_CLKVal = HAL_GPIO_ReadPin(ENC2_CLK_GPIO_Port, ENC2_CLK_Pin);

	if ((HAL_GetTick() - ENCODER2_AValDeb) < CALIBRATE.ENCODER2_DEBOUNCE)
		return;

	if (!CALIBRATE.ENCODER_ON_FALLING || ENCODER2_CLKVal == 0)
	{
		if (ENCODER2_DTVal != ENCODER2_CLKVal)
		{ // Если вывод A изменился первым - вращение по часовой стрелке
			PERIPH_ENCODER2_Rotated(CALIBRATE.ENCODER2_INVERT ? 1 : -1);
		}
		else
		{ // иначе B изменил свое состояние первым - вращение против часовой стрелки
			PERIPH_ENCODER2_Rotated(CALIBRATE.ENCODER2_INVERT ? -1 : 1);
		}
	}
	ENCODER2_AValDeb = HAL_GetTick();
}

static void PERIPH_ENCODER_Rotated(int8_t direction) //энкодер повернули, здесь обработчик, direction -1 - влево, 1 - вправо
{
	if (TRX.Locked)
		return;

	if (LCD_systemMenuOpened)
	{
		eventRotateSystemMenu(direction);
		return;
	}
	VFO *vfo = CurrentVFO();
	if (TRX.Fast)
	{
		TRX_setFrequency((uint32_t)((int32_t)vfo->Freq + ((int32_t)TRX.FRQ_FAST_STEP * direction)), vfo);
		if ((vfo->Freq % TRX.FRQ_FAST_STEP) > 0)
			TRX_setFrequency(vfo->Freq / TRX.FRQ_FAST_STEP * TRX.FRQ_FAST_STEP, vfo);
	}
	else
	{
		TRX_setFrequency((uint32_t)((int32_t)vfo->Freq + ((int32_t)TRX.FRQ_STEP * direction)), vfo);
		if ((vfo->Freq % TRX.FRQ_STEP) > 0)
			TRX_setFrequency(vfo->Freq / TRX.FRQ_STEP * TRX.FRQ_STEP, vfo);
	}
	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

static void PERIPH_ENCODER2_Rotated(int8_t direction) //энкодер повернули, здесь обработчик, direction -1 - влево, 1 - вправо
{
	if (TRX.Locked)
		return;

	if (LCD_systemMenuOpened)
	{
		eventSecRotateSystemMenu(direction);
		return;
	}

	//NOTCH - default action
	if (CurrentVFO()->ManualNotchFilter)
	{
		if (CurrentVFO()->NotchFC > 50 && direction < 0)
			CurrentVFO()->NotchFC -= 25;
		else if (CurrentVFO()->NotchFC < CurrentVFO()->LPF_Filter_Width && direction > 0)
			CurrentVFO()->NotchFC += 25;
		LCD_UpdateQuery.StatusInfoGUI = true;
		NeedReinitNotch = true;
	}
	else
	{
		VFO *vfo = CurrentVFO();
		if (TRX.Fast)
		{
			TRX_setFrequency((uint32_t)((int32_t)vfo->Freq + (int32_t)TRX.FRQ_ENC_FAST_STEP * direction), vfo);
			if ((vfo->Freq % TRX.FRQ_ENC_FAST_STEP) > 0)
				TRX_setFrequency(vfo->Freq / TRX.FRQ_ENC_FAST_STEP * TRX.FRQ_ENC_FAST_STEP, vfo);
		}
		else
		{
			TRX_setFrequency((uint32_t)((int32_t)vfo->Freq + (int32_t)TRX.FRQ_ENC_STEP * direction), vfo);
			if ((vfo->Freq % TRX.FRQ_ENC_STEP) > 0)
				TRX_setFrequency(vfo->Freq / TRX.FRQ_ENC_STEP * TRX.FRQ_ENC_STEP, vfo);
		}
		LCD_UpdateQuery.FreqInfo = true;
	}
}

void PERIPH_ENCODER2_checkSwitch(void)
{
	if (TRX.Locked)
		return;

	bool ENCODER2_SWNow = HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin);
	if (ENCODER2_SWLast != ENCODER2_SWNow)
	{
		ENCODER2_SWLast = ENCODER2_SWNow;
		if (!ENCODER2_SWNow)
		{
			//ENC2 CLICK
			NeedReinitNotch = true;
			LCD_UpdateQuery.StatusInfoGUI = true;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}
	}
}

static uint8_t getBPFByFreq(uint32_t freq)
{
	if (freq >= CALIBRATE.BPF_0_START && freq < CALIBRATE.BPF_0_END)
		return 0;
	if (freq >= CALIBRATE.BPF_1_START && freq < CALIBRATE.BPF_1_END)
		return 1;
	if (freq >= CALIBRATE.BPF_2_START && freq < CALIBRATE.BPF_2_END)
		return 2;
	if (freq >= CALIBRATE.BPF_3_START && freq < CALIBRATE.BPF_3_END)
		return 3;
	if (freq >= CALIBRATE.BPF_4_START && freq < CALIBRATE.BPF_4_END)
		return 4;
	if (freq >= CALIBRATE.BPF_5_START && freq < CALIBRATE.BPF_5_END)
		return 5;
	if (freq >= CALIBRATE.BPF_6_START && freq < CALIBRATE.BPF_6_END)
		return 6;
	if (freq >= CALIBRATE.BPF_7_HPF)
		return 7;
	return 255;
}

void PERIPH_RF_UNIT_UpdateState(bool clean) //передаём значения в RF-UNIT
{
	bool hpf_lock = false;

	bool dualrx_lpf_disabled = false;
	bool dualrx_bpf_disabled = false;
	if ((TRX.Dual_RX_Type != VFO_SEPARATE) && SecondaryVFO()->Freq > CALIBRATE.LPF_END)
		dualrx_lpf_disabled = true;
	if ((TRX.Dual_RX_Type != VFO_SEPARATE) && getBPFByFreq(CurrentVFO()->Freq) != getBPFByFreq(SecondaryVFO()->Freq))
		dualrx_bpf_disabled = true;

	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); //защёлка
	MINI_DELAY
	for (uint8_t registerNumber = 0; registerNumber < 24; registerNumber++)
	{
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); //клок данных
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); //данные
		MINI_DELAY
		if (!clean)
		{
			//REGISTER 1
			//if(registerNumber==0) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			if (registerNumber == 1 && !TRX_on_TX() && TRX.LNA)
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // LNA
			//if(registerNumber==2) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==3) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==4) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==5) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==6) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			if (registerNumber == 7 && TRX_on_TX() && CurrentVFO()->Mode != TRX_MODE_LOOPBACK)
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //TX_RX

			//REGISTER 2
			if (registerNumber == 8 && TRX_on_TX() && CurrentVFO()->Mode != TRX_MODE_LOOPBACK) //TX_AMP
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 9 && TRX.ATT) //ATT_ON
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 10 && (!TRX.LPF || CurrentVFO()->Freq > CALIBRATE.LPF_END || dualrx_lpf_disabled)) //LPF_OFF
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 11 && (!TRX.BPF || CurrentVFO()->Freq < CALIBRATE.BPF_1_START || dualrx_bpf_disabled)) //BPF_OFF
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 12 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 0) //BPF_0
			{
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				hpf_lock = true; //блокируем HPF для выделенного BPF фильтра УКВ
			}
			if (registerNumber == 13 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 7 && !hpf_lock && !dualrx_bpf_disabled)
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_7_HPF
			if (registerNumber == 14 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 6 && !dualrx_bpf_disabled)
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_6
			//if(registerNumber==15) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused

			//REGISTER 3
			//if(registerNumber==16) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==17) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			if (registerNumber == 18 && ((TRX_on_TX() && CurrentVFO()->Mode != TRX_MODE_LOOPBACK) || TRX_Fan_Timeout > 0)) //FAN
			{
				if(TRX_Fan_Timeout < (30 * 100)) //PWM
				{
					const uint8_t on_ticks = 1;
					const uint8_t off_ticks = 1;
					static bool pwm_status = false; //true - on false - off
					static uint8_t pwm_ticks = 0;
					pwm_ticks++;
					if(pwm_status)
						HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
					if((pwm_status && pwm_ticks==on_ticks) || (!pwm_status && pwm_ticks==off_ticks))
					{
						pwm_status = !pwm_status;
						pwm_ticks = 0;
					}
				}
				else
					HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
				if (TRX_Fan_Timeout > 0)
					TRX_Fan_Timeout--;
			}
			if (registerNumber == 19 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 1 && !dualrx_bpf_disabled) //BPF_1
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 20 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 2 && !dualrx_bpf_disabled) //BPF_2
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 21 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 3 && !dualrx_bpf_disabled) //BPF_3
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 22 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 4 && !dualrx_bpf_disabled) //BPF_4
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET);
			if (registerNumber == 23 && TRX.BPF && getBPFByFreq(CurrentVFO()->Freq) == 5 && !dualrx_bpf_disabled) //BPF_5
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

void PERIPH_InitFrontPanel(void)
{
	uint16_t test_value = PERIPH_ReadMCP3008_Value(0, AD1_CS_GPIO_Port, AD1_CS_Pin);
	if (test_value == 65535)
	{
		FRONTPanel_MCP3008_1_Enabled = false;
		sendToDebug_strln("[ERR] Frontpanel MCP3008 - 1 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 1 init error (FPGA I2S CLK?)", true);
	}
	test_value = PERIPH_ReadMCP3008_Value(0, AD2_CS_GPIO_Port, AD2_CS_Pin);
	if (test_value == 65535)
	{
		FRONTPanel_MCP3008_2_Enabled = false;
		sendToDebug_strln("[ERR] Frontpanel MCP3008 - 2 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 2 init error", true);
	}
	PERIPH_ProcessFrontPanel();
}

void PERIPH_ProcessFrontPanel(void)
{
	if (PERIPH_SPI_process)
		return;
	else
		PERIPH_SPI_process = true;
	uint16_t mcp3008_value = 0;

	PERIPH_ENCODER2_checkSwitch();

	if (FRONTPanel_MCP3008_1_Enabled)
	{
		//MCP3008 - 1 (10bit - 1024values)
		mcp3008_value = PERIPH_ReadMCP3008_Value(0, AD1_CS_GPIO_Port, AD1_CS_Pin); // AB
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_ab = true;
		else
			PERIPH_FrontPanel.key_ab = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(1, AD1_CS_GPIO_Port, AD1_CS_Pin); // TUNE
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_tune = true;
		else
			PERIPH_FrontPanel.key_tune = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(2, AD1_CS_GPIO_Port, AD1_CS_Pin); // PREATT
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_preatt = true;
		else
			PERIPH_FrontPanel.key_preatt = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(3, AD1_CS_GPIO_Port, AD1_CS_Pin); // FAST
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_fast = true;
		else
			PERIPH_FrontPanel.key_fast = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(4, AD1_CS_GPIO_Port, AD1_CS_Pin); // MODE+
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_modep = true;
		else
			PERIPH_FrontPanel.key_modep = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(5, AD1_CS_GPIO_Port, AD1_CS_Pin); // MODE-
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_moden = true;
		else
			PERIPH_FrontPanel.key_moden = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(6, AD1_CS_GPIO_Port, AD1_CS_Pin); // BAND+
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_bandp = true;
		else
			PERIPH_FrontPanel.key_bandp = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(7, AD1_CS_GPIO_Port, AD1_CS_Pin); // BAND-
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_bandn = true;
		else
			PERIPH_FrontPanel.key_bandn = false;

		//A/B
		if (PERIPH_FrontPanel.key_ab_prev != PERIPH_FrontPanel.key_ab && PERIPH_FrontPanel.key_ab && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_ab_starttime = HAL_GetTick();
			PERIPH_FrontPanel.key_ab_afterhold = false;
		}
		//A/B HOLD - DUAL RX SWITCH
		if (PERIPH_FrontPanel.key_ab_prev == PERIPH_FrontPanel.key_ab && PERIPH_FrontPanel.key_ab && (HAL_GetTick() - PERIPH_FrontPanel.key_ab_starttime) > KEY_HOLD_TIME && !PERIPH_FrontPanel.key_ab_afterhold && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_ab_afterhold = true;
			if (TRX.Dual_RX_Type == VFO_SEPARATE)
				TRX.Dual_RX_Type = VFO_A_AND_B;
			else if (TRX.Dual_RX_Type == VFO_A_AND_B)
				TRX.Dual_RX_Type = VFO_A_PLUS_B;
			else if (TRX.Dual_RX_Type == VFO_A_PLUS_B)
				TRX.Dual_RX_Type = VFO_SEPARATE;
			LCD_UpdateQuery.TopButtons = true;
			ReinitAudioFilters();
		}
		//A/B CLICK
		if (PERIPH_FrontPanel.key_ab_prev != PERIPH_FrontPanel.key_ab && !PERIPH_FrontPanel.key_ab && (HAL_GetTick() - PERIPH_FrontPanel.key_ab_starttime) < KEY_HOLD_TIME && !PERIPH_FrontPanel.key_ab_afterhold && !TRX.Locked && !LCD_systemMenuOpened)
		{
			TRX.current_vfo = !TRX.current_vfo;
			TRX_setFrequency(CurrentVFO()->Freq, CurrentVFO());
			LCD_UpdateQuery.TopButtons = true;
			LCD_UpdateQuery.FreqInfo = true;
			LCD_UpdateQuery.StatusInfoGUI = true;
			NeedSaveSettings = true;
			ReinitAudioFilters();
			LCD_redraw();
		}
		//TUNE
		if (PERIPH_FrontPanel.key_tune_prev != PERIPH_FrontPanel.key_tune && PERIPH_FrontPanel.key_tune && !TRX.Locked)
		{
			TRX_Tune = !TRX_Tune;
			TRX_ptt_hard = TRX_Tune;
			LCD_UpdateQuery.StatusInfoGUI = true;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
			TRX_Restart_Mode();
		}
		//PREATT
		if (PERIPH_FrontPanel.key_preatt_prev != PERIPH_FrontPanel.key_preatt && PERIPH_FrontPanel.key_preatt && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_preatt_starttime = HAL_GetTick();
			PERIPH_FrontPanel.key_preatt_afterhold = false;
		}
		//PREATT HOLD - ADC DRIVER AND ADC PREAMP
		if (PERIPH_FrontPanel.key_preatt_prev == PERIPH_FrontPanel.key_preatt && PERIPH_FrontPanel.key_preatt && (HAL_GetTick() - PERIPH_FrontPanel.key_preatt_starttime) > KEY_HOLD_TIME && !PERIPH_FrontPanel.key_preatt_afterhold && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_preatt_afterhold = true;

			if (!TRX.ADC_Driver && !TRX.ADC_PGA)
			{
				TRX.ADC_Driver = true;
				TRX.ADC_PGA = false;
			}
			else if (TRX.ADC_Driver && !TRX.ADC_PGA)
			{
				TRX.ADC_Driver = true;
				TRX.ADC_PGA = true;
			}
			else if (TRX.ADC_Driver && TRX.ADC_PGA)
			{
				TRX.ADC_Driver = false;
				TRX.ADC_PGA = true;
			}
			else if (!TRX.ADC_Driver && TRX.ADC_PGA)
			{
				TRX.ADC_Driver = false;
				TRX.ADC_PGA = false;
			}
			int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
			if (band > 0)
			{
				TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
				TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
			}
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}
		//PREATT CLICK
		if (PERIPH_FrontPanel.key_preatt_prev != PERIPH_FrontPanel.key_preatt && !PERIPH_FrontPanel.key_preatt && (HAL_GetTick() - PERIPH_FrontPanel.key_preatt_starttime) < KEY_HOLD_TIME && !PERIPH_FrontPanel.key_preatt_afterhold && !TRX.Locked && !LCD_systemMenuOpened)
		{
			if (!TRX.LNA && !TRX.ATT)
			{
				TRX.LNA = true;
				TRX.ATT = false;
			}
			else if (TRX.LNA && !TRX.ATT)
			{
				TRX.LNA = true;
				TRX.ATT = true;
			}
			else if (TRX.LNA && TRX.ATT)
			{
				TRX.LNA = false;
				TRX.ATT = true;
			}
			else if (!TRX.LNA && TRX.ATT)
			{
				TRX.LNA = false;
				TRX.ATT = false;
			}
			int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
			if (band > 0)
			{
				TRX.BANDS_SAVED_SETTINGS[band].LNA = TRX.LNA;
				TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
			}
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}
		//FAST
		if (PERIPH_FrontPanel.key_fast_prev != PERIPH_FrontPanel.key_fast && PERIPH_FrontPanel.key_fast && !TRX.Locked)
		{
			TRX.Fast = !TRX.Fast;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}
		//MODE+
		if (PERIPH_FrontPanel.key_modep_prev != PERIPH_FrontPanel.key_modep && PERIPH_FrontPanel.key_modep && !TRX.Locked)
		{
			int8_t mode = (int8_t)CurrentVFO()->Mode;
			mode++;
			if (mode < 0)
				mode = TRX_MODE_COUNT - 2;
			if (mode >= (TRX_MODE_COUNT - 1))
				mode = 0;
			TRX_setMode((uint8_t)mode, CurrentVFO());
			int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
			if (band > 0)
				TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
			LCD_UpdateQuery.TopButtons = true;
		}
		//MODE-
		if (PERIPH_FrontPanel.key_moden_prev != PERIPH_FrontPanel.key_moden && PERIPH_FrontPanel.key_moden && !TRX.Locked)
		{
			int8_t mode = (int8_t)CurrentVFO()->Mode;
			mode--;
			if (mode < 0)
				mode = TRX_MODE_COUNT - 2;
			if (mode >= (TRX_MODE_COUNT - 1))
				mode = 0;
			TRX_setMode((uint8_t)mode, CurrentVFO());
			int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
			if (band > 0)
				TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
			LCD_UpdateQuery.TopButtons = true;
		}
		//BAND+
		if (PERIPH_FrontPanel.key_bandp_prev != PERIPH_FrontPanel.key_bandp && PERIPH_FrontPanel.key_bandp && !TRX.Locked)
		{
			int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
			band++;
			if (band >= BANDS_COUNT)
				band = 0;
			if (band < 0)
				band = BANDS_COUNT - 1;

			TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO());
			TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO());
			TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
			TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
			TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
			TRX.FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
			TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
			CurrentVFO()->DNR = TRX.BANDS_SAVED_SETTINGS[band].DNR;
			CurrentVFO()->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
			
			LCD_UpdateQuery.TopButtons = true;
			LCD_UpdateQuery.FreqInfo = true;
		}
		//BAND-
		if (PERIPH_FrontPanel.key_bandn_prev != PERIPH_FrontPanel.key_bandn && PERIPH_FrontPanel.key_bandn && !TRX.Locked)
		{
			int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
			band--;
			if (band >= BANDS_COUNT)
				band = 0;
			if (band < 0)
				band = BANDS_COUNT - 1;

			TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO());
			TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO());
			TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
			TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
			TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
			TRX.FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
			TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
			CurrentVFO()->DNR = TRX.BANDS_SAVED_SETTINGS[band].DNR;
			CurrentVFO()->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;

			LCD_UpdateQuery.TopButtons = true;
			LCD_UpdateQuery.FreqInfo = true;
		}

		PERIPH_FrontPanel.key_ab_prev = PERIPH_FrontPanel.key_ab;
		PERIPH_FrontPanel.key_tune_prev = PERIPH_FrontPanel.key_tune;
		PERIPH_FrontPanel.key_preatt_prev = PERIPH_FrontPanel.key_preatt;
		PERIPH_FrontPanel.key_fast_prev = PERIPH_FrontPanel.key_fast;
		PERIPH_FrontPanel.key_modep_prev = PERIPH_FrontPanel.key_modep;
		PERIPH_FrontPanel.key_moden_prev = PERIPH_FrontPanel.key_moden;
		PERIPH_FrontPanel.key_bandp_prev = PERIPH_FrontPanel.key_bandp;
		PERIPH_FrontPanel.key_bandn_prev = PERIPH_FrontPanel.key_bandn;
	}

	if (FRONTPanel_MCP3008_2_Enabled)
	{
		//MCP3008 - 2 (10bit - 1024values)
		mcp3008_value = PERIPH_ReadMCP3008_Value(0, AD2_CS_GPIO_Port, AD2_CS_Pin); // MENU
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_menu = true;
		else
			PERIPH_FrontPanel.key_menu = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(1, AD2_CS_GPIO_Port, AD2_CS_Pin); // CLAR
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_clar = true;
		else
			PERIPH_FrontPanel.key_clar = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(2, AD2_CS_GPIO_Port, AD2_CS_Pin); // NOTCH
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_notch = true;
		else
			PERIPH_FrontPanel.key_notch = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(3, AD2_CS_GPIO_Port, AD2_CS_Pin); // A=B
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_a_set_b = true;
		else
			PERIPH_FrontPanel.key_a_set_b = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(4, AD2_CS_GPIO_Port, AD2_CS_Pin); // DNR
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_dnr = true;
		else
			PERIPH_FrontPanel.key_dnr = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(5, AD2_CS_GPIO_Port, AD2_CS_Pin); // AGC
		if (mcp3008_value < MCP3008_THRESHOLD)
			PERIPH_FrontPanel.key_agc = true;
		else
			PERIPH_FrontPanel.key_agc = false;

		mcp3008_value = PERIPH_ReadMCP3008_Value(6, AD2_CS_GPIO_Port, AD2_CS_Pin); // AF_GAIN
		TRX_Volume = (uint16_t)(1023.0f - mcp3008_value);

		mcp3008_value = PERIPH_ReadMCP3008_Value(7, AD2_CS_GPIO_Port, AD2_CS_Pin); // SHIFT или IF Gain
		if (TRX.ShiftEnabled)
		{
			TRX_SHIFT = (int_fast16_t)(((1023.0f - mcp3008_value) * TRX.SHIFT_INTERVAL * 2 / 1023.0f) - TRX.SHIFT_INTERVAL);
			//if (abs(TRX_SHIFT) < (TRX.SHIFT_INTERVAL / 10.0f)) //при минимальных отклонениях - игнорируем
			//TRX_SHIFT = 0;
		}
		else
		{
			TRX_SHIFT = 0;
			TRX.IF_Gain = (uint8_t)(30.0f + ((1023.0f - mcp3008_value) * 50.0f / 1023.0f));
		}

		//F1 AGC
		if (PERIPH_FrontPanel.key_agc_prev != PERIPH_FrontPanel.key_agc && PERIPH_FrontPanel.key_agc && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_agc_starttime = HAL_GetTick();
			PERIPH_FrontPanel.key_agc_afterhold = false;
		}
		//F1 AGC HOLD - RF-POWER
		if (PERIPH_FrontPanel.key_agc_prev == PERIPH_FrontPanel.key_agc && PERIPH_FrontPanel.key_agc && (HAL_GetTick() - PERIPH_FrontPanel.key_agc_starttime) > KEY_HOLD_TIME && !PERIPH_FrontPanel.key_agc_afterhold && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_agc_afterhold = true;

			if (!LCD_systemMenuOpened)
			{
				LCD_systemMenuOpened = true;
				SYSMENU_RFPOWER_HOTKEY();
				drawSystemMenu(true);
			}
			else
			{
				eventCloseSystemMenu();
				eventCloseSystemMenu();
			}
			LCD_redraw();
		}
		//F1 AGC CLICK
		if (PERIPH_FrontPanel.key_agc_prev != PERIPH_FrontPanel.key_agc && !PERIPH_FrontPanel.key_agc && (HAL_GetTick() - PERIPH_FrontPanel.key_agc_starttime) < KEY_HOLD_TIME && !PERIPH_FrontPanel.key_agc_afterhold && !TRX.Locked && !LCD_systemMenuOpened)
		{
			CurrentVFO()->AGC = !CurrentVFO()->AGC;
			int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
			if (band > 0)
				TRX.BANDS_SAVED_SETTINGS[band].AGC = CurrentVFO()->AGC;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}

		//F2 DNR
		if (PERIPH_FrontPanel.key_dnr_prev != PERIPH_FrontPanel.key_dnr && PERIPH_FrontPanel.key_dnr && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_dnr_starttime = HAL_GetTick();
			PERIPH_FrontPanel.key_dnr_afterhold = false;
		}
		//F2 DNR HOLD - KEY WPM
		if (PERIPH_FrontPanel.key_dnr_prev == PERIPH_FrontPanel.key_dnr && PERIPH_FrontPanel.key_dnr && (HAL_GetTick() - PERIPH_FrontPanel.key_dnr_starttime) > KEY_HOLD_TIME && !PERIPH_FrontPanel.key_dnr_afterhold && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_dnr_afterhold = true;

			if (!LCD_systemMenuOpened)
			{
				LCD_systemMenuOpened = true;
				SYSMENU_CW_WPM_HOTKEY();
				drawSystemMenu(true);
			}
			else
			{
				eventCloseSystemMenu();
				eventCloseSystemMenu();
			}
			LCD_redraw();
		}
		//F2 DNR CLICK
		if (PERIPH_FrontPanel.key_dnr_prev != PERIPH_FrontPanel.key_dnr && !PERIPH_FrontPanel.key_dnr && (HAL_GetTick() - PERIPH_FrontPanel.key_dnr_starttime) < KEY_HOLD_TIME && !PERIPH_FrontPanel.key_dnr_afterhold && !TRX.Locked && !LCD_systemMenuOpened)
		{
			CurrentVFO()->DNR = !CurrentVFO()->DNR;
			int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
			if (band > 0)
				TRX.BANDS_SAVED_SETTINGS[band].DNR = CurrentVFO()->DNR;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}

		//F3 A=B
		if (PERIPH_FrontPanel.key_a_set_b_prev != PERIPH_FrontPanel.key_a_set_b && PERIPH_FrontPanel.key_a_set_b && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_a_set_b_starttime = HAL_GetTick();
			PERIPH_FrontPanel.key_a_set_b_afterhold = false;
		}
		//F3 A=B HOLD - KEY WPM
		if (PERIPH_FrontPanel.key_a_set_b_prev == PERIPH_FrontPanel.key_a_set_b && PERIPH_FrontPanel.key_a_set_b && (HAL_GetTick() - PERIPH_FrontPanel.key_a_set_b_starttime) > KEY_HOLD_TIME && !PERIPH_FrontPanel.key_a_set_b_afterhold && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_a_set_b_afterhold = true;

			if (!LCD_systemMenuOpened)
			{
				LCD_systemMenuOpened = true;
				if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
					SYSMENU_AUDIO_CW_HOTKEY();
				else if (CurrentVFO()->Mode == TRX_MODE_NFM || CurrentVFO()->Mode == TRX_MODE_WFM)
					SYSMENU_AUDIO_FM_HOTKEY();
				else if (CurrentVFO()->Mode == TRX_MODE_AM)
					SYSMENU_AUDIO_AM_HOTKEY();
				else
					SYSMENU_AUDIO_SSB_HOTKEY();
				drawSystemMenu(true);
			}
			else
			{
				eventCloseSystemMenu();
				eventCloseSystemMenu();
			}
			LCD_redraw();
		}
		//F3 A=B CLICK
		if (PERIPH_FrontPanel.key_a_set_b_prev != PERIPH_FrontPanel.key_a_set_b && !PERIPH_FrontPanel.key_a_set_b && (HAL_GetTick() - PERIPH_FrontPanel.key_a_set_b_starttime) < KEY_HOLD_TIME && !PERIPH_FrontPanel.key_a_set_b_afterhold && !TRX.Locked && !LCD_systemMenuOpened)
		{
			if (TRX.current_vfo)
			{
				TRX.VFO_A.LPF_Filter_Width = TRX.VFO_B.LPF_Filter_Width;
				TRX.VFO_A.HPF_Filter_Width = TRX.VFO_B.HPF_Filter_Width;
				TRX.VFO_A.Freq = TRX.VFO_B.Freq;
				TRX.VFO_A.Mode = TRX.VFO_B.Mode;
			}
			else
			{
				TRX.VFO_B.LPF_Filter_Width = TRX.VFO_A.LPF_Filter_Width;
				TRX.VFO_B.HPF_Filter_Width = TRX.VFO_A.HPF_Filter_Width;
				TRX.VFO_B.Freq = TRX.VFO_A.Freq;
				TRX.VFO_B.Mode = TRX.VFO_A.Mode;
			}
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}

		//F4 NOTCH
		if (PERIPH_FrontPanel.key_notch_prev != PERIPH_FrontPanel.key_notch && PERIPH_FrontPanel.key_notch && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_notch_starttime = HAL_GetTick();
			PERIPH_FrontPanel.key_notch_afterhold = false;
		}
		//F4 NOTCH HOLD - ENABLE MENUAL NOTCH
		if (PERIPH_FrontPanel.key_notch_prev == PERIPH_FrontPanel.key_notch && PERIPH_FrontPanel.key_notch && (HAL_GetTick() - PERIPH_FrontPanel.key_notch_starttime) > KEY_HOLD_TIME && !PERIPH_FrontPanel.key_notch_afterhold && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_notch_afterhold = true;

			if (CurrentVFO()->NotchFC > CurrentVFO()->LPF_Filter_Width)
				CurrentVFO()->NotchFC = CurrentVFO()->LPF_Filter_Width;
			CurrentVFO()->AutoNotchFilter = false;
			if (!CurrentVFO()->ManualNotchFilter)
				CurrentVFO()->ManualNotchFilter = true;
			else
				CurrentVFO()->ManualNotchFilter = false;

			NeedReinitNotch = true;
			LCD_UpdateQuery.StatusInfoGUI = true;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}
		//F4 NOTCH CLICK
		if (PERIPH_FrontPanel.key_notch_prev != PERIPH_FrontPanel.key_notch && !PERIPH_FrontPanel.key_notch && (HAL_GetTick() - PERIPH_FrontPanel.key_notch_starttime) < KEY_HOLD_TIME && !PERIPH_FrontPanel.key_notch_afterhold && !TRX.Locked && !LCD_systemMenuOpened)
		{
			if (CurrentVFO()->NotchFC > CurrentVFO()->LPF_Filter_Width)
				CurrentVFO()->NotchFC = CurrentVFO()->LPF_Filter_Width;
			CurrentVFO()->ManualNotchFilter = false;
			if (!CurrentVFO()->AutoNotchFilter)
				CurrentVFO()->AutoNotchFilter = true;
			else
				CurrentVFO()->AutoNotchFilter = false;

			NeedReinitNotch = true;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}

		//F5 CLAR
		if (PERIPH_FrontPanel.key_clar_prev != PERIPH_FrontPanel.key_clar && PERIPH_FrontPanel.key_clar && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_clar_starttime = HAL_GetTick();
			PERIPH_FrontPanel.key_clar_afterhold = false;
		}
		//F5 CLAR HOLD - ENABLE SHIFT
		if (PERIPH_FrontPanel.key_clar_prev == PERIPH_FrontPanel.key_clar && PERIPH_FrontPanel.key_clar && (HAL_GetTick() - PERIPH_FrontPanel.key_clar_starttime) > KEY_HOLD_TIME && !PERIPH_FrontPanel.key_clar_afterhold && !TRX.Locked)
		{
			PERIPH_FrontPanel.key_clar_afterhold = true;

			TRX.ShiftEnabled = !TRX.ShiftEnabled;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}
		//F5 CLAR CLICK
		if (PERIPH_FrontPanel.key_clar_prev != PERIPH_FrontPanel.key_clar && !PERIPH_FrontPanel.key_clar && (HAL_GetTick() - PERIPH_FrontPanel.key_clar_starttime) < KEY_HOLD_TIME && !PERIPH_FrontPanel.key_clar_afterhold && !TRX.Locked && !LCD_systemMenuOpened)
		{
			TRX.CLAR = !TRX.CLAR;
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}

		//F6 MENU
		if (PERIPH_FrontPanel.key_menu_prev != PERIPH_FrontPanel.key_menu && PERIPH_FrontPanel.key_menu)
		{
			PERIPH_FrontPanel.key_menu_starttime = HAL_GetTick();
			PERIPH_FrontPanel.key_menu_afterhold = false;
		}
		//F6 MENU HOLD - LOCK
		if (PERIPH_FrontPanel.key_menu_prev == PERIPH_FrontPanel.key_menu && PERIPH_FrontPanel.key_menu && (HAL_GetTick() - PERIPH_FrontPanel.key_menu_starttime) > KEY_HOLD_TIME && !PERIPH_FrontPanel.key_menu_afterhold)
		{
			PERIPH_FrontPanel.key_menu_afterhold = true;

			if (!LCD_systemMenuOpened)
				TRX.Locked = !TRX.Locked;
			else
			{
				sysmenu_hiddenmenu_enabled = true;
				LCD_redraw();
			}
			LCD_UpdateQuery.TopButtons = true;
			NeedSaveSettings = true;
		}
		//F6 MENU CLICK
		if (PERIPH_FrontPanel.key_menu_prev != PERIPH_FrontPanel.key_menu && !PERIPH_FrontPanel.key_menu && (HAL_GetTick() - PERIPH_FrontPanel.key_menu_starttime) < KEY_HOLD_TIME && !PERIPH_FrontPanel.key_menu_afterhold && !TRX.Locked)
		{
			if (!LCD_systemMenuOpened)
				LCD_systemMenuOpened = true;
			else
				eventCloseSystemMenu();
			LCD_redraw();
		}
		//

		PERIPH_FrontPanel.key_agc_prev = PERIPH_FrontPanel.key_agc;
		PERIPH_FrontPanel.key_dnr_prev = PERIPH_FrontPanel.key_dnr;
		PERIPH_FrontPanel.key_a_set_b_prev = PERIPH_FrontPanel.key_a_set_b;
		PERIPH_FrontPanel.key_notch_prev = PERIPH_FrontPanel.key_notch;
		PERIPH_FrontPanel.key_clar_prev = PERIPH_FrontPanel.key_clar;
		PERIPH_FrontPanel.key_menu_prev = PERIPH_FrontPanel.key_menu;
	}
	PERIPH_SPI_process = false;
}

void PERIPH_ProcessSWRMeter(void)
{
	float32_t forward = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2) * TRX_STM32_VREF / 65535.0f;
	float32_t backward = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1) * TRX_STM32_VREF / 65535.0f;
	static float32_t TRX_VLT_forward = 0.0f;
	static float32_t TRX_VLT_backward = 0.0f;
	
	forward = forward / (510.0f / (0.1f + 510.0f)); //корректируем напряжение исходя из делителя напряжения (0.1ом и 510ом)
	if (forward < 0.01f)							//меньше 10mV не измеряем
	{
		TRX_VLT_forward = 0.0f;
		TRX_VLT_backward = 0.0f;
		TRX_SWR = 1.0f;
		return;
	}

	forward += 0.62f;							  // падение на диоде
	forward = forward * CALIBRATE.swr_trans_rate; // Коэффициент трансформации КСВ метра

	backward = backward / (510.0f / (0.1f + 510.0f)); //корректируем напряжение исходя из делителя напряжения (0.1ом и 510ом)
	if (backward >= 0.01f)							  //меньше 10mV не измеряем
	{
		backward += 0.62f;								// падение на диоде
		backward = backward * CALIBRATE.swr_trans_rate; //Коэффициент трансформации КСВ метра
	}
	else
		backward = 0.001f;

	TRX_VLT_forward = TRX_VLT_forward + (forward - TRX_VLT_forward) / 2;
	TRX_VLT_backward = TRX_VLT_backward + (backward - TRX_VLT_backward) / 2;
	TRX_SWR = (TRX_VLT_forward + TRX_VLT_backward) / (TRX_VLT_forward - TRX_VLT_backward);

	if (TRX_VLT_backward > TRX_VLT_forward)
		TRX_SWR = 10.0f;
	if (TRX_SWR > 10.0f)
		TRX_SWR = 10.0f;
	
	TRX_PWR_Forward = (TRX_VLT_forward * TRX_VLT_forward) / 50.0f;
	if (TRX_PWR_Forward < 0.0f)
			TRX_PWR_Forward = 0.0f;
	TRX_PWR_Backward = (TRX_VLT_backward * TRX_VLT_backward) / 50.0f;
	if (TRX_PWR_Backward < 0.0f)
			TRX_PWR_Backward = 0.0f;

	LCD_UpdateQuery.StatusInfoBar = true;
}

static uint16_t PERIPH_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN)
{
	uint8_t outData[3] = {0};
	uint8_t inData[3] = {0};
	uint16_t mcp3008_value = 0;

	outData[0] = 0x18 | channel;
	bool res = PERIPH_SPI_Transmit(outData, inData, 3, CS_PORT, CS_PIN, false);
	if (res == false)
		return 65535;
	mcp3008_value = (uint16_t)(0 | ((inData[1] & 0x3F) << 4) | (inData[2] & 0xF0 >> 4));

	return mcp3008_value;
}

bool PERIPH_SPI_Transmit(uint8_t *out_data, uint8_t *in_data, uint8_t count, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN, bool hold_cs)
{
	if (PERIPH_SPI_busy)
	{
		sendToDebug_strln("SPI Busy");
		return false;
	}
	const int32_t timeout = 0x200; //HAL_MAX_DELAY
	PERIPH_SPI_busy = true;
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef res = 0;
	if (in_data == NULL)
	{
		res = HAL_SPI_Transmit(&hspi2, out_data, count, timeout);
	}
	else if (out_data == NULL)
	{
    memset(in_data, 0x00, count);
		res = HAL_SPI_Receive(&hspi2, in_data, count, timeout);
	}
	else
	{
    memset(in_data, 0x00, count);
		res = HAL_SPI_TransmitReceive(&hspi2, out_data, in_data, count, timeout);
	}
	if (!hold_cs)
		HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
	PERIPH_SPI_busy = false;
	if (res == HAL_TIMEOUT || res == HAL_ERROR)
		return false;
	else
		return true;
}
