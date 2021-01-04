#ifndef FRONT_UNIT_h
#define FRONT_UNIT_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define MCP3008_SINGLE_THRESHOLD 500
#define FUNCBUTTONS_COUNT 24
#define FUNCBUTTONS_ON_PAGE 8
#define FUNCBUTTONS_PAGES (FUNCBUTTONS_COUNT / FUNCBUTTONS_ON_PAGE)

typedef enum
{
	FUNIT_CTRL_BUTTON,
	FUNIT_CTRL_AF_GAIN,
	FUNIT_CTRL_SHIFT,
	FUNIT_CTRL_PTT,
} FRONT_UNIT_CONTROL_TYPE;

typedef struct
{
	bool work_in_menu;
	char name[16];
	void (*clickHandler)(uint32_t parameter);
	void (*holdHandler)(uint32_t parameter);
} PERIPH_FrontPanel_FuncButton;

typedef struct
{
	uint8_t port;
	uint8_t channel;
	FRONT_UNIT_CONTROL_TYPE type;
	uint16_t tres_min;
	uint16_t tres_max;
	bool state;
	bool prev_state;
	uint32_t start_hold_time;
	bool afterhold;
	bool work_in_menu;
	char name[16];
	uint32_t parameter;
	void (*clickHandler)(uint32_t parameter);
	void (*holdHandler)(uint32_t parameter);
} PERIPH_FrontPanel_Button;

extern PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[];
extern const PERIPH_FrontPanel_FuncButton PERIPH_FrontPanel_FuncButtonsList[FUNCBUTTONS_COUNT];
extern uint8_t FRONTPANEL_funcbuttons_page;

extern void FRONTPANEL_ENCODER_checkRotate(void);
extern void FRONTPANEL_ENCODER2_checkRotate(void);
extern void FRONTPANEL_check_ENC2SW(void);
extern void FRONTPANEL_Init(void);
extern void FRONTPANEL_Process(void);

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
extern void FRONTPANEL_BUTTONHANDLER_NB(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_NOTCH(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_FAST(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_MUTE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_AsB(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_ArB(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_TUNE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_RF_POWER(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_ANT(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_BW(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_HPF(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SERVICES(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_MENU(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_LOCK(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SETBAND(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SETSECBAND(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SETMODE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SETSECMODE(uint32_t parameter);
extern void FRONTPANEL_BUTTONHANDLER_SETBW(uint32_t parameter);

#endif
