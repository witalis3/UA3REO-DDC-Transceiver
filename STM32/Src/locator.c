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

//Prototypes
static float32_t LOCINFO_get_latlon_from_locator(char *locator, bool return_lat);

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
	LCDDriver_printText(tmp, 10, 30, FG_COLOR, BG_COLOR, 2);
	
	sprintf(tmp, "LAT: %f", LOCINFO_get_latlon_from_locator(entered_locator, true));
	addSymbols(tmp, tmp, 15+8, " ", true);
	LCDDriver_printText(tmp, 10, 50, FG_COLOR, BG_COLOR, 2);
	
	sprintf(tmp, "LON: %f", LOCINFO_get_latlon_from_locator(entered_locator, false));
	addSymbols(tmp, tmp, 15+8, " ", true);
	LCDDriver_printText(tmp, 10, 70, FG_COLOR, BG_COLOR, 2);
	
	LCD_keyboardHandler = LOCINFO_keyboardHandler;
	LCD_printKeyboard();

	LCD_busy = false;
}

// events to the encoder
void LOCINFO_EncRotate(int8_t direction)
{
	/*if (LCD_busy)
		return;
	LCD_busy = true;

	LCD_busy = false;*/
}

static float32_t LOCINFO_get_latlon_from_locator(char *locator, bool return_lat)
{
	//origin
	float32_t lon = -180.0f; //Positive: East, negative: West.
	float32_t lat = -90.0f; //Positive: North, negative: South.
	
	if(strlen(locator) >= 1)
		lon += (locator[0] - 65) * 20.0f;
	if(strlen(locator) >= 2)
		lat += (locator[1] - 65) * 10.0f;
	if(strlen(locator) >= 3)
		lon += charToInt(locator[2]) * 2.0f;
	if(strlen(locator) >= 4)
		lat += charToInt(locator[3]) * 1.0f;
	if(strlen(locator) >= 5)
		lon += (locator[4] - 65) * (5.0f / 60.0f);
	if(strlen(locator) >= 6)
		lat += (locator[5] - 65) * (2.5f / 60.0f);
	
	//averaging
	if(strlen(locator) >= 5)
	{
		lon += (2.5f / 60.0f);
		lat += (1.25f / 60.0f);
	}
	else if(strlen(locator) >= 3)
	{
		//averaging
		lon += 1.0f;
		lat += 0.5f;
	}
	else if(strlen(locator) >= 1)
	{
		//averaging
		lon += 10.0f;
		lat += 5.0f;
	}
	
	if (return_lat)
		return lat;
	else
		return lon;
}
