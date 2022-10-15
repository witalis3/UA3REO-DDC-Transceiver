#include "auto_calibration.h"
#include "main.h"
#include "lcd_driver.h"
#include "trx_manager.h"
#include "functions.h"
#include "lcd.h"
#include "fonts.h"
#include "front_unit.h"
#include "settings.h"

// Public variables
bool SYSMENU_auto_calibration_opened = false;

// Private Variables
static int8_t AUTO_CALIBRATION_current_page = 0;
static int8_t AUTO_CALIBRATION_calibration_type = 0; // 0-swr 1-power

static bool LastAutogainer = false;
static uint64_t LastFreq = 0;
static bool LastLNA = false;
static bool LastDRV = false;
static bool LastPGA = false;
static bool LastATT = false;
static float32_t LastATT_DB = false;

// Prototypes

// start
static void AUTO_CALIBRATION_Start(void)
{
	LCD_busy = true;

	LastLNA = TRX.LNA;
	LastDRV = TRX.ADC_Driver;
	LastPGA = TRX.ADC_PGA;
	LastATT = TRX.ATT;
	LastATT_DB = TRX.ATT_DB;
	LastAutogainer = TRX.AutoGain;
	LastFreq = CurrentVFO->Freq;
	
	TRX.TUNER_Enabled = false;
	TRX.ATU_Enabled = false;
	
	AUTO_CALIBRATION_current_page = 0;
	LCDDriver_Fill(BG_COLOR);

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

void AUTO_CALIBRATION_Start_SWR(void)
{
	AUTO_CALIBRATION_calibration_type = 0;
	AUTO_CALIBRATION_Start();
}

void AUTO_CALIBRATION_Start_POWER(void)
{
	AUTO_CALIBRATION_calibration_type = 1;
	AUTO_CALIBRATION_Start();
}

// stop
void AUTO_CALIBRATION_Stop(void)
{
	TRX_Tune = false;
	TRX.AutoGain = LastAutogainer;
	TRX_setFrequency(LastFreq, CurrentVFO);
	
	TRX.LNA = LastLNA;
	TRX.ADC_Driver = LastDRV;
	TRX.ADC_PGA = LastPGA;
	TRX.ATT = LastATT;
	TRX.ATT_DB = LastATT_DB;
	TRX.TUNER_Enabled = true;
	TRX.ATU_Enabled = true;
	
	NeedSaveCalibration = true;
}

// draw
void AUTO_CALIBRATION_Draw(void)
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

	// print pages
	if (AUTO_CALIBRATION_calibration_type == 0) //swr test
	{
		if (AUTO_CALIBRATION_current_page == 0) //Info HF
		{
			TRX_Tune = false;
			
			TRX_setFrequency(14000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_selected = false;
			TRX.RF_Power = 30;
			
			LCDDriver_printText("Plug HF dummy load", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("and power meter to ANT1", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 1) //HF FWD
		{
			TRX_Tune = true;
			
			TRX_setFrequency(14000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_selected = false;
			TRX.RF_Power = 30;
			
			LCDDriver_printText("14Mhz, 30% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust FORWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Forward", TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Backward", TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W FWD Calibrate", CALIBRATE.SWR_FWD_Calibration_HF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W BWD Calibrate", CALIBRATE.SWR_BWD_Calibration_HF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 2) //HF BWD
		{
			TRX_Tune = true;
			
			TRX_setFrequency(14000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_selected = false;
			TRX.RF_Power = 30;
			
			LCDDriver_printText("14Mhz, 30% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust BACKWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Forward", TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Backward", TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W FWD Calibrate", CALIBRATE.SWR_FWD_Calibration_HF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W BWD Calibrate", CALIBRATE.SWR_BWD_Calibration_HF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 3) //Info 6m
		{
			TRX_Tune = false;
			
			TRX_setFrequency(52000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_selected = false;
			TRX.RF_Power = 30;
			
			LCDDriver_printText("Plug 6M dummy load", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("and power meter to ANT1", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 4) //6m FWD
		{
			TRX_Tune = true;
			
			TRX_setFrequency(52000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_selected = false;
			TRX.RF_Power = 30;
			
			LCDDriver_printText("52Mhz, 30% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust FORWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Forward", TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Backward", TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W FWD Calibrate", CALIBRATE.SWR_FWD_Calibration_6M);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W BWD Calibrate", CALIBRATE.SWR_BWD_Calibration_6M);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 5) //6m BWD
		{
			TRX_Tune = true;
			
			TRX_setFrequency(52000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_selected = false;
			TRX.RF_Power = 30;
			
			LCDDriver_printText("52Mhz, 30% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust BACKWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Forward", TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Backward", TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W FWD Calibrate", CALIBRATE.SWR_FWD_Calibration_6M);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W BWD Calibrate", CALIBRATE.SWR_BWD_Calibration_6M);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 6) //Info VFH
		{
			TRX_Tune = false;
			
			TRX_setFrequency(145000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_selected = false;
			TRX.RF_Power = 100;
			
			LCDDriver_printText("Plug VHF dummy load", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("and power meter to ANT1", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 7) //VHF FWD
		{
			TRX_Tune = true;
			
			TRX_setFrequency(145000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_selected = false;
			TRX.RF_Power = 100;
			
			LCDDriver_printText("145Mhz, 100% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust FORWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Forward", TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Backward", TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W FWD Calibrate", CALIBRATE.SWR_FWD_Calibration_VHF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W BWD Calibrate", CALIBRATE.SWR_BWD_Calibration_VHF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 8) //VHF BWD
		{
			TRX_Tune = true;
			
			TRX_setFrequency(145000000, CurrentVFO);
			TRX_setMode(TRX_MODE_CW, CurrentVFO);
			TRX.ANT_selected = false;
			TRX.RF_Power = 100;
			
			LCDDriver_printText("145Mhz, 100% power", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			LCDDriver_printText("Rotate to adjust BACKWARD", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Forward", TRX_PWR_Forward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.1f W Backward", TRX_PWR_Backward_SMOOTHED);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W FWD Calibrate", CALIBRATE.SWR_FWD_Calibration_VHF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			sprintf(str, "%.2f W BWD Calibrate", CALIBRATE.SWR_BWD_Calibration_VHF);
			LCDDriver_printText(str, margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		if (AUTO_CALIBRATION_current_page == 9) //Done
		{
			TRX_Tune = false;
			
			LCDDriver_printText("Calibration complete", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
			pos_y += margin_bottom;
			
			// redraw loop
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		
		if (AUTO_CALIBRATION_current_page > 9)
			AUTO_CALIBRATION_current_page = 9;
	}

	// Pager
	pos_y += margin_bottom;
	LCDDriver_printText("Rotate ENC2 to select page", margin_left, pos_y, FG_COLOR, BG_COLOR, font_size);
	pos_y += margin_bottom;

	LCD_busy = false;
}

// events to the encoder
void AUTO_CALIBRATION_EncRotate(int8_t direction)
{
	if (LCD_busy)
		return;

	if (AUTO_CALIBRATION_calibration_type == 0) //swr test
	{
		if (AUTO_CALIBRATION_current_page == 1) //HF FWD
		{
			CALIBRATE.SWR_FWD_Calibration_HF += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_FWD_Calibration_HF < 1.0f)
				CALIBRATE.SWR_FWD_Calibration_HF = 1.0f;
			if (CALIBRATE.SWR_FWD_Calibration_HF > 200.0f)
				CALIBRATE.SWR_FWD_Calibration_HF = 200.0f;
		}
		if (AUTO_CALIBRATION_current_page == 2) //HF BWD
		{
			CALIBRATE.SWR_BWD_Calibration_HF += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_BWD_Calibration_HF < 1.0f)
				CALIBRATE.SWR_BWD_Calibration_HF = 1.0f;
			if (CALIBRATE.SWR_BWD_Calibration_HF > 200.0f)
				CALIBRATE.SWR_BWD_Calibration_HF = 200.0f;
		}
		if (AUTO_CALIBRATION_current_page == 4) //6m FWD
		{
			CALIBRATE.SWR_FWD_Calibration_6M += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_FWD_Calibration_6M < 1.0f)
				CALIBRATE.SWR_FWD_Calibration_6M = 1.0f;
			if (CALIBRATE.SWR_FWD_Calibration_6M > 200.0f)
				CALIBRATE.SWR_FWD_Calibration_6M = 200.0f;
		}
		if (AUTO_CALIBRATION_current_page == 5) //6m BWD
		{
			CALIBRATE.SWR_BWD_Calibration_6M += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_BWD_Calibration_6M < 1.0f)
				CALIBRATE.SWR_BWD_Calibration_6M = 1.0f;
			if (CALIBRATE.SWR_BWD_Calibration_6M > 200.0f)
				CALIBRATE.SWR_BWD_Calibration_6M = 200.0f;
		}
		if (AUTO_CALIBRATION_current_page == 7) //VHF FWD
		{
			CALIBRATE.SWR_FWD_Calibration_VHF += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_FWD_Calibration_VHF < 1.0f)
				CALIBRATE.SWR_FWD_Calibration_VHF = 1.0f;
			if (CALIBRATE.SWR_FWD_Calibration_VHF > 200.0f)
				CALIBRATE.SWR_FWD_Calibration_VHF = 200.0f;
		}
		if (AUTO_CALIBRATION_current_page == 8) //VHF BWD
		{
			CALIBRATE.SWR_BWD_Calibration_VHF += (float32_t)direction * 0.1f;
			if (CALIBRATE.SWR_BWD_Calibration_VHF < 1.0f)
				CALIBRATE.SWR_BWD_Calibration_VHF = 1.0f;
			if (CALIBRATE.SWR_BWD_Calibration_VHF > 200.0f)
				CALIBRATE.SWR_BWD_Calibration_VHF = 200.0f;
		}
	}

	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void AUTO_CALIBRATION_Enc2Rotate(int8_t direction)
{
	if (LCD_busy)
		return;

	LCD_busy = true;
	LCDDriver_Fill(BG_COLOR);
	LCD_busy = false;

	AUTO_CALIBRATION_current_page += direction;
	if (AUTO_CALIBRATION_current_page < 0)
		AUTO_CALIBRATION_current_page = 0;

	LCD_UpdateQuery.SystemMenuRedraw = true;
}
