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

uint8_t FRONTPANEL_funcbuttons_page = 0;

static void FRONTPANEL_ENCODER_Rotated(float32_t direction);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static uint16_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);

static void FRONTPANEL_BUTTONHANDLER_MODE_P(void);
static void FRONTPANEL_BUTTONHANDLER_MODE_N(void);
static void FRONTPANEL_BUTTONHANDLER_BAND_P(void);
static void FRONTPANEL_BUTTONHANDLER_BAND_N(void);
static void FRONTPANEL_BUTTONHANDLER_SQUELCH(void);
static void FRONTPANEL_BUTTONHANDLER_WPM(void);
static void FRONTPANEL_BUTTONHANDLER_KEYER(void);
static void FRONTPANEL_BUTTONHANDLER_SCAN(void);
static void FRONTPANEL_BUTTONHANDLER_REC(void);
static void FRONTPANEL_BUTTONHANDLER_PLAY(void);
static void FRONTPANEL_BUTTONHANDLER_SHIFT(void);
static void FRONTPANEL_BUTTONHANDLER_CLAR(void);
static void FRONTPANEL_BUTTONHANDLER_STEP(void);
static void FRONTPANEL_BUTTONHANDLER_BANDMAP(void);
static void FRONTPANEL_BUTTONHANDLER_AUTOGAINER(void);
static void FRONTPANEL_BUTTONHANDLER_UP(void);
static void FRONTPANEL_BUTTONHANDLER_DOWN(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC1(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC1H(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC2(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC2H(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC3(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC3H(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC4(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC4H(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC5(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC5H(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC6(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC6H(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC7(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC7H(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC8(void);
static void FRONTPANEL_BUTTONHANDLER_FUNC8H(void);
static void FRONTPANEL_ENC2SW_click_handler(void);
static void FRONTPANEL_ENC2SW_hold_handler(void);

#ifdef HRDW_MCP3008_1
static bool FRONTPanel_MCP3008_1_Enabled = true;
#endif
#ifdef HRDW_MCP3008_2
static bool FRONTPanel_MCP3008_2_Enabled = true;
#endif
#ifdef HRDW_MCP3008_3
static bool FRONTPanel_MCP3008_3_Enabled = true;
#endif

static int32_t ENCODER_slowler = 0;
static uint32_t ENCODER_AValDeb = 0;
static uint32_t ENCODER2_AValDeb = 0;
static bool enc2_func_mode = false; //false - fast-step, true - func mode (WPM, etc...)

#ifdef FRONTPANEL_SMALL_V1
PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[] = {
	{.port = 1, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PRE, .holdHandler = FRONTPANEL_BUTTONHANDLER_PGA},		  //PRE-PGA
	{.port = 1, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ATT, .holdHandler = FRONTPANEL_BUTTONHANDLER_ATTHOLD},	  //ATT-ATTHOLD
	{.port = 1, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MUTE, .holdHandler = FRONTPANEL_BUTTONHANDLER_SCAN},		  //MUTE-SCAN
	{.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_AGC, .holdHandler = FRONTPANEL_BUTTONHANDLER_AGC_SPEED},	  //AGC-AGCSPEED
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ArB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ANT},		  //A=B-ANT
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .holdHandler = FRONTPANEL_BUTTONHANDLER_TUNE},		  //TUNE
	{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_SQUELCH}, //RFPOWER-SQUELCH
	{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW, .holdHandler = FRONTPANEL_BUTTONHANDLER_HPF},			  //BW-HPF

	{.port = 2, .channel = 7, .type = FUNIT_CTRL_SHIFT}, //SHIFT
	{.port = 2, .channel = 6, .type = FUNIT_CTRL_AF_GAIN}, //AF GAIN
	{.port = 2, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_DNR, .holdHandler = FRONTPANEL_BUTTONHANDLER_NB},			 //DNR-NB
	{.port = 2, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_NOTCH, .holdHandler = FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL}, //NOTCH-MANUAL
	{.port = 2, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_CLAR, .holdHandler = FRONTPANEL_BUTTONHANDLER_SHIFT},		 //CLAR-SHIFT
	{.port = 2, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PLAY, .holdHandler = FRONTPANEL_BUTTONHANDLER_REC},			 //REC-PLAY
	{.port = 2, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_SERVICES, .holdHandler = FRONTPANEL_BUTTONHANDLER_SERVICES},	 //SERVICES
	{.port = 2, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_LOCK},			 //MENU-LOCK

	{.port = 3, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_WPM, .holdHandler = FRONTPANEL_BUTTONHANDLER_KEYER},			//WPM-KEYER
	{.port = 3, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_DOUBLE, .holdHandler = FRONTPANEL_BUTTONHANDLER_DOUBLEMODE}, //DOUBLE-&+
	{.port = 3, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_FAST, .holdHandler = FRONTPANEL_BUTTONHANDLER_STEP},			//FAST-FASTSETT
	{.port = 3, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N},		//BAND-
	{.port = 3, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_P},		//BAND+
	{.port = 3, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P},		//MODE+
	{.port = 3, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N},		//MODE-
	{.port = 3, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP},		//A/B-BANDMAP
};
#endif

#ifdef FRONTPANEL_BIG_V1
PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[] = {
	//buttons
	{.port = 1, .channel = 0, .type = FUNIT_CTRL_AF_GAIN}, //AF GAIN
	{.port = 1, .channel = 1, .type = FUNIT_CTRL_SHIFT}, //SHIFT
#ifdef TANGENT_YAESU_MH36
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_PTT, .tres_min = 200, .tres_max = 400},		//PTT_SW1 - PTT
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 400, .tres_max = 575, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_DOWN, .holdHandler = FRONTPANEL_BUTTONHANDLER_DOWN},		//PTT_SW1 - DOWN
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 575, .tres_max = 730, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_UP, .holdHandler = FRONTPANEL_BUTTONHANDLER_UP},		//PTT_SW1 - UP
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 730, .tres_max = 820, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AGC, .holdHandler = FRONTPANEL_BUTTONHANDLER_AGC},		//PTT_SW1 - AGC
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 200, .tres_max = 400, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB},		//PTT_SW2 - VFO
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 400, .tres_max = 575, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N},		//PTT_SW2 - P1
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 575, .tres_max = 730, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P},		//PTT_SW2 - P2
#endif
	{.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 450, .tres_max = 650, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_ENC2SW_click_handler, .holdHandler = FRONTPANEL_ENC2SW_hold_handler}, //ENC2_SW
	{.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 250, .tres_max = 450, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC8, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNC8H}, //FUNC8
	{.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 000, .tres_max = 250, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC7, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNC7H}, //FUNC7
	{.port = 1, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 450, .tres_max = 650, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N}, //ENC_B_3
	{.port = 1, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 250, .tres_max = 450, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //ENC_B_2
	{.port = 1, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 000, .tres_max = 250, .state = false, .prev_state = false, .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB}, //ENC_B_1
	{.port = 1, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 450, .tres_max = 650, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC4, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNC4H}, //FUNC4
	{.port = 1, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 250, .tres_max = 450, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC6, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNC6H}, //FUNC6
	{.port = 1, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 000, .tres_max = 250, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC5, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNC5H}, //FUNC5
	{.port = 1, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 450, .tres_max = 650, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC3, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNC3H}, //FUNC3
	{.port = 1, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 250, .tres_max = 450, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC2, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNC2H}, //FUNC2
	{.port = 1, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 000, .tres_max = 250, .state = false, .prev_state = false, .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC1, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNC1H}, //FUNC1
};
#endif

const PERIPH_FrontPanel_FuncButton PERIPH_FrontPanel_FuncButtonsList[FUNCBUTTONS_COUNT] = {
	{.name = "A / B", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_AsB},
	{.name = "B=A", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ArB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB},
	{.name = "TUNE", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .holdHandler = FRONTPANEL_BUTTONHANDLER_TUNE},
	{.name = "POWER", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER},
	{.name = "ANT", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ANT, .holdHandler = FRONTPANEL_BUTTONHANDLER_ANT},
	{.name = "BW", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW, .holdHandler = FRONTPANEL_BUTTONHANDLER_BW},
	{.name = "SERVICE", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_SERVICES, .holdHandler = FRONTPANEL_BUTTONHANDLER_SERVICES},
	{.name = "MENU", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_MENU},
	{.name = "DOUBLE", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_DOUBLE, .holdHandler = FRONTPANEL_BUTTONHANDLER_DOUBLEMODE},
	{.name = "HPF", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_HPF, .holdHandler = FRONTPANEL_BUTTONHANDLER_HPF},
	{.name = "LOCK", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_LOCK, .holdHandler = FRONTPANEL_BUTTONHANDLER_LOCK},
	{.name = "MODE+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P},
	{.name = "MODE-", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N},
	{.name = "BAND+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_P},
	{.name = "BAND-", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N},
	{.name = "SQL", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SQUELCH, .holdHandler = FRONTPANEL_BUTTONHANDLER_SQUELCH},
	{.name = "WPM", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_WPM, .holdHandler = FRONTPANEL_BUTTONHANDLER_WPM},
	{.name = "SCAN", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SCAN, .holdHandler = FRONTPANEL_BUTTONHANDLER_SCAN},
	{.name = "REC", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_REC, .holdHandler = FRONTPANEL_BUTTONHANDLER_REC},
	{.name = "PLAY", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PLAY, .holdHandler = FRONTPANEL_BUTTONHANDLER_PLAY},
	{.name = "SHIFT", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SHIFT, .holdHandler = FRONTPANEL_BUTTONHANDLER_SHIFT},
	{.name = "CLAR", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_CLAR, .holdHandler = FRONTPANEL_BUTTONHANDLER_CLAR},
	{.name = "BANDMAP", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP, .holdHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP},
	{.name = "AUTOGAIN", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AUTOGAINER, .holdHandler = FRONTPANEL_BUTTONHANDLER_AUTOGAINER},
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
					TRX_ScanMode = false;
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
					TRX_ScanMode = false;
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
			FRONTPANEL_ENCODER2_Rotated(CALIBRATE.ENCODER2_INVERT ? 1 : -1);
		}
		else
		{ // otherwise B changed its state first - counterclockwise rotation
			FRONTPANEL_ENCODER2_Rotated(CALIBRATE.ENCODER2_INVERT ? -1 : 1);
		}
		TRX_ScanMode = false;
	}
	ENCODER2_AValDeb = HAL_GetTick();
}

static void FRONTPANEL_ENCODER_Rotated(float32_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	if (TRX.Locked)
		return;
	if (LCD_systemMenuOpened)
	{
		SYSMENU_eventRotateSystemMenu((int8_t)direction);
		return;
	}
	if (fabsf(direction) <= ENCODER_MIN_RATE_ACCELERATION)
		direction = (direction < 0.0f) ? -1.0f : 1.0f;

	VFO *vfo = CurrentVFO();
	uint32_t newfreq = 0;
	if (TRX.Fast)
	{
		newfreq = (uint32_t)((int32_t)vfo->Freq + (int32_t)((float32_t)TRX.FRQ_FAST_STEP * direction));
		if ((vfo->Freq % TRX.FRQ_FAST_STEP) > 0 && fabsf(direction) <= 1.0f)
			newfreq = vfo->Freq / TRX.FRQ_FAST_STEP * TRX.FRQ_FAST_STEP;
	}
	else
	{
		newfreq = (uint32_t)((int32_t)vfo->Freq + (int32_t)((float32_t)TRX.FRQ_STEP * direction));
		if ((vfo->Freq % TRX.FRQ_STEP) > 0 && fabsf(direction) <= 1.0f)
			newfreq = vfo->Freq / TRX.FRQ_STEP * TRX.FRQ_STEP;
	}
	TRX_setFrequency(newfreq, vfo);
	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_ENCODER2_Rotated(int8_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	if (TRX.Locked)
		return;

	if (LCD_systemMenuOpened)
	{
		SYSMENU_eventSecRotateSystemMenu(direction);
		return;
	}

	//NOTCH - default action
	if (CurrentVFO()->ManualNotchFilter)
	{
		if (CurrentVFO()->NotchFC > 50 && direction < 0)
			CurrentVFO()->NotchFC -= 100;
		else if (CurrentVFO()->NotchFC < CurrentVFO()->LPF_Filter_Width && direction > 0)
			CurrentVFO()->NotchFC += 100;
		LCD_UpdateQuery.StatusInfoGUI = true;
		NeedReinitNotch = true;
	}
	else
	{
		if (!enc2_func_mode || ((CurrentVFO()->Mode != TRX_MODE_CW_L && CurrentVFO()->Mode != TRX_MODE_CW_U)))
		{
			VFO *vfo = CurrentVFO();
			uint32_t newfreq = 0;
			float32_t freq_round = 0;
			if (TRX.Fast)
			{
				freq_round = roundf((float32_t)vfo->Freq / (float32_t)TRX.FRQ_ENC_FAST_STEP) * (float32_t)TRX.FRQ_ENC_FAST_STEP;
				newfreq = (uint32_t)((int32_t)freq_round + (int32_t)TRX.FRQ_ENC_FAST_STEP * direction);
			}
			else
			{
				freq_round = roundf((float32_t)vfo->Freq / (float32_t)TRX.FRQ_ENC_STEP) * (float32_t)TRX.FRQ_ENC_STEP;
				newfreq = (uint32_t)((int32_t)freq_round + (int32_t)TRX.FRQ_ENC_STEP * direction);
			}
			TRX_setFrequency(newfreq, vfo);
			LCD_UpdateQuery.FreqInfo = true;
		}
		else
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
		}
	}
}

void FRONTPANEL_check_ENC2SW(void)
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

	bool ENC2SW_AND_TOUCH_Now = HAL_GPIO_ReadPin(ENC2SW_AND_TOUCHPAD_GPIO_Port, ENC2SW_AND_TOUCHPAD_Pin);
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
		FRONTPANEL_ENC2SW_hold_handler();
	}

	//ENC2 Button click
	if (ENC2SW_clicked)
	{
		menu_enc2_click_starttime = HAL_GetTick();
		FRONTPANEL_ENC2SW_click_handler();
	}
}

static void FRONTPANEL_ENC2SW_click_handler(void)
{
	//ENC2 CLICK
	if ((CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U) && !LCD_systemMenuOpened)
	{
		enc2_func_mode = !enc2_func_mode; //enc2 rotary mode

		if (!enc2_func_mode)
			LCD_showTooltip("FAST STEP");
		else
			LCD_showTooltip("SET WPM");
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

static void FRONTPANEL_ENC2SW_hold_handler(void)
{
	FRONTPANEL_BUTTONHANDLER_MENU();
}

void FRONTPANEL_Init(void)
{
	uint16_t test_value = 0;
	#ifdef HRDW_MCP3008_1
	test_value = FRONTPANEL_ReadMCP3008_Value(0, AD1_CS_GPIO_Port, AD1_CS_Pin);
	if (test_value == 65535)
	{
		FRONTPanel_MCP3008_1_Enabled = false;
		sendToDebug_strln("[ERR] Frontpanel MCP3008 - 1 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 1 init error (FPGA I2S CLK?)", true);
	}
	#endif
	#ifdef HRDW_MCP3008_2
	test_value = FRONTPANEL_ReadMCP3008_Value(0, AD2_CS_GPIO_Port, AD2_CS_Pin);
	if (test_value == 65535)
	{
		FRONTPanel_MCP3008_2_Enabled = false;
		sendToDebug_strln("[ERR] Frontpanel MCP3008 - 2 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 2 init error", true);
	}
	#endif
	#ifdef HRDW_MCP3008_3
	test_value = FRONTPANEL_ReadMCP3008_Value(0, AD3_CS_GPIO_Port, AD3_CS_Pin);
	if (test_value == 65535)
	{
		FRONTPanel_MCP3008_3_Enabled = false;
		sendToDebug_strln("[ERR] Frontpanel MCP3008 - 3 not found, disabling... (FPGA SPI/I2S CLOCK ERROR?)");
		LCD_showError("MCP3008 - 3 init error", true);
	}
	#endif
	FRONTPANEL_Process();
}

void FRONTPANEL_Process(void)
{
	if (SPI_process)
		return;
	SPI_process = true;

	#ifndef HAS_TOUCHPAD
	FRONTPANEL_check_ENC2SW();
	#endif

	uint16_t buttons_count = sizeof(PERIPH_FrontPanel_Buttons) / sizeof(PERIPH_FrontPanel_Button);
	uint16_t mcp3008_value = 0;

	//process buttons
	for (uint16_t b = 0; b < buttons_count; b++)
	{
		//check disabled ports
		#ifdef HRDW_MCP3008_1
		if (PERIPH_FrontPanel_Buttons[b].port == 1 && !FRONTPanel_MCP3008_1_Enabled)
			continue;
		#endif
		#ifdef HRDW_MCP3008_2
		if (PERIPH_FrontPanel_Buttons[b].port == 2 && !FRONTPanel_MCP3008_2_Enabled)
			continue;
		#endif
		#ifdef HRDW_MCP3008_3
		if (PERIPH_FrontPanel_Buttons[b].port == 3 && !FRONTPanel_MCP3008_3_Enabled)
			continue;
		#endif

		//get state from ADC MCP3008 (10bit - 1024values)
		#ifdef HRDW_MCP3008_1
		if (PERIPH_FrontPanel_Buttons[b].port == 1)
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(PERIPH_FrontPanel_Buttons[b].channel, AD1_CS_GPIO_Port, AD1_CS_Pin);
		else 
		#endif
		#ifdef HRDW_MCP3008_2
		if (PERIPH_FrontPanel_Buttons[b].port == 2)
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(PERIPH_FrontPanel_Buttons[b].channel, AD2_CS_GPIO_Port, AD2_CS_Pin);
		else 
		#endif
		#ifdef HRDW_MCP3008_3
		if (PERIPH_FrontPanel_Buttons[b].port == 3)
			mcp3008_value = FRONTPANEL_ReadMCP3008_Value(PERIPH_FrontPanel_Buttons[b].channel, AD3_CS_GPIO_Port, AD3_CS_Pin);
		else 
		#endif
			continue;

		/*if(PERIPH_FrontPanel_Buttons[b].channel == 2)
		{
			sendToDebug_str("SW1 ");
			sendToDebug_uint16(mcp3008_value, false);
		}
		if(PERIPH_FrontPanel_Buttons[b].channel == 3)
		{
			sendToDebug_str("SW2 ");
			sendToDebug_uint16(mcp3008_value, false);
			sendToDebug_newline();
		}*/
		
		// AF_GAIN
		if(PERIPH_FrontPanel_Buttons[b].type == FUNIT_CTRL_AF_GAIN)
		{
			TRX_Volume = (uint16_t)(1023.0f - mcp3008_value);
			if (TRX_Volume < 50)
				TRX_Volume = 0;
		}

		// SHIFT or IF Gain
		if(PERIPH_FrontPanel_Buttons[b].type == FUNIT_CTRL_SHIFT)
		{
			if (TRX.ShiftEnabled)
			{
				int_fast16_t TRX_SHIFT_old = TRX_SHIFT;
				TRX_SHIFT = (int_fast16_t)(((1023.0f - mcp3008_value) * TRX.SHIFT_INTERVAL * 2 / 1023.0f) - TRX.SHIFT_INTERVAL);
				if (TRX_SHIFT_old != TRX_SHIFT)
				{
					TRX_setFrequency(CurrentVFO()->Freq, CurrentVFO());
					uint16_t LCD_bw_trapez_stripe_pos_new = LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2;
					LCD_bw_trapez_stripe_pos_new = LCD_bw_trapez_stripe_pos_new + (int16_t)((float32_t)(LAYOUT->BW_TRAPEZ_WIDTH * 0.9f) / 2.0f * ((float32_t)TRX_SHIFT / (float32_t)TRX.SHIFT_INTERVAL));
					if (abs(LCD_bw_trapez_stripe_pos_new - LCD_bw_trapez_stripe_pos) > 2)
					{
						LCD_bw_trapez_stripe_pos = LCD_bw_trapez_stripe_pos_new;
						LCD_UpdateQuery.StatusInfoGUI = true;
					}
				}
			}
			else
			{
				TRX_SHIFT = 0;
				TRX.IF_Gain = (uint8_t)(0.0f + ((1023.0f - mcp3008_value) * 50.0f / 1023.0f));
			}
		}
		
		// PTT
		if(PERIPH_FrontPanel_Buttons[b].type == FUNIT_CTRL_PTT)
		{
			static bool frontunit_ptt_state_prev = false;
			bool frontunit_ptt_state_now = false;
			if (mcp3008_value > PERIPH_FrontPanel_Buttons[b].tres_min && mcp3008_value < PERIPH_FrontPanel_Buttons[b].tres_max)
				frontunit_ptt_state_now = true;
			if(frontunit_ptt_state_prev != frontunit_ptt_state_now)
			{
				TRX_ptt_soft = frontunit_ptt_state_now;
				TRX_ptt_change();
				frontunit_ptt_state_prev = frontunit_ptt_state_now;
			}
		}
		
		//BUTTONS
		if(PERIPH_FrontPanel_Buttons[b].type == FUNIT_CTRL_BUTTON)
		{
			//set state
			if (mcp3008_value > PERIPH_FrontPanel_Buttons[b].tres_min && mcp3008_value < PERIPH_FrontPanel_Buttons[b].tres_max)
				PERIPH_FrontPanel_Buttons[b].state = true;
			else
				PERIPH_FrontPanel_Buttons[b].state = false;

			//check state
			if ((PERIPH_FrontPanel_Buttons[b].prev_state != PERIPH_FrontPanel_Buttons[b].state) && PERIPH_FrontPanel_Buttons[b].state)
			{
				PERIPH_FrontPanel_Buttons[b].start_hold_time = HAL_GetTick();
				PERIPH_FrontPanel_Buttons[b].afterhold = false;
			}

			//check hold state
			if ((PERIPH_FrontPanel_Buttons[b].prev_state == PERIPH_FrontPanel_Buttons[b].state) && PERIPH_FrontPanel_Buttons[b].state && ((HAL_GetTick() - PERIPH_FrontPanel_Buttons[b].start_hold_time) > KEY_HOLD_TIME) && !PERIPH_FrontPanel_Buttons[b].afterhold)
			{
				PERIPH_FrontPanel_Buttons[b].afterhold = true;
				if (!TRX.Locked || (PERIPH_FrontPanel_Buttons[b].port == 2 && PERIPH_FrontPanel_Buttons[b].channel == 0)) //LOCK BUTTON
					if (!LCD_systemMenuOpened || PERIPH_FrontPanel_Buttons[b].work_in_menu)
						if (PERIPH_FrontPanel_Buttons[b].holdHandler != NULL)
						{
							WM8731_Beep();
							PERIPH_FrontPanel_Buttons[b].holdHandler();
						}
			}

			//check click state
			if ((PERIPH_FrontPanel_Buttons[b].prev_state != PERIPH_FrontPanel_Buttons[b].state) && !PERIPH_FrontPanel_Buttons[b].state && ((HAL_GetTick() - PERIPH_FrontPanel_Buttons[b].start_hold_time) < KEY_HOLD_TIME) && !PERIPH_FrontPanel_Buttons[b].afterhold && !TRX.Locked)
			{
				if (!LCD_systemMenuOpened || PERIPH_FrontPanel_Buttons[b].work_in_menu)
					if (PERIPH_FrontPanel_Buttons[b].clickHandler != NULL)
					{
						WM8731_Beep();
						PERIPH_FrontPanel_Buttons[b].clickHandler();
					}
			}

			//save prev state
			PERIPH_FrontPanel_Buttons[b].prev_state = PERIPH_FrontPanel_Buttons[b].state;
		}
	}
	SPI_process = false;
}

void FRONTPANEL_BUTTONHANDLER_DOUBLE(void)
{
	TRX.Dual_RX = !TRX.Dual_RX;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedReinitAudioFilters = true;
}

void FRONTPANEL_BUTTONHANDLER_DOUBLEMODE(void)
{
	if (!TRX.Dual_RX)
		return;

	if (TRX.Dual_RX_Type == VFO_A_AND_B)
		TRX.Dual_RX_Type = VFO_A_PLUS_B;
	else if (TRX.Dual_RX_Type == VFO_A_PLUS_B)
		TRX.Dual_RX_Type = VFO_A_AND_B;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedReinitAudioFilters = true;
}

void FRONTPANEL_BUTTONHANDLER_AsB(void) // A/B
{
	TRX_TemporaryMute();
	TRX.current_vfo = !TRX.current_vfo;
	TRX_setFrequency(CurrentVFO()->Freq, CurrentVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
	NeedReinitAudioFiltersClean = true;
	NeedReinitAudioFilters = true;
	resetVAD();
	FFT_Init();
	TRX_ScanMode = false;
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_TUNE(void)
{
	TRX_Tune = !TRX_Tune;
	TRX_ptt_hard = TRX_Tune;
	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	TRX_Restart_Mode();
}

void FRONTPANEL_BUTTONHANDLER_PRE(void)
{
	TRX.LNA = !TRX.LNA;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].LNA = TRX.LNA;
	}
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_ATT(void)
{
	TRX.ATT = !TRX.ATT;

	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_ATTHOLD(void)
{
	TRX.ATT_DB += TRX.ATT_STEP;
	if (TRX.ATT_DB > 31.0f)
		TRX.ATT_DB = TRX.ATT_STEP;

	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].ATT = TRX.ATT;
		TRX.BANDS_SAVED_SETTINGS[band].ATT_DB = TRX.ATT_DB;
	}

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_ANT(void)
{
	TRX.ANT = !TRX.ANT;

	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].ANT = TRX.ANT;

	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_PGA(void)
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
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
		TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
	}
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_PGA_ONLY(void)
{
	TRX.ADC_PGA = !TRX.ADC_PGA;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_DRV_ONLY(void)
{
	TRX.ADC_Driver = !TRX.ADC_Driver;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
	resetVAD();
}

void FRONTPANEL_BUTTONHANDLER_FAST(void)
{
	TRX.Fast = !TRX.Fast;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_MODE_P(void)
{
	int8_t mode = (int8_t)CurrentVFO()->Mode;
	if (mode == TRX_MODE_LSB)
		mode = TRX_MODE_USB;
	else if (mode == TRX_MODE_USB)
		mode = TRX_MODE_LSB;
	else if (mode == TRX_MODE_CW_L)
		mode = TRX_MODE_CW_U;
	else if (mode == TRX_MODE_CW_U)
		mode = TRX_MODE_CW_L;
	else if (mode == TRX_MODE_NFM)
		mode = TRX_MODE_WFM;
	else if (mode == TRX_MODE_WFM)
		mode = TRX_MODE_NFM;
	else if (mode == TRX_MODE_DIGI_L)
		mode = TRX_MODE_DIGI_U;
	else if (mode == TRX_MODE_DIGI_U)
		mode = TRX_MODE_DIGI_L;
	else if (mode == TRX_MODE_AM)
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

	TRX_setMode((uint8_t)mode, CurrentVFO());
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	TRX_ScanMode = false;
}

static void FRONTPANEL_BUTTONHANDLER_MODE_N(void)
{
	int8_t mode = (int8_t)CurrentVFO()->Mode;
	if(mode == TRX_MODE_LOOPBACK)
		LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	if (mode == TRX_MODE_LSB)
		mode = TRX_MODE_CW_L;
	else if (mode == TRX_MODE_USB)
		mode = TRX_MODE_CW_U;
	else if (mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U)
		mode = TRX_MODE_DIGI_U;
	else if (mode == TRX_MODE_DIGI_L || mode == TRX_MODE_DIGI_U)
		mode = TRX_MODE_NFM;
	else if (mode == TRX_MODE_NFM || mode == TRX_MODE_WFM)
		mode = TRX_MODE_AM;
	else
	{
		if (CurrentVFO()->Freq < 10000000)
			mode = TRX_MODE_LSB;
		else
			mode = TRX_MODE_USB;
	}

	TRX_setMode((uint8_t)mode, CurrentVFO());
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	TRX_ScanMode = false;
}

static void FRONTPANEL_BUTTONHANDLER_BAND_P(void)
{
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	band++;
	if (band >= BANDS_COUNT)
		band = 0;
	while (!BANDS[band].selectable)
	{
		band++;
		if (band >= BANDS_COUNT)
			band = 0;
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO());
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO());
	TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	TRX_AutoGain_Stage = TRX.BANDS_SAVED_SETTINGS[band].AutoGain_Stage;
	CurrentVFO()->DNR = TRX.BANDS_SAVED_SETTINGS[band].DNR;
	CurrentVFO()->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
	resetVAD();
	TRX_ScanMode = false;
}

static void FRONTPANEL_BUTTONHANDLER_BAND_N(void)
{
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	band--;
	if (band < 0)
		band = BANDS_COUNT - 1;
	while (!BANDS[band].selectable)
	{
		band--;
		if (band < 0)
			band = BANDS_COUNT - 1;
	}

	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, CurrentVFO());
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO());
	TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
	TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	TRX_AutoGain_Stage = TRX.BANDS_SAVED_SETTINGS[band].AutoGain_Stage;
	CurrentVFO()->DNR = TRX.BANDS_SAVED_SETTINGS[band].DNR;
	CurrentVFO()->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
	resetVAD();
	TRX_ScanMode = false;
}

void FRONTPANEL_BUTTONHANDLER_RF_POWER(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		SYSMENU_TRX_RFPOWER_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void FRONTPANEL_BUTTONHANDLER_AGC(void)
{
	CurrentVFO()->AGC = !CurrentVFO()->AGC;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].AGC = CurrentVFO()->AGC;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_AGC_SPEED(void)
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

static void FRONTPANEL_BUTTONHANDLER_SQUELCH(void)
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

static void FRONTPANEL_BUTTONHANDLER_WPM(void)
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

static void FRONTPANEL_BUTTONHANDLER_KEYER(void)
{
	TRX.CW_KEYER = !TRX.CW_KEYER;
	if (TRX.CW_KEYER)
		LCD_showTooltip("KEYER ON");
	else
		LCD_showTooltip("KEYER OFF");
}

static void FRONTPANEL_BUTTONHANDLER_STEP(void)
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

void FRONTPANEL_BUTTONHANDLER_DNR(void)
{
	TRX_TemporaryMute();
	CurrentVFO()->DNR = !CurrentVFO()->DNR;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].DNR = CurrentVFO()->DNR;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_NB(void)
{
	TRX.NOISE_BLANKER = !TRX.NOISE_BLANKER;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_BW(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
			SYSMENU_AUDIO_BW_CW_HOTKEY();
		else if (CurrentVFO()->Mode == TRX_MODE_NFM || CurrentVFO()->Mode == TRX_MODE_WFM)
			SYSMENU_AUDIO_BW_FM_HOTKEY();
		else if (CurrentVFO()->Mode == TRX_MODE_AM)
			SYSMENU_AUDIO_BW_AM_HOTKEY();
		else
			SYSMENU_AUDIO_BW_SSB_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void FRONTPANEL_BUTTONHANDLER_HPF(void)
{
	if (!LCD_systemMenuOpened)
	{
		LCD_systemMenuOpened = true;
		if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
			SYSMENU_AUDIO_HPF_CW_HOTKEY();
		else
			SYSMENU_AUDIO_HPF_SSB_HOTKEY();
	}
	else
	{
		SYSMENU_eventCloseAllSystemMenu();
	}
}

void FRONTPANEL_BUTTONHANDLER_ArB(void) //A=B
{
	if (TRX.current_vfo)
		memcpy(&TRX.VFO_A, &TRX.VFO_B, sizeof TRX.VFO_B);
	else
		memcpy(&TRX.VFO_B, &TRX.VFO_A, sizeof TRX.VFO_B);

	LCD_showTooltip("VFO COPIED");

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_NOTCH(void)
{
	TRX_TemporaryMute();

	if (CurrentVFO()->NotchFC > CurrentVFO()->LPF_Filter_Width)
	{
		CurrentVFO()->NotchFC = CurrentVFO()->LPF_Filter_Width;
		NeedReinitNotch = true;
	}
	CurrentVFO()->ManualNotchFilter = false;

	if (!CurrentVFO()->AutoNotchFilter)
	{
		InitAutoNotchReduction();
		CurrentVFO()->AutoNotchFilter = true;
	}
	else
		CurrentVFO()->AutoNotchFilter = false;

	LCD_UpdateQuery.StatusInfoGUI = true;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL(void)
{
	if (CurrentVFO()->NotchFC > CurrentVFO()->LPF_Filter_Width)
		CurrentVFO()->NotchFC = CurrentVFO()->LPF_Filter_Width;
	CurrentVFO()->AutoNotchFilter = false;
	if (!CurrentVFO()->ManualNotchFilter)
		CurrentVFO()->ManualNotchFilter = true;
	else
		CurrentVFO()->ManualNotchFilter = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedReinitNotch = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_SHIFT(void)
{
	TRX.ShiftEnabled = !TRX.ShiftEnabled;
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_CLAR(void)
{
	TRX.CLAR = !TRX.CLAR;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

void FRONTPANEL_BUTTONHANDLER_LOCK(void)
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

void FRONTPANEL_BUTTONHANDLER_MENU(void)
{
	if (!LCD_systemMenuOpened)
		LCD_systemMenuOpened = true;
	else
		SYSMENU_eventCloseSystemMenu();
	LCD_redraw(false);
}

void FRONTPANEL_BUTTONHANDLER_MUTE(void)
{
	TRX_Mute = !TRX_Mute;
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_BANDMAP(void)
{
	TRX.BandMapEnabled = !TRX.BandMapEnabled;

	if (TRX.BandMapEnabled)
		LCD_showTooltip("BANDMAP ON");
	else
		LCD_showTooltip("BANDMAP OFF");

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_AUTOGAINER(void)
{
	TRX.AutoGain = !TRX.AutoGain;

	if (TRX.AutoGain)
		LCD_showTooltip("AUTOGAINER ON");
	else
		LCD_showTooltip("AUTOGAINER OFF");

	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static uint16_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN)
{
	uint8_t outData[3] = {0};
	uint8_t inData[3] = {0};
	uint16_t mcp3008_value = 0;

	outData[0] = 0x18 | channel;
	bool res = SPI_Transmit(outData, inData, 3, CS_PORT, CS_PIN, false, SPI_FRONT_UNIT_PRESCALER);
	if (res == false)
		return 65535;
	mcp3008_value = (uint16_t)(0 | ((inData[1] & 0x3F) << 4) | (inData[2] & 0xF0 >> 4));

	return mcp3008_value;
}

void FRONTPANEL_BUTTONHANDLER_SERVICES(void)
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

static void FRONTPANEL_BUTTONHANDLER_SCAN(void)
{
	TRX_ScanMode = !TRX_ScanMode;
}

static void FRONTPANEL_BUTTONHANDLER_PLAY(void)
{
	
}

static void FRONTPANEL_BUTTONHANDLER_REC(void)
{
	
}

static void FRONTPANEL_BUTTONHANDLER_FUNC1(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 0]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 0]].clickHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC1H(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 0]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 0]].holdHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC2(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 1]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 1]].clickHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC2H(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 1]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 1]].holdHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC3(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 2]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 2]].clickHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC3H(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 2]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 2]].holdHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC4(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 3]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 3]].clickHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC4H(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 3]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 3]].holdHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC5(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 4]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 4]].clickHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC5H(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 4]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 4]].holdHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC6(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 5]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 5]].clickHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC6H(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 5]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 5]].holdHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC7(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 6]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 6]].clickHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC7H(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 6]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 6]].holdHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC8(void)
{
	if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 7]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + 7]].clickHandler();
}

static void FRONTPANEL_BUTTONHANDLER_FUNC8H(void)
{
	if (LCD_systemMenuOpened)
	{
		SYSMENU_hiddenmenu_enabled = true;
		LCD_redraw(false);
	}
	else if(!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[7]].work_in_menu)
		PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[7]].holdHandler();
}

static void FRONTPANEL_BUTTONHANDLER_UP(void)
{
	uint32_t newfreq = CurrentVFO()->Freq + 500;
	newfreq = newfreq / 500 * 500;
	TRX_setFrequency(newfreq, CurrentVFO());
	LCD_UpdateQuery.FreqInfo = true;
}

static void FRONTPANEL_BUTTONHANDLER_DOWN(void)
{
	uint32_t newfreq = CurrentVFO()->Freq - 500;
	newfreq = newfreq / 500 * 500;
	TRX_setFrequency(newfreq, CurrentVFO());
	LCD_UpdateQuery.FreqInfo = true;
}
