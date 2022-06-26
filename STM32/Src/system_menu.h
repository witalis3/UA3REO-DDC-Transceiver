#ifndef SYSTEM_MENU_H
#define SYSTEM_MENU_H

#include "stm32h7xx.h"
#include <stdbool.h>
#include <stdint.h>
#include "lcd_driver.h"

#define ENUM_MAX_COUNT 9

#ifdef LAY_160x128
#define ENUM_MAX_LENGTH 7
#else
#define ENUM_MAX_LENGTH 8
#endif

typedef enum
{
	SYSMENU_BOOLEAN,
	SYSMENU_RUN,
	SYSMENU_UINT8,
	SYSMENU_UINT16,
	SYSMENU_UINT32,
	SYSMENU_UINT32R,
	SYSMENU_UINT64,
	SYSMENU_INT8,
	SYSMENU_INT16,
	SYSMENU_INT32,
	SYSMENU_FLOAT32,
	SYSMENU_MENU,
	SYSMENU_INFOLINE,
	SYSMENU_FUNCBUTTON,
	SYSMENU_ENUM,
	SYSMENU_ENUMR,
	SYSMENU_B4,
	SYSMENU_ATU_I,
	SYSMENU_ATU_C,
} SystemMenuType;

struct sysmenu_item_handler
{
	char *title;
	SystemMenuType type;
	bool (*checkVisibleHandler)(void);
	uint32_t *value;
	void (*menuHandler)(int8_t direction);
	char enumerate[ENUM_MAX_COUNT][ENUM_MAX_LENGTH];
};

struct sysmenu_menu_wrapper
{
	const struct sysmenu_item_handler *menu_handler;
	uint8_t currentIndex;
};

extern bool SYSMENU_FT8_DECODER_opened;
extern bool sysmenu_ota_opened;
extern uint8_t sysmenu_ota_opened_state;

extern void SYSMENU_drawSystemMenu(bool draw_background, bool only_infolines);
extern void SYSMENU_redrawCurrentItem(void);
extern void SYSMENU_eventRotateSystemMenu(int8_t direction);
extern void SYSMENU_eventSecEncoderClickSystemMenu(void);
extern void SYSMENU_eventSecRotateSystemMenu(int8_t direction);
extern void SYSMENU_eventCloseSystemMenu(void);
extern void SYSMENU_eventCloseAllSystemMenu(void);
extern bool SYSMENU_spectrum_opened;
extern bool SYSMENU_hiddenmenu_enabled;
extern void SYSMENU_TRX_RFPOWER_HOTKEY(void);
extern void SYSMENU_TRX_STEP_HOTKEY(void);
extern void SYSMENU_CW_WPM_HOTKEY(void);
extern void SYSMENU_CW_KEYER_HOTKEY(void);
extern void SYSMENU_AUDIO_IF_HOTKEY(void);
extern void SYSMENU_AUDIO_BW_SSB_HOTKEY(void);
extern void SYSMENU_AUDIO_BW_CW_HOTKEY(void);
extern void SYSMENU_AUDIO_BW_AM_HOTKEY(void);
extern void SYSMENU_AUDIO_BW_FM_HOTKEY(void);
extern void SYSMENU_AUDIO_HPF_SSB_HOTKEY(void);
extern void SYSMENU_AUDIO_SQUELCH_HOTKEY(void);
extern void SYSMENU_AUDIO_DNR_HOTKEY(void);
extern void SYSMENU_AUDIO_AGC_HOTKEY(void);
extern void SYSMENU_SERVICE_FT8_HOTKEY(void);
extern void SYSMENU_HANDL_SERVICESMENU(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_SSB_HPF_RX_pass(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_SSB_HPF_TX_pass(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_CW_LPF_pass(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_DIGI_LPF_pass(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_SSB_LPF_RX_pass(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_SSB_LPF_TX_pass(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_AM_LPF_RX_pass(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_AM_LPF_TX_pass(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_FM_LPF_RX_pass(int8_t direction);
extern void SYSMENU_HANDL_AUDIO_FM_LPF_TX_pass(int8_t direction);

extern bool SYSMENU_HANDL_CHECK_HAS_ATU(void);

#endif
