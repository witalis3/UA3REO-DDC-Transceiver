#ifndef LCD_h
#define LCD_h

#include "stm32h7xx_hal.h"
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
	bool StatusInfoBar;
	bool StatusInfoBarRedraw;
	bool SystemMenu;
	bool TextBar;
	bool Tooltip;
} DEF_LCD_UpdateQuery;

typedef struct
{
	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;
	void (*clickHandler)(void);
	void (*holdHandler)(void);
} TouchpadButton_handler;

extern void LCD_Init(void);
extern void LCD_doEvents(void);
extern void LCD_showError(char text[], bool redraw);
extern void LCD_showInfo(char text[], bool autohide);
extern void LCD_redraw(void);
extern void LCD_processTouch(uint16_t x, uint16_t y);
extern void LCD_processHoldTouch(uint16_t x, uint16_t y);
extern bool LCD_processSwipeTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy);
extern void LCD_showTooltip(char text[]);

volatile extern DEF_LCD_UpdateQuery LCD_UpdateQuery;
volatile extern bool LCD_busy;
volatile extern bool LCD_systemMenuOpened;
extern STRUCT_COLOR_THEME* COLOR_THEME;

#endif
