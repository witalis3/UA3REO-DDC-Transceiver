#ifndef FRONT_UNIT_h
#define FRONT_UNIT_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define MCP3008_SINGLE_THRESHOLD 500

typedef enum
{
	FUNIT_CTRL_BUTTON,
	FUNIT_CTRL_AF_GAIN,
	FUNIT_CTRL_SHIFT,
} FRONT_UNIT_CONTROL_TYPE;

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
	void (*clickHandler)(void);
	void (*holdHandler)(void);
} PERIPH_FrontPanel_Button;

extern PERIPH_FrontPanel_Button PERIPH_FrontPanel_Buttons[];

extern void FRONTPANEL_ENCODER_checkRotate(void);
extern void FRONTPANEL_ENCODER2_checkRotate(void);
extern void FRONTPANEL_check_ENC2SW(void);
extern void FRONTPANEL_Init(void);
extern void FRONTPANEL_Process(void);

extern void FRONTPANEL_BUTTONHANDLER_DOUBLE(void);
extern void FRONTPANEL_BUTTONHANDLER_DOUBLEMODE(void);
extern void FRONTPANEL_BUTTONHANDLER_PRE(void);
extern void FRONTPANEL_BUTTONHANDLER_ATT(void);
extern void FRONTPANEL_BUTTONHANDLER_ATTHOLD(void);
extern void FRONTPANEL_BUTTONHANDLER_PGA(void);
extern void FRONTPANEL_BUTTONHANDLER_PGA_ONLY(void);
extern void FRONTPANEL_BUTTONHANDLER_DRV_ONLY(void);
extern void FRONTPANEL_BUTTONHANDLER_AGC(void);
extern void FRONTPANEL_BUTTONHANDLER_AGC_SPEED(void);
extern void FRONTPANEL_BUTTONHANDLER_DNR(void);
extern void FRONTPANEL_BUTTONHANDLER_NB(void);
extern void FRONTPANEL_BUTTONHANDLER_NOTCH(void);
extern void FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL(void);
extern void FRONTPANEL_BUTTONHANDLER_FAST(void);
extern void FRONTPANEL_BUTTONHANDLER_MUTE(void);
extern void FRONTPANEL_BUTTONHANDLER_AsB(void);
extern void FRONTPANEL_BUTTONHANDLER_ArB(void);
extern void FRONTPANEL_BUTTONHANDLER_TUNE(void);
extern void FRONTPANEL_BUTTONHANDLER_RF_POWER(void);
extern void FRONTPANEL_BUTTONHANDLER_ANT(void);
extern void FRONTPANEL_BUTTONHANDLER_BW(void);
extern void FRONTPANEL_BUTTONHANDLER_HPF(void);
extern void FRONTPANEL_BUTTONHANDLER_SERVICES(void);
extern void FRONTPANEL_BUTTONHANDLER_MENU(void);
extern void FRONTPANEL_BUTTONHANDLER_LOCK(void);

#endif
