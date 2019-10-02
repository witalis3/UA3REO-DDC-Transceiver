#ifndef LCD_h
#define LCD_h

#include "stm32f4xx_hal.h"
#include "trx_manager.h"
#include "lcd_driver.h"

#define COLOR_BUTTON_TEXT COLOR_DGREEN
#define COLOR_BUTTON_INACTIVE_TEXT rgb888torgb565(150, 150, 150)

#define METER_WIDTH 220

typedef struct {
	bool Background;
	bool TopButtons;
	bool FreqInfo;
	bool StatusInfoGUI;
	bool StatusInfoBar;
	bool SystemMenu;
	bool TextBar;
} DEF_LCD_UpdateQuery;

extern IWDG_HandleTypeDef hiwdg;

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
