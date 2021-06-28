#include "locator.h"
#include "main.h"
#include "lcd_driver.h"
#include "trx_manager.h"
#include "functions.h"
#include "fpga.h"
#include "lcd.h"
#include "fonts.h"

//Public variables
bool SYSMENU_locator_info_opened = false;

// start
void LOCINFO_Start(void)
{
	LCD_busy = true;

	// draw the GUI
	LCDDriver_Fill(BG_COLOR);
	LCDDriver_printTextFont("Locator Info", 10, 30, FG_COLOR, BG_COLOR, &FreeSans9pt7b);

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

// stop
void LOCINFO_Stop(void)
{
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
}

// draw
void LOCINFO_Draw(void)
{
	if (LCD_busy)
	{
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	LCD_busy = true;

	//events

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// events to the encoder
void LOCINFO_EncRotate(int8_t direction)
{
#pragma unused(direction)
	if (LCD_busy)
		return;
	LCD_busy = true;

	LCD_busy = false;
}
