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

//Private Variables
char entered_locator[32] = {0};

// start
void LOCINFO_Start(void)
{
	LCD_busy = true;

	memset(entered_locator, 0x00, sizeof(entered_locator));
	
	// draw the GUI
	LCDDriver_Fill(BG_COLOR);

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

// stop
void LOCINFO_Stop(void)
{
	LCD_hideKeyboard();
}

static void LOCINFO_keyboardHandler(uint32_t parameter)
{
	char str[2] = {0};
	str[0] = parameter;
	if(parameter == '<') //backspace
	{
		if(strlen(entered_locator) > 0)
			entered_locator[strlen(entered_locator) - 1] = 0;
	}
	else if(strlen(entered_locator) < 8)
		strcat(entered_locator, str);
	
	LCD_UpdateQuery.SystemMenuRedraw = true;
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
	
	char tmp[64] = {0};
	sprintf(tmp, "Enter Locator: %s", entered_locator);
	addSymbols(tmp, tmp, 15+8, " ", true);
	LCDDriver_printText(tmp, 10, 40, FG_COLOR, BG_COLOR, 2);
	
	LCD_keyboardHandler = LOCINFO_keyboardHandler;
	LCD_printKeyboard();

	LCD_busy = false;
	//LCD_UpdateQuery.SystemMenuRedraw = true;
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
