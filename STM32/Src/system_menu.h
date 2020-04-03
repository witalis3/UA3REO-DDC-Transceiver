#ifndef SYSTEM_MENU_H
#define SYSTEM_MENU_H

#include "stm32h7xx.h"
#include <stdbool.h>

typedef enum
{
	SYSMENU_BOOLEAN = 0x01,
	SYSMENU_RUN = 0x02,
	SYSMENU_UINT8 = 0x03,
	SYSMENU_UINT16 = 0x04,
	SYSMENU_UINT32 = 0x05,
	SYSMENU_UINT32R = 0x06,
	SYSMENU_INT8 = 0x07,
	SYSMENU_INT16 = 0x08,
	SYSMENU_INT32 = 0x09,
	SYSMENU_FLOAT32 = 0x0A,
	SYSMENU_MENU = 0x0B,
	SYSMENU_HIDDEN_MENU = 0x0C,
} SystemMenuType;

struct sysmenu_item_handler
{
	char *title;
	SystemMenuType type;
	uint32_t *value;
	void (*menuHandler)(int8_t direction);
};

extern void drawSystemMenu(bool draw_background);
extern void eventRotateSystemMenu(int8_t direction);
extern void eventSecRotateSystemMenu(int8_t direction);
extern void eventCloseSystemMenu(void);
extern bool sysmenu_spectrum_opened;
extern bool sysmenu_hiddenmenu_enabled;
extern void SYSMENU_RFPOWER_HOTKEY(void);
extern void SYSMENU_CW_WPM_HOTKEY(void);
extern void SYSMENU_AUDIO_SSB_HOTKEY(void);
extern void SYSMENU_AUDIO_CW_HOTKEY(void);
extern void SYSMENU_AUDIO_AM_HOTKEY(void);
extern void SYSMENU_AUDIO_FM_HOTKEY(void);

#endif
