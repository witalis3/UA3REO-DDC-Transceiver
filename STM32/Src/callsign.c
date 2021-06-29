#include "locator.h"
#include "main.h"
#include "lcd_driver.h"
#include "trx_manager.h"
#include "functions.h"
#include "fpga.h"
#include "lcd.h"
#include "fonts.h"

//Public variables
bool SYSMENU_callsign_info_opened = false;

//Private Variables
char entered_callsign[32] = {0};

//Prototypes

// start
void CALSIGN_INFO_Start(void)
{
	LCD_busy = true;

	memset(entered_callsign, 0x00, sizeof(entered_callsign));
	
	// draw the GUI
	LCDDriver_Fill(BG_COLOR);

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

// stop
void CALSIGN_INFO_Stop(void)
{
	LCD_hideKeyboard();
}

static void CALSIGN_INFO_keyboardHandler(uint32_t parameter)
{
	char str[2] = {0};
	str[0] = parameter;
	if(parameter == '<') //backspace
	{
		if(strlen(entered_callsign) > 0)
			entered_callsign[strlen(entered_callsign) - 1] = 0;
	}
	else if(strlen(entered_callsign) < 8)
		strcat(entered_callsign, str);
	
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// draw
void CALSIGN_INFO_Draw(void)
{
	if (LCD_busy)
	{
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	LCD_busy = true;
	
	char tmp[64] = {0};
	
	sprintf(tmp, "Enter Callsign: %s", entered_callsign);
	addSymbols(tmp, tmp, 15+8, " ", true);
	LCDDriver_printText(tmp, 10, 30, FG_COLOR, BG_COLOR, 2);
	
	LCD_printKeyboard(CALSIGN_INFO_keyboardHandler);

	LCD_busy = false;
}

// events to the encoder
void CALSIGN_INFO_EncRotate(int8_t direction)
{
	/*if (LCD_busy)
		return;
	LCD_busy = true;

	LCD_busy = false;*/
}
