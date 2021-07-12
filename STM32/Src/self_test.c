#include "self_test.h"
#include "main.h"
#include "lcd_driver.h"
#include "trx_manager.h"
#include "functions.h"
#include "fpga.h"
#include "lcd.h"
#include "fonts.h"
#include "front_unit.h"

//Public variables
bool SYSMENU_selftest_opened = false;

//Private Variables
static int8_t SELF_TEST_current_page = 0;

//Prototypes
static void SELF_TEST_printResult(bool result, uint16_t pos_y);

// start
void SELF_TEST_Start(void)
{
	LCD_busy = true;

	SELF_TEST_current_page = 0;
	
	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

// stop
void SELF_TEST_Stop(void)
{
	
}

// draw
void SELF_TEST_Draw(void)
{
	if (LCD_busy)
	{
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	LCD_busy = true;
	
	// draw the GUI
	LCDDriver_Fill(BG_COLOR);

	//predefine
	#define margin_left 5
	#define margin_bottom 20
	uint16_t pos_y = margin_left;
	bool pass = true;
	
	//print pages
	if(SELF_TEST_current_page == 0)
	{
		//FPGA BUS test
		LCDDriver_printText("FPGA", 5, pos_y, FG_COLOR, BG_COLOR, 2);
		SELF_TEST_printResult(FPGA_bus_test_result, pos_y);
		pos_y += margin_bottom;
		
		//MCP3008 test
		pass = true;
		#ifdef HRDW_MCP3008_1
		if(!FRONTPanel_MCP3008_1_Enabled)
			pass = false;
		#endif
		#ifdef HRDW_MCP3008_2
		if(!FRONTPanel_MCP3008_2_Enabled)
			pass = false;
		#endif
		#ifdef HRDW_MCP3008_3
		if(!FRONTPanel_MCP3008_3_Enabled)
			pass = false;
		#endif
		if(FPGA_bus_test_result)
		{
			LCDDriver_printText("MCP3008", 5, pos_y, FG_COLOR, BG_COLOR, 2);
			SELF_TEST_printResult(pass, pos_y);
			pos_y += margin_bottom;
		}
		
		//STM32 EEPROM test
		if(FPGA_bus_test_result)
		{
			LCDDriver_printText("STM32 EEPROM", 5, pos_y, FG_COLOR, BG_COLOR, 2);
			SELF_TEST_printResult(EEPROM_Enabled, pos_y);
			pos_y += margin_bottom;
		}
		
		//WM8731 test
		LCDDriver_printText("WM8731", 5, pos_y, FG_COLOR, BG_COLOR, 2);
		SELF_TEST_printResult(WM8731_test_result, pos_y);
		pos_y += margin_bottom;
	}
	
	//Pager
	pos_y += margin_bottom;
	LCDDriver_printText("Rotate ENC2 to print next page", 5, pos_y, FG_COLOR, BG_COLOR, 2);
	pos_y += margin_bottom;
	
	LCD_busy = false;
}

static void SELF_TEST_printResult(bool result, uint16_t pos_y)
{
	char pass[] = " OK";
	char error[] = " ERROR";
	
	if(result)
		LCDDriver_printText(pass, LCDDriver_GetCurrentXOffset(), pos_y, COLOR_GREEN, BG_COLOR, 2);
	else
		LCDDriver_printText(error, LCDDriver_GetCurrentXOffset(), pos_y, COLOR_GREEN, BG_COLOR, 2);
}

// events to the encoder
void SELF_TEST_EncRotate(int8_t direction)
{
	SELF_TEST_current_page += direction;
	if (SELF_TEST_current_page < 0)
		SELF_TEST_current_page = 0;
	if (SELF_TEST_current_page >= SELF_TEST_pages)
		SELF_TEST_current_page = SELF_TEST_pages - 1;
}
