#ifndef LCD_h
#define LCD_h

#include "hardware.h"
#include "trx_manager.h"
#include "lcd_driver.h"
#include "touchpad.h"
#include "screen_layout.h"

typedef struct
{
	bool Background;
	bool TopButtons;
	bool TopButtonsRedraw;
	bool BottomButtons;
	bool BottomButtonsRedraw;
	bool FreqInfo;
	bool FreqInfoRedraw;
	bool StatusInfoGUI;
	bool StatusInfoGUIRedraw;
	bool StatusInfoBar;
	bool StatusInfoBarRedraw;
	bool SystemMenu;
	bool SystemMenuRedraw;
	bool SystemMenuCurrent;
	bool SystemMenuInfolines;
	bool TextBar;
	bool Tooltip;
} DEF_LCD_UpdateQuery;

typedef struct
{
	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;
	uint32_t parameter;
	void (*clickHandler)(uint32_t parameter);
	void (*holdHandler)(uint32_t parameter);
} TouchpadButton_handler;

typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
	bool opened;
	TouchpadButton_handler buttons[64];
	uint16_t buttons_count;
} WindowType;

extern void LCD_Init(void);
extern bool LCD_doEvents(void);
extern void LCD_showError(char text[], bool redraw);
extern void LCD_showInfo(char text[], bool autohide);
extern void LCD_redraw(bool do_now);
extern void LCD_processTouch(uint16_t x, uint16_t y);
extern void LCD_processHoldTouch(uint16_t x, uint16_t y);
extern bool LCD_processSwipeTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy);
extern bool LCD_processSwipeTwoFingersTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy);
extern void LCD_showTooltip(char text[]);
extern void LCD_openWindow(uint16_t w, uint16_t h);
extern void LCD_closeWindow(void);
extern void LCD_showRFPowerWindow(void);
extern void LCD_showManualFreqWindow(bool secondary_vfo);
extern void LCD_printKeyboard(void (*keyboardHandler)(uint32_t parameter), bool lowcase);
extern void LCD_hideKeyboard(void);

volatile extern DEF_LCD_UpdateQuery LCD_UpdateQuery;
volatile extern bool LCD_busy;
volatile extern bool LCD_systemMenuOpened;
extern uint16_t LCD_bw_trapez_stripe_pos;
extern WindowType LCD_window;
extern STRUCT_COLOR_THEME *COLOR;
extern STRUCT_LAYOUT_THEME *LAYOUT;
extern bool TRX_X1_VLT_CUR_Mode; //false - VLT, true - CUR

#endif
