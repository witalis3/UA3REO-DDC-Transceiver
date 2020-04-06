#ifndef LCD_h
#define LCD_h

#include "stm32h7xx_hal.h"
#include "trx_manager.h"
#include "lcd_driver.h"

typedef struct 
{
	bool Background;
	bool TopButtons;
	bool FreqInfo;
	bool StatusInfoGUI;
	bool StatusInfoBar;
	bool SystemMenu;
	bool TextBar;
} DEF_LCD_UpdateQuery;

extern void LCD_Init(void);
extern void LCD_doEvents(void);
extern void LCD_showError(char text[], bool redraw);
extern void LCD_redraw(void);

volatile extern DEF_LCD_UpdateQuery LCD_UpdateQuery;
volatile extern bool LCD_busy;
volatile extern bool LCD_timeMenuOpened;
volatile extern bool LCD_systemMenuOpened;
volatile extern uint8_t TimeMenuSelection;

#endif
