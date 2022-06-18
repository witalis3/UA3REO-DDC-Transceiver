#ifndef FRONT_UNIT_h
#define FRONT_UNIT_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include "settings.h"

#define MCP3008_SINGLE_THRESHOLD 300

typedef enum
{
	FUNIT_CTRL_BUTTON,
	FUNIT_CTRL_AF_GAIN,
	FUNIT_CTRL_RIT_XIT,
	FUNIT_CTRL_PTT,
	FUNIT_CTRL_TANGENT,
	FUNIT_CTRL_BUTTON_DEBUG,
	FUNIT_CTRL_ENC2SW,
} FRONT_UNIT_CONTROL_TYPE;

typedef struct
{
	bool work_in_menu;
	char name[16];
	void (*clickHandler)(uint32_t parameter);
	void (*holdHandler)(uint32_t parameter);
	uint32_t *checkBool;
} PERIPH_FrontPanel_FuncButton;

typedef struct
{
  uint32_t parameter;
  uint32_t start_hold_time;
	void (*clickHandler)(uint32_t parameter);
	void (*holdHandler)(uint32_t parameter);
  uint16_t tres_min;
	uint16_t tres_max;
	uint8_t port;
	uint8_t channel;
	bool state;
	bool prev_state;
	bool afterhold;
	bool work_in_menu;
  FRONT_UNIT_CONTROL_TYPE type;
	char name[16];
} PERIPH_FrontPanel_Button;

extern PERIPH_FrontPanel_Button PERIPH_FrontPanel_TANGENT_MH48[6];
extern PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[];
extern const PERIPH_FrontPanel_FuncButton PERIPH_FrontPanel_FuncButtonsList[FUNCBUTTONS_COUNT];
extern bool FRONTPanel_MCP3008_1_Enabled;
extern bool FRONTPanel_MCP3008_2_Enabled;
extern bool FRONTPanel_MCP3008_3_Enabled;

extern void FRONTPANEL_ENCODER_checkRotate(void);
extern void FRONTPANEL_ENCODER2_checkRotate(void);
extern void FRONTPANEL_check_ENC2SW(bool state);
extern void FRONTPANEL_Init(void);
extern void FRONTPANEL_Process(void);
extern void FRONTPANEL_CheckButton(PERIPH_FrontPanel_Button *button, uint16_t mcp3008_value);

extern void FRONTPANEL_BUTTONHANDLER_DOUBLE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_DOUBLEMODE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_PRE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_ATT(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_ATTHOLD(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_PGA(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_PGA_ONLY(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_DRV_ONLY(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_AGC(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_AGC_SPEED(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_DNR(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_DNR_HOLD(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_NB(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_NOTCH(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_FAST(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_MUTE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_MUTE_AFAMP(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_AsB(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_ArB(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_TUNE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_RF_POWER(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_ANT(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_BW(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_HPF(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SERVICES(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_MENU(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_MENUHOLD(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_LOCK(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SET_CUR_VFO_BAND(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SET_VFOA_BAND(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SET_VFOB_BAND(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SET_BAND_MEMORY(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_GET_BAND_MEMORY(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SETMODE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SETSECMODE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SET_RX_BW(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SET_TX_BW(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SETRF_POWER(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SET_ATT_DB(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_LEFT_ARR(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_RIGHT_ARR(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SQL(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SQUELCH(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_ZOOM_N(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_ZOOM_P(uint32_t parameter);
extern void FRONTPANEL_SelectMemoryChannelsButtonHandler(uint32_t parameter);
extern void FRONTPANEL_SaveMemoryChannelsButtonHandler(uint32_t parameter);

#endif
