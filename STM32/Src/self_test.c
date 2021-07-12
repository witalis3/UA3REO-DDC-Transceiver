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
static bool SELF_TEST_old_autogainer = false;
static uint32_t SELF_TEST_old_freq = SELF_TEST_frequency;

//Prototypes
static void SELF_TEST_printResult(bool result, uint16_t pos_y);

// start
void SELF_TEST_Start(void)
{
	LCD_busy = true;

	SELF_TEST_old_autogainer = TRX.AutoGain;
	SELF_TEST_old_freq = CurrentVFO->Freq;
	SELF_TEST_current_page = 0;
	TRX_setFrequency(SELF_TEST_frequency, CurrentVFO);
	LCDDriver_Fill(BG_COLOR);
	
	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

// stop
void SELF_TEST_Stop(void)
{
	TRX.AutoGain = SELF_TEST_old_autogainer;
	TRX_setFrequency(SELF_TEST_old_freq, CurrentVFO);
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
	LCD_UpdateQuery.SystemMenu = false;
	LCD_UpdateQuery.SystemMenuRedraw = false;

	//predefine
	#define margin_left 5
	#define margin_bottom 20
	uint16_t pos_y = margin_left;
	char str[64] = {0};
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
		
		//ADC symmetry
		pass = true;
		if(TRX_ADC_MINAMPLITUDE > 0)
			pass = false;
		if(TRX_ADC_MAXAMPLITUDE < 0)
			pass = false;
		if(abs(TRX_ADC_MINAMPLITUDE) > abs(TRX_ADC_MAXAMPLITUDE) * 3)
			pass = false;
		if(abs(TRX_ADC_MINAMPLITUDE) * 3 > abs(TRX_ADC_MAXAMPLITUDE))
			pass = false;
		LCDDriver_printText("ADC Symmetry", 5, pos_y, FG_COLOR, BG_COLOR, 2);
		SELF_TEST_printResult(WM8731_test_result, pos_y);
		pos_y += margin_bottom;
	}
	
	if(SELF_TEST_current_page == 1)
	{
		static uint8_t current_test = 0;
		static uint32_t current_test_start_time = 0;
		
		LCDDriver_printText("Testing...", 5, pos_y, FG_COLOR, BG_COLOR, 2);
		pos_y += margin_bottom;
		
		//predefine
		static float32_t base_signal = 0;
		
		//get base signal
		if(current_test == 0)
		{
			TRX.AutoGain = false;
			TRX.ATT = false;
			TRX.ATT_DB = 0;
			TRX.LNA = false;
			TRX.ADC_PGA = false;
			TRX.ADC_Driver = false;
			FPGA_NeedSendParams = true;
			current_test = 1;
			current_test_start_time = HAL_GetTick();
		}
		if(current_test == 1 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			base_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			LCDDriver_printText("Signal strength", 5, pos_y, FG_COLOR, BG_COLOR, 2);
			sprintf(str, " %d          ", (uint16_t)base_signal);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (base_signal > 200 && base_signal < 10000) ? COLOR_GREEN : COLOR_RED, BG_COLOR, 2);
			pos_y += margin_bottom;
			current_test = 2;
		}
		else
			pos_y += margin_bottom;
		
		//test ADC Driver
		if(current_test == 2)
		{
			TRX.ADC_Driver = true;
			FPGA_NeedSendParams = true;
			current_test = 3;
			current_test_start_time = HAL_GetTick();
		}
		if(current_test == 3 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ADC_Driver_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ADC_Driver_db = rate2dbV(ADC_Driver_signal / base_signal);
			
			LCDDriver_printText("ADC Driver signal", 5, pos_y, FG_COLOR, BG_COLOR, 2);
			sprintf(str, " %d          ", (uint16_t)ADC_Driver_signal);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_Driver_signal < 25000.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, 2);
			pos_y += margin_bottom;
			
			LCDDriver_printText("ADC Driver gain", 5, pos_y, FG_COLOR, BG_COLOR, 2);
			sprintf(str, " %.2f dB          ", ADC_Driver_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_Driver_db > 17.0f && ADC_Driver_db < 23.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, 2);
			pos_y += margin_bottom;
			
			current_test = 4;
		}
		else
			pos_y += margin_bottom * 2;
		
		//test ADC PGA
		if(current_test == 4)
		{
			TRX.ADC_Driver = false;
			TRX.ADC_PGA = true;
			FPGA_NeedSendParams = true;
			current_test = 5;
			current_test_start_time = HAL_GetTick();
		}
		if(current_test == 5 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ADC_PGA_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ADC_PGA_db = rate2dbV(ADC_PGA_signal / base_signal);
			
			LCDDriver_printText("ADC PGA signal", 5, pos_y, FG_COLOR, BG_COLOR, 2);
			sprintf(str, " %d          ", (uint16_t)ADC_PGA_signal);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_PGA_signal < 25000.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, 2);
			pos_y += margin_bottom;
			
			LCDDriver_printText("ADC PGA gain", 5, pos_y, FG_COLOR, BG_COLOR, 2);
			sprintf(str, " %.2f dB          ", ADC_PGA_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_PGA_db > 2.0f && ADC_PGA_db < 5.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, 2);
			pos_y += margin_bottom;
			
			current_test = 0;
		}
		else
			pos_y += margin_bottom * 2;
		
		//redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;
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
	if (LCD_busy)
		return;
	LCD_busy = true;
	
	SELF_TEST_current_page += direction;
	if (SELF_TEST_current_page < 0)
		SELF_TEST_current_page = 0;
	if (SELF_TEST_current_page >= SELF_TEST_pages)
		SELF_TEST_current_page = SELF_TEST_pages - 1;
	
	LCDDriver_Fill(BG_COLOR);
	
	LCD_busy = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}
