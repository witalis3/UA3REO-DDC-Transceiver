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

uint8_t FRONTPANEL_funcbuttons_page = 0;
int8_t FRONTPANEL_ProcessEncoder1 = 0;
int8_t FRONTPANEL_ProcessEncoder2 = 0;

static void FRONTPANEL_ENCODER_Rotated(float32_t direction);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);
static uint16_t FRONTPANEL_ReadMCP3008_Value(uint8_t channel, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN);
static void FRONTPANEL_ENCODER2_Rotated(int8_t direction);

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
static void FRONTPANEL_BUTTONHANDLER_SHIFT(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_CLAR(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_STEP(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_BANDMAP(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_AUTOGAINER(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_UP(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_DOWN(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_FUNC(uint32_t parameter);
static void FRONTPANEL_BUTTONHANDLER_FUNCH(uint32_t parameter);
static void FRONTPANEL_ENC2SW_click_handler(uint32_t parameter);
static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter);
static void FRONTPANEL_CheckButton(PERIPH_FrontPanel_Button *button, uint16_t mcp3008_value);

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
	{.port = 1, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_PRE, .holdHandler = FRONTPANEL_BUTTONHANDLER_PGA},		  //PRE-PGA
	{.port = 1, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_ATT, .holdHandler = FRONTPANEL_BUTTONHANDLER_ATTHOLD},	  //ATT-ATTHOLD
	{.port = 1, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_MUTE, .holdHandler = FRONTPANEL_BUTTONHANDLER_SCAN},		  //MUTE-SCAN
	{.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_AGC, .holdHandler = FRONTPANEL_BUTTONHANDLER_AGC_SPEED},	  //AGC-AGCSPEED
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_ArB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ANT},		  //A=B-ANT
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .holdHandler = FRONTPANEL_BUTTONHANDLER_TUNE},		  //TUNE
	{.port = 1, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_SQUELCH}, //RFPOWER-SQUELCH
	{.port = 1, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW, .holdHandler = FRONTPANEL_BUTTONHANDLER_HPF},			  //BW-HPF

	{.port = 2, .channel = 7, .type = FUNIT_CTRL_SHIFT},																																																										 //SHIFT
	{.port = 2, .channel = 6, .type = FUNIT_CTRL_AF_GAIN},																																																										 //AF GAIN
	{.port = 2, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_DNR, .holdHandler = FRONTPANEL_BUTTONHANDLER_NB},			 //DNR-NB
	{.port = 2, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_NOTCH, .holdHandler = FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL}, //NOTCH-MANUAL
	{.port = 2, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_CLAR, .holdHandler = FRONTPANEL_BUTTONHANDLER_SHIFT},		 //CLAR-SHIFT
	{.port = 2, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_PLAY, .holdHandler = FRONTPANEL_BUTTONHANDLER_REC},			 //REC-PLAY
	{.port = 2, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_SERVICES, .holdHandler = FRONTPANEL_BUTTONHANDLER_SERVICES},	 //SERVICES
	{.port = 2, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_LOCK},			 //MENU-LOCK

	{.port = 3, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_WPM, .holdHandler = FRONTPANEL_BUTTONHANDLER_KEYER},			//WPM-KEYER
	{.port = 3, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_DOUBLE, .holdHandler = FRONTPANEL_BUTTONHANDLER_DOUBLEMODE}, //DOUBLE-&+
	{.port = 3, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_FAST, .holdHandler = FRONTPANEL_BUTTONHANDLER_STEP},			//FAST-FASTSETT
	{.port = 3, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N},		//BAND-
	{.port = 3, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_P},		//BAND+
	{.port = 3, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P},		//MODE+
	{.port = 3, .channel = 1, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N},		//MODE-
	{.port = 3, .channel = 0, .type = FUNIT_CTRL_BUTTON, .tres_min = 0, .tres_max = MCP3008_SINGLE_THRESHOLD, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP},		//A/B-BANDMAP
};
#endif

#ifdef FRONTPANEL_BIG_V1
PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[] = {
	//buttons
	{.port = 1, .channel = 0, .type = FUNIT_CTRL_AF_GAIN}, //AF GAIN
	{.port = 1, .channel = 1, .type = FUNIT_CTRL_SHIFT},   //SHIFT

	{.port = 1, .channel = 2, .type = FUNIT_CTRL_TANGENT}, //TANGENT_SW1
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_TANGENT}, //TANGENT_SW2

	{.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 450, .tres_max = 650, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_ENC2SW_click_handler, .holdHandler = FRONTPANEL_ENC2SW_hold_handler},	 //ENC2_SW
	{.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 250, .tres_max = 450, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 7, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH},	 //FUNC8
	{.port = 1, .channel = 4, .type = FUNIT_CTRL_BUTTON, .tres_min = 000, .tres_max = 250, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 6, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH},	 //FUNC7
	{.port = 1, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 450, .tres_max = 650, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N}, //ENC_B_3
	{.port = 1, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 250, .tres_max = 450, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //ENC_B_2
	{.port = 1, .channel = 5, .type = FUNIT_CTRL_BUTTON, .tres_min = 000, .tres_max = 250, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB},		 //ENC_B_1
	{.port = 1, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 450, .tres_max = 650, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 3, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH},	 //FUNC4
	{.port = 1, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 250, .tres_max = 450, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 5, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH},	 //FUNC6
	{.port = 1, .channel = 6, .type = FUNIT_CTRL_BUTTON, .tres_min = 000, .tres_max = 250, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 4, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH},	 //FUNC5
	{.port = 1, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 450, .tres_max = 650, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 2, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH},	 //FUNC3
	{.port = 1, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 250, .tres_max = 450, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 1, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH},	 //FUNC2
	{.port = 1, .channel = 7, .type = FUNIT_CTRL_BUTTON, .tres_min = 000, .tres_max = 250, .state = false, .prev_state = false, .work_in_menu = true, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_FUNC, .holdHandler = FRONTPANEL_BUTTONHANDLER_FUNCH},	 //FUNC1
};
#endif

PERIPH_FrontPanel_Button PERIPH_FrontPanel_TANGENT_MH36[] = {
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_PTT, .tres_min = 200, .tres_max = 430},																																												 //PTT_SW1 - PTT
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 430, .tres_max = 640, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_DOWN, .holdHandler = FRONTPANEL_BUTTONHANDLER_DOWN},	 //PTT_SW1 - DOWN
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 640, .tres_max = 805, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_UP, .holdHandler = FRONTPANEL_BUTTONHANDLER_UP},		 //PTT_SW1 - UP
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 805, .tres_max = 920, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_AGC, .holdHandler = FRONTPANEL_BUTTONHANDLER_AGC},		 //PTT_SW1 - AGC
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 200, .tres_max = 430, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB},		 //PTT_SW2 - VFO
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 430, .tres_max = 640, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N}, //PTT_SW2 - P1
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 640, .tres_max = 805, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //PTT_SW2 - P2
};

PERIPH_FrontPanel_Button PERIPH_FrontPanel_TANGENT_MH48[] = {
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 553, .tres_max = 633, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_DOWN, .holdHandler = FRONTPANEL_BUTTONHANDLER_DOWN},	 //PTT_SW1 - DOWN
	{.port = 1, .channel = 2, .type = FUNIT_CTRL_BUTTON, .tres_min = 341, .tres_max = 421, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_UP, .holdHandler = FRONTPANEL_BUTTONHANDLER_UP},		 //PTT_SW1 - UP
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 339, .tres_max = 419, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_AGC, .holdHandler = FRONTPANEL_BUTTONHANDLER_AGC},		 //PTT_SW2 - P1
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 542, .tres_max = 622, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB},		 //PTT_SW2 - P2
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 715, .tres_max = 795, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N}, //PTT_SW2 - P3
	{.port = 1, .channel = 3, .type = FUNIT_CTRL_BUTTON, .tres_min = 860, .tres_max = 930, .state = false, .prev_state = false, .work_in_menu = false, .parameter = 0, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P}, //PTT_SW2 - P4
};

const PERIPH_FrontPanel_FuncButton PERIPH_FrontPanel_FuncButtonsList[FUNCBUTTONS_COUNT] = {
	{.name = "A / B", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AsB, .holdHandler = FRONTPANEL_BUTTONHANDLER_AsB},
	{.name = "B=A", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ArB, .holdHandler = FRONTPANEL_BUTTONHANDLER_ArB},
	{.name = "TUNE", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_TUNE, .holdHandler = FRONTPANEL_BUTTONHANDLER_TUNE},
	{.name = "POWER", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER, .holdHandler = FRONTPANEL_BUTTONHANDLER_RF_POWER},
	{.name = "ANT", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ANT, .holdHandler = FRONTPANEL_BUTTONHANDLER_ANT},
	{.name = "SHIFT", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SHIFT, .holdHandler = FRONTPANEL_BUTTONHANDLER_CLAR},
	{.name = "SERVICE", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_SERVICES, .holdHandler = FRONTPANEL_BUTTONHANDLER_SERVICES},
	{.name = "MENU", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_MENU},

	{.name = "WPM", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_WPM, .holdHandler = FRONTPANEL_BUTTONHANDLER_WPM},
	{.name = "SQL", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_SQUELCH, .holdHandler = FRONTPANEL_BUTTONHANDLER_SQUELCH},
	{.name = "DOUBLE", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_DOUBLE, .holdHandler = FRONTPANEL_BUTTONHANDLER_DOUBLEMODE},
	{.name = "CLAR", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_CLAR, .holdHandler = FRONTPANEL_BUTTONHANDLER_SHIFT},
	{.name = "SCAN", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SCAN, .holdHandler = FRONTPANEL_BUTTONHANDLER_SCAN},
	{.name = "PLAY", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_PLAY, .holdHandler = FRONTPANEL_BUTTONHANDLER_PLAY},
	{.name = "REC", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_REC, .holdHandler = FRONTPANEL_BUTTONHANDLER_REC},
	{.name = "MENU", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_MENU},

	{.name = "BW", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_BW, .holdHandler = FRONTPANEL_BUTTONHANDLER_BW},
	{.name = "MODE+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_P},
	{.name = "MODE-", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_MODE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_MODE_N},
	{.name = "BAND+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_P},
	{.name = "BAND-", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BAND_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_BAND_N},
	{.name = "BANDMP", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP, .holdHandler = FRONTPANEL_BUTTONHANDLER_BANDMAP},
	{.name = "AUTOGN", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_AUTOGAINER, .holdHandler = FRONTPANEL_BUTTONHANDLER_AUTOGAINER},
	{.name = "MENU", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_MENU},
	
	{.name = "SAMPLE-", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_SAMPLE_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_SAMPLE_N},
	{.name = "SAMPLE+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_SAMPLE_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_SAMPLE_P},
	{.name = "ZOOM-", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_N, .holdHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_N},
	{.name = "ZOOM+", .work_in_menu = false, .clickHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P, .holdHandler = FRONTPANEL_BUTTONHANDLER_ZOOM_P},
	{.name = "LOCK", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_LOCK, .holdHandler = FRONTPANEL_BUTTONHANDLER_LOCK},
	{.name = "HPF", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_HPF, .holdHandler = FRONTPANEL_BUTTONHANDLER_HPF},
	{.name = "SERVICE", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_SERVICES, .holdHandler = FRONTPANEL_BUTTONHANDLER_SERVICES},
	{.name = "MENU", .work_in_menu = true, .clickHandler = FRONTPANEL_BUTTONHANDLER_MENU, .holdHandler = FRONTPANEL_BUTTONHANDLER_MENU},
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
			FRONTPANEL_ProcessEncoder2 = CALIBRATE.ENCODER2_INVERT ? 1 : -1;
		}
		else
		{ // otherwise B changed its state first - counterclockwise rotation
			FRONTPANEL_ProcessEncoder2 = CALIBRATE.ENCODER2_INVERT ? -1 : 1;
		}
		TRX_ScanMode = false;
	}
	ENCODER2_AValDeb = HAL_GetTick();
}

static void FRONTPANEL_ENCODER_Rotated(float32_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	if (TRX.Locked || LCD_window.opened)
		return;
	if (LCD_systemMenuOpened)
	{
		FRONTPANEL_ProcessEncoder1 = (int8_t)direction;
		return;
	}
	if (fabsf(direction) <= ENCODER_MIN_RATE_ACCELERATION)
		direction = (direction < 0.0f) ? -1.0f : 1.0f;

	uint32_t newfreq = 0;
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
		TRX.CLAR = BANDS[band].channels_clar;
		if(BANDS[band].channels_clar)
			TRX_setFrequency(BANDS[band].channels[new_channel].txFreq, SecondaryVFO);
		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
	}
	else if (TRX.Fast)
	{
		uint32_t step = TRX.FRQ_FAST_STEP;
		if (CurrentVFO->Mode == TRX_MODE_CW)
					step = step / TRX.FRQ_CW_STEP_DIVIDER;
		newfreq = (uint32_t)((int32_t)CurrentVFO->Freq + (int32_t)((float32_t)step * direction));
		if ((CurrentVFO->Freq % step) > 0 && fabsf(direction) <= 1.0f)
			newfreq = CurrentVFO->Freq / step * step;
	}
	else
	{
		uint32_t step = TRX.FRQ_STEP;
		if (CurrentVFO->Mode == TRX_MODE_CW)
					step = step / TRX.FRQ_CW_STEP_DIVIDER;
		newfreq = (uint32_t)((int32_t)CurrentVFO->Freq + (int32_t)((float32_t)step * direction));
		if ((CurrentVFO->Freq % step) > 0 && fabsf(direction) <= 1.0f)
			newfreq = CurrentVFO->Freq / step * step;
	}
	TRX_setFrequency(newfreq, CurrentVFO);
	LCD_UpdateQuery.FreqInfo = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_ENCODER2_Rotated(int8_t direction) // rotated encoder, handler here, direction -1 - left, 1 - right
{
	if (TRX.Locked || LCD_window.opened)
		return;

	if (LCD_systemMenuOpened)
	{
		SYSMENU_eventSecRotateSystemMenu(direction);
		return;
	}

	//NOTCH - default action
	if (CurrentVFO->ManualNotchFilter)
	{
		if (CurrentVFO->NotchFC > 50 && direction < 0)
			CurrentVFO->NotchFC -= 50;
		else if (CurrentVFO->NotchFC < CurrentVFO->LPF_RX_Filter_Width && direction > 0)
			CurrentVFO->NotchFC += 50;
		LCD_UpdateQuery.StatusInfoGUI = true;
		NeedReinitNotch = true;
	}
	else
	{
		if (!enc2_func_mode || CurrentVFO->Mode != TRX_MODE_CW)
		{
			uint32_t newfreq = 0;
			float64_t freq_round = 0;
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
				TRX.CLAR = BANDS[band].channels_clar;
				if(BANDS[band].channels_clar)
					TRX_setFrequency(BANDS[band].channels[new_channel].txFreq, SecondaryVFO);
				LCD_UpdateQuery.FreqInfoRedraw = true;
				LCD_UpdateQuery.StatusInfoGUI = true;
				LCD_UpdateQuery.StatusInfoBarRedraw = true;
			}
			else if (TRX.Fast)
			{
				step = (float64_t)TRX.FRQ_ENC_FAST_STEP;
				if (CurrentVFO->Mode == TRX_MODE_WFM)
					step = step * 2.0;
				if (CurrentVFO->Mode == TRX_MODE_CW)
					step = step / TRX.FRQ_CW_STEP_DIVIDER;
				freq_round = roundf((float64_t)CurrentVFO->Freq / step) * step;
				newfreq = (uint32_t)((int32_t)freq_round + (int32_t)step * direction);
			}
			else
			{
				step = (float64_t)TRX.FRQ_ENC_STEP;
				if (CurrentVFO->Mode == TRX_MODE_WFM)
					step = step * 2.0;
				if (CurrentVFO->Mode == TRX_MODE_CW)
					step = step / TRX.FRQ_CW_STEP_DIVIDER;
				freq_round = roundf((float64_t)CurrentVFO->Freq / step) * step;
				newfreq = (uint32_t)((int32_t)freq_round + (int32_t)step * direction);
			}
			TRX_setFrequency(newfreq, CurrentVFO);
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
	//ENC2 CLICK
	if (CurrentVFO->Mode == TRX_MODE_CW && !LCD_systemMenuOpened && !LCD_window.opened)
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

static void FRONTPANEL_ENC2SW_hold_handler(uint32_t parameter)
{
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

#ifndef HAS_TOUCHPAD
	FRONTPANEL_check_ENC2SW();
#endif

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
			//Yaesu MH-36
			if(CALIBRATE.TangentType == TANGENT_MH36)
				for (uint16_t tb = 0; tb < (sizeof(PERIPH_FrontPanel_TANGENT_MH36) / sizeof(PERIPH_FrontPanel_Button)); tb++)
					if(button->channel == PERIPH_FrontPanel_TANGENT_MH36[tb].channel)
						FRONTPANEL_CheckButton(&PERIPH_FrontPanel_TANGENT_MH36[tb], mcp3008_value);
			
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

static void FRONTPANEL_CheckButton(PERIPH_FrontPanel_Button *button, uint16_t mcp3008_value)
{
	// AF_GAIN
	if (button->type == FUNIT_CTRL_AF_GAIN)
	{
		TRX_Volume = (uint16_t)(1023.0f - mcp3008_value);
		if (TRX_Volume < 50)
			TRX_Volume = 0;
	}

	// SHIFT or IF Gain
	if (button->type == FUNIT_CTRL_SHIFT)
	{
		if (TRX.ShiftEnabled)
		{
			int_fast16_t TRX_SHIFT_old = TRX_SHIFT;
			TRX_SHIFT = (int_fast16_t)(((1023.0f - mcp3008_value) * TRX.SHIFT_INTERVAL * 2 / 1023.0f) - TRX.SHIFT_INTERVAL);
			if (TRX_SHIFT_old != TRX_SHIFT)
			{
				TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
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
			TRX.IF_Gain = (uint8_t)(0.0f + ((1023.0f - mcp3008_value) * 60.0f / 1023.0f));
		}
	}

	// PTT
	if (button->type == FUNIT_CTRL_PTT)
	{
		static bool frontunit_ptt_state_prev = false;
		bool frontunit_ptt_state_now = false;
		if (mcp3008_value > button->tres_min && mcp3008_value < button->tres_max)
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
		//set state
		if (mcp3008_value >= button->tres_min && mcp3008_value < button->tres_max)
		{
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
	TRX.ANT = TRX.BANDS_SAVED_SETTINGS[band].ANT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	CurrentVFO->FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
	CurrentVFO->DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	CurrentVFO->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	CurrentVFO->SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_shadow = CurrentVFO->FM_SQL_threshold;

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
	TRX_ScanMode = false;
}

void FRONTPANEL_BUTTONHANDLER_TUNE(uint32_t parameter)
{
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

void FRONTPANEL_BUTTONHANDLER_ATT(uint32_t parameter)
{
	TRX.ATT = !TRX.ATT;

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

void FRONTPANEL_BUTTONHANDLER_ANT(uint32_t parameter)
{
	TRX.ANT = !TRX.ANT;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].ANT = TRX.ANT;

	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
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

	TRX_setMode((uint8_t)mode, CurrentVFO);
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].Mode = (uint8_t)mode;
	TRX_Temporary_Stop_BandMap = true;
	resetVAD();
	TRX_ScanMode = false;
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
	else if (mode == TRX_MODE_DIGI_L || mode == TRX_MODE_DIGI_U)
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
	TRX_ScanMode = false;
}

static void FRONTPANEL_BUTTONHANDLER_BAND_P(uint32_t parameter)
{
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
	TRX.ANT = TRX.BANDS_SAVED_SETTINGS[band].ANT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	CurrentVFO->FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
	CurrentVFO->DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	CurrentVFO->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	CurrentVFO->SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_shadow = CurrentVFO->FM_SQL_threshold;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	resetVAD();
	TRX_ScanMode = false;
}

static void FRONTPANEL_BUTTONHANDLER_BAND_N(uint32_t parameter)
{
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
	TRX.ANT = TRX.BANDS_SAVED_SETTINGS[band].ANT;
	TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	CurrentVFO->DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	CurrentVFO->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	CurrentVFO->SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	CurrentVFO->FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_shadow = CurrentVFO->FM_SQL_threshold;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	resetVAD();
	TRX_ScanMode = false;
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
		else if (CurrentVFO->Mode == TRX_MODE_AM)
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
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_SHIFT(uint32_t parameter)
{
	TRX.ShiftEnabled = !TRX.ShiftEnabled;
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_CLAR(uint32_t parameter)
{
	TRX.CLAR = !TRX.CLAR;
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
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].SQL = CurrentVFO->SQL;
	
	LCD_UpdateQuery.TopButtons = true;
	NeedSaveSettings = true;
}

static void FRONTPANEL_BUTTONHANDLER_SCAN(uint32_t parameter)
{
	TRX_ScanMode = !TRX_ScanMode;
}

static void FRONTPANEL_BUTTONHANDLER_PLAY(uint32_t parameter)
{
	if (SD_RecordInProcess)
		SD_NeedStopRecord = true;
	
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
		if (!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].work_in_menu)
			PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].clickHandler(0);
}

static void FRONTPANEL_BUTTONHANDLER_FUNCH(uint32_t parameter)
{
	if (parameter == 7 && LCD_systemMenuOpened)
	{
		SYSMENU_hiddenmenu_enabled = true;
		LCD_redraw(false);
	}
	else if (!TRX.Locked || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].holdHandler == FRONTPANEL_BUTTONHANDLER_LOCK) //LOCK BUTTON
		if (!LCD_systemMenuOpened || PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].work_in_menu)
			PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + parameter]].holdHandler(0);
}

static void FRONTPANEL_BUTTONHANDLER_UP(uint32_t parameter)
{
	uint32_t newfreq = CurrentVFO->Freq + 500;
	newfreq = newfreq / 500 * 500;
	TRX_setFrequency(newfreq, CurrentVFO);
	LCD_UpdateQuery.FreqInfo = true;
}

static void FRONTPANEL_BUTTONHANDLER_DOWN(uint32_t parameter)
{
	uint32_t newfreq = CurrentVFO->Freq - 500;
	newfreq = newfreq / 500 * 500;
	TRX_setFrequency(newfreq, CurrentVFO);
	LCD_UpdateQuery.FreqInfo = true;
}

void FRONTPANEL_BUTTONHANDLER_SETBAND(uint32_t parameter)
{
	int8_t band = parameter;
	if (band >= BANDS_COUNT)
		band = 0;

	//manual freq enter
	if(TRX.BANDS_SAVED_SETTINGS[band].Freq == TRX.VFO_A.Freq)
	{
		TRX_Temporary_Stop_BandMap = false;
		resetVAD();
		TRX_ScanMode = false;
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
	TRX.ANT = TRX.BANDS_SAVED_SETTINGS[band].ANT;
	TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
	TRX.VFO_A.SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	TRX.VFO_A.FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
	TRX.SQL_shadow = CurrentVFO->SQL;
	TRX.FM_SQL_threshold_shadow = CurrentVFO->FM_SQL_threshold;
	TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
	TRX.VFO_A.DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	TRX.VFO_A.AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	
	resetVAD();
	TRX_ScanMode = false;
	LCD_closeWindow();
}

void FRONTPANEL_BUTTONHANDLER_SETSECBAND(uint32_t parameter)
{
	int8_t band = parameter;
	if (band >= BANDS_COUNT)
		band = 0;

	//manual freq enter
	if(TRX.BANDS_SAVED_SETTINGS[band].Freq == TRX.VFO_B.Freq)
	{
		TRX_Temporary_Stop_BandMap = false;
		resetVAD();
		TRX_ScanMode = false;
		LCD_closeWindow();
		LCD_redraw(true);
		LCD_showManualFreqWindow(true);
		return;
	}
	//
	
	TRX_setFrequency(TRX.BANDS_SAVED_SETTINGS[band].Freq, &TRX.VFO_B);
	TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, &TRX.VFO_B);
	TRX.VFO_B.FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
	TRX.VFO_B.DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
	TRX.VFO_B.AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
	TRX.VFO_B.SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
	TRX_Temporary_Stop_BandMap = false;

	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	
	resetVAD();
	TRX_ScanMode = false;
	LCD_closeWindow();
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
	TRX_ScanMode = false;
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
	TRX_ScanMode = false;
	LCD_closeWindow();
}

void FRONTPANEL_BUTTONHANDLER_SET_RX_BW(uint32_t parameter)
{
	if (CurrentVFO->Mode == TRX_MODE_CW)
		TRX.CW_LPF_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U)
		TRX.SSB_LPF_RX_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_AM)
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
	if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U)
		TRX.SSB_LPF_TX_Filter = parameter;
	if (CurrentVFO->Mode == TRX_MODE_AM)
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

void FRONTPANEL_BUTTONHANDLER_LEFT_ARR(uint32_t parameter)
{
	if (FRONTPANEL_funcbuttons_page == 0)
		FRONTPANEL_funcbuttons_page = (FUNCBUTTONS_PAGES - 1);
	else
		FRONTPANEL_funcbuttons_page--;

	LCD_UpdateQuery.BottomButtons = true;
	LCD_UpdateQuery.TopButtons = true;
}

void FRONTPANEL_BUTTONHANDLER_RIGHT_ARR(uint32_t parameter)
{
	if (FRONTPANEL_funcbuttons_page >= (FUNCBUTTONS_PAGES - 1))
		FRONTPANEL_funcbuttons_page = 0;
	else
		FRONTPANEL_funcbuttons_page++;

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
