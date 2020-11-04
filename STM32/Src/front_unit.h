#ifndef FRONT_UNIT_h
#define FRONT_UNIT_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define MCP3008_THRESHOLD 100

typedef struct
{
	uint8_t port;
	uint8_t channel;
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
extern void FRONTPANEL_check_ENC2SW_and_Touchpad(void);
extern void FRONTPANEL_Init(void);
extern void FRONTPANEL_Process(void);

#endif
