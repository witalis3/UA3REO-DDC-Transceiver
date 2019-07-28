#include "system_menu.h"
#include "lcd.h"
#include "settings.h"
#include "audio_filters.h"
#include "bootloader.h"
#include "functions.h"
#include "wifi.h"
#include "LCD/xpt2046_spi.h"
#include "LCD/fonts.h"

static void drawSystemMenuElement(char* title, SystemMenuType type, uint32_t *value, bool onlyVal);
static void redrawCurrentItem(void);
static void SYSMENU_WIFI_DrawSelectAPMenu(bool full_redraw);
static void SYSMENU_WIFI_SelectAPMenuMove(int8_t dir);
static void SYSMENU_WIFI_DrawAPpasswordMenu(bool full_redraw);
static void SYSMENU_WIFI_RotatePasswordChar(int8_t dir);

static void SYSMENU_HANDL_FFTEnabled(int8_t direction);
static void SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ(int8_t direction);
static void SYSMENU_HANDL_LCD_Brightness(int8_t direction);
static void SYSMENU_HANDL_ENCODER_SLOW_RATE(int8_t direction);
static void SYSMENU_HANDL_Touch_Calibrate(int8_t direction);
static void SYSMENU_HANDL_SETTIME(int8_t direction);
static void SYSMENU_HANDL_Standby_Time(int8_t direction);
static void SYSMENU_HANDL_Beeping(int8_t direction);
static void SYSMENU_HANDL_Key_timeout(int8_t direction);
static void SYSMENU_HANDL_FFT_Averaging(int8_t direction);
static void SYSMENU_HANDL_SSB_HPF_pass(int8_t direction);
static void SYSMENU_HANDL_Bootloader(int8_t direction);
static void SYSMENU_HANDL_CWDecoder(int8_t direction);
static void SYSMENU_HANDL_WIFI_Enabled(int8_t direction);
static void SYSMENU_HANDL_WIFI_SelectAP(int8_t direction);
static void SYSMENU_HANDL_WIFI_SetAPpassword(int8_t direction);
static void SYSMENU_HANDL_WIFI_Timezone(int8_t direction);
	
static void SYSMENU_HANDL_CWMENU(int8_t direction);
static void SYSMENU_HANDL_LCDMENU(int8_t direction);
static void SYSMENU_HANDL_FFTMENU(int8_t direction);
static void SYSMENU_HANDL_WIFIMENU(int8_t direction);

static const uint8_t sysmenu_x1 = 5;
static const uint8_t sysmenu_x2 = 240;
static const uint16_t sysmenu_w = 288;

static struct sysmenu_item_handler sysmenu_handlers[] =
{
	{"SSB HPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.SSB_HPF_pass, SYSMENU_HANDL_SSB_HPF_pass},
	{"CW Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_CWMENU},
	{"LCD Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_LCDMENU},
	{"FFT Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_FFTMENU},
	{"Encoder slow rate", SYSMENU_UINT8, (uint32_t *)&TRX.ENCODER_SLOW_RATE, SYSMENU_HANDL_ENCODER_SLOW_RATE},
	{"WIFI Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_WIFIMENU},
	{"Set Clock Time", SYSMENU_RUN, 0, SYSMENU_HANDL_SETTIME},
	{"Flash update", SYSMENU_RUN, 0, SYSMENU_HANDL_Bootloader},
};
static uint8_t sysmenu_item_count = sizeof(sysmenu_handlers) / sizeof(sysmenu_handlers[0]);

static struct sysmenu_item_handler sysmenu_cw_handlers[] =
{
	{"CW Key timeout", SYSMENU_UINT16, (uint32_t *)&TRX.Key_timeout, SYSMENU_HANDL_Key_timeout},
	{"CW Generator shift", SYSMENU_UINT16, (uint32_t *)&TRX.CW_GENERATOR_SHIFT_HZ, SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ},
	{"CW Decoder", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CWDecoder, SYSMENU_HANDL_CWDecoder},
};
static uint8_t sysmenu_cw_item_count = sizeof(sysmenu_cw_handlers) / sizeof(sysmenu_cw_handlers[0]);

static struct sysmenu_item_handler sysmenu_lcd_handlers[] =
{
	{"Touchpad beeping", SYSMENU_BOOLEAN, (uint32_t *)&TRX.Beeping, SYSMENU_HANDL_Beeping},
	{"LCD Calibrate", SYSMENU_RUN, 0, SYSMENU_HANDL_Touch_Calibrate},
	{"LCD Brightness", SYSMENU_UINT8, (uint32_t *)&TRX.LCD_Brightness, SYSMENU_HANDL_LCD_Brightness},
	{"LCD Sleep Time", SYSMENU_UINT8, (uint32_t *)&TRX.Standby_Time, SYSMENU_HANDL_Standby_Time},
};
static uint8_t sysmenu_lcd_item_count = sizeof(sysmenu_lcd_handlers) / sizeof(sysmenu_lcd_handlers[0]);

static struct sysmenu_item_handler sysmenu_fft_handlers[] =
{
	{"FFT Enabled", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Enabled, SYSMENU_HANDL_FFTEnabled},
	{"FFT Averaging", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Averaging, SYSMENU_HANDL_FFT_Averaging},
};
static uint8_t sysmenu_fft_item_count = sizeof(sysmenu_fft_handlers) / sizeof(sysmenu_fft_handlers[0]);

static struct sysmenu_item_handler sysmenu_wifi_handlers[] =
{
	{"WIFI Enabled", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WIFI_Enabled, SYSMENU_HANDL_WIFI_Enabled},
	{"WIFI Select AP", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SelectAP},
	{"WIFI Set AP Pass", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SetAPpassword},
	{"WIFI Timezone", SYSMENU_INT8, (uint32_t *)&TRX.WIFI_TIMEZONE, SYSMENU_HANDL_WIFI_Timezone},
};
static uint8_t sysmenu_wifi_item_count = sizeof(sysmenu_wifi_handlers) / sizeof(sysmenu_wifi_handlers[0]);

static struct sysmenu_item_handler *sysmenu_handlers_selected = &sysmenu_handlers[0];
static uint8_t *sysmenu_item_count_selected = &sysmenu_item_count;
static uint8_t systemMenuIndex = 0;
static uint8_t sysmenu_y = 5;
static uint8_t sysmenu_i = 0;
static bool sysmenu_onroot = true;

//WIFI
static bool sysmenu_wifi_selectap_menu_opened = false;
static bool sysmenu_wifi_setAPpassword_menu_opened = false;
uint16_t sysmenu_wifi_rescan_interval=0;
uint8_t sysmenu_wifi_selected_ap_index=0;
uint8_t sysmenu_wifi_selected_ap_password_char_index=0;
//

void drawSystemMenu(bool draw_background)
{
	if (LCD_busy)
	{
		LCD_UpdateQuery.SystemMenu = true;
		return;
	}
	if (!LCD_systemMenuOpened) return;
	if (LCD_timeMenuOpened) { LCD_Handler_SETTIME(); return; }
	if (sysmenu_wifi_selectap_menu_opened) { SYSMENU_WIFI_DrawSelectAPMenu(false); return; }
	if (sysmenu_wifi_setAPpassword_menu_opened) { SYSMENU_WIFI_DrawAPpasswordMenu(false); return; }
	LCD_busy = true;

	sysmenu_i = 0;
	sysmenu_y = 5;

	if (draw_background) LCDDriver_Fill(COLOR_BLACK);

	for (uint8_t m = 0; m < *sysmenu_item_count_selected; m++)
		drawSystemMenuElement(sysmenu_handlers_selected[m].title, sysmenu_handlers_selected[m].type, sysmenu_handlers_selected[m].value, false);

	LCDDriver_Fill_RectXY(290, 0, 320, 30, COLOR_GREEN);
	LCDDriver_printText("X", 298, 5, COLOR_BLACK, COLOR_GREEN, 3);

	LCDDriver_Fill_RectXY(290, 80, 320, 110, COLOR_GREEN);
	LCDDriver_printText("<", 298, 85, COLOR_BLACK, COLOR_GREEN, 3);

	LCDDriver_Fill_RectXY(290, 140, 320, 170, COLOR_GREEN);
	LCDDriver_printText(">", 298, 145, COLOR_BLACK, COLOR_GREEN, 3);

	LCD_UpdateQuery.SystemMenu = false;
	LCD_busy = false;
}

static void SYSMENU_HANDL_CWDecoder(int8_t direction)
{
	if(direction>0) TRX.CWDecoder = true;
	if(direction<0) TRX.CWDecoder = false;
}

static void SYSMENU_HANDL_FFTEnabled(int8_t direction)
{
	if(direction>0) TRX.FFT_Enabled = true;
	if(direction<0) TRX.FFT_Enabled = false;
}

static void SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ(int8_t direction)
{
	TRX.CW_GENERATOR_SHIFT_HZ += direction * 100;
	if (TRX.CW_GENERATOR_SHIFT_HZ < 100) TRX.CW_GENERATOR_SHIFT_HZ = 100;
	if (TRX.CW_GENERATOR_SHIFT_HZ > 10000) TRX.CW_GENERATOR_SHIFT_HZ = 10000;
}

static void SYSMENU_HANDL_LCD_Brightness(int8_t direction)
{
	TRX.LCD_Brightness += direction;
	if (TRX.LCD_Brightness < 1) TRX.LCD_Brightness = 1;
	if (TRX.LCD_Brightness > 100) TRX.LCD_Brightness = 100;
	LCDDriver_setBrightness(TRX.LCD_Brightness);
}

static void SYSMENU_HANDL_ENCODER_SLOW_RATE(int8_t direction)
{
	TRX.ENCODER_SLOW_RATE += direction;
	if (TRX.ENCODER_SLOW_RATE < 1) TRX.ENCODER_SLOW_RATE = 1;
	if (TRX.ENCODER_SLOW_RATE > 100) TRX.ENCODER_SLOW_RATE = 100;
}

static void SYSMENU_HANDL_Touch_Calibrate(int8_t direction)
{
	WM8731_CleanBuffer();
	HAL_Delay(500);
	Touch_Calibrate();
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_SETTIME(int8_t direction)
{
	LCD_Handler_SETTIME();
}

static void SYSMENU_HANDL_Standby_Time(int8_t direction)
{
	if (TRX.Standby_Time > 0 || direction > 0) TRX.Standby_Time += direction;
	if (TRX.Standby_Time > 250) TRX.Standby_Time = 250;
}

static void SYSMENU_HANDL_Beeping(int8_t direction)
{
	if(direction>0) TRX.Beeping = true;
	if(direction<0) TRX.Beeping = false;
}

static void SYSMENU_HANDL_Key_timeout(int8_t direction)
{
	if (TRX.Key_timeout > 0 || direction > 0) TRX.Key_timeout += direction * 50;
	if (TRX.Key_timeout > 5000) TRX.Key_timeout = 5000;
}

static void SYSMENU_HANDL_FFT_Averaging(int8_t direction)
{
	TRX.FFT_Averaging += direction;
	if (TRX.FFT_Averaging < 1) TRX.FFT_Averaging = 1;
	if (TRX.FFT_Averaging > 10) TRX.FFT_Averaging = 10;
}

static void SYSMENU_HANDL_SSB_HPF_pass(int8_t direction)
{
	TRX.SSB_HPF_pass += direction * 100;
	if (TRX.SSB_HPF_pass < 100) TRX.SSB_HPF_pass = 100;
	if (TRX.SSB_HPF_pass > 500) TRX.SSB_HPF_pass = 500;
	ReinitAudioFilters();
}

static void SYSMENU_HANDL_Bootloader(int8_t direction)
{
	WM8731_CleanBuffer();
	JumpToBootloader();
}

static void SYSMENU_HANDL_WIFI_Enabled(int8_t direction)
{
	if(direction>0) TRX.WIFI_Enabled = true;
	if(direction<0) TRX.WIFI_Enabled = false;
}

static void SYSMENU_HANDL_WIFI_SelectAP(int8_t direction)
{
	sysmenu_wifi_selectap_menu_opened = true;
	SYSMENU_WIFI_DrawSelectAPMenu(true);
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_WIFI_SetAPpassword(int8_t direction)
{
	sysmenu_wifi_setAPpassword_menu_opened = true;
	SYSMENU_WIFI_DrawAPpasswordMenu(true);
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_WIFI_Timezone(int8_t direction)
{
	TRX.WIFI_TIMEZONE += direction;
	if (TRX.WIFI_TIMEZONE < -12) TRX.WIFI_TIMEZONE = -12;
	if (TRX.WIFI_TIMEZONE > 12) TRX.WIFI_TIMEZONE = 12;
	WIFI_State = WIFI_INITED;
}

static void SYSMENU_HANDL_CWMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_cw_handlers[0];
	sysmenu_item_count_selected = &sysmenu_cw_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_LCDMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_lcd_handlers[0];
	sysmenu_item_count_selected = &sysmenu_lcd_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_FFTMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_fft_handlers[0];
	sysmenu_item_count_selected = &sysmenu_fft_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_WIFIMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_wifi_handlers[0];
	sysmenu_item_count_selected = &sysmenu_wifi_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

void eventRotateSystemMenu(int8_t direction)
{
	if(sysmenu_wifi_selectap_menu_opened)
	{
		SYSMENU_WIFI_SelectAPMenuMove(0);
		return;
	}
	if(sysmenu_wifi_setAPpassword_menu_opened)
	{
		SYSMENU_WIFI_RotatePasswordChar(direction);
		return;
	}
	sysmenu_handlers_selected[systemMenuIndex].menuHandler(direction);
	if (sysmenu_handlers_selected[systemMenuIndex].type != SYSMENU_RUN)
		redrawCurrentItem();
}

void eventClickSystemMenu(uint16_t x, uint16_t y)
{
	//wifi select AP menu
	if(sysmenu_wifi_selectap_menu_opened)
	{
		if (y < 120)
			SYSMENU_WIFI_SelectAPMenuMove(-1);
		else
			SYSMENU_WIFI_SelectAPMenuMove(1);
		return;
	}
	//wifi set password menu
	if(sysmenu_wifi_setAPpassword_menu_opened)
	{
		if (x >= 290 && x <= 320 && y >= 1 && y <= 30)
		{
			sysmenu_wifi_setAPpassword_menu_opened = false;
			systemMenuIndex = 0;
			drawSystemMenu(true);
			WIFI_State = WIFI_INITED;
		}
		else if (y < 120 && sysmenu_wifi_selected_ap_password_char_index>0)
		{
			sysmenu_wifi_selected_ap_password_char_index--;
			SYSMENU_WIFI_DrawAPpasswordMenu(true);
		}
		else if (sysmenu_wifi_selected_ap_password_char_index<(MAX_WIFIPASS_LENGTH-1))
		{
			sysmenu_wifi_selected_ap_password_char_index++;
			SYSMENU_WIFI_DrawAPpasswordMenu(true);
		}
		return;
	}
	//other
	if (x >= 290 && x <= 320 && y >= 1 && y <= 30)
	{
		if(sysmenu_onroot)
		{
			LCD_systemMenuOpened = false;
			LCD_mainMenuOpened = true;
			LCD_UpdateQuery.Background = true;
			LCD_UpdateQuery.MainMenu = true;
			LCD_redraw();
			NeedSaveSettings = true;
		}
		else
		{
			sysmenu_handlers_selected = &sysmenu_handlers[0];
			sysmenu_item_count_selected = &sysmenu_item_count;
			sysmenu_onroot = true;
			systemMenuIndex = 0;
			drawSystemMenu(true);
		}
	}
	else if (y < 120)
	{
		LCDDriver_drawFastHLine(0, (5 + systemMenuIndex * 18) + 17, sysmenu_w, COLOR_BLACK);
		if (systemMenuIndex > 0)
			systemMenuIndex--;
		redrawCurrentItem();
	}
	else
	{
		LCDDriver_drawFastHLine(0, (5 + systemMenuIndex * 18) + 17, sysmenu_w, COLOR_BLACK);
		if (systemMenuIndex < (*sysmenu_item_count_selected - 1))
			systemMenuIndex++;
		redrawCurrentItem();
	}
}

static void redrawCurrentItem(void)
{
	sysmenu_i = systemMenuIndex;
	sysmenu_y = 5 + systemMenuIndex * 18;
	drawSystemMenuElement(sysmenu_handlers_selected[systemMenuIndex].title, sysmenu_handlers_selected[systemMenuIndex].type, sysmenu_handlers_selected[systemMenuIndex].value, true);
}

static void drawSystemMenuElement(char* title, SystemMenuType type, uint32_t *value, bool onlyVal)
{
	char ctmp[10];
	if (!onlyVal)
	{
		LCDDriver_Fill_RectXY(0, sysmenu_y, sysmenu_w, sysmenu_y + 17, COLOR_BLACK);
		LCDDriver_printText(title, sysmenu_x1, sysmenu_y, COLOR_WHITE, COLOR_BLACK, 2);
	}
	switch (type)
	{
	case SYSMENU_UINT8:
		sprintf(ctmp, "%d", (uint8_t)*value);
		break;
	case SYSMENU_UINT16:
		sprintf(ctmp, "%d", (uint16_t)*value);
		break;
	case SYSMENU_UINT32:
		sprintf(ctmp, "%d", (uint32_t)*value);
		break;
	case SYSMENU_INT8:
		sprintf(ctmp, "%d", (int8_t)*value);
		break;
	case SYSMENU_INT16:
		sprintf(ctmp, "%d", (int16_t)*value);
		break;
	case SYSMENU_INT32:
		sprintf(ctmp, "%d", (int32_t)*value);
		break;
	case SYSMENU_BOOLEAN:
		sprintf(ctmp, "%d", (int8_t)*value);
		if ((uint8_t)*value == 1)
			sprintf(ctmp, "YES");
		else
			sprintf(ctmp, "NO");
		break;
	case SYSMENU_RUN:
		sprintf(ctmp, "RUN");
		break;
	case SYSMENU_MENU:
		sprintf(ctmp, "->");
		break;
	}
	if (onlyVal)
		LCDDriver_Fill_RectWH(sysmenu_x2, sysmenu_y, 4 * 12, 13, COLOR_BLACK);
	LCDDriver_printText(ctmp, sysmenu_x2, sysmenu_y, COLOR_WHITE, COLOR_BLACK, 2);
	if (systemMenuIndex == sysmenu_i) LCDDriver_drawFastHLine(0, sysmenu_y + 17, sysmenu_w, COLOR_WHITE);
	sysmenu_i++;
	sysmenu_y += 18;
}

static void SYSMENU_WIFI_DrawSelectAPMenu(bool full_redraw)
{
	if(full_redraw || sysmenu_wifi_rescan_interval == 0)
	{
		LCDDriver_Fill(COLOR_BLACK);
		LCDDriver_printText("AP Found:", 5, 5, COLOR_WHITE, COLOR_BLACK, 1);
		for(uint8_t i=0; i<WIFI_FOUNDED_AP_MAXCOUNT; i++)
			LCDDriver_printText((char*)WIFI_FoundedAP[i], 10, 25+i*12, COLOR_GREEN, COLOR_BLACK, 1);
		LCDDriver_drawFastHLine(0, 37+sysmenu_wifi_selected_ap_index*12, sysmenu_w, COLOR_WHITE);
		WIFI_ListAP();
	}
	sysmenu_wifi_rescan_interval++;
	if(sysmenu_wifi_rescan_interval>100)
		sysmenu_wifi_rescan_interval = 0;
	LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_WIFI_SelectAPMenuMove(int8_t dir)
{
	if(dir<0 && sysmenu_wifi_selected_ap_index>0) sysmenu_wifi_selected_ap_index--;
	if(dir>0 && sysmenu_wifi_selected_ap_index<WIFI_FOUNDED_AP_MAXCOUNT) sysmenu_wifi_selected_ap_index++;
	SYSMENU_WIFI_DrawSelectAPMenu(true);
	if(dir==0)
	{
		strcpy(TRX.WIFI_AP,(char*)&WIFI_FoundedAP[sysmenu_wifi_selected_ap_index]);
		WIFI_State = WIFI_INITED;
		sysmenu_wifi_selectap_menu_opened = false;
		systemMenuIndex = 0;
		drawSystemMenu(true);
	}
}

static void SYSMENU_WIFI_DrawAPpasswordMenu(bool full_redraw)
{
	if(full_redraw)
	{
		LCDDriver_Fill(COLOR_BLACK);
		LCDDriver_Fill_RectXY(290, 0, 320, 30, COLOR_GREEN);
		LCDDriver_printText("X", 298, 5, COLOR_BLACK, COLOR_GREEN, 3);
		LCDDriver_printText("AP Password:", 5, 5, COLOR_WHITE, COLOR_BLACK, 1);
	}
	
	LCDDriver_printText(TRX.WIFI_PASSWORD, 10, 25, COLOR_GREEN, COLOR_BLACK, 1);
	LCDDriver_drawFastHLine(10+sysmenu_wifi_selected_ap_password_char_index*6, 37, 6, COLOR_RED);
}

static void SYSMENU_WIFI_RotatePasswordChar(int8_t dir)
{
	bool full_redraw=false;
	if(TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]==0)
		full_redraw=true;
	TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]+=dir;
	
	//не показываем спецсимволы
	if(TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]>=1 && TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]<=32 && dir>0)
		TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]=33;
	if(TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]>=1 && TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]<=32 && dir<0)
		TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]=0;
	if(TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]>=127)
		TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]=0;
	if(TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index]==0)
		full_redraw=true;
	
	if(full_redraw)
		SYSMENU_WIFI_DrawAPpasswordMenu(true);
	else
		SYSMENU_WIFI_DrawAPpasswordMenu(false);
}
