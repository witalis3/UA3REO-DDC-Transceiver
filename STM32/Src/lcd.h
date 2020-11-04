#ifndef LCD_h
#define LCD_h

#include "stm32h7xx_hal.h"
#include "trx_manager.h"
#include "lcd_driver.h"
#include "touchpad.h"

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
} DEF_LCD_UpdateQuery;

extern void LCD_Init(void);
extern void LCD_doEvents(void);
extern void LCD_showError(char text[], bool redraw);
extern void LCD_redraw(void);

volatile extern DEF_LCD_UpdateQuery LCD_UpdateQuery;
volatile extern bool LCD_busy;
volatile extern bool LCD_systemMenuOpened;

#endif
