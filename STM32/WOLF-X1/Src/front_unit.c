#include "stm32h7xx_hal.h"
#include "main.h"
#include "front_unit.h"
#include "lcd.h"
#include "trx_manager.h"
#include "settings.h"
#include "system_menu.h"
#include "functions.h"
#include "audio_filters.h"
#include "auto_notch.h"
#include "agc.h"
#include "vad.h"
#include "sd.h"
#include "noise_reduction.h"
#include "rf_unit.h"

int8_t FRONTPANEL_ProcessEncoder1 = 0;
int8_t FRONTPANEL_ProcessEncoder2 = 0;

#ifdef HRDW_MCP3008_1
bool FRONTPanel_MCP3008_1_Enabled = true;
#endif
#ifdef HRDW_MCP3008_2
bool FRONTPanel_MCP3008_2_Enabled = true;
#endif
#ifdef HRDW_MCP3008_3
bool FRONTPanel_MCP3008_3_Enabled = true;
#endif

static void FRONTPANEL_ENCODER_Rotated(float32_t direction);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static uint16_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN);

static void FRONTPANEL_BUTTONHANDLER_MODE_P(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_MODE_N(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_BAND_P(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_BAND_N(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_SAMPLE_N(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_SAMPLE_P(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_WPM(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_KEYER(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_SCAN(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_REC(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_PLAY(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_RIT(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_XIT(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_SPLIT(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_STEP(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_BANDMAP(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_AUTOGAINER(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_UP(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_DOWN(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_FUNC(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_FUNCH(uint32_t parameter);
static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter);
static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_IF(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_VLT(uint32_t parameter);

static int32_t ENCODER_slowler = 0;
static uint32_t ENCODER_AValDeb = 0;
static uint32_t ENCODER2_AValDeb = 0;

#ifdef FRONTPANEL_X1
PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[] = {
	//buttons
	{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_LOCK}, //SB1 MENU
	{.port = 1, .channel = 1, .type = FUNIT_CTRL_PTT, .tres_min = 0, .tres_max = 500}, //SB2 PTT?
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 3, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH}, //SB3 VFO/A=B
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 2, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH}, //SB4 PRE/ATT
	{.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 1, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH}, //SB5 BAND/MODE+
	{.port = 1, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = 500, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH}, //SB6 BAND/MODE-
	{.port = 1, .channel = 6, .type = FUNIT_CTRL_ENC2SW, .tres_min = 0, .tres_max = 500}, //ENC2_SW
	{.port = 1, .channel = 7, .type = FUNIT_CTRL_BUTTON}, //NONE
};

const PERIPH_FrontPanel_FuncButton PERIPH_FrontPanel_FuncButtonsList[FUNCBUTTONS_COUNT] = {
	{.name = "BAND-", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .checkBool = NULL},
	{.name = "BAND+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P, .checkBool = NULL},
	{.name = "PRE", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PRE, .holdHandler = FRONTPANEL_BUTTONHANDLER_PRE, .checkBool = (uint32_t *)&TRX.LNA},
	{.name = "A/B", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_AsB, .checkBool = NULL},
	
	{.name = "MODE-", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .checkBool = NULL},
	{.name = "MODE+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P, .checkBool = NULL},	
	{.name = "TUNE", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .holdHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .checkBool = (uint32_t *)&TRX_Tune},
	{.name = "POWER", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .checkBool = NULL},
	
	{.name = "NOTCH", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_NOTCH, .holdHandler = FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL, .checkBool = (uint32_t *)&TRX.Notch_on_shadow},
	{.name = "ATT", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ATT, .holdHandler = FRONTPANEL_BUTTONHANDLER_ATTHOLD, .checkBool = (uint32_t *)&TRX.ATT},
	{.name = "DRV", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_DRV_ONLY, .holdHandler = FRONTPANEL_BUTTONHANDLER_DRV_ONLY, .checkBool = (uint32_t *)&TRX.ADC_Driver},
	{.name = "PGA", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PGA_ONLY, .holdHandler = FRONTPANEL_BUTTONHANDLER_PGA_ONLY, .checkBool = (uint32_t *)&TRX.ADC_PGA},
	{.name = "AGC", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AGC, .holdHandler = FRONTPANEL_BUTTONHANDLER_AGC, .checkBool = (uint32_t *)&TRX.AGC_shadow},
	
	{.name = "IF", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_IF, .holdHandler = FRONTPANEL_BUTTONHANDLER_IF, .checkBool = NULL},
	{.name = "DNR", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_DNR, .holdHandler = FRONTPANEL_BUTTONHANDLER_DNR, .checkBool = NULL},
	{.name = "FAST", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_FAST, .holdHandler = FRONTPANEL_BUTTONHANDLER_STEP, .checkBool = (uint32_t *)&TRX.Fast},
	
	{.name = "SQL", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SQL, .holdHandler = FRONTPANEL_BUTTONHANDLER_SQUELCH, .checkBool = (uint32_t *)&TRX.SQL_shadow},
	{.name = "BW", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW, .holdHandler = FRONTPANEL_BUTTONHANDLER_BW, .checkBool = NULL},
	{.name = "SCAN", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SCAN, .holdHandler = FRONTPANEL_BUTTONHANDLER_SCAN, .checkBool = (uint32_t *)&TRX_ScanMode},
	{.name = "SERV", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_SERVICES, .holdHandler = FRONTPANEL_BUTTONHANDLER_SERVICES, .checkBool = NULL},
	
	{.name = "WPM", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_WPM, .holdHandler = FRONTPANEL_BUTTONHANDLER_WPM, .checkBool = NULL},
	{.name = "RIT", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_RIT, .holdHandler = FRONTPANEL_BUTTONHANDLER_XIT, .checkBool = (uint32_t *)&TRX.RIT_Enabled},
	{.name = "PLAY", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PLAY, .holdHandler = FRONTPANEL_BUTTONHANDLER_PLAY, .checkBool = (uint32_t *)&SD_PlayInProcess},
	{.name = "REC", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_REC, .holdHandler = FRONTPANEL_BUTTONHANDLER_REC, .checkBool = (uint32_t *)&SD_RecordInProcess},
	
	{.name = "SMPL-", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SAMPLE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_SAMPLE_N, .checkBool = NULL},
	{.name = "SMPL+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SAMPLE_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_SAMPLE_P, .checkBool = NULL},
	{.name = "ZOOM-", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_N, .checkBool = NULL},
	{.name = "ZOOM+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P, .checkBool = NULL},
	
	{.name = "VLT", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_VLT, .holdHandler = FRONTPANEL_BUTTONHANDLER_VLT, .checkBool = (uint32_t *)&TRX_X1_VLT_CUR_Mode},
	{.name = "AUTGN", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AUTOGAINER, .holdHandler = FRONTPANEL_BUTTONHANDLER_AUTOGAINER, .checkBool = (uint32_t *)&TRX.AutoGain},
	{.name = "B=A", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ArB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB, .checkBool = NULL},
	{.name = "LOCK", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_LOCK, .holdHandler = FRONTPANEL_BUTTONHANDLER_LOCK, .checkBool = (uint32_t *)&TRX.Locked},
};
#endif

PERIPH_FrontPanel_Button PERIPH_FrontPanel_TANGENT_MH48[6] = {
	{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 1600, .tres_max = 2200, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_DOWN, .holdHandler = FRONTPANEL_BUTTONHANDLER_DOWN},	 //PTT_SW1 - DOWN
	{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 1000, .tres_max = 1500, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_UP, .holdHandler = FRONTPANEL_BUTTONHANDLER_UP},		 //PTT_SW1 - UP
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 1000, .tres_max = 1500, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .holdHandler = FRONTPANEL_BUTTONHANDLER_TUNE},		 //PTT_SW2 - P1
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 1500, .tres_max = 2100, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB},		 //PTT_SW2 - P2
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 2100, .tres_max = 2600, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N}, //PTT_SW2 - P3
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 2700, .tres_max = 3100, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //PTT_SW2 - P4
};

void FRONTPANEL_ENCODER_checkRotate(void)
{
	static uint32_t ENCstartMeasureTime = 0;
	static int16_t ENCticksInInterval = 0;
	static float32_t ENCAcceleration = 0;
	static uint8_t ENClastClkVal = 0;
	static bool ENCfirst = true;
	uint8_t ENCODER_DTVal = HAL_GPIO_ReadPin(ENC_DT_GPIO_Port, ENC_DT_Pin);
	uint8_t ENCODER_CLKVal = HAL_GPIO_ReadPin(ENC_CLK_GPIO_Port, ENC_CLK_Pin);

	for(uint16_t recheck = 0; recheck < 64; recheck++)
	{
		if(ENCODER_DTVal != HAL_GPIO_ReadPin(ENC_DT_GPIO_Port, ENC_DT_Pin))
			return;
		if(ENCODER_CLKVal != HAL_GPIO_ReadPin(ENC_CLK_GPIO_Port, ENC_CLK_Pin))
			return;
	}
	
	if (ENCfirst)
	{
		ENClastClkVal = ENCODER_CLKVal;
		ENCfirst = false;
	}
	if ((HAL_GetTick() - ENCODER_AValDeb) < CALIBRATE.ENCODER_DEBOUNCE)
		return;

	if (ENClastClkVal != ENCODER_CLKVal)
	{
		if (!CALIBRATE.ENCODER_ON_FALLING || ENCODER_CLKVal == 0)
		{
			if (ENCODER_DTVal != ENCODER_CLKVal)
			{ // If pin A changed first - clockwise rotation
				ENCODER_slowler--;
				if (ENCODER_slowler <= -CALIBRATE.ENCODER_SLOW_RATE)
				{
					//acceleration
					ENCticksInInterval++;
					if ((HAL_GetTick() - ENCstartMeasureTime) > CALIBRATE.ENCODER_ACCELERATION)
					{
						ENCstartMeasureTime = HAL_GetTick();
						ENCAcceleration = (10.0f + ENCticksInInterval - 1.0f) / 10.0f;
						ENCticksInInterval = 0;
					}
					//do rotate
					FRONTPANEL_ENCODER_Rotated(CALIBRATE.ENCODER_INVERT ? ENCAcceleration : -ENCAcceleration);
					ENCODER_slowler = 0;
					if(TRX_ScanMode) {
						TRX_ScanMode = false;
						LCD_UpdateQuery.TopButtons = true;
					}
				}
			}
			else
			{ // otherwise B changed its state first - counterclockwise rotation
				ENCODER_slowler++;
				if (ENCODER_slowler >= CALIBRATE.ENCODER_SLOW_RATE)
				{
					//acceleration
					ENCticksInInterval++;
					if ((HAL_GetTick() - ENCstartMeasureTime) > CALIBRATE.ENCODER_ACCELERATION)
					{
						ENCstartMeasureTime = HAL_GetTick();
						ENCAcceleration = (10.0f + ENCticksInInterval - 1.0f) / 10.0f;
						ENCticksInInterval = 0;
					}
					//do rotate
					FRONTPANEL_ENCODER_Rotated(CALIBRATE.ENCODER_INVERT ? -ENCAcceleration : ENCAcceleration);
					ENCODER_slowler = 0;
					if(TRX_ScanMode) {
						TRX_ScanMode = false;
						LCD_UpdateQuery.TopButtons = true;
					}
				}
			}
		}
		ENCODER_AValDeb = HAL_GetTick();
		ENClastClkVal = ENCODER_CLKVal;
	}
}

void FRONTPANEL_ENCODER2_checkRotate(void)
{
	uint8_t ENCODER2_DTVal = HAL_GPIO_ReadPin(ENC2_DT_GPIO_Port, ENC2_DT_Pin);
	uint8_t ENCODER2_CLKVal = HAL_GPIO_ReadPin(ENC2_CLK_GPIO_Port, ENC2_CLK_Pin);

	if ((HAL_GetTick() - ENCODER2_AValDeb) < CALIBRATE.ENCODER2_DEBOUNCE)
		return;

	if (!CALIBRATE.ENCODER_ON_FALLING || ENCODER2_CLKVal == 0)
	{
		if (ENCODER2_DTVal != ENCODER2_CLKVal)
		{ // If pin A changed first - clockwise rotation
			FRONTPANEL_ProcessEncoder2 = CALIBRATE.ENCODER2_INVERT ? 1 : -1;
		}
		else
		{ // otherwise B changed its state first - counterclockwise rotation
			FRONTPANEL_ProcessEncoder2 = CALIBRATE.ENCODER2_INVERT ? -1 : 1;
		}
		if(TRX_ScanMode) {
			TRX_ScanMode = false;
			LCD_UpdateQuery.TopButtons = true;
		}
	}
	ENCODER2_AValDeb = HAL_GetTick();
}

static void FRONTPANEL_ENCODER_Rotated(float32_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened)
		return;
	if (LCD_systemMenuOpened)
	{
		FRONTPANEL_ProcessEncoder1 = (int8_t)direction;
		return;
	}
	if (fabsf(direction) <= ENCODER_MIN_RATE_ACCELERATION)
		direction = (direction < 0.0f) ? -1.0f : 1.0f;

	if (TRX.RIT_Enabled) {
		TRX_RIT += direction * 10;
		TRX_RIT = (TRX_RIT / 10) * 10;
		
		if(TRX_RIT < -TRX.RIT_INTERVAL)
			TRX_RIT = -TRX.RIT_INTERVAL;
		if(TRX_RIT > TRX.RIT_INTERVAL)
			TRX_RIT = TRX.RIT_INTERVAL;
		LCD_UpdateQuery.StatusInfoGUI = true;
		TRX_XIT = 0;
		TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
		return;
	}
	
	if (TRX.XIT_Enabled) {
		TRX_XIT += direction * 10;
		TRX_XIT = (TRX_XIT / 10) * 10;
		
		if(TRX_XIT < -TRX.XIT_INTERVAL)
			TRX_XIT = -TRX.XIT_INTERVAL;
		if(TRX_XIT > TRX.XIT_INTERVAL)
			TRX_XIT = TRX.XIT_INTERVAL;
		LCD_UpdateQuery.StatusInfoGUI = true;
		TRX_RIT = 0;
		TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
		return;
	}
	
	float64_t newfreq = CurrentVFO->Freq;
	if (TRX.ChannelMode && getBandFromFreq(CurrentVFO->Freq, false) != -1 && BANDS[getBandFromFreq(CurrentVFO->Freq, false)].channelsCount > 0)
	{
		int_fast8_t band = getBandFromFreq(CurrentVFO->Freq, false);
		int_fast8_t channel = getChannelbyFreq(CurrentVFO->Freq, false);
		int_fast8_t new_channel = channel + direction;
		if(new_channel < 0)
			new_channel = BANDS[band].channelsCount - 1;
		if(new_channel >= BANDS[band].channelsCount)
			new_channel = 0;
		
		newfreq = BANDS[band].channels[new_channel].rxFreq;
		TRX.SPLIT_Enabled = (BANDS[band].channels[new_channel].rxFreq != BANDS[band].channels[new_channel].txFreq);
		if(TRX.SPLIT_Enabled)
			TRX_setFrequency(BANDS[band].channels[new_channel].txFreq, SecondaryVFO);
		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
	}
	else if (TRX.Fast)
	{
		float64_t step = TRX.FRQ_FAST_STEP;
		if (CurrentVFO->Mode == TRX_MODE_CW)
					step = step / (float64_t)TRX.FRQ_CW_STEP_DIVIDER;
		
		if (direction == -1.0f)
			newfreq = ceill(newfreq / step) * step;
		if (direction == 1.0f)
			newfreq = floorl(newfreq / step) * step;
		newfreq = newfreq + step * direction;
	}
	else
	{
		float64_t step = TRX.FRQ_STEP;
		if (CurrentVFO->Mode == TRX_MODE_CW)
					step = step / (float64_t)TRX.FRQ_CW_STEP_DIVIDER;
		
		if (direction == -1.0f)
			newfreq = ceill(newfreq / step) * step;
		if (direction == 1.0f)
			newfreq = floorl(newfreq / step) * step;
		newfreq = newfreq + step * direction;
	}
	
	TRX_setFrequency(newfreq, CurrentVFO);
	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_ENCODER2_Rotated(int8_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	TRX_Inactive_Time = 0;
	if (TRX.Locked || LCD_window.opened)
		return;

	if (LCD_systemMenuOpened)
	{
		SYSMENU_eventSecRotateSystemMenu(direction);
		return;
	}

	//NOTCH - default action
	float64_t step = 50;
	if(CurrentVFO->Mode == TRX_MODE_CW)
		step = 10;
	
	if (TRX.ENC2_func_mode_idx == 0) //buttons pager
	{
		if(direction < 0)
			FRONTPANEL_BUTTONHANDLER_LEFT_ARR(0);
		if(direction > 0)
			FRONTPANEL_BUTTONHANDLER_RIGHT_ARR(0);
	}
	if (TRX.ENC2_func_mode_idx == 1) //fast step mode
	{
		float64_t newfreq = (float64_t)CurrentVFO->Freq;
		float64_t step = 0;
		if (TRX.ChannelMode && getBandFromFreq(CurrentVFO->Freq, false) != -1 && BANDS[getBandFromFreq(CurrentVFO->Freq, false)].channelsCount > 0)
		{
			int_fast8_t band = getBandFromFreq(CurrentVFO->Freq, false);
			int_fast8_t channel = getChannelbyFreq(CurrentVFO->Freq, false);
			int_fast8_t new_channel = channel + direction;
			if(new_channel < 0)
				new_channel = BANDS[band].channelsCount - 1;
			if(new_channel >= BANDS[band].channelsCount)
				new_channel = 0;
			
			newfreq = BANDS[band].channels[new_channel].rxFreq;
			TRX.SPLIT_Enabled = (BANDS[band].channels[new_channel].rxFreq != BANDS[band].channels[new_channel].txFreq);
			if(TRX.SPLIT_Enabled)
				TRX_setFrequency(BANDS[band].channels[new_channel].txFreq, SecondaryVFO);
			LCD_UpdateQuery.FreqInfoRedraw = true;
			LCD_UpdateQuery.StatusInfoGUI = true;
			LCD_UpdateQuery.StatusInfoBarRedraw = true;
		}
		else if (TRX.Fast)
		{
			step = TRX.FRQ_ENC_FAST_STEP;
			if (CurrentVFO->Mode == TRX_MODE_WFM)
				step = step * 2.0;
			if (CurrentVFO->Mode == TRX_MODE_CW)
				step = step / (float64_t)TRX.FRQ_CW_STEP_DIVIDER;
			
			if (direction == -1.0f)
				newfreq = ceill(newfreq / step) * step;
			if (direction == 1.0f)
				newfreq = floorl(newfreq / step) * step;
			newfreq = newfreq + step * direction;
		}
		else
		{
			step = TRX.FRQ_ENC_STEP;
			if (CurrentVFO->Mode == TRX_MODE_WFM)
				step = step * 2.0;
			if (CurrentVFO->Mode == TRX_MODE_CW)
				step = step / (float64_t)TRX.FRQ_CW_STEP_DIVIDER;
			
			if (direction == -1.0f)
				newfreq = ceill(newfreq / step) * step;
			if (direction == 1.0f)
				newfreq = floorl(newfreq / step) * step;
			newfreq = newfreq + step * direction;
		}
			
		TRX_setFrequency(newfreq, CurrentVFO);
		LCD_UpdateQuery.FreqInfo = true;
	}
	if (TRX.ENC2_func_mode_idx == 2) //function mode
	{
		if (CurrentVFO->ManualNotchFilter)
		{
			if (CurrentVFO->NotchFC > step && direction < 0)
				CurrentVFO->NotchFC -= step;
			else if (CurrentVFO->NotchFC < CurrentVFO->LPF_RX_Filter_Width && direction > 0)
				CurrentVFO->NotchFC += step;
			
			CurrentVFO->NotchFC = roundf((float64_t)CurrentVFO->NotchFC / step) * step;
			
			if(CurrentVFO->NotchFC < step)
				CurrentVFO->NotchFC = step;
			
			if(CurrentVFO->NotchFC > CurrentVFO->LPF_RX_Filter_Width)
				CurrentVFO->NotchFC = CurrentVFO->LPF_RX_Filter_Width;
			
			LCD_UpdateQuery.StatusInfoGUI = true;
			NeedReinitNotch = true;
			NeedWTFRedraw = true;
		}
		/*else if(CurrentVFO->Mode == TRX_MODE_CW)
		{
			//ENC2 Func mode (WPM)
			TRX.CW_KEYER_WPM += direction;
			if (TRX.CW_KEYER_WPM < 1)
				TRX.CW_KEYER_WPM = 1;
			if (TRX.CW_KEYER_WPM > 200)
				TRX.CW_KEYER_WPM = 200;
			char sbuff[32] = {0};
			sprintf(sbuff, "WPM: %u", TRX.CW_KEYER_WPM);
			LCD_showTooltip(sbuff);
		}*/
		else
		{
			//ENC2 Volume control
			if(direction < 0 && TRX.Volume < 5)
				TRX.Volume = 0;
			if(direction > 0 || TRX.Volume >= 5)
				TRX.Volume += direction * 5;
			if (TRX.Volume > 100)
				TRX.Volume = 100;
			char sbuff[32] = {0};
			sprintf(sbuff, "Vol: %u%%", TRX.Volume);
			LCD_showTooltip(sbuff);
		}
	}
}

void FRONTPANEL_check_ENC2SW(bool state)
{
// check touchpad events
#ifdef HAS_TOUCHPAD
	return;
#endif

	static uint32_t menu_enc2_click_starttime = 0;
	static bool ENC2SW_Last = true;
	static bool ENC2SW_clicked = false;
	static bool ENC2SW_hold_start = false;
	static bool ENC2SW_holded = false;
	ENC2SW_clicked = false;
	ENC2SW_holded = false;

	if (TRX.Locked)
		return;

	bool ENC2SW_AND_TOUCH_Now = state;
	//check hold and click
	if (ENC2SW_Last != ENC2SW_AND_TOUCH_Now)
	{
		ENC2SW_Last = ENC2SW_AND_TOUCH_Now;
		if (!ENC2SW_AND_TOUCH_Now)
		{
			menu_enc2_click_starttime = HAL_GetTick();
			ENC2SW_hold_start = true;
		}
	}
	if (!ENC2SW_AND_TOUCH_Now && ENC2SW_hold_start)
	{
		if ((HAL_GetTick() - menu_enc2_click_starttime) > KEY_HOLD_TIME)
		{
			ENC2SW_holded = true;
			ENC2SW_hold_start = false;
		}
	}
	if (ENC2SW_AND_TOUCH_Now && ENC2SW_hold_start)
	{
		if ((HAL_GetTick() - menu_enc2_click_starttime) > 1)
		{
			ENC2SW_clicked = true;
			ENC2SW_hold_start = false;
		}
	}

	//ENC2 Button hold
	if (ENC2SW_holded)
	{
		FRONTPANEL_ENC2SW_hold_handler(0);
	}

	//ENC2 Button click
	if (ENC2SW_clicked)
	{
		menu_enc2_click_starttime = HAL_GetTick();
		FRONTPANEL_ENC2SW_click_handler(0);
	}
}

static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter)
{
	TRX_Inactive_Time = 0;
	//ENC2 CLICK
	if (!LCD_systemMenuOpened && !LCD_window.opened)
	{
		TRX.ENC2_func_mode_idx++; //enc2 rotary mode
		if(TRX.ENC2_func_mode_idx >= 3)
			TRX.ENC2_func_mode_idx = 0;

		if (TRX.ENC2_func_mode_idx == 0)
		{
			LCD_showTooltip("BUTTONS");
			LCD_UpdateQuery.TopButtonsRedraw = true;
		}
		if (TRX.ENC2_func_mode_idx == 1)
		{
			LCD_showTooltip("FAST STEP");
			LCD_UpdateQuery.TopButtonsRedraw = true;
			LCD_UpdateQuery.FreqInfo = true;
		}
		if (TRX.ENC2_func_mode_idx == 2)
		{
			if(CurrentVFO->Mode == TRX_MODE_CW)
				LCD_showTooltip("SET WPM");
			else if(CurrentVFO->ManualNotchFilter)
				LCD_showTooltip("SET NOTCH");
			else
				LCD_showTooltip("SET VOLUME");
				
			LCD_UpdateQuery.FreqInfo = true;
		}
	}
	else
	{
		if (LCD_systemMenuOpened)
		{
			//navigate in menu
			SYSMENU_eventSecEncoderClickSystemMenu();
		}
	}
}

static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter)
{
	TRX_Inactive_Time = 0;
	FRONTPANEL_BUTTONHANDLER_MENU(0);
}

void FRONTPANEL_Init(void)
{
	uint16_t test_value = 0;
#ifdef HRDW_MCP3008_1
	test_value = FRONTPANEL_ReadMCP3008_Value(0, AD1_CS_GPIO_Port, AD1_CS_Pin);
	if (test_value == 65535)
	{
		FRONTPanel_MCP3008_1_Enabled = false;
		println("[ERR] Frontpanel MCP3008 - 1 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 1 init error (FPGA I2S CLK?)", true);
	}
#endif
#ifdef HRDW_MCP3008_2
	test_value = FRONTPANEL_ReadMCP3008_Value(0, AD2_CS_GPIO_Port, AD2_CS_Pin);
	if (test_value == 65535)
	{
		FRONTPanel_MCP3008_2_Enabled = false;
		println("[ERR] Frontpanel MCP3008 - 2 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 2 init error", true);
	}
#endif
#ifdef HRDW_MCP3008_3
	test_value = FRONTPANEL_ReadMCP3008_Value(0, AD3_CS_GPIO_Port, AD3_CS_Pin);
	if (test_value == 65535)
	{
		FRONTPanel_MCP3008_3_Enabled = false;
		println("[ERR] Frontpanel MCP3008 - 3 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 3 init error", true);
	}
#endif
	FRONTPANEL_Process();
}

void FRONTPANEL_Process(void)
{
	if (LCD_systemMenuOpened && !LCD_busy && FRONTPANEL_ProcessEncoder1 != 0)
	{
		SYSMENU_eventRotateSystemMenu(FRONTPANEL_ProcessEncoder1);
		FRONTPANEL_ProcessEncoder1 = 0;
	}

	if (FRONTPANEL_ProcessEncoder2 != 0)
	{
		FRONTPANEL_ENCODER2_Rotated(FRONTPANEL_ProcessEncoder2);
		FRONTPANEL_ProcessEncoder2 = 0;
	}

	if(SD_USBCardReader)
		return;
	if (SPI_process)
		return;
	SPI_process = true;
	
	static uint32_t fu_debug_lasttime = 0;
	uint16_t buttons_count = sizeof(PERIPH_FrontPanel_Buttons) / sizeof(PERIPH_FrontPanel_Button);
	uint16_t mcp3008_value = 0;

	//process buttons
	for (uint16_t b = 0; b < buttons_count; b++)
	{
		PERIPH_FrontPanel_Button *button = &PERIPH_FrontPanel_Buttons[b];
//check disabled ports
#ifdef HRDW_MCP3008_1
		if (button->port == 1 && !FRONTPanel_MCP3008_1_Enabled)
			continue;
#endif
#ifdef HRDW_MCP3008_2
		if (button->port == 2 && !FRONTPanel_MCP3008_2_Enabled)
			continue;
#endif
#ifdef HRDW_MCP3008_3
		if (button->port == 3 && !FRONTPanel_MCP3008_3_Enabled)
			continue;
#endif
		
//get state from ADC MCP3008 (10bit - 1024values)
#ifdef HRDW_MCP3008_1
		if (button->port == 1)
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(button->channel, AD1_CS_GPIO_Port, AD1_CS_Pin);
		else
#endif
#ifdef HRDW_MCP3008_2
			if (button->port == 2)
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(button->channel, AD2_CS_GPIO_Port, AD2_CS_Pin);
		else
#endif
#ifdef HRDW_MCP3008_3
			if (button->port == 3)
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(button->channel, AD3_CS_GPIO_Port, AD3_CS_Pin);
		else
#endif
			continue;

		if (TRX.Debug_Type == TRX_DEBUG_BUTTONS)
		{
			static uint8_t fu_gebug_lastchannel = 255;
			if ((HAL_GetTick() - fu_debug_lasttime > 500 && fu_gebug_lastchannel != button->channel) || fu_debug_lasttime == 0)
			{
				println("F_UNIT: port ", button->port, " channel ", button->channel, " value ", mcp3008_value);
				fu_gebug_lastchannel = button->channel;
			}
		}

		//TANGENT
		if (button->type == FUNIT_CTRL_TANGENT)
		{
			//Yaesu MH-48
			if(CALIBRATE.TangentType == TANGENT_MH48)
				for (uint16_t tb = 0; tb < (sizeof(PERIPH_FrontPanel_TANGENT_MH48) / sizeof(PERIPH_FrontPanel_Button)); tb++)
					if(button->channel == PERIPH_FrontPanel_TANGENT_MH48[tb].channel)
						FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH48[tb], mcp3008_value);
		}
		else
			FRONTPANEL_CheckButton(button, mcp3008_value); //other buttons / resistors
	}

	if (TRX.Debug_Type == TRX_DEBUG_BUTTONS)
	{
		if (HAL_GetTick() - fu_debug_lasttime > 500)
		{
			println("");
			fu_debug_lasttime = HAL_GetTick();
		}
	}

	SPI_process = false;
}

void FRONTPANEL_CheckButton(PERIPH_FrontPanel_Button *button, uint16_t mcp3008_value)
{
	// AF_GAIN
	if (button->type == FUNIT_CTRL_AF_GAIN)
	{
		TRX.Volume = (uint16_t)(MAX_VOLUME_VALUE - mcp3008_value);
		if (TRX.Volume < 50)
			TRX.Volume = 0;
	}

	// RIT / XIT or IF Gain
	if (button->type == FUNIT_CTRL_RIT_XIT)
	{
		if (TRX.RIT_Enabled)
		{
			int_fast16_t TRX_RIT_old = TRX_RIT;
			TRX_RIT = (int_fast16_t)(((1023.0f - mcp3008_value) * TRX.RIT_INTERVAL * 2 / 1023.0f) - TRX.RIT_INTERVAL);
			if (TRX_RIT_old != TRX_RIT)
			{
				TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
				uint16_t LCD_bw_trapez_stripe_pos_new = LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2;
				LCD_bw_trapez_stripe_pos_new = LCD_bw_trapez_stripe_pos_new + (int16_t)((float32_t)(LAYOUT->BW_TRAPEZ_WIDTH * 0.9f) / 2.0f * ((float32_t)TRX_RIT / (float32_t)TRX.RIT_INTERVAL));
				if (abs(LCD_bw_trapez_stripe_pos_new - LCD_bw_trapez_stripe_pos) > 2)
				{
					LCD_bw_trapez_stripe_pos = LCD_bw_trapez_stripe_pos_new;
					LCD_UpdateQuery.StatusInfoGUI = true;
				}
			}
			TRX_XIT = 0;
		}
		else if (TRX.XIT_Enabled)
		{
			int_fast16_t TRX_XIT_old = TRX_XIT;
			TRX_XIT = (int_fast16_t)(((1023.0f - mcp3008_value) * TRX.XIT_INTERVAL * 2 / 1023.0f) - TRX.XIT_INTERVAL);
			if (TRX_XIT_old != TRX_XIT && !TRX_on_TX)
			{
				TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
				uint16_t LCD_bw_trapez_stripe_pos_new = LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2;
				LCD_bw_trapez_stripe_pos_new = LCD_bw_trapez_stripe_pos_new + (int16_t)((float32_t)(LAYOUT->BW_TRAPEZ_WIDTH * 0.9f) / 2.0f * ((float32_t)TRX_XIT / (float32_t)TRX.XIT_INTERVAL));
				if (abs(LCD_bw_trapez_stripe_pos_new - LCD_bw_trapez_stripe_pos) > 2)
				{
					LCD_bw_trapez_stripe_pos = LCD_bw_trapez_stripe_pos_new;
					LCD_UpdateQuery.StatusInfoGUI = true;
				}
			}
			TRX_RIT = 0;
		}
		else
		{
			TRX_RIT = 0;
			TRX_XIT = 0;
			TRX.IF_Gain = (uint8_t)(0.0f + ((1023.0f - mcp3008_value) * 60.0f / 1023.0f));
		}
	}
	
	//ENC2_SW
	if (button->type == FUNIT_CTRL_ENC2SW)
	{
		bool frontunit_enc2_state_now = true;
		if (mcp3008_value >= button->tres_min && mcp3008_value < button->tres_max)
			frontunit_enc2_state_now = false;
		FRONTPANEL_check_ENC2SW(frontunit_enc2_state_now);
	}
	
	// PTT
	if (button->type == FUNIT_CTRL_PTT)
	{
		static bool frontunit_ptt_state_prev = false;
		bool frontunit_ptt_state_now = false;
		if (mcp3008_value >= button->tres_min && mcp3008_value < button->tres_max)
			frontunit_ptt_state_now = true;
		if (frontunit_ptt_state_prev != frontunit_ptt_state_now)
		{
			TRX_ptt_soft = frontunit_ptt_state_now;
			TRX_ptt_change();
			frontunit_ptt_state_prev = frontunit_ptt_state_now;
		}
	}
	
	//BUTTONS
	if (button->type == FUNIT_CTRL_BUTTON)
	{
		//println(mcp3008_value, " ", button->tres_min, " ", button->tres_max);
		
		//set state
		if (mcp3008_value >= button->tres_min && mcp3008_value < button->tres_max)
		{
			TRX_Inactive_Time = 0;
			button->state = true;
			if (TRX.Debug_Type == TRX_DEBUG_BUTTONS)
				println("Button pressed: port ", button->port, " channel ", button->channel, " value: ", mcp3008_value);
		}
		else
			button->state = false;

		//check state
		if ((button->prev_state != button->state) && button->state)
		{
			button->start_hold_time = HAL_GetTick();
			button->afterhold = false;
		}

		//check hold state
		if ((button->prev_state == button->state) && button->state && ((HAL_GetTick() - button->start_hold_time) > KEY_HOLD_TIME) && !button->afterhold)
		{
			button->afterhold = true;
			if (!LCD_systemMenuOpened || button->work_in_menu)
			{
				if (!LCD_window.opened)
				{
					if (button->holdHandler != NULL)
					{
						WM8731_Beep();
						button->holdHandler(button->parameter);
					}
				}
				else
				{
					LCD_closeWindow();
				}
			}
		}

		//check click state
		if ((button->prev_state != button->state) && !button->state && ((HAL_GetTick() - button->start_hold_time) < KEY_HOLD_TIME) && !button->afterhold && !TRX.Locked)
		{
			if (!LCD_systemMenuOpened || button->work_in_menu)
			{
				if (!LCD_window.opened)
				{
					if (button->clickHandler != NULL)
					{
						WM8731_Beep();
						button->clickHandler(button->parameter);
					}
				}
				else
				{
					LCD_closeWindow();
				}
			}
		}

		//save prev state
		button->prev_state = button->state;
	}
	
	//DEBUG BUTTONS
	if (button->type == FUNIT_CTRL_BUTTON_DEBUG)
	{
		char str[64] = {0};
		sprintf(str, "%d: %d       ", button->channel, mcp3008_value);
		LCDDriver_printText(str, 10, 200 + button->channel * 20, COLOR_RED, BG_COLOR, 2);
	}
}

void FRONTPANEL_BUTTONHANDLER_DOUBLE(uint32_t parameter)
{
	TRX.Dual_RX = !TRX.Dual_RX;
	FPGA_NeedSendParams = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedReinitAudioFilters = true;
}

void FRONTPANEL_BUTTONHANDLER_DOUBLEMODE(uint32_t parameter)
{
	if (!TRX.Dual_RX)
		return;

	if (TRX.Dual_RX_Type == VFO_A_AND_B)
		TRX.Dual_RX_Type = VFO_A_PLUS_B;
	else
		TRX.Dual_RX_Type = VFO_A_AND_B;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedReinitAudioFilters = true;
}

void FRONTPANEL_BUTTONHANDLER_AsB(uint32_t parameter) // A/B
{
	//TX block
	if(TRX_on_TX)
		return;
	
	TRX_TemporaryMute();
	
	TRX.selected_vfo = !TRX.selected_vfo;
	//VFO settings
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
	TRX.SAMPLERATE_MAIN = TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE;
	TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	CurrentVFO->FM_SQL_threshold_dbm = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dbm;
	CurrentVFO->DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	CurrentVFO->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	CurrentVFO->SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_dbm_shadow = CurrentVFO->FM_SQL_threshold_dbm;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.BottomButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	NeedSaveSettings = true;
	NeedReinitAudioFiltersClean = true;
	NeedReinitAudioFilters = true;
	resetVAD();
	FFT_Init();
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
}

void FRONTPANEL_BUTTONHANDLER_TUNE(uint32_t parameter)
{
	APROC_TX_tune_power = 0.0f;
	ATU_TunePowerStabilized = false;
	TRX_PWR_Forward_SMOOTHED = 0;
	TRX_PWR_Backward_SMOOTHED = 0;
	TRX_SWR_SMOOTHED = 1.0f;
	RF_UNIT_ATU_Invalidate();
	
	TRX_Tune = !TRX_Tune;
	TRX_ptt_hard = TRX_Tune;
	
	if(TRX_TX_Disabled(CurrentVFO->Freq))
	{
		TRX_Tune = false;
		TRX_ptt_hard = false;
	}
	
	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	TRX_Restart_Mode();
}

void FRONTPANEL_BUTTONHANDLER_PRE(uint32_t parameter)
{
	TRX.LNA = !TRX.LNA;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].LNA = TRX.LNA;
	}
	LCD_UpdateQuery.TopButtons = true;
	FPGA_NeedSendParams = true;
	NeedSaveSettings = true;
	resetVAD();
}

static void FRONTPANEL_BUTTONHANDLER_VLT(uint32_t parameter)
{
	TRX_X1_VLT_CUR_Mode = !TRX_X1_VLT_CUR_Mode;
	LCD_UpdateQuery.StatusInfoBar = true;
	LCD_UpdateQuery.TopButtons = true;
}

void FRONTPANEL_BUTTONHANDLER_ATT(uint32_t parameter)
{
	TRX.ATT = !TRX.ATT;
	if(TRX.ATT && TRX.ATT_DB < 1.0f)
		TRX.ATT_DB = TRX.ATT_STEP;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_ATTHOLD(uint32_t parameter)
{
	TRX.ATT_DB += TRX.ATT_STEP;
	if (TRX.ATT_DB > 31.0f)
		TRX.ATT_DB = TRX.ATT_STEP;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_PGA(uint32_t parameter)
{
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
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
		TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
	}
	LCD_UpdateQuery.TopButtons = true;
	FPGA_NeedSendParams = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_PGA_ONLY(uint32_t parameter)
{
	TRX.ADC_PGA = !TRX.ADC_PGA;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
	LCD_UpdateQuery.TopButtons = true;
	FPGA_NeedSendParams = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_DRV_ONLY(uint32_t parameter)
{
	TRX.ADC_Driver = !TRX.ADC_Driver;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
	LCD_UpdateQuery.TopButtons = true;
	FPGA_NeedSendParams = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_FAST(uint32_t parameter)
{
	TRX.Fast = !TRX.Fast;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_MODE_P(uint32_t parameter)
{
	int8_t mode = (int8_t)CurrentVFO->Mode;
	if (mode == TRX_MODE_LSB)
		mode = TRX_MODE_USB;
	else if (mode == TRX_MODE_USB)
		mode = TRX_MODE_LSB;
	else if (mode == TRX_MODE_CW)
		mode = TRX_MODE_CW;
	else if (mode == TRX_MODE_NFM)
		mode = TRX_MODE_WFM;
	else if (mode == TRX_MODE_WFM)
		mode = TRX_MODE_NFM;
	else if (mode == TRX_MODE_DIGI_L)
		mode = TRX_MODE_DIGI_U;
	else if (mode == TRX_MODE_DIGI_U)
		mode = TRX_MODE_RTTY;
	else if (mode == TRX_MODE_RTTY)
		mode = TRX_MODE_DIGI_L;
	else if (mode == TRX_MODE_AM)
		mode = TRX_MODE_SAM;
	else if (mode == TRX_MODE_SAM)
		mode = TRX_MODE_IQ;
	else if (mode == TRX_MODE_IQ)
	{
		mode = TRX_MODE_LOOPBACK;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}
	else if (mode == TRX_MODE_LOOPBACK)
	{
		mode = TRX_MODE_AM;
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	}

	TRX_setMode((uint8_t)mode, CurrentVFO);
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
}

static void FRONTPANEL_BUTTONHANDLER_MODE_N(uint32_t parameter)
{
	int8_t mode = (int8_t)CurrentVFO->Mode;
	if (mode == TRX_MODE_LOOPBACK)
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	if (mode == TRX_MODE_LSB)
		mode = TRX_MODE_CW;
	else if (mode == TRX_MODE_USB)
		mode = TRX_MODE_CW;
	else if (mode == TRX_MODE_CW)
		mode = TRX_MODE_DIGI_U;
	else if (mode == TRX_MODE_DIGI_L || mode == TRX_MODE_DIGI_U || mode == TRX_MODE_RTTY)
		mode = TRX_MODE_NFM;
	else if (mode == TRX_MODE_NFM || mode == TRX_MODE_WFM)
		mode = TRX_MODE_AM;
	else
	{
		if (CurrentVFO->Freq < 10000000)
			mode = TRX_MODE_LSB;
		else
			mode = TRX_MODE_USB;
	}

	TRX_setMode((uint8_t)mode, CurrentVFO);
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
}

static void FRONTPANEL_BUTTONHANDLER_BAND_P(uint32_t parameter)
{
	//TX block
	if(TRX_on_TX)
		return;
	
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	band++;
	if (band >= BANDS_COUNT)
		band = 0;
	while (!BANDS[band].selectable)
	{
		band++;
		if (band >= BANDS_COUNT)
			band = 0;
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO);
	if(TRX.SAMPLERATE_MAIN != TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE)
	{
		TRX.SAMPLERATE_MAIN = TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE;
		FFT_Init();
		NeedReinitAudioFilters = true;
	}
	TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	CurrentVFO->FM_SQL_threshold_dbm = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dbm;
	CurrentVFO->DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	CurrentVFO->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	CurrentVFO->SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_dbm_shadow = CurrentVFO->FM_SQL_threshold_dbm;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	resetVAD();
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
	TRX_DXCluster_UpdateTime = 0;
}

static void FRONTPANEL_BUTTONHANDLER_BAND_N(uint32_t parameter)
{
	//TX block
	if(TRX_on_TX)
		return;
	
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	band--;
	if (band < 0)
		band = BANDS_COUNT - 1;
	while (!BANDS[band].selectable)
	{
		band--;
		if (band < 0)
			band = BANDS_COUNT - 1;
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO);
	if(TRX.SAMPLERATE_MAIN != TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE)
	{
		TRX.SAMPLERATE_MAIN = TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE;
		FFT_Init();
		NeedReinitAudioFilters = true;
	}
	TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	CurrentVFO->DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	CurrentVFO->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	CurrentVFO->SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	CurrentVFO->FM_SQL_threshold_dbm = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dbm;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_dbm_shadow = CurrentVFO->FM_SQL_threshold_dbm;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	resetVAD();
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
	TRX_DXCluster_UpdateTime = 0;
}

void FRONTPANEL_BUTTONHANDLER_RF_POWER(uint32_t parameter)
{
#ifdef HAS_TOUCHPAD
	LCD_showRFPowerWindow();
#else
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_TRX_RFPOWER_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
#endif
}

void FRONTPANEL_BUTTONHANDLER_AGC(uint32_t parameter)
{
	CurrentVFO->AGC = !CurrentVFO->AGC;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].AGC = CurrentVFO->AGC;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_AGC_SPEED(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_AUDIO_AGC_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void FRONTPANEL_BUTTONHANDLER_IF(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_AUDIO_IF_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void FRONTPANEL_BUTTONHANDLER_SQUELCH(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_AUDIO_SQUELCH_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

static void FRONTPANEL_BUTTONHANDLER_WPM(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_CW_WPM_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

static void FRONTPANEL_BUTTONHANDLER_KEYER(uint32_t parameter)
{
	TRX.CW_KEYER = !TRX.CW_KEYER;
	if (TRX.CW_KEYER)
		LCD_showTooltip("KEYER ON");
	else
		LCD_showTooltip("KEYER OFF");
}

static void FRONTPANEL_BUTTONHANDLER_STEP(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_TRX_STEP_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void FRONTPANEL_BUTTONHANDLER_DNR(uint32_t parameter)
{
	TRX_TemporaryMute();
	InitNoiseReduction();
	if (CurrentVFO->DNR_Type == 0)
		CurrentVFO->DNR_Type = 1;
	else if (CurrentVFO->DNR_Type == 1)
		CurrentVFO->DNR_Type = 2;
	else
		CurrentVFO->DNR_Type = 0;

	TRX.DNR_shadow = CurrentVFO->DNR_Type;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].DNR_Type = CurrentVFO->DNR_Type;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_NB(uint32_t parameter)
{
	TRX.NOISE_BLANKER = !TRX.NOISE_BLANKER;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_BW(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		if (CurrentVFO->Mode == TRX_MODE_CW)
			SYSMENU_AUDIO_BW_CW_HOTKEY();
		else if (CurrentVFO->Mode == TRX_MODE_NFM || CurrentVFO->Mode == TRX_MODE_WFM)
			SYSMENU_AUDIO_BW_FM_HOTKEY();
		else if (CurrentVFO->Mode == TRX_MODE_AM ||CurrentVFO->Mode == TRX_MODE_SAM)
			SYSMENU_AUDIO_BW_AM_HOTKEY();
		else
			SYSMENU_AUDIO_BW_SSB_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void FRONTPANEL_BUTTONHANDLER_HPF(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_AUDIO_HPF_SSB_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void FRONTPANEL_BUTTONHANDLER_ArB(uint32_t parameter) //A=B
{
	if (TRX.selected_vfo)
		dma_memcpy(&TRX.VFO_A, &TRX.VFO_B, sizeof TRX.VFO_B);
	else
		dma_memcpy(&TRX.VFO_B, &TRX.VFO_A, sizeof TRX.VFO_B);

	LCD_showTooltip("VFO COPIED");

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_NOTCH(uint32_t parameter)
{
	TRX_TemporaryMute();

	if (CurrentVFO->NotchFC > CurrentVFO->LPF_RX_Filter_Width)
	{
		CurrentVFO->NotchFC = CurrentVFO->LPF_RX_Filter_Width;
		NeedReinitNotch = true;
	}
	CurrentVFO->ManualNotchFilter = false;

	if (!CurrentVFO->AutoNotchFilter)
	{
		InitAutoNotchReduction();
		CurrentVFO->AutoNotchFilter = true;
	}
	else
		CurrentVFO->AutoNotchFilter = false;

	LCD_UpdateQuery.StatusInfoGUI = true;
	LCD_UpdateQuery.TopButtons = true;
	NeedWTFRedraw = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL(uint32_t parameter)
{
	if (CurrentVFO->NotchFC > CurrentVFO->LPF_RX_Filter_Width)
		CurrentVFO->NotchFC = CurrentVFO->LPF_RX_Filter_Width;
	CurrentVFO->AutoNotchFilter = false;
	if (!CurrentVFO->ManualNotchFilter)
		CurrentVFO->ManualNotchFilter = true;
	else
		CurrentVFO->ManualNotchFilter = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedReinitNotch = true;
	NeedWTFRedraw = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_RIT(uint32_t parameter)
{
	TRX.RIT_Enabled = !TRX.RIT_Enabled;
	TRX_RIT = 0;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	TRX_setFrequency(SecondaryVFO->Freq, SecondaryVFO);
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_XIT(uint32_t parameter)
{
	TRX.XIT_Enabled = !TRX.XIT_Enabled;
	TRX_XIT = 0;
	TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
	TRX_setFrequency(SecondaryVFO->Freq, SecondaryVFO);
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_SPLIT(uint32_t parameter)
{
	TRX.SPLIT_Enabled = !TRX.SPLIT_Enabled;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_LOCK(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
		TRX.Locked = !TRX.Locked;
	else
	{
		SYSMENU_hiddenmenu_enabled = true;
		LCD_redraw(false);
	}
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoBar = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_MENU(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
		LCD_systemMenuOpened = true;
	else
		SYSMENU_eventCloseSystemMenu();
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_MENUHOLD(uint32_t parameter)
{
	if (LCD_systemMenuOpened)
	{
		SYSMENU_hiddenmenu_enabled = true;
		LCD_redraw(false);
		return;
	}
	else
	{
		FRONTPANEL_BUTTONHANDLER_MENU(parameter);
	}
}

void FRONTPANEL_BUTTONHANDLER_MUTE(uint32_t parameter)
{
	TRX_Mute = !TRX_Mute;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_BANDMAP(uint32_t parameter)
{
	TRX.BandMapEnabled = !TRX.BandMapEnabled;

	if (TRX.BandMapEnabled)
		LCD_showTooltip("BANDMAP ON");
	else
		LCD_showTooltip("BANDMAP OFF");

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_AUTOGAINER(uint32_t parameter)
{
	TRX.AutoGain = !TRX.AutoGain;

	if (TRX.AutoGain)
		LCD_showTooltip("AUTOGAIN ON");
	else
		LCD_showTooltip("AUTOGAIN OFF");

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static uint16_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN)
{
	uint8_t outData[3] = {0};
	uint8_t inData[3] = {0};
	uint16_t mcp3008_value = 0;

	outData[0] = 0x18 | channel;
	bool res = SPI_Transmit(outData, inData, 3, CS_PORT, CS_PIN, false, SPI_FRONT_UNIT_PRESCALER, false);
	if (res == false)
		return 65535;
	mcp3008_value = (uint16_t)(0 | ((inData[1] & 0x3F) << 4) | (inData[2] & 0xF0 >> 4));

	return mcp3008_value;
}

void FRONTPANEL_BUTTONHANDLER_SERVICES(uint32_t parameter)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_HANDL_SERVICESMENU(1);
	}
	else
	{
		SYSMENU_eventCloseSystemMenu();
	}
}

void FRONTPANEL_BUTTONHANDLER_SQL(uint32_t parameter)
{
	CurrentVFO->SQL = !CurrentVFO->SQL;
	TRX.SQL_shadow = CurrentVFO->SQL;
	
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].SQL = CurrentVFO->SQL;
	
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_SCAN(uint32_t parameter)
{
	TRX_ScanMode = !TRX_ScanMode;
	LCD_UpdateQuery.TopButtons = true;
}

static void FRONTPANEL_BUTTONHANDLER_PLAY(uint32_t parameter)
{
	if (SD_RecordInProcess)
		SD_NeedStopRecord = true;
	
	//go tx
	TRX_ptt_soft = true;
	TRX_ptt_change();
	
	//start play cq message
	SD_PlayCQMessageInProcess = true;
	dma_memset(SD_workbuffer_A, 0, sizeof(SD_workbuffer_A));
	strcat((char*)SD_workbuffer_A, SD_CQ_MESSAGE_FILE);
	SD_doCommand(SDCOMM_START_PLAY, false);
}

static void FRONTPANEL_BUTTONHANDLER_REC(uint32_t parameter)
{
	if (!SD_RecordInProcess)
		SD_doCommand(SDCOMM_START_RECORD, false);
	else
		SD_NeedStopRecord = true;
}

static void FRONTPANEL_BUTTONHANDLER_FUNC(uint32_t parameter)
{
	if (!TRX.Locked) //LOCK BUTTON
		if (!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].work_in_menu)
			PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].clickHandler(0);
}

static void FRONTPANEL_BUTTONHANDLER_FUNCH(uint32_t parameter)
{
	if (parameter == 7 && LCD_systemMenuOpened)
	{
		SYSMENU_hiddenmenu_enabled = true;
		LCD_redraw(false);
	}
	else if (!TRX.Locked || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].holdHandler == FRONTPANEL_BUTTONHANDLER_LOCK) //LOCK BUTTON
		if (!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].work_in_menu)
			PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[TRX.FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].holdHandler(0);
}

static void FRONTPANEL_BUTTONHANDLER_UP(uint32_t parameter)
{
	/*uint32_t newfreq = CurrentVFO->Freq + 500;
	newfreq = newfreq / 500 * 500;
	TRX_setFrequency(newfreq, CurrentVFO);
	LCD_UpdateQuery.FreqInfo = true;*/
	
	int8_t direction = 1;
	if(direction < 0 && TRX.Volume < 5)
		TRX.Volume = 0;
	if(direction > 0 || TRX.Volume >= 5)
		TRX.Volume += direction * 5;
	if (TRX.Volume > 100)
		TRX.Volume = 100;
	char sbuff[32] = {0};
	sprintf(sbuff, "Vol: %u%%", TRX.Volume);
	LCD_showTooltip(sbuff);
}

static void FRONTPANEL_BUTTONHANDLER_DOWN(uint32_t parameter)
{
	/*uint32_t newfreq = CurrentVFO->Freq - 500;
	newfreq = newfreq / 500 * 500;
	TRX_setFrequency(newfreq, CurrentVFO);
	LCD_UpdateQuery.FreqInfo = true;*/
	
	int8_t direction = -1;
	if(direction < 0 && TRX.Volume < 5)
		TRX.Volume = 0;
	if(direction > 0 || TRX.Volume >= 5)
		TRX.Volume += direction * 5;
	if (TRX.Volume > 100)
		TRX.Volume = 100;
	char sbuff[32] = {0};
	sprintf(sbuff, "Vol: %u%%", TRX.Volume);
	LCD_showTooltip(sbuff);
}

void FRONTPANEL_BUTTONHANDLER_SET_CUR_VFO_BAND(uint32_t parameter)
{
	int8_t band = parameter;
	if (band >= BANDS_COUNT)
		band = 0;

	//manual freq enter
	if(LCD_window.opened && TRX.BANDS_SAVED_SETTINGS[band].Freq == CurrentVFO->Freq)
	{
		TRX_Temporary_Stop_BandMap = false;
		resetVAD();
		if(TRX_ScanMode) {
			TRX_ScanMode = false;
			LCD_UpdateQuery.TopButtons = true;
		}
		LCD_closeWindow();
		LCD_redraw(true);
		LCD_showManualFreqWindow(false);
		return;
	}
	//
	
	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO);
	if(TRX.SAMPLERATE_MAIN != TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE)
	{
		TRX.SAMPLERATE_MAIN = TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE;
		FFT_Init();
		NeedReinitAudioFilters = true;
	}
	TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	CurrentVFO->SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	CurrentVFO->FM_SQL_threshold_dbm = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dbm;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_dbm_shadow = CurrentVFO->FM_SQL_threshold_dbm;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	CurrentVFO->DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	CurrentVFO->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	
	resetVAD();
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
	if(LCD_window.opened)
		LCD_closeWindow();
	TRX_DXCluster_UpdateTime = 0;
}

void FRONTPANEL_BUTTONHANDLER_SET_VFOA_BAND(uint32_t parameter)
{
	int8_t band = parameter;
	if (band >= BANDS_COUNT)
		band = 0;

	//manual freq enter
	if(LCD_window.opened && TRX.BANDS_SAVED_SETTINGS[band].Freq == TRX.VFO_A.Freq)
	{
		TRX_Temporary_Stop_BandMap = false;
		resetVAD();
		if(TRX_ScanMode) {
			TRX_ScanMode = false;
			LCD_UpdateQuery.TopButtons = true;
		}
		LCD_closeWindow();
		LCD_redraw(true);
		LCD_showManualFreqWindow(false);
		return;
	}
	//
	
	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, &TRX.VFO_A);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, &TRX.VFO_A);
	if(TRX.SAMPLERATE_MAIN != TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE)
	{
		TRX.SAMPLERATE_MAIN = TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE;
		FFT_Init();
		NeedReinitAudioFilters = true;
	}
	TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.VFO_A.SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	TRX.VFO_A.FM_SQL_threshold_dbm = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dbm;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_dbm_shadow = CurrentVFO->FM_SQL_threshold_dbm;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	TRX.VFO_A.DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	TRX.VFO_A.AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	
	resetVAD();
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
	if(LCD_window.opened)
		LCD_closeWindow();
	TRX_DXCluster_UpdateTime = 0;
}

void FRONTPANEL_BUTTONHANDLER_SET_VFOB_BAND(uint32_t parameter)
{
	int8_t band = parameter;
	if (band >= BANDS_COUNT)
		band = 0;

	//manual freq enter
	if(TRX.BANDS_SAVED_SETTINGS[band].Freq == TRX.VFO_B.Freq)
	{
		TRX_Temporary_Stop_BandMap = false;
		resetVAD();
		if(TRX_ScanMode) {
			TRX_ScanMode = false;
			LCD_UpdateQuery.TopButtons = true;
		}
		LCD_closeWindow();
		LCD_redraw(true);
		LCD_showManualFreqWindow(true);
		return;
	}
	//
	
	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, &TRX.VFO_B);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, &TRX.VFO_B);
	TRX.VFO_B.FM_SQL_threshold_dbm = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dbm;
	TRX.VFO_B.DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	TRX.VFO_B.AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX.VFO_B.SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	
	resetVAD();
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
	LCD_closeWindow();
	TRX_DXCluster_UpdateTime = 0;
}

void FRONTPANEL_BUTTONHANDLER_SETMODE(uint32_t parameter)
{
	int8_t mode = parameter;
	TRX_setMode((uint8_t)mode, &TRX.VFO_A);
	int8_t band = getBandFromFreq(TRX.VFO_A.Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	if(CurrentVFO->NotchFC > CurrentVFO->LPF_RX_Filter_Width)
	{
		CurrentVFO->NotchFC = CurrentVFO->LPF_RX_Filter_Width;
		NeedReinitNotch = true;
	}
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
	LCD_closeWindow();
}

void FRONTPANEL_BUTTONHANDLER_SETSECMODE(uint32_t parameter)
{
	int8_t mode = parameter;
	TRX_setMode((uint8_t)mode, &TRX.VFO_B);
	int8_t band = getBandFromFreq(TRX.VFO_B.Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	if(SecondaryVFO->NotchFC > SecondaryVFO->LPF_RX_Filter_Width)
	{
		SecondaryVFO->NotchFC = SecondaryVFO->LPF_RX_Filter_Width;
		NeedReinitNotch = true;
	}
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
	LCD_closeWindow();
}

void FRONTPANEL_BUTTONHANDLER_SET_RX_BW(uint32_t parameter)
{
	if (CurrentVFO->Mode == TRX_MODE_CW)
		TRX.CW_LPF_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB)
		TRX.SSB_LPF_RX_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY)
		TRX.DIGI_LPF_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM)
		TRX.AM_LPF_RX_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_NFM)
		TRX.FM_LPF_RX_Filter = parameter;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);

	LCD_closeWindow();
}

void FRONTPANEL_BUTTONHANDLER_SET_TX_BW(uint32_t parameter)
{
	if (CurrentVFO->Mode == TRX_MODE_CW)
		TRX.CW_LPF_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB)
		TRX.SSB_LPF_TX_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U || CurrentVFO->Mode == TRX_MODE_RTTY)
		TRX.DIGI_LPF_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_AM || CurrentVFO->Mode == TRX_MODE_SAM)
		TRX.AM_LPF_TX_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_NFM)
		TRX.FM_LPF_TX_Filter = parameter;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);

	LCD_closeWindow();
}

void FRONTPANEL_BUTTONHANDLER_SETRF_POWER(uint32_t parameter)
{
	TRX.RF_Power = parameter;
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
	LCD_closeWindow();
}

void FRONTPANEL_BUTTONHANDLER_SET_ATT_DB(uint32_t parameter)
{
	TRX.ATT_DB = parameter;
	
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
	
	LCD_closeWindow();
}

void FRONTPANEL_BUTTONHANDLER_LEFT_ARR(uint32_t parameter)
{
	if (TRX.FRONTPANEL_funcbuttons_page == 0)
		TRX.FRONTPANEL_funcbuttons_page = (FUNCBUTTONS_PAGES - 1);
	else
		TRX.FRONTPANEL_funcbuttons_page--;

	LCD_UpdateQuery.BottomButtons = true;
	LCD_UpdateQuery.TopButtons = true;
}

void FRONTPANEL_BUTTONHANDLER_RIGHT_ARR(uint32_t parameter)
{
	if (TRX.FRONTPANEL_funcbuttons_page >= (FUNCBUTTONS_PAGES - 1))
		TRX.FRONTPANEL_funcbuttons_page = 0;
	else
		TRX.FRONTPANEL_funcbuttons_page++;

	LCD_UpdateQuery.BottomButtons = true;
	LCD_UpdateQuery.TopButtons = true;
}

static void FRONTPANEL_BUTTONHANDLER_SAMPLE_N(uint32_t parameter)
{
	if(CurrentVFO->Mode == TRX_MODE_WFM)
	{
		if (TRX.SAMPLERATE_FM > 0)
			TRX.SAMPLERATE_FM -= 1;
	}
	else
	{
		if (TRX.SAMPLERATE_MAIN > 0)
			TRX.SAMPLERATE_MAIN -= 1;
		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE = TRX.SAMPLERATE_MAIN;
	}
	
	FFT_Init();
	NeedReinitAudioFilters = true;
	LCD_UpdateQuery.StatusInfoBar = true;
}

static void FRONTPANEL_BUTTONHANDLER_SAMPLE_P(uint32_t parameter)
{
	if(CurrentVFO->Mode == TRX_MODE_WFM)
	{
		if (TRX.SAMPLERATE_FM < 3)
			TRX.SAMPLERATE_FM += 1;
	}
	else
	{
		if (TRX.SAMPLERATE_MAIN < 3)
			TRX.SAMPLERATE_MAIN += 1;
		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE = TRX.SAMPLERATE_MAIN;
	}
	
	FFT_Init();
	NeedReinitAudioFilters = true;
	LCD_UpdateQuery.StatusInfoBar = true;
}

void FRONTPANEL_BUTTONHANDLER_ZOOM_N(uint32_t parameter)
{
	if(CurrentVFO->Mode == TRX_MODE_CW)
	{
		if (TRX.FFT_ZoomCW == 2)
			TRX.FFT_ZoomCW = 1;
		else if (TRX.FFT_ZoomCW == 4)
			TRX.FFT_ZoomCW = 2;
		else if (TRX.FFT_ZoomCW == 8)
			TRX.FFT_ZoomCW = 4;
		else if (TRX.FFT_ZoomCW == 16)
			TRX.FFT_ZoomCW = 8;
	}
	else
	{
		if (TRX.FFT_Zoom == 2)
			TRX.FFT_Zoom = 1;
		else if (TRX.FFT_Zoom == 4)
			TRX.FFT_Zoom = 2;
		else if (TRX.FFT_Zoom == 8)
			TRX.FFT_Zoom = 4;
		else if (TRX.FFT_Zoom == 16)
			TRX.FFT_Zoom = 8;
	}
	
	FFT_Init();
	LCD_UpdateQuery.StatusInfoBar = true;
}

void FRONTPANEL_BUTTONHANDLER_ZOOM_P(uint32_t parameter)
{
	if(CurrentVFO->Mode == TRX_MODE_CW)
	{
		if (TRX.FFT_ZoomCW == 1)
			TRX.FFT_ZoomCW = 2;
		else if (TRX.FFT_ZoomCW == 2)
			TRX.FFT_ZoomCW = 4;
		else if (TRX.FFT_ZoomCW == 4)
			TRX.FFT_ZoomCW = 8;
		else if (TRX.FFT_ZoomCW == 8)
			TRX.FFT_ZoomCW = 16;
	}
	else
	{
		if (TRX.FFT_Zoom == 1)
			TRX.FFT_Zoom = 2;
		else if (TRX.FFT_Zoom == 2)
			TRX.FFT_Zoom = 4;
		else if (TRX.FFT_Zoom == 4)
			TRX.FFT_Zoom = 8;
		else if (TRX.FFT_Zoom == 8)
			TRX.FFT_Zoom = 16;
	}
	
	FFT_Init();
	LCD_UpdateQuery.StatusInfoBar = true;
}

void FRONTPANEL_SelectMemoryChannelsButtonHandler(uint32_t parameter)
{
	int8_t channel = parameter;
	if (channel >= MEMORY_CHANNELS_COUNT)
		channel = 0;
	
	TRX_setFrequency(CALIBRATE.MEMORY_CHANNELS[channel].Freq, CurrentVFO);
	TRX_setMode(CALIBRATE.MEMORY_CHANNELS[channel].Mode, CurrentVFO);
	if(TRX.SAMPLERATE_MAIN != CALIBRATE.MEMORY_CHANNELS[channel].SAMPLERATE)
	{
		TRX.SAMPLERATE_MAIN = CALIBRATE.MEMORY_CHANNELS[channel].SAMPLERATE;
		FFT_Init();
		NeedReinitAudioFilters = true;
	}
	TRX.LNA = CALIBRATE.MEMORY_CHANNELS[channel].LNA;
	TRX.ATT = CALIBRATE.MEMORY_CHANNELS[channel].ATT;
	TRX.ATT_DB = CALIBRATE.MEMORY_CHANNELS[channel].ATT_DB;
	TRX.ADC_Driver = CALIBRATE.MEMORY_CHANNELS[channel].ADC_Driver;
	CurrentVFO->SQL = CALIBRATE.MEMORY_CHANNELS[channel].SQL;
	CurrentVFO->FM_SQL_threshold_dbm = CALIBRATE.MEMORY_CHANNELS[channel].FM_SQL_threshold_dbm;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_dbm_shadow = CurrentVFO->FM_SQL_threshold_dbm;
	TRX.ADC_PGA = CALIBRATE.MEMORY_CHANNELS[channel].ADC_PGA;
	CurrentVFO->DNR_Type = CALIBRATE.MEMORY_CHANNELS[channel].DNR_Type;
	CurrentVFO->AGC = CALIBRATE.MEMORY_CHANNELS[channel].AGC;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	
	resetVAD();
	if(TRX_ScanMode) {
		TRX_ScanMode = false;
		LCD_UpdateQuery.TopButtons = true;
	}
	LCD_closeWindow();
	TRX_DXCluster_UpdateTime = 0;
}

void FRONTPANEL_SaveMemoryChannelsButtonHandler(uint32_t parameter)
{
	int8_t channel = parameter;
	if (channel >= MEMORY_CHANNELS_COUNT)
		channel = 0;
	
	CALIBRATE.MEMORY_CHANNELS[channel].Freq = CurrentVFO->Freq;
	CALIBRATE.MEMORY_CHANNELS[channel].Mode = CurrentVFO->Mode;
	CALIBRATE.MEMORY_CHANNELS[channel].SAMPLERATE = TRX.SAMPLERATE_MAIN;
	CALIBRATE.MEMORY_CHANNELS[channel].LNA = TRX.LNA;
	CALIBRATE.MEMORY_CHANNELS[channel].ATT = TRX.ATT;
	CALIBRATE.MEMORY_CHANNELS[channel].ATT_DB = TRX.ATT_DB;
	CALIBRATE.MEMORY_CHANNELS[channel].ADC_Driver = TRX.ADC_Driver;
	CALIBRATE.MEMORY_CHANNELS[channel].SQL = CurrentVFO->SQL;
	CALIBRATE.MEMORY_CHANNELS[channel].FM_SQL_threshold_dbm = CurrentVFO->FM_SQL_threshold_dbm;
	CALIBRATE.MEMORY_CHANNELS[channel].ADC_PGA = TRX.ADC_PGA;
	CALIBRATE.MEMORY_CHANNELS[channel].DNR_Type = CurrentVFO->DNR_Type;
	CALIBRATE.MEMORY_CHANNELS[channel].AGC = CurrentVFO->AGC;
	
	LCD_closeWindow();
	
	NeedSaveCalibration = true;
	LCD_showTooltip("Channel saved");
}
