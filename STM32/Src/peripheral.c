#include "stm32f4xx_hal.h"
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

static uint8_t ENCODER_ALast = 0;
static uint8_t ENCODER_AVal = 0;
static int32_t ENCODER_slowler = 0;
static uint8_t ENCODER2_ALast = 0;
static bool ENCODER2_SWLast = true;
static uint8_t ENCODER2_AVal = 0;

volatile PERIPH_FrontPanel_Type PERIPH_FrontPanel = { 0 };

static void PERIPH_ENCODER_Rotated(int direction);
static void PERIPH_ENCODER2_Rotated(int direction);
static uint16_t PERIPH_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef* CS_PORT, uint16_t CS_PIN);
	
void PERIPH_ENCODER_checkRotate(void) {
	if (HAL_GPIO_ReadPin(ENC_DT_GPIO_Port, ENC_DT_Pin) != HAL_GPIO_ReadPin(ENC_CLK_GPIO_Port, ENC_CLK_Pin)) {  // Если вывод A изменился первым - вращение по часовой стрелке
		ENCODER_slowler--;
		if (ENCODER_slowler < -TRX.ENCODER_SLOW_RATE)
		{
			PERIPH_ENCODER_Rotated(ENCODER_INVERT ? 1 : -1);
			ENCODER_slowler = 0;
		}
	}
	else {// иначе B изменил свое состояние первым - вращение против часовой стрелки
		ENCODER_slowler++;
		if (ENCODER_slowler > TRX.ENCODER_SLOW_RATE)
		{
			PERIPH_ENCODER_Rotated(ENCODER_INVERT ? -1 : 1);
			ENCODER_slowler = 0;
		}
	}
}

void PERIPH_ENCODER2_checkRotate(void) {
	if (HAL_GPIO_ReadPin(ENC2_DT_GPIO_Port, ENC2_DT_Pin) != HAL_GPIO_ReadPin(ENC2_CLK_GPIO_Port, ENC2_CLK_Pin)) {  // Если вывод A изменился первым - вращение по часовой стрелке
		PERIPH_ENCODER2_Rotated(ENCODER2_INVERT ? 1 : -1);
	}
	else {// иначе B изменил свое состояние первым - вращение против часовой стрелки
		PERIPH_ENCODER2_Rotated(ENCODER2_INVERT ? -1 : 1);
	}
}

static void PERIPH_ENCODER_Rotated(int direction) //энкодер повернули, здесь обработчик, direction -1 - влево, 1 - вправо
{
	if (LCD_systemMenuOpened && !LCD_timeMenuOpened)
	{
		eventRotateSystemMenu(direction);
		return;
	}
	if (!LCD_mainMenuOpened)
	{
		switch (TRX.LCD_menu_freq_index) {
			case MENU_FREQ_HZ:
				if (TRX.Fast)
					TRX_setFrequency(TRX_getFrequency() + 100 * direction);
				else
					TRX_setFrequency(TRX_getFrequency() + 10 * direction);
				break;
			case MENU_FREQ_KHZ:
				if (TRX.Fast)
					TRX_setFrequency(TRX_getFrequency() + 10000 * direction);
				else
					TRX_setFrequency(TRX_getFrequency() + 1000 * direction);
				break;
			case MENU_FREQ_MHZ:
				TRX_setFrequency(TRX_getFrequency() + 1000000 * direction);
				break;
			default:
				break;
		}
		if((TRX_getFrequency() % 10) > 0)
			TRX_setFrequency(TRX_getFrequency()/10*10);
		LCD_UpdateQuery.FreqInfo = true;
	}
	if (LCD_mainMenuOpened)
	{
		if (LCD_timeMenuOpened)
		{
			uint32_t Time = RTC->TR;
			RTC_TimeTypeDef sTime;
			sTime.TimeFormat = RTC_HOURFORMAT12_PM;
			sTime.SubSeconds = 0;
			sTime.SecondFraction = 0;
			sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
			sTime.StoreOperation = RTC_STOREOPERATION_SET;
			sTime.Hours = (uint8_t)(((Time >> 20) & 0x03) * 10 + ((Time >> 16) & 0x0f));
			sTime.Minutes = (uint8_t)(((Time >> 12) & 0x07) * 10 + ((Time >> 8) & 0x0f));
			sTime.Seconds = (uint8_t)(((Time >> 4) & 0x07) * 10 + ((Time >> 0) & 0x0f));
			if (TimeMenuSelection == 0)
			{
				if (sTime.Hours == 0 && direction < 0) return;
				sTime.Hours = sTime.Hours + direction;
			}
			if (TimeMenuSelection == 1)
			{
				if (sTime.Minutes == 0 && direction < 0) return;
				sTime.Minutes = sTime.Minutes + direction;
			}
			if (TimeMenuSelection == 2)
			{
				if (sTime.Seconds == 0 && direction < 0) return;
				sTime.Seconds = sTime.Seconds + direction;
			}
			if (sTime.Hours >= 24) sTime.Hours = 0;
			if (sTime.Minutes >= 60) sTime.Minutes = 0;
			if (sTime.Seconds >= 60) sTime.Seconds = 0;
			HAL_RTC_DeInit(&hrtc);
			HAL_RTC_Init(&hrtc);
			HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
			LCD_UpdateQuery.SystemMenu = true;
			return;
		}
		switch (LCD_menu_main_index) {
		case MENU_MAIN_VOLUME:
			TRX.Volume = TRX.Volume + direction;
			if (TRX.Volume < 1) TRX.Volume = 1;
			if (TRX.Volume > 100) TRX.Volume = 100;
			LCD_UpdateQuery.MainMenu = true;
			break;
		case MENU_MAIN_RF_GAIN:
			TRX.RF_Gain = TRX.RF_Gain + direction;
			if (TRX.RF_Gain < 1) TRX.RF_Gain = 1;
			if (TRX.RF_Gain > 250) TRX.RF_Gain = 250;
			LCD_UpdateQuery.MainMenu = true;
			break;
		case MENU_MAIN_FM_SQL:
			if (direction > 0 || TRX.FM_SQL_threshold > 0) TRX.FM_SQL_threshold = TRX.FM_SQL_threshold + direction;
			if (TRX.FM_SQL_threshold > 10) TRX.FM_SQL_threshold = 10;
			LCD_UpdateQuery.MainMenu = true;
			break;
		case MENU_MAIN_RF_POWER:
			TRX.RF_Power = TRX.RF_Power + direction;
			if (TRX.RF_Power < 1) TRX.RF_Power = 1;
			if (TRX.RF_Power > 100) TRX.RF_Power = 100;
			LCD_UpdateQuery.MainMenu = true;
			break;
		case MENU_MAIN_AGCSPEED:
			if (direction > 0 || TRX.Agc_speed > 0) TRX.Agc_speed = TRX.Agc_speed + direction;
			if (TRX.Agc_speed < 1) TRX.Agc_speed = 1;
			if (TRX.Agc_speed > 4) TRX.Agc_speed = 4;
			InitAGC();
			LCD_UpdateQuery.MainMenu = true;
			break;
		default:
			break;
		}
		NeedSaveSettings = true;
	}
}

static void PERIPH_ENCODER2_Rotated(int direction) //энкодер повернули, здесь обработчик, direction -1 - влево, 1 - вправо
{
	//VOLUME+NOTCH
	if (LCD_NotchEdit)
	{
		if (TRX.NotchFC > 50 && direction < 0)
			TRX.NotchFC -= 25;
		else if (TRX.NotchFC < CurrentVFO()->Filter_Width && direction > 0)
			TRX.NotchFC += 25;
		LCD_UpdateQuery.StatusInfoGUI = true;
		NeedReinitNotch = true;
	}
	else
	{
		if (((TRX.Volume + direction) > 0) && ((TRX.Volume + direction) < 100))
			TRX.Volume += direction;
		LCD_UpdateQuery.MainMenu = true;
	}
}

void PERIPH_ENCODER2_checkSwitch(void) {
	bool ENCODER2_SWNow = HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin);
	if(ENCODER2_SWLast != ENCODER2_SWNow)
	{
		ENCODER2_SWLast = ENCODER2_SWNow;
		if (!ENCODER2_SWNow)
		{
			//NOTCH
			LCD_Handler_NOTCH();
			LCD_UpdateQuery.TopButtons = true;
		}
	}
}

void PERIPH_RF_UNIT_UpdateState(bool clean) //передаём значения в RF-UNIT
{
	bool hpf_lock = false;
	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_RESET); //защёлка
	MINI_DELAY
	for (uint8_t registerNumber = 0; registerNumber < 16; registerNumber++) {
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET); //клок данных
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_RESET); //данные
		MINI_DELAY
		if (!clean)
		{
			if (registerNumber == 0 && TRX_on_TX() && TRX_getMode() != TRX_MODE_LOOPBACK && TRX.TX_Amplifier) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //TX_AMP
			if (registerNumber == 1 && TRX.ATT) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //ATT_ON
			if (registerNumber == 2 && (!TRX.LPF || TRX_getFrequency() > LPF_END)) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //LPF_OFF
			if (registerNumber == 3 && (!TRX.BPF || TRX_getFrequency() < BPF_1_START)) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_OFF
			if (registerNumber == 4 && TRX.BPF && TRX_getFrequency() >= BPF_0_START && TRX_getFrequency() < BPF_0_END)
			{
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_0
				hpf_lock = true; //блокируем HPF для выделенного BPF фильтра УКВ
			}
			if (registerNumber == 5 && TRX.BPF && TRX_getFrequency() >= BPF_1_START && TRX_getFrequency() < BPF_1_END) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_1
			if (registerNumber == 6 && TRX.BPF && TRX_getFrequency() >= BPF_2_START && TRX_getFrequency() < BPF_2_END) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_2
			if (registerNumber == 7 && TRX_on_TX() && TRX_getMode() != TRX_MODE_LOOPBACK) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //TX_RX

			//if(registerNumber==8) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			//if(registerNumber==9) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); // unused
			if (registerNumber == 10 && ((TRX_on_TX() && TRX_getMode() != TRX_MODE_LOOPBACK) || TRX_Fan_Timeout > 0))
			{
				HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //FAN
				if (TRX_Fan_Timeout > 0) TRX_Fan_Timeout--;
			}
			if (registerNumber == 11 && TRX.BPF && TRX_getFrequency() >= BPF_7_HPF && !hpf_lock) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_7_HPF
			if (registerNumber == 12 && TRX.BPF && TRX_getFrequency() >= BPF_6_START && TRX_getFrequency() < BPF_6_END) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_6
			if (registerNumber == 13 && TRX.BPF && TRX_getFrequency() >= BPF_5_START && TRX_getFrequency() < BPF_5_END) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_5
			if (registerNumber == 14 && TRX.BPF && TRX_getFrequency() >= BPF_4_START && TRX_getFrequency() < BPF_4_END) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_4
			if (registerNumber == 15 && TRX.BPF && TRX_getFrequency() >= BPF_3_START && TRX_getFrequency() < BPF_3_END) HAL_GPIO_WritePin(RFUNIT_DATA_GPIO_Port, RFUNIT_DATA_Pin, GPIO_PIN_SET); //BPF_3
		}
		MINI_DELAY
		HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_SET);
	}
	MINI_DELAY
	HAL_GPIO_WritePin(RFUNIT_CLK_GPIO_Port, RFUNIT_CLK_Pin, GPIO_PIN_RESET);
	MINI_DELAY
	HAL_GPIO_WritePin(RFUNIT_RCLK_GPIO_Port, RFUNIT_RCLK_Pin, GPIO_PIN_SET);
}

void PERIPH_ProcessFrontPanel(void)
{
	uint16_t mcp3008_value = 0;
	
	PERIPH_ENCODER2_checkSwitch();
	
	//MCP3008 - 1 (10bit - 1024values)
	mcp3008_value = PERIPH_ReadMCP3008_Value(0, AD1_CS_GPIO_Port, AD1_CS_Pin); // AB
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_ab = true; 
	else 
		PERIPH_FrontPanel.key_ab = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(1, AD1_CS_GPIO_Port, AD1_CS_Pin); // TUNE
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_tune = true; 
	else 
		PERIPH_FrontPanel.key_tune = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(2, AD1_CS_GPIO_Port, AD1_CS_Pin); // PREATT
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_preatt = true; 
	else 
		PERIPH_FrontPanel.key_preatt = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(3, AD1_CS_GPIO_Port, AD1_CS_Pin); // FAST
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_fast = true; 
	else 
		PERIPH_FrontPanel.key_fast = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(4, AD1_CS_GPIO_Port, AD1_CS_Pin); // MODE+
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_modep = true; 
	else 
		PERIPH_FrontPanel.key_modep = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(5, AD1_CS_GPIO_Port, AD1_CS_Pin); // MODE-
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_moden = true; 
	else 
		PERIPH_FrontPanel.key_moden = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(6, AD1_CS_GPIO_Port, AD1_CS_Pin); // BAND+
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_bandp = true; 
	else 
		PERIPH_FrontPanel.key_bandp = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(7, AD1_CS_GPIO_Port, AD1_CS_Pin); // BAND-
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_bandn = true; 
	else 
		PERIPH_FrontPanel.key_bandn = false;
	
	//AB
	if (PERIPH_FrontPanel.key_ab_prev != PERIPH_FrontPanel.key_ab && PERIPH_FrontPanel.key_ab)
	{
		LCD_Handler_VFO();
	}
	//TUNE
	if (PERIPH_FrontPanel.key_tune_prev != PERIPH_FrontPanel.key_tune && PERIPH_FrontPanel.key_tune)
	{
		LCD_Handler_TUNE();
	}
	//PREATT
	if (PERIPH_FrontPanel.key_preatt_prev != PERIPH_FrontPanel.key_preatt && PERIPH_FrontPanel.key_preatt)
	{
		if(!TRX.Preamp && !TRX.ATT)
		{
			TRX.Preamp = true;
			TRX.ATT = false;
		}
		else if(TRX.Preamp && !TRX.ATT)
		{
			TRX.Preamp = true;
			TRX.ATT = true;
		}
		else if(TRX.Preamp && TRX.ATT)
		{
			TRX.Preamp = false;
			TRX.ATT = true;
		}
		else if(!TRX.Preamp && TRX.ATT)
		{
			TRX.Preamp = false;
			TRX.ATT = false;
		}
		LCD_UpdateQuery.TopButtons = true;
		NeedSaveSettings = true;
	}
	//FAST
	if (PERIPH_FrontPanel.key_fast_prev != PERIPH_FrontPanel.key_fast && PERIPH_FrontPanel.key_fast)
	{
		LCD_Handler_FAST();
	}
	//MODE+
	if (PERIPH_FrontPanel.key_modep_prev != PERIPH_FrontPanel.key_modep && PERIPH_FrontPanel.key_modep)
	{
		int8_t mode = CurrentVFO()->Mode;
		mode++;
		if (mode < 0) mode = TRX_MODE_COUNT - 2;
		if (mode >= (TRX_MODE_COUNT - 1)) mode = 0;
		TRX_setMode(mode);
		LCD_UpdateQuery.TopButtons = true;
	}
	//MODE-
	if (PERIPH_FrontPanel.key_moden_prev != PERIPH_FrontPanel.key_moden && PERIPH_FrontPanel.key_moden)
	{
		int8_t mode = CurrentVFO()->Mode;
		mode--;
		if (mode < 0) mode = TRX_MODE_COUNT - 2;
		if (mode >= (TRX_MODE_COUNT - 1)) mode = 0;
		TRX_setMode(mode);
		LCD_UpdateQuery.TopButtons = true;
	}
	//BAND+
	if (PERIPH_FrontPanel.key_bandp_prev != PERIPH_FrontPanel.key_bandp && PERIPH_FrontPanel.key_bandp)
	{
		int8_t band = getBandFromFreq(CurrentVFO()->Freq);
		band++;
		if (band >= BANDS_COUNT) band = 0;
		if (band < 0) band = BANDS_COUNT - 1;
		if (band >= 0) TRX_setFrequency(TRX.saved_freq[band]);
		LCD_UpdateQuery.TopButtons = true;
		LCD_UpdateQuery.FreqInfo = true;
	}
	//BAND-
	if (PERIPH_FrontPanel.key_bandn_prev != PERIPH_FrontPanel.key_bandn && PERIPH_FrontPanel.key_bandn == 1)
	{
		int8_t band = getBandFromFreq(CurrentVFO()->Freq);
		band--;
		if (band >= BANDS_COUNT) band = 0;
		if (band < 0) band = BANDS_COUNT - 1;
		if (band >= 0) TRX_setFrequency(TRX.saved_freq[band]);
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
	
	//MCP3008 - 2 (10bit - 1024values)
	mcp3008_value = PERIPH_ReadMCP3008_Value(0, AD2_CS_GPIO_Port, AD2_CS_Pin); // F6
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_f6 = true; 
	else 
		PERIPH_FrontPanel.key_f6 = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(1, AD2_CS_GPIO_Port, AD2_CS_Pin); // F5
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_f5 = true; 
	else 
		PERIPH_FrontPanel.key_f5 = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(2, AD2_CS_GPIO_Port, AD2_CS_Pin); // F4
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_f4 = true; 
	else 
		PERIPH_FrontPanel.key_f4 = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(3, AD2_CS_GPIO_Port, AD2_CS_Pin); // F3
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_f3 = true; 
	else 
		PERIPH_FrontPanel.key_f3 = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(4, AD2_CS_GPIO_Port, AD2_CS_Pin); // F2
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_f2 = true; 
	else 
		PERIPH_FrontPanel.key_f2 = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(5, AD2_CS_GPIO_Port, AD2_CS_Pin); // F1
	if(mcp3008_value < MCP3008_THRESHOLD) 
		PERIPH_FrontPanel.key_f1 = true; 
	else 
		PERIPH_FrontPanel.key_f1 = false;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(6, AD2_CS_GPIO_Port, AD2_CS_Pin); // AF_GAIN
	TRX.Volume=(1023.0f-mcp3008_value)/1023.0f*100.0f;
	
	mcp3008_value = PERIPH_ReadMCP3008_Value(7, AD2_CS_GPIO_Port, AD2_CS_Pin); // SHIFT
	
	PERIPH_FrontPanel.key_f1_prev = PERIPH_FrontPanel.key_f1;
	PERIPH_FrontPanel.key_f2_prev = PERIPH_FrontPanel.key_f2;
	PERIPH_FrontPanel.key_f3_prev = PERIPH_FrontPanel.key_f3;
	PERIPH_FrontPanel.key_f4_prev = PERIPH_FrontPanel.key_f4;
	PERIPH_FrontPanel.key_f5_prev = PERIPH_FrontPanel.key_f5;
	PERIPH_FrontPanel.key_f6_prev = PERIPH_FrontPanel.key_f6;
}

static uint16_t PERIPH_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef* CS_PORT, uint16_t CS_PIN)
{
	uint8_t outData[3] = {0};
	uint8_t inData[3] = {0};
	uint16_t mcp3008_value = 0;
	
	outData[0] = 0x18 | channel;
	PERIPH_SPI_Transmit(outData, inData, 3, CS_PORT, CS_PIN);
	mcp3008_value = 0 | ((inData[1] & 0x3F) << 4) | (inData[2] & 0xF0 >> 4);
	
	return mcp3008_value;
}

bool PERIPH_SPI_Transmit(uint8_t* out_data, uint8_t* in_data, uint8_t count, GPIO_TypeDef* CS_PORT, uint16_t CS_PIN)
{
	if(PERIPH_SPI_busy) return false;
	PERIPH_SPI_busy=true;
	memset(in_data, 0x00, count);
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi2, out_data, in_data, count, 0x1000);
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
	PERIPH_SPI_busy=false;
	return true;
}
