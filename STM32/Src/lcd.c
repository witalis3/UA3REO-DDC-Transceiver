#include "main.h"
#include "lcd.h"
#include "functions.h"
#include "arm_math.h"
#include "agc.h"
#include "settings.h"
#include "system_menu.h"
#include "wm8731.h"
#include "audio_filters.h"
#include "fonts.h"
#include "wm8731.h"
#include "usbd_ua3reo.h"
#include "noise_reduction.h"
#include "cw_decoder.h"
#include "peripheral.h"
#include "screen_layout.h"
#include "images.h"
#include "wifi.h"

volatile bool LCD_busy = false;
volatile DEF_LCD_UpdateQuery LCD_UpdateQuery = {false};
volatile bool LCD_systemMenuOpened = false;

static char LCD_freq_string_hz[6];
static char LCD_freq_string_khz[6];
static char LCD_freq_string_mhz[6];
static uint32_t LCD_last_showed_freq = 0;
static uint16_t LCD_last_showed_freq_mhz = 9999;
static uint16_t LCD_last_showed_freq_khz = 9999;
static uint16_t LCD_last_showed_freq_hz = 9999;
static int16_t LCD_last_s_meter = 1;
static uint32_t Time;
static uint8_t Hours;
static uint8_t Last_showed_Hours = 255;
static uint8_t Minutes;
static uint8_t Last_showed_Minutes = 255;
static uint8_t Seconds;
static uint8_t Last_showed_Seconds = 255;

static void printInfo(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t in_active_color, bool active);
static void LCD_displayFreqInfo(void);
static void LCD_displayTopButtons(bool redraw);
static void LCD_displayStatusInfoBar(void);
static void LCD_displayStatusInfoGUI(void);
static void LCD_displayTextBar(void);

void LCD_Init(void)
{
	LCDDriver_setBrightness(TRX.LCD_Brightness);
	LCDDriver_Init();
#if SCREEN_ROTATE
	LCDDriver_setRotation(2);
#else
	LCDDriver_setRotation(4);
#endif
	LCDDriver_Fill(BACKGROUND_COLOR);
}

static void LCD_displayTopButtons(bool redraw)
{ //вывод верхних кнопок
	if (LCD_systemMenuOpened)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.TopButtons = true;
		return;
	}
	LCD_busy = true;
	if (redraw)
		LCDDriver_Fill_RectWH(LAY_TOPBUTTONS_X1, LAY_TOPBUTTONS_Y1, LAY_TOPBUTTONS_X2, LAY_TOPBUTTONS_Y2, BACKGROUND_COLOR);

	//вывод инфо о работе трансивера
	if (!TRX.current_vfo) //VFO-A
	{
		if (TRX.Dual_RX_Type == VFO_SEPARATE)
			printInfo(LAY_TOPBUTTONS_VFO_X, LAY_TOPBUTTONS_VFO_Y, LAY_TOPBUTTONS_VFO_W, LAY_TOPBUTTONS_VFO_H, "VFOA", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);
		else if (TRX.Dual_RX_Type == VFO_A_AND_B)
			printInfo(LAY_TOPBUTTONS_VFO_X, LAY_TOPBUTTONS_VFO_Y, LAY_TOPBUTTONS_VFO_W, LAY_TOPBUTTONS_VFO_H, "A&B", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);
		else if (TRX.Dual_RX_Type == VFO_A_PLUS_B)
			printInfo(LAY_TOPBUTTONS_VFO_X, LAY_TOPBUTTONS_VFO_Y, LAY_TOPBUTTONS_VFO_W, LAY_TOPBUTTONS_VFO_H, "A+B", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);
	}
	else //VFO-B
	{
		if (TRX.Dual_RX_Type == VFO_SEPARATE)
			printInfo(LAY_TOPBUTTONS_VFO_X, LAY_TOPBUTTONS_VFO_Y, LAY_TOPBUTTONS_VFO_W, LAY_TOPBUTTONS_VFO_H, "VFOB", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);
		else if (TRX.Dual_RX_Type == VFO_A_AND_B)
			printInfo(LAY_TOPBUTTONS_VFO_X, LAY_TOPBUTTONS_VFO_Y, LAY_TOPBUTTONS_VFO_W, LAY_TOPBUTTONS_VFO_H, "B&A", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);
		else if (TRX.Dual_RX_Type == VFO_A_PLUS_B)
			printInfo(LAY_TOPBUTTONS_VFO_X, LAY_TOPBUTTONS_VFO_Y, LAY_TOPBUTTONS_VFO_W, LAY_TOPBUTTONS_VFO_H, "B+A", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);
	}
	printInfo(LAY_TOPBUTTONS_MODE_X, LAY_TOPBUTTONS_MODE_Y, LAY_TOPBUTTONS_MODE_W, LAY_TOPBUTTONS_MODE_H, (char *)MODE_DESCR[CurrentVFO()->Mode], BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);
	printInfo(LAY_TOPBUTTONS_PRE_X, LAY_TOPBUTTONS_PRE_Y, LAY_TOPBUTTONS_PRE_W, LAY_TOPBUTTONS_PRE_H, "PRE", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, TRX.LNA);
	printInfo(LAY_TOPBUTTONS_ATT_X, LAY_TOPBUTTONS_ATT_Y, LAY_TOPBUTTONS_ATT_W, LAY_TOPBUTTONS_ATT_H, "ATT", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, TRX.ATT);
	if (!TRX.ADC_Driver && !TRX.ADC_PGA)
		printInfo(LAY_TOPBUTTONS_DRV_X, LAY_TOPBUTTONS_DRV_Y, LAY_TOPBUTTONS_DRV_W, LAY_TOPBUTTONS_DRV_H, "DRV", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, false);
	else if (TRX.ADC_Driver && !TRX.ADC_PGA)
		printInfo(LAY_TOPBUTTONS_DRV_X, LAY_TOPBUTTONS_DRV_Y, LAY_TOPBUTTONS_DRV_W, LAY_TOPBUTTONS_DRV_H, "DRV", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);
	else if (!TRX.ADC_Driver && TRX.ADC_PGA)
		printInfo(LAY_TOPBUTTONS_DRV_X, LAY_TOPBUTTONS_DRV_Y, LAY_TOPBUTTONS_DRV_W, LAY_TOPBUTTONS_DRV_H, "PGA", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);
	else if (TRX.ADC_Driver && TRX.ADC_PGA)
		printInfo(LAY_TOPBUTTONS_DRV_X, LAY_TOPBUTTONS_DRV_Y, LAY_TOPBUTTONS_DRV_W, LAY_TOPBUTTONS_DRV_H, "DRPG", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, true);

	printInfo(LAY_TOPBUTTONS_FAST_X, LAY_TOPBUTTONS_FAST_Y, LAY_TOPBUTTONS_FAST_W, LAY_TOPBUTTONS_FAST_H, "FAST", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, (TRX.Fast == true));
	printInfo(LAY_TOPBUTTONS_AGC_X, LAY_TOPBUTTONS_AGC_Y, LAY_TOPBUTTONS_AGC_W, LAY_TOPBUTTONS_AGC_H, "AGC", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, CurrentVFO()->AGC);
	printInfo(LAY_TOPBUTTONS_DNR_X, LAY_TOPBUTTONS_DNR_Y, LAY_TOPBUTTONS_DNR_W, LAY_TOPBUTTONS_DNR_H, "DNR", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, CurrentVFO()->DNR);
	printInfo(LAY_TOPBUTTONS_CLAR_X, LAY_TOPBUTTONS_CLAR_Y, LAY_TOPBUTTONS_CLAR_W, LAY_TOPBUTTONS_CLAR_H, "CLAR", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, TRX.CLAR);
	printInfo(LAY_TOPBUTTONS_SHIFT_X, LAY_TOPBUTTONS_SHIFT_Y, LAY_TOPBUTTONS_SHIFT_W, LAY_TOPBUTTONS_SHIFT_H, "SHIFT", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, TRX.ShiftEnabled);
	printInfo(LAY_TOPBUTTONS_NOTCH_X, LAY_TOPBUTTONS_NOTCH_Y, LAY_TOPBUTTONS_NOTCH_W, LAY_TOPBUTTONS_NOTCH_H, "NOTCH", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, (CurrentVFO()->ManualNotchFilter || CurrentVFO()->AutoNotchFilter));
	printInfo(LAY_TOPBUTTONS_NB_X, LAY_TOPBUTTONS_NB_Y, LAY_TOPBUTTONS_NB_W, LAY_TOPBUTTONS_NB_H, "NB", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, TRX.NOISE_BLANKER);

	printInfo(LAY_TOPBUTTONS_LOCK_X, LAY_TOPBUTTONS_LOCK_Y, LAY_TOPBUTTONS_LOCK_W, LAY_TOPBUTTONS_LOCK_H, "LOCK", BACKGROUND_COLOR, COLOR_BUTTON_TEXT, COLOR_BUTTON_INACTIVE_TEXT, TRX.Locked);

	LCD_busy = false;
	LCD_UpdateQuery.TopButtons = false;
}

static void LCD_displayFreqInfo()
{ //вывод частоты на экран
	if (LCD_systemMenuOpened)
		return;
	if (LCD_last_showed_freq == CurrentVFO()->Freq)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.FreqInfo = true;
		return;
	}
	LCD_busy = true;
	uint16_t mhz_x_offset = 0;
	LCD_last_showed_freq = CurrentVFO()->Freq;
	if (CurrentVFO()->Freq >= 100000000)
		mhz_x_offset = LAY_FREQ_X_OFFSET_100;
	else if (CurrentVFO()->Freq >= 10000000)
		mhz_x_offset = LAY_FREQ_X_OFFSET_10;
	else
		mhz_x_offset = LAY_FREQ_X_OFFSET_1;
	LCDDriver_Fill_RectWH(0, LAY_FREQ_Y - LAY_FREQ_HEIGHT, mhz_x_offset, LAY_FREQ_HEIGHT, BACKGROUND_COLOR);

	//добавляем пробелов для вывода частоты
	uint16_t hz = (CurrentVFO()->Freq % 1000);
	uint16_t khz = ((CurrentVFO()->Freq / 1000) % 1000);
	uint16_t mhz = ((CurrentVFO()->Freq / 1000000) % 1000000);
	sprintf(LCD_freq_string_hz, "%d", hz);
	sprintf(LCD_freq_string_khz, "%d", khz);
	sprintf(LCD_freq_string_mhz, "%d", mhz);

	if (LCD_last_showed_freq_mhz != mhz)
	{
		LCDDriver_printTextFont(LCD_freq_string_mhz, mhz_x_offset, LAY_FREQ_Y, LAY_FREQ_COLOR_MHZ, BACKGROUND_COLOR, LAY_FREQ_FONT);
		LCD_last_showed_freq_mhz = mhz;
	}

	char buff[50] = "";
	if (LCD_last_showed_freq_khz != khz)
	{
		addSymbols(buff, LCD_freq_string_khz, 3, "0", false);
		LCDDriver_printTextFont(buff, LAY_FREQ_X_OFFSET_KHZ, LAY_FREQ_Y, LAY_FREQ_COLOR_KHZ, BACKGROUND_COLOR, LAY_FREQ_FONT);
		LCD_last_showed_freq_khz = khz;
	}
	if (LCD_last_showed_freq_hz != hz)
	{
		addSymbols(buff, LCD_freq_string_hz, 3, "0", false);
		LCDDriver_printTextFont(buff, LAY_FREQ_X_OFFSET_HZ, LAY_FREQ_Y, LAY_FREQ_COLOR_HZ, BACKGROUND_COLOR, LAY_FREQ_FONT);
		LCD_last_showed_freq_hz = hz;
	}
	NeedSaveSettings = true;
	LCD_UpdateQuery.FreqInfo = false;
	LCD_busy = false;
}

static void LCD_displayStatusInfoGUI(void)
{ //вывод RX/TX и с-метра
	if (LCD_systemMenuOpened)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.StatusInfoGUI = true;
		return;
	}
	LCD_busy = true;

	LCDDriver_Fill_RectWH(0, LAY_STATUS_Y_OFFSET, LCD_WIDTH, LAY_STATUS_HEIGHT, BACKGROUND_COLOR);

	if (TRX_on_TX())
	{
		if (TRX_Tune)
			LCDDriver_printTextFont("TU", LAY_STATUS_TXRX_X_OFFSET, (LAY_STATUS_Y_OFFSET + LAY_STATUS_TXRX_Y_OFFSET), LAY_STATUS_TX_COLOR, BACKGROUND_COLOR, LAY_STATUS_TXRX_FONT);
		else
			LCDDriver_printTextFont("TX", LAY_STATUS_TXRX_X_OFFSET, (LAY_STATUS_Y_OFFSET + LAY_STATUS_TXRX_Y_OFFSET), LAY_STATUS_TX_COLOR, BACKGROUND_COLOR, LAY_STATUS_TXRX_FONT);

		LCDDriver_drawRectXY(LAY_STATUS_BAR_X_OFFSET, LAY_STATUS_Y_OFFSET, LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_PMETER_WIDTH, LAY_STATUS_Y_OFFSET + LAY_STATUS_BAR_HEIGHT, LAY_STATUS_BAR_COLOR); //рамка SWR-метра
		LCDDriver_printText("SWR:", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_TX_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("FWD:", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X + LAY_STATUS_TX_LABELS_MARGIN_X, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_TX_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("REF:", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X + LAY_STATUS_TX_LABELS_MARGIN_X * 2, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_TX_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);

		LCDDriver_drawRectXY(LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_PMETER_WIDTH + LAY_STATUS_ALC_BAR_X_OFFSET, LAY_STATUS_Y_OFFSET, LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_PMETER_WIDTH + LAY_STATUS_ALC_BAR_X_OFFSET + LAY_STATUS_AMETER_WIDTH, LAY_STATUS_Y_OFFSET + LAY_STATUS_BAR_HEIGHT, LAY_STATUS_BAR_COLOR); //рамка ALC-метра
		LCDDriver_printText("ALC:", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_PMETER_WIDTH + LAY_STATUS_ALC_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_TX_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
	}
	else
	{
		LCDDriver_drawRectXY(LAY_STATUS_BAR_X_OFFSET, LAY_STATUS_Y_OFFSET, LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_SMETER_WIDTH, LAY_STATUS_Y_OFFSET + LAY_STATUS_BAR_HEIGHT, LAY_STATUS_BAR_COLOR); //рамка S-метра
		LCDDriver_printTextFont("RX", LAY_STATUS_TXRX_X_OFFSET, (LAY_STATUS_Y_OFFSET + LAY_STATUS_TXRX_Y_OFFSET), LAY_STATUS_RX_COLOR, BACKGROUND_COLOR, LAY_STATUS_TXRX_FONT);

		LCDDriver_printTextFont(".", LAY_FREQ_DELIMITER_X1_OFFSET, LAY_FREQ_Y + LAY_FREQ_DELIMITER_Y_OFFSET, LAY_FREQ_COLOR_KHZ, BACKGROUND_COLOR, LAY_FREQ_FONT); //разделители частоты
		LCDDriver_printTextFont(".", LAY_FREQ_DELIMITER_X2_OFFSET, LAY_FREQ_Y + LAY_FREQ_DELIMITER_Y_OFFSET, LAY_FREQ_COLOR_HZ, BACKGROUND_COLOR, LAY_FREQ_FONT);

		LCDDriver_printText("dBm", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_SMETER_WIDTH + LAY_STATUS_LABEL_DBM_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABEL_DBM_Y_OFFSET, COLOR_GREEN, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);

		uint16_t step = LAY_STATUS_SMETER_WIDTH / 8;
		LCDDriver_printText("1", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TXRX_X_OFFSET + step * 0, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("3", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TXRX_X_OFFSET + step * 1, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("5", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TXRX_X_OFFSET + step * 2, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("7", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TXRX_X_OFFSET + step * 3, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("9", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TXRX_X_OFFSET + step * 4, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("+20", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TXRX_X_OFFSET + step * 5, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("+40", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TXRX_X_OFFSET + step * 6, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("+60", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TXRX_X_OFFSET + step * 7, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);

		if (CurrentVFO()->ManualNotchFilter)
		{
			char buff[10] = "";
			sprintf(buff, "%uhz", CurrentVFO()->NotchFC);
			addSymbols(buff, buff, 7, " ", false);
			LCDDriver_printText(buff, LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_SMETER_WIDTH + LAY_STATUS_LABEL_NOTCH_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_NOTCH_COLOR, BACKGROUND_COLOR, 1);
		}
		else
		{
			LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_SMETER_WIDTH + LAY_STATUS_LABEL_NOTCH_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABEL_BW_WIDTH, LAY_STATUS_LABEL_BW_HEIGHT, BACKGROUND_COLOR);
			if (TRX.FFT_Zoom == 1)
				LCDDriver_printText("48kHz", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_SMETER_WIDTH + LAY_STATUS_LABEL_NOTCH_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_BW_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
			if (TRX.FFT_Zoom == 2)
				LCDDriver_printText("24kHz", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_SMETER_WIDTH + LAY_STATUS_LABEL_NOTCH_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_BW_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
			if (TRX.FFT_Zoom == 4)
				LCDDriver_printText("12kHz", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_SMETER_WIDTH + LAY_STATUS_LABEL_NOTCH_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_BW_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
			if (TRX.FFT_Zoom == 8)
				LCDDriver_printText(" 6kHz", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_SMETER_WIDTH + LAY_STATUS_LABEL_NOTCH_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_BW_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
			if (TRX.FFT_Zoom == 16)
				LCDDriver_printText(" 3kHz", LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_SMETER_WIDTH + LAY_STATUS_LABEL_NOTCH_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_LABELS_BW_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		}
	}

	//Redraw CW decoder
	if (TRX.CWDecoder && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U))
	{
		LCDDriver_Fill_RectWH(0, LCD_HEIGHT - LAY_FFT_CWDECODER_OFFSET, LAY_FFT_PRINT_SIZE, LAY_FFT_CWDECODER_OFFSET, BACKGROUND_COLOR);
		LCD_UpdateQuery.TextBar = true;
	}

	LCD_UpdateQuery.StatusInfoGUI = false;
	LCD_busy = false;
}

static void LCD_displayStatusInfoBar(void)
{ //S-метра и прочей информации
	if (LCD_systemMenuOpened)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.StatusInfoBar = true;
		return;
	}
	LCD_busy = true;
	char ctmp[50];
	const int width = LAY_STATUS_SMETER_WIDTH - 2;

	if (!TRX_on_TX())
	{
		float32_t TRX_s_meter = (127.0f + TRX_RX_dBm) / 6; //127dbm - S0, 6dBm - 1S div
		if (CurrentVFO()->Freq >= 144000000)
			TRX_s_meter = (127.0f + TRX_RX_dBm) / 6; //147dbm - S0 для частот выше 144мгц
		if (TRX_s_meter <= 9.0f)
			TRX_s_meter = TRX_s_meter * ((width / 9.0f) * 5.0f / 9.0f); //первые 9 баллов по 6 дб, первые 5 из 8 рисок (9 участков)
		else
			TRX_s_meter = ((width / 9.0f) * 5.0f) + (TRX_s_meter - 9.0f) * ((width / 9.0f) * 3.0f / 10.0f); //остальные 3 балла по 10 дб
		if (TRX_s_meter > width)
			TRX_s_meter = width;
		if (TRX_s_meter < 0.0f)
			TRX_s_meter = 0.0f;

		int16_t s_width = (int16_t)TRX_s_meter;
		if (LCD_last_s_meter > s_width)
			s_width = LCD_last_s_meter - ((LCD_last_s_meter - s_width) / 4); //сглаживаем движение с-метра
		else if (LCD_last_s_meter < s_width)
			s_width = s_width - ((s_width - LCD_last_s_meter) / 2);
		if (LCD_last_s_meter != s_width)
		{
			if (!TRX.S_METER_Style) //полоса
			{
				LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + 1 + (uint16_t)s_width, LAY_STATUS_Y_OFFSET + 1, (uint16_t)(LCD_last_s_meter - s_width), LAY_STATUS_BAR_HEIGHT - 3, BACKGROUND_COLOR);
				LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + 1, LAY_STATUS_Y_OFFSET + 1, (uint16_t)s_width, LAY_STATUS_BAR_HEIGHT - 3, LAY_STATUS_SMETER_COLOR);
			}
			else //линия
			{
				LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + 1, LAY_STATUS_Y_OFFSET + 1, width, LAY_STATUS_BAR_HEIGHT - 2, BACKGROUND_COLOR);
				LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + 1 + (uint16_t)s_width, LAY_STATUS_Y_OFFSET + 1, 1, LAY_STATUS_BAR_HEIGHT - 2, LAY_STATUS_SMETER_COLOR);
			}

			LCD_last_s_meter = s_width;
		}

		sprintf(ctmp, "%d", TRX_RX_dBm);
		LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + 1 + width + 5, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABEL_DBM_Y_OFFSET, LAY_STATUS_LABEL_DBM_WIDTH, LAY_STATUS_LABEL_DBM_HEIGHT, BACKGROUND_COLOR);
		LCDDriver_printText(ctmp, LAY_STATUS_BAR_X_OFFSET + 1 + width + 5, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABEL_DBM_Y_OFFSET, COLOR_GREEN, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
	}
	else
	{
		//SWR
		LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X + LAY_STATUS_SMETER_TXLABELS_PADDING, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_TX_LABELS_VAL_WIDTH, LAY_STATUS_TX_LABELS_VAL_HEIGHT, BACKGROUND_COLOR);
		sprintf(ctmp, "%.1f", (double)TRX_SWR);
		LCDDriver_printText(ctmp, LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X + LAY_STATUS_SMETER_TXLABELS_PADDING, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, COLOR_RED, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);

		//FWD
		LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X + LAY_STATUS_SMETER_TXLABELS_MARGIN + LAY_STATUS_SMETER_TXLABELS_PADDING, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_TX_LABELS_VAL_WIDTH, LAY_STATUS_TX_LABELS_VAL_HEIGHT, BACKGROUND_COLOR);
		float32_t fwd_power = (TRX_SWR_forward * TRX_SWR_forward) / 50.0f;
		if (fwd_power < 0.0f)
			fwd_power = 0.0f;
		sprintf(ctmp, "%.1fW", (double)fwd_power);
		LCDDriver_printText(ctmp, LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X + LAY_STATUS_SMETER_TXLABELS_MARGIN + LAY_STATUS_SMETER_TXLABELS_PADDING, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);

		//REF
		LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X + LAY_STATUS_SMETER_TXLABELS_MARGIN * 2 + LAY_STATUS_SMETER_TXLABELS_PADDING, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_TX_LABELS_VAL_WIDTH, LAY_STATUS_TX_LABELS_VAL_HEIGHT, BACKGROUND_COLOR);
		float32_t ref_power = (TRX_SWR_backward * TRX_SWR_backward) / 50.0f;
		if (ref_power < 0.0f)
			ref_power = 0.0f;
		sprintf(ctmp, "%.1fW", (double)ref_power);
		LCDDriver_printText(ctmp, LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_TX_LABELS_OFFSET_X + LAY_STATUS_SMETER_TXLABELS_MARGIN * 2 + LAY_STATUS_SMETER_TXLABELS_PADDING, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);

		//SWR Meter
		if (fwd_power > MAX_RF_POWER)
			fwd_power = MAX_RF_POWER;
		uint16_t ref_width = (uint16_t)(ref_power * (LAY_STATUS_PMETER_WIDTH - 2) / MAX_RF_POWER);
		uint16_t fwd_width = (uint16_t)(fwd_power * (LAY_STATUS_PMETER_WIDTH - 2) / MAX_RF_POWER);
		uint16_t est_width = (uint16_t)((MAX_RF_POWER - fwd_power) * (LAY_STATUS_PMETER_WIDTH - 2) / MAX_RF_POWER);
		if (ref_width > fwd_width)
			ref_width = fwd_width;
		fwd_width -= ref_width;
		LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + 1, LAY_STATUS_Y_OFFSET + 1, fwd_width, LAY_STATUS_BAR_HEIGHT - 2, LAY_STATUS_LABELS_TX_COLOR);
		LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + 1 + fwd_width, LAY_STATUS_Y_OFFSET + 1, ref_width, LAY_STATUS_BAR_HEIGHT - 2, LAY_STATUS_BAR_COLOR);
		LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + 1 + fwd_width + ref_width, LAY_STATUS_Y_OFFSET + 1, est_width, LAY_STATUS_BAR_HEIGHT - 2, BACKGROUND_COLOR);

		//ALC
		LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_PMETER_WIDTH + LAY_STATUS_TX_ALC_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_TX_LABELS_VAL_WIDTH, LAY_STATUS_TX_LABELS_VAL_HEIGHT, BACKGROUND_COLOR);
		uint8_t alc_level = (uint8_t)(TRX_GetALC() * 100);
		sprintf(ctmp, "%d%%", alc_level);
		LCDDriver_printText(ctmp, LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_PMETER_WIDTH + LAY_STATUS_TX_ALC_X_OFFSET, LAY_STATUS_Y_OFFSET + LAY_STATUS_LABELS_OFFSET_Y, LAY_STATUS_BAR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
		uint8_t alc_level_width = (LAY_STATUS_AMETER_WIDTH - 2) * alc_level / 100;
		if (alc_level_width > (LAY_STATUS_AMETER_WIDTH - 2))
			alc_level_width = LAY_STATUS_AMETER_WIDTH - 2;
		LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_PMETER_WIDTH + LAY_STATUS_ALC_BAR_X_OFFSET + 1, LAY_STATUS_Y_OFFSET + 1, alc_level_width, LAY_STATUS_BAR_HEIGHT - 2, LAY_STATUS_LABELS_TX_COLOR);
		if (alc_level < 100)
			LCDDriver_Fill_RectWH(LAY_STATUS_BAR_X_OFFSET + LAY_STATUS_PMETER_WIDTH + LAY_STATUS_ALC_BAR_X_OFFSET + 1 + alc_level_width, LAY_STATUS_Y_OFFSET + 1, LAY_STATUS_AMETER_WIDTH - 2 - alc_level_width, LAY_STATUS_BAR_HEIGHT - 2, LAY_STATUS_LABELS_NOTCH_COLOR);
	}

	LCDDriver_Fill_RectWH(LAY_STATUS_ERR_OFFSET_X, LAY_STATUS_ERR_OFFSET_Y, LAY_STATUS_ERR_WIDTH, LAY_STATUS_ERR_HEIGHT, BACKGROUND_COLOR);
	if (TRX_ADC_OTR && !TRX_on_TX())
		LCDDriver_printText("OVR", LAY_STATUS_ERR_OFFSET_X, LAY_STATUS_ERR_OFFSET_Y, LAY_STATUS_ERR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
	if (TRX_DAC_OTR && TRX_on_TX())
		LCDDriver_printText("OVR", LAY_STATUS_ERR_OFFSET_X, LAY_STATUS_ERR_OFFSET_Y, LAY_STATUS_ERR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
	//else if (TRX_ADC_MAXAMPLITUDE > (ADC_FULL_SCALE * 0.49f) || TRX_ADC_MINAMPLITUDE < -(ADC_FULL_SCALE * 0.49f))
	//LCDDriver_printText("MVR", LAY_STATUS_ERR_OFFSET_X, LAY_STATUS_ERR_OFFSET_Y, LAY_STATUS_ERR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
	if (WM8731_Buffer_underrun && !TRX_on_TX())
		LCDDriver_printText("WBF", LAY_STATUS_ERR_OFFSET_X + LAY_STATUS_ERR2_OFFSET_X, LAY_STATUS_ERR_OFFSET_Y, LAY_STATUS_ERR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
	if (FPGA_Buffer_underrun && TRX_on_TX())
		LCDDriver_printText("FBF", LAY_STATUS_ERR_OFFSET_X + LAY_STATUS_ERR2_OFFSET_X, LAY_STATUS_ERR_OFFSET_Y, LAY_STATUS_ERR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);
	if (RX_USB_AUDIO_underrun)
		LCDDriver_printText("UBF", LAY_STATUS_ERR_OFFSET_X + LAY_STATUS_ERR2_OFFSET_X, LAY_STATUS_ERR_OFFSET_Y, LAY_STATUS_ERR_COLOR, BACKGROUND_COLOR, LAY_STATUS_LABELS_FONT_SIZE);

	Time = RTC->TR;
	Hours = ((Time >> 20) & 0x03) * 10 + ((Time >> 16) & 0x0f);
	Minutes = ((Time >> 12) & 0x07) * 10 + ((Time >> 8) & 0x0f);
	Seconds = ((Time >> 4) & 0x07) * 10 + ((Time >> 0) & 0x0f);

	if (Hours != Last_showed_Hours)
	{
		sprintf(ctmp, "%d", Hours);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printTextFont(ctmp, LAY_CLOCK_POS_HRS_X, LAY_CLOCK_POS_Y, COLOR_WHITE, BACKGROUND_COLOR, LAY_CLOCK_FONT);
		LCDDriver_printTextFont(":", LCDDriver_GetCurrentXOffset(), LAY_CLOCK_POS_Y, COLOR_WHITE, COLOR_BLACK, LAY_CLOCK_FONT);
		Last_showed_Hours = Hours;
	}
	if (Minutes != Last_showed_Minutes)
	{
		sprintf(ctmp, "%d", Minutes);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printTextFont(ctmp, LAY_CLOCK_POS_MIN_X, LAY_CLOCK_POS_Y, COLOR_WHITE, BACKGROUND_COLOR, LAY_CLOCK_FONT);
		LCDDriver_printTextFont(":", LCDDriver_GetCurrentXOffset(), LAY_CLOCK_POS_Y, COLOR_WHITE, BACKGROUND_COLOR, LAY_CLOCK_FONT);
		Last_showed_Minutes = Minutes;
	}
	if (Seconds != Last_showed_Seconds)
	{
		sprintf(ctmp, "%d", Seconds);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printTextFont(ctmp, LAY_CLOCK_POS_SEC_X, LAY_CLOCK_POS_Y, COLOR_WHITE, BACKGROUND_COLOR, LAY_CLOCK_FONT);
		Last_showed_Seconds = Seconds;
	}

	if (WIFI_connected)
		LCDDriver_printImage_RLECompressed(LAY_STATUS_WIFI_ICON_X, LAY_STATUS_WIFI_ICON_Y, &IMAGES_wifi_active);
	else
		LCDDriver_printImage_RLECompressed(LAY_STATUS_WIFI_ICON_X, LAY_STATUS_WIFI_ICON_Y, &IMAGES_wifi_inactive);

	LCD_UpdateQuery.StatusInfoBar = false;
	LCD_busy = false;
}

static void LCD_displayTextBar(void)
{ //вывод текста под водопадом
	if (LCD_systemMenuOpened)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.TextBar = true;
		return;
	}
	LCD_busy = true;

	if (TRX.CWDecoder && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U || CurrentVFO()->Mode == TRX_MODE_LOOPBACK))
	{
		char ctmp[50];
		sprintf(ctmp, "WPM:%d ", CW_Decoder_WPM);
		LCDDriver_printText(ctmp, 0, LCD_HEIGHT - LAY_FFT_CWDECODER_OFFSET + 1, COLOR_WHITE, BACKGROUND_COLOR, LAY_TEXTBAR_FONT);
		LCDDriver_printText((char *)&CW_Decoder_Text, LAY_TEXTBAR_TEXT_X_OFFSET, LCD_HEIGHT - LAY_FFT_CWDECODER_OFFSET + 1, COLOR_WHITE, BACKGROUND_COLOR, LAY_TEXTBAR_FONT);
	}

	LCD_UpdateQuery.TextBar = false;
	LCD_busy = false;
}

void LCD_redraw(void)
{
	LCD_UpdateQuery.Background = true;
	LCD_UpdateQuery.FreqInfo = true;
	LCD_UpdateQuery.StatusInfoBar = true;
	LCD_UpdateQuery.StatusInfoGUI = true;
	LCD_UpdateQuery.TopButtons = true;
	LCD_UpdateQuery.SystemMenu = true;
	LCD_UpdateQuery.TextBar = true;
	LCD_last_s_meter = 0;
	LCD_last_showed_freq = 0;
	Last_showed_Hours = 255;
	Last_showed_Minutes = 255;
	Last_showed_Seconds = 255;
	LCD_last_showed_freq_mhz = 9999;
	LCD_last_showed_freq_khz = 9999;
	LCD_last_showed_freq_hz = 9999;
	LCD_doEvents();
}

void LCD_doEvents(void)
{
	if (LCD_busy)
		return;
	if (TRX_Time_InActive > TRX.Standby_Time && TRX.Standby_Time > 0)
		LCDDriver_setBrightness(5);
	else
		LCDDriver_setBrightness(TRX.LCD_Brightness);

	if (LCD_UpdateQuery.Background)
	{
		LCD_busy = true;
		LCDDriver_Fill(BACKGROUND_COLOR);
		LCD_UpdateQuery.Background = false;
		LCD_busy = false;
	}
	if (LCD_UpdateQuery.TopButtons)
		LCD_displayTopButtons(false);
	if (LCD_UpdateQuery.FreqInfo)
		LCD_displayFreqInfo();
	if (LCD_UpdateQuery.StatusInfoGUI)
		LCD_displayStatusInfoGUI();
	LCD_displayStatusInfoBar();
	if (LCD_UpdateQuery.SystemMenu)
		drawSystemMenu(false);
	if (LCD_UpdateQuery.TextBar)
		LCD_displayTextBar();
}

static void printInfo(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t inactive_color, bool active)
{
	uint16_t x1, y1, w, h;
	LCDDriver_Fill_RectWH(x, y, width, height, back_color);
	LCDDriver_getTextBounds(text, x, y, &x1, &y1, &w, &h, (GFXfont *)&FreeSans9pt7b);
	//sendToDebug_str(text); sendToDebug_str(" "); sendToDebug_uint16(w, false);
	LCDDriver_printTextFont(text, x + (width - w) / 2, y + (height / 2) + h / 2 - 1, active ? text_color : inactive_color, back_color, (GFXfont *)&FreeSans9pt7b);
}

void LCD_showError(char text[], bool redraw)
{
	LCD_busy = true;
	LCDDriver_Fill(COLOR_RED);
	LCDDriver_printTextFont(text, 5, LCD_HEIGHT / 2, COLOR_WHITE, COLOR_RED, (GFXfont *)&FreeSans12pt7b);
	if (redraw)
		HAL_Delay(2000);
	LCD_busy = false;
	if (redraw)
		LCD_redraw();
}
