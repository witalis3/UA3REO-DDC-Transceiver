#include "self_test.h"
#include "main.h"
#include "lcd_driver.h"
#include "trx_manager.h"
#include "functions.h"
#include "fpga.h"
#include "lcd.h"
#include "fonts.h"
#include "front_unit.h"

// Public variables
bool SYSMENU_selftest_opened = false;

// Private Variables
static int8_t SELF_TEST_current_page = 0;
static bool SELF_TEST_old_autogainer = false;
static uint32_t SELF_TEST_old_freq = SELF_TEST_frequency;
static bool LastLNA = false;
static bool LastDRV = false;
static bool LastPGA = false;
static bool LastATT = false;
static float32_t LastATT_DB = false;

// Prototypes
static void SELF_TEST_printResult(bool result, uint16_t pos_y);

// start
void SELF_TEST_Start(void)
{
	LCD_busy = true;

	LastLNA = TRX.LNA;
	LastDRV = TRX.ADC_Driver;
	LastPGA = TRX.ADC_PGA;
	LastATT = TRX.ATT;
	LastATT_DB = TRX.ATT_DB;
	
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
	
	TRX.LNA = LastLNA;
	TRX.ADC_Driver = LastDRV;
	TRX.ADC_PGA = LastPGA;
	TRX.ATT = LastATT;
	TRX.ATT_DB = LastATT_DB;
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

// predefine
	#ifdef LCD_SMALL_INTERFACE
		#define margin_left 5
		#define margin_bottom 10
		#define font_size 1
	#else
		#define margin_left 5
		#define margin_bottom 20
		#define font_size 2
	#endif
	
	uint16_t pos_y = margin_left;
	char str[64] = {0};
	bool pass = true;

	// print pages
	if (SELF_TEST_current_page == 0)
	{
		// FPGA BUS test
		LCDDriver_printText("FPGA", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		SELF_TEST_printResult(FPGA_bus_test_result, pos_y);
		pos_y += margin_bottom;

		// MCP3008 test
		pass = true;
#ifdef HRDW_MCP3008_1
		if (!FRONTPanel_MCP3008_1_Enabled)
			pass = false;
#endif
#ifdef HRDW_MCP3008_2
		if (!FRONTPanel_MCP3008_2_Enabled)
			pass = false;
#endif
#ifdef HRDW_MCP3008_3
		if (!FRONTPanel_MCP3008_3_Enabled)
			pass = false;
#endif
		if (FPGA_bus_test_result)
		{
			LCDDriver_printText("MCP3008", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			SELF_TEST_printResult(pass, pos_y);
			pos_y += margin_bottom;
		}

		// STM32 EEPROM test
		if (FPGA_bus_test_result)
		{
			LCDDriver_printText("STM32 EEPROM", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			SELF_TEST_printResult(EEPROM_Enabled, pos_y);
			pos_y += margin_bottom;
		}

		// WM8731 test
		LCDDriver_printText("WM8731", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		SELF_TEST_printResult(WM8731_test_result, pos_y);
		pos_y += margin_bottom;
		
		#if !defined(FRONTPANEL_LITE)
		// TCXO test
		LCDDriver_printText("TCXO", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		SELF_TEST_printResult(abs(TRX_VCXO_ERROR) < 10, pos_y);
		pos_y += margin_bottom;
		#endif
	}

	if (SELF_TEST_current_page == 1)
	{
		static bool ok[16] = {false};
		static int8_t prev_adc_state[16] = {0};

		LCDDriver_printText("ADC RAW Data", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		sprintf(str, " %d         ", ADC_RAW_IN);
		LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, COLOR_GREEN, BG_COLOR, font_size);
		pos_y += margin_bottom;

		for (uint8_t i = 0; i < 16; i++)
		{
			int8_t bit = bitRead(ADC_RAW_IN, i);
			if (bit == 0)
				bit = -1;

			if (prev_adc_state[i] != 0 && prev_adc_state[i] != bit)
				ok[i] = true;

			prev_adc_state[i] = bit;
		}

		LCDDriver_printText("ADC BITS: ", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		LCDDriver_printText("1 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[0]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("2 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[1]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("3 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[2]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("4 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[3]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("5 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[4]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("6 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[5]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("7 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[6]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("8 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[7]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("9 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[8]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		pos_y += margin_bottom;
		LCDDriver_printText("10 ", margin_left, pos_y, (ok[9]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("11 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[10]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("12 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[11]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		#if !defined(FRONTPANEL_LITE)
		LCDDriver_printText("13 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[12]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("14 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[13]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("15 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[14]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		LCDDriver_printText("16 ", LCDDriver_GetCurrentXOffset(), pos_y, (ok[15]) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
		#endif
		pos_y += margin_bottom;

		// redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}

	if (SELF_TEST_current_page == 2)
	{
		static uint8_t current_test = 0;
		static uint32_t current_test_start_time = 0;

		LCDDriver_printText("Testing on 14mhz...", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		pos_y += margin_bottom;

		// predefine
		static float32_t base_signal = 0;

		// get base signal
		if (current_test == 0)
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
		if (current_test == 1 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			// get base signal strength
			base_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			LCDDriver_printText("Signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %d         ", TRX_ADC_MINAMPLITUDE, TRX_ADC_MAXAMPLITUDE);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (base_signal > 20 && base_signal < 2000) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			// ADC symmetry
			pass = true;
			if (TRX_ADC_MINAMPLITUDE > 0)
				pass = false;
			if (TRX_ADC_MAXAMPLITUDE < 0)
				pass = false;
			if ((abs(TRX_ADC_MINAMPLITUDE) > abs(TRX_ADC_MAXAMPLITUDE)) && (abs(TRX_ADC_MINAMPLITUDE) > (abs(TRX_ADC_MAXAMPLITUDE) * 4)))
				pass = false;
			if ((abs(TRX_ADC_MINAMPLITUDE) < abs(TRX_ADC_MAXAMPLITUDE)) && ((abs(TRX_ADC_MINAMPLITUDE) * 4) < abs(TRX_ADC_MAXAMPLITUDE)))
				pass = false;
			LCDDriver_printText("ADC Symmetry", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			SELF_TEST_printResult(pass, pos_y);
			pos_y += margin_bottom;

			current_test = 2;
		}
		else
			pos_y += margin_bottom * 2;

		// test ADC Driver
		if (current_test == 2)
		{
			TRX.ADC_PGA = false;
			TRX.LNA = false;
			TRX.ADC_Driver = true;
			FPGA_NeedSendParams = true;
			current_test = 3;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 3 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ADC_Driver_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ADC_Driver_db = rate2dbV(ADC_Driver_signal / base_signal);

			LCDDriver_printText("ADC Driver signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d          ", (uint16_t)ADC_Driver_signal);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_Driver_signal < 32000.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			LCDDriver_printText("ADC Driver gain", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %.2f dB          ", ADC_Driver_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_Driver_db > 17.0f && ADC_Driver_db < 30.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 4;
		}
		else
			pos_y += margin_bottom * 2;

		// test ADC PGA
		if (current_test == 4)
		{
			TRX.ADC_Driver = false;
			TRX.LNA = false;
			TRX.ADC_PGA = true;
			FPGA_NeedSendParams = true;
			current_test = 5;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 5 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ADC_PGA_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ADC_PGA_db = rate2dbV(ADC_PGA_signal / base_signal);

			#if !defined(FRONTPANEL_LITE)
			LCDDriver_printText("ADC PGA signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d          ", (uint16_t)ADC_PGA_signal);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_PGA_signal < 32000.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			#endif
			pos_y += margin_bottom;

			#if !defined(FRONTPANEL_LITE)
			LCDDriver_printText("ADC PGA gain", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %.2f dB          ", ADC_PGA_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_PGA_db > 2.0f && ADC_PGA_db < 5.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			#endif
			pos_y += margin_bottom;

			current_test = 6;
		}
		else
			pos_y += margin_bottom * 2;

		// test LNA
		if (current_test == 6)
		{
			TRX.ADC_PGA = false;
			TRX.ADC_Driver = false;
			TRX.LNA = true;
			FPGA_NeedSendParams = true;
			current_test = 7;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 7 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ADC_LNA_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ADC_LNA_db = rate2dbV(ADC_LNA_signal / base_signal);

			#if !defined(FRONTPANEL_LITE)
			LCDDriver_printText("LNA signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d          ", (uint16_t)ADC_LNA_signal);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_LNA_signal < 32000.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			#endif
			pos_y += margin_bottom;

			#if !defined(FRONTPANEL_LITE)
			LCDDriver_printText("LNA gain", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %.2f dB          ", ADC_LNA_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ADC_LNA_db > 15.0f && ADC_LNA_db < 30.0f) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			#endif
			pos_y += margin_bottom;

			current_test = 0;
		}
		else
			pos_y += margin_bottom * 2;

		// redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}

	if (SELF_TEST_current_page == 3)
	{
		static uint8_t current_test = 0;
		static uint32_t current_test_start_time = 0;

		LCDDriver_printText("Testing on 14mhz...", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
		pos_y += margin_bottom;

		// predefine
		static float32_t base_signal = 0;

		// get base signal
		if (current_test == 0)
		{
			TRX.AutoGain = false;
			TRX.ATT = false;
			TRX.ATT_DB = 0;
			TRX.LNA = true;
			TRX.ADC_PGA = true;
			TRX.ADC_Driver = false;
			FPGA_NeedSendParams = true;
			current_test = 1;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 1 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			// get base signal strength
			base_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			LCDDriver_printText("Signal", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %d         ", TRX_ADC_MINAMPLITUDE, TRX_ADC_MAXAMPLITUDE);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (base_signal > 1000 && base_signal < 32000) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 2;
		}
		else
			pos_y += margin_bottom;

		// ATT ON
		if (current_test == 2)
		{
			TRX.ATT = true;
			TRX.ATT_DB = 0;
			FPGA_NeedSendParams = true;
			current_test = 3;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 3 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_db = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT ON", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, ATT_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_db > -2.0f && ATT_db < 1.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 4;
		}
		else
			pos_y += margin_bottom;

		// ATT 0.5 ON
		if (current_test == 4)
		{
			TRX.ATT = true;
			TRX.ATT_DB = 0.5f;
			FPGA_NeedSendParams = true;
			current_test = 5;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 5 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_db = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 0.5", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, ATT_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_db > -2.0f && ATT_db < 1.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 6;
		}
		else
			pos_y += margin_bottom;

		// ATT 1 ON
		if (current_test == 6)
		{
			TRX.ATT = true;
			TRX.ATT_DB = 1.0f;
			FPGA_NeedSendParams = true;
			current_test = 7;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 7 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_db = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 1.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, ATT_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_db > -2.0f && ATT_db < 1.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 8;
		}
		else
			pos_y += margin_bottom;

		// ATT 2 ON
		if (current_test == 8)
		{
			TRX.ATT = true;
			TRX.ATT_DB = 2.0f;
			FPGA_NeedSendParams = true;
			current_test = 9;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 9 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_db = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 2.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, ATT_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_db > -3.0f && ATT_db < -1.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 10;
		}
		else
			pos_y += margin_bottom;

		// ATT 4 ON
		if (current_test == 10)
		{
			TRX.ATT = true;
			TRX.ATT_DB = 4.0f;
			FPGA_NeedSendParams = true;
			current_test = 11;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 11 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_db = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 4.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, ATT_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_db > -6.0f && ATT_db < -2.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 12;
		}
		else
			pos_y += margin_bottom;

		// ATT 8 ON
		if (current_test == 12)
		{
			TRX.ATT = true;
			TRX.ATT_DB = 8.0f;
			FPGA_NeedSendParams = true;
			current_test = 13;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 13 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_db = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 8.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, ATT_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_db > -10.0f && ATT_db < -4.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 14;
		}
		else
			pos_y += margin_bottom;

		// ATT 16 ON
		if (current_test == 14)
		{
			TRX.ATT = true;
			TRX.ATT_DB = 16.0f;
			FPGA_NeedSendParams = true;
			current_test = 15;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 15 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_db = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 16.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, ATT_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_db > -20.0f && ATT_db < -8.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 16;
		}
		else
			pos_y += margin_bottom;

		// ATT 32 ON
		if (current_test == 16)
		{
			TRX.ATT = true;
			TRX.ATT_DB = 32.0f;
			FPGA_NeedSendParams = true;
			current_test = 17;
			current_test_start_time = HAL_GetTick();
		}
		if (current_test == 17 && (HAL_GetTick() - current_test_start_time) > SELF_TEST_adc_test_latency)
		{
			float32_t ATT_signal = fmaxf(fabsf((float32_t)TRX_ADC_MINAMPLITUDE), fabsf((float32_t)TRX_ADC_MAXAMPLITUDE));
			float32_t ATT_db = rate2dbV(ATT_signal / base_signal);

			LCDDriver_printText("ATT 32.0", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			sprintf(str, " %d / %.2f dB         ", (uint16_t)ATT_signal, ATT_db);
			LCDDriver_printText(str, LCDDriver_GetCurrentXOffset(), pos_y, (ATT_signal < 32000.0f && (ATT_db > -33.0f && ATT_db < -16.0f)) ? COLOR_GREEN : COLOR_RED, BG_COLOR, font_size);
			pos_y += margin_bottom;

			current_test = 0;
		}
		else
			pos_y += margin_bottom;

		// redraw loop
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}

	// Pager
	pos_y += margin_bottom;
	LCDDriver_printText("Rotate ENC2 to print next page", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
	pos_y += margin_bottom;

	LCD_busy = false;
}

static void SELF_TEST_printResult(bool result, uint16_t pos_y)
{
	char pass[] = " OK     ";
	char error[] = " ERROR";

	if (result)
		LCDDriver_printText(pass, LCDDriver_GetCurrentXOffset(), pos_y, COLOR_GREEN, BG_COLOR, font_size);
	else
		LCDDriver_printText(error, LCDDriver_GetCurrentXOffset(), pos_y, COLOR_RED, BG_COLOR, font_size);
}

// events to the encoder
void SELF_TEST_EncRotate(int8_t direction)
{
	if (LCD_busy)
		return;

	LCD_busy = true;
	LCDDriver_Fill(BG_COLOR);
	LCD_busy = false;

	SELF_TEST_current_page += direction;
	if (SELF_TEST_current_page < 0)
		SELF_TEST_current_page = 0;
	if (SELF_TEST_current_page >= SELF_TEST_pages)
	{
		SELF_TEST_current_page = SELF_TEST_pages - 1;
		FRONTPANEL_BUTTONHANDLER_SERVICES(0);
	}

	LCD_UpdateQuery.SystemMenuRedraw = true;
}
