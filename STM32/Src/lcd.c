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
#include "rds_decoder.h"
#include "front_unit.h"
#include "screen_layout.h"
#include "images.h"
#include "wifi.h"
#include "fft.h"
#include "sd.h"

volatile bool LCD_busy = false;
volatile DEF_LCD_UpdateQuery LCD_UpdateQuery = {false};
volatile bool LCD_systemMenuOpened = false;
uint16_t LCD_bw_trapez_stripe_pos = 0;
IRAM2 WindowType LCD_window = {0};
STRUCT_COLOR_THEME *COLOR = &COLOR_THEMES[0];
STRUCT_LAYOUT_THEME *LAYOUT = &LAYOUT_THEMES[0];
static uint32_t manualFreqEnter = 0;

static char LCD_freq_string_hz[6] = {0};
static char LCD_freq_string_khz[6] = {0};
static char LCD_freq_string_mhz[6] = {0};
static uint64_t LCD_last_showed_freq = 0;
static uint16_t LCD_last_showed_freq_mhz = 9999;
static uint16_t LCD_last_showed_freq_khz = 9999;
static uint16_t LCD_last_showed_freq_hz = 9999;
#if (defined(LAY_800x480))
static char LCD_freq_string_hz_B[6] = {0};
static char LCD_freq_string_khz_B[6] = {0};
static char LCD_freq_string_mhz_B[6] = {0};
static uint64_t LCD_last_showed_freq_B = 0;
static uint16_t LCD_last_showed_freq_mhz_B = 9999;
static uint16_t LCD_last_showed_freq_khz_B = 9999;
static uint16_t LCD_last_showed_freq_hz_B = 9999;
#endif

static bool LCD_inited = false;
static float32_t LCD_last_s_meter = 1.0f;
static uint32_t Time;
static uint8_t Hours;
static uint8_t Last_showed_Hours = 255;
static uint8_t Minutes;
static uint8_t Last_showed_Minutes = 255;
static uint8_t Seconds;
static uint8_t Last_showed_Seconds = 255;

static uint32_t Tooltip_DiplayStartTime = 0;
static bool Tooltip_first_draw = true;
static char Tooltip_string[64] = {0};
static bool LCD_showInfo_opened = false;

static TouchpadButton_handler TouchpadButton_handlers[64] = {0};
static uint8_t TouchpadButton_handlers_count = 0;

static void printInfoSmall(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t in_active_color, bool active);
static void printInfo(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t in_active_color, bool active);
static void LCD_displayFreqInfo(bool redraw);
static void LCD_displayTopButtons(bool redraw);
static void LCD_displayStatusInfoBar(bool redraw);
static void LCD_displayStatusInfoGUI(bool redraw);
static void LCD_displayTextBar(void);
static void LCD_printTooltip(void);
static void LCD_showBandWindow(bool secondary_vfo);
static void LCD_showModeWindow(bool secondary_vfo);
static void LCD_showBWWindow(void);
static void LCD_ManualFreqButtonHandler(uint32_t parameter);
#if (defined(LAY_800x480))
static void printButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, bool active, bool show_lighter, bool in_window, uint32_t parameter, void (*clickHandler)(uint32_t parameter), void (*holdHandler)(uint32_t parameter), uint16_t active_color, uint16_t inactive_color);
#endif

void LCD_Init(void)
{
	COLOR = &COLOR_THEMES[TRX.ColorThemeId];
	LAYOUT = &LAYOUT_THEMES[TRX.LayoutThemeId];

	//DMA2D LCD addr
	WRITE_REG(hdma2d.Instance->OMAR, LCD_FSMC_DATA_ADDR);
	//DMA2D FILL MODE
	MODIFY_REG(hdma2d.Instance->CR, DMA2D_CR_MODE | DMA2D_CR_LOM, DMA2D_R2M | DMA2D_LOM_PIXELS);
	//DMA2D PIXEL FORMAT
	MODIFY_REG(hdma2d.Instance->OPFCCR, DMA2D_OPFCCR_CM | DMA2D_OPFCCR_SB, DMA2D_OUTPUT_ARGB8888 | DMA2D_BYTES_REGULAR);

	LCDDriver_Init();
#if SCREEN_ROTATE
	LCDDriver_setRotation(2);
#else
	LCDDriver_setRotation(4);
#endif
#ifdef HAS_TOUCHPAD
	TOUCHPAD_Init();
#endif
	LCDDriver_Fill(BG_COLOR);

	LCD_inited = true;
}

static void LCD_displayTopButtons(bool redraw)
{ // display the top buttons
	if (LCD_systemMenuOpened || LCD_window.opened)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.TopButtons = true;
		return;
	}
	LCD_busy = true;
	if (redraw)
		LCDDriver_Fill_RectWH(LAYOUT->TOPBUTTONS_X1, LAYOUT->TOPBUTTONS_Y1, LAYOUT->TOPBUTTONS_X2, LAYOUT->TOPBUTTONS_Y2, BG_COLOR);

// display information about the operation of the transceiver
#if (defined(LAY_800x480))
	printButton(LAYOUT->TOPBUTTONS_PRE_X, LAYOUT->TOPBUTTONS_PRE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PRE", TRX.LNA, true, false, 0, FRONTPANEL_BUTTONHANDLER_PRE, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	char buff[64] = {0};
	sprintf(buff, "ATT%d", (uint8_t)TRX.ATT_DB);
	printButton(LAYOUT->TOPBUTTONS_ATT_X, LAYOUT->TOPBUTTONS_ATT_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, buff, TRX.ATT, true, false, 0, FRONTPANEL_BUTTONHANDLER_ATT, FRONTPANEL_BUTTONHANDLER_ATTHOLD, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->TOPBUTTONS_PGA_X, LAYOUT->TOPBUTTONS_PGA_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PGA", TRX.ADC_PGA, true, false, 0, FRONTPANEL_BUTTONHANDLER_PGA_ONLY, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->TOPBUTTONS_DRV_X, LAYOUT->TOPBUTTONS_DRV_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DRV", TRX.ADC_Driver, true, false, 0, FRONTPANEL_BUTTONHANDLER_DRV_ONLY, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->TOPBUTTONS_FAST_X, LAYOUT->TOPBUTTONS_FAST_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "FAST", TRX.Fast, true, false, 0, FRONTPANEL_BUTTONHANDLER_FAST, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->TOPBUTTONS_AGC_X, LAYOUT->TOPBUTTONS_AGC_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "AGC", CurrentVFO->AGC, true, false, 0, FRONTPANEL_BUTTONHANDLER_AGC, FRONTPANEL_BUTTONHANDLER_AGC_SPEED, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	if (CurrentVFO->DNR_Type == 1)
		printButton(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR1", true, true, false, 0, FRONTPANEL_BUTTONHANDLER_DNR, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	else if (CurrentVFO->DNR_Type == 2)
		printButton(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR2", true, true, false, 0, FRONTPANEL_BUTTONHANDLER_DNR, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	else
		printButton(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR", false, true, false, 0, FRONTPANEL_BUTTONHANDLER_DNR, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB", TRX.NOISE_BLANKER, true, false, 0, FRONTPANEL_BUTTONHANDLER_NB, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->TOPBUTTONS_NOTCH_X, LAYOUT->TOPBUTTONS_NOTCH_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NOTCH", (CurrentVFO->AutoNotchFilter || CurrentVFO->ManualNotchFilter), true, false, 0, FRONTPANEL_BUTTONHANDLER_NOTCH, FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->TOPBUTTONS_SQL_X, LAYOUT->TOPBUTTONS_SQL_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "SQL", CurrentVFO->SQL, true, false, 0, FRONTPANEL_BUTTONHANDLER_SQL, FRONTPANEL_BUTTONHANDLER_SQUELCH, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->TOPBUTTONS_MUTE_X, LAYOUT->TOPBUTTONS_MUTE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "MUTE", TRX_Mute, true, false, 0, FRONTPANEL_BUTTONHANDLER_MUTE, NULL, COLOR_RED, COLOR->BUTTON_INACTIVE_TEXT);
#else
	printInfo(LAYOUT->TOPBUTTONS_PRE_X, LAYOUT->TOPBUTTONS_PRE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PRE", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, TRX.LNA);
	char buff[64] = {0};
	sprintf(buff, "ATT%d", (uint8_t)TRX.ATT_DB);
	if (TRX.ATT_DB == 0)
		sprintf(buff, "ATT");
	printInfo(LAYOUT->TOPBUTTONS_ATT_X, LAYOUT->TOPBUTTONS_ATT_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, buff, COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, TRX.ATT);
	printInfo(LAYOUT->TOPBUTTONS_PGA_X, LAYOUT->TOPBUTTONS_PGA_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PGA", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, TRX.ADC_PGA);
	printInfo(LAYOUT->TOPBUTTONS_DRV_X, LAYOUT->TOPBUTTONS_DRV_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DRV", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, TRX.ADC_Driver);
	printInfo(LAYOUT->TOPBUTTONS_FAST_X, LAYOUT->TOPBUTTONS_FAST_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "FAST", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, TRX.Fast);
	printInfo(LAYOUT->TOPBUTTONS_AGC_X, LAYOUT->TOPBUTTONS_AGC_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "AGC", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, CurrentVFO->AGC);
	if (CurrentVFO->DNR_Type == 1)
		printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR1", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, true);
	else if (CurrentVFO->DNR_Type == 2)
		printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR2", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, true);
	else
		printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, false);
	printInfo(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, TRX.NOISE_BLANKER);
	printInfo(LAYOUT->TOPBUTTONS_MUTE_X, LAYOUT->TOPBUTTONS_MUTE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "MUTE", COLOR->BUTTON_BACKGROUND, COLOR_RED, COLOR->BUTTON_INACTIVE_TEXT, TRX_Mute);
	printInfo(LAYOUT->TOPBUTTONS_LOCK_X, LAYOUT->TOPBUTTONS_LOCK_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "LOCK", COLOR->BUTTON_BACKGROUND, COLOR_RED, COLOR->BUTTON_INACTIVE_TEXT, TRX.Locked);
#endif

	LCD_UpdateQuery.TopButtons = false;
	if (redraw)
		LCD_UpdateQuery.TopButtonsRedraw = false;
	LCD_busy = false;
}

static void LCD_displayBottomButtons(bool redraw)
{
	// display the bottom buttons
	if (LCD_systemMenuOpened || LCD_window.opened)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.BottomButtons = true;
		return;
	}
	LCD_busy = true;

#if (defined(LAY_800x480))
	if (redraw)
		LCDDriver_Fill_RectWH(0, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LCD_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, BG_COLOR);
	TouchpadButton_handlers_count = 0;
	uint16_t curr_x = 0;
	printButton(0, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ARROWS_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "<-", true, false, false, 0, FRONTPANEL_BUTTONHANDLER_LEFT_ARR, FRONTPANEL_BUTTONHANDLER_LEFT_ARR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	curr_x += LAYOUT->BOTTOM_BUTTONS_ARROWS_WIDTH;
	for (uint8_t i = 0; i < 8; i++)
	{
		printButton(curr_x, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, (char *)PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + i]].name, true, false, false, 0, PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + i]].clickHandler, PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[FRONTPANEL_funcbuttons_page * FUNCBUTTONS_ON_PAGE + i]].holdHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
		curr_x += LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH;
	}
	printButton(curr_x, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ARROWS_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "->", true, false, false, 0, FRONTPANEL_BUTTONHANDLER_RIGHT_ARR, FRONTPANEL_BUTTONHANDLER_RIGHT_ARR, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
#endif

	LCD_UpdateQuery.BottomButtons = false;
	if (redraw)
		LCD_UpdateQuery.BottomButtonsRedraw = false;
	LCD_busy = false;
}

static void LCD_displayFreqInfo(bool redraw)
{ // display the frequency on the screen
	if (LCD_systemMenuOpened || LCD_window.opened)
		return;
	if (!redraw && (LCD_last_showed_freq == CurrentVFO->Freq)
#if (defined(LAY_800x480))
		&& (LCD_last_showed_freq_B == SecondaryVFO->Freq)
#endif
	)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.FreqInfo = true;
		if (redraw)
			LCD_UpdateQuery.FreqInfoRedraw = true;
		return;
	}
	LCD_busy = true;

	uint64_t display_freq = CurrentVFO->Freq;
#if (defined(LAY_800x480))
	display_freq = TRX.VFO_A.Freq;
#endif
	if (TRX.Transverter_Enabled)
		display_freq += (uint64_t)TRX.Transverter_Offset_Mhz * 1000 * 1000;

	LCD_last_showed_freq = display_freq;
	uint16_t hz = (display_freq % 1000);
	uint16_t khz = ((display_freq / 1000) % 1000);
	uint16_t mhz = ((display_freq / 1000000) % 1000);
	uint16_t ghz = ((display_freq / 1000000000) % 1000);
	if(display_freq >= 1000000000) //>= 1GHZ
	{
		hz = khz;
		khz = mhz;
		mhz = ghz;
	}

	uint16_t mhz_x_offset = 0;
	if (mhz >= 100)
		mhz_x_offset = LAYOUT->FREQ_X_OFFSET_100;
	else if (mhz >= 10)
		mhz_x_offset = LAYOUT->FREQ_X_OFFSET_10;
	else
		mhz_x_offset = LAYOUT->FREQ_X_OFFSET_1;

	if (redraw)
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_Y_TOP, LCD_WIDTH - LAYOUT->FREQ_LEFT_MARGIN - LAYOUT->FREQ_RIGHT_MARGIN, LAYOUT->FREQ_BLOCK_HEIGHT, BG_COLOR);

	if ((mhz_x_offset - LAYOUT->FREQ_LEFT_MARGIN) > 0)
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_Y_TOP, mhz_x_offset - LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_BLOCK_HEIGHT, BG_COLOR);

	// add spaces to output the frequency
	sprintf(LCD_freq_string_hz, "%d", hz);
	sprintf(LCD_freq_string_khz, "%d", khz);
	sprintf(LCD_freq_string_mhz, "%d", mhz);

	if (redraw || (LCD_last_showed_freq_mhz != mhz))
	{
		LCDDriver_printTextFont(LCD_freq_string_mhz, mhz_x_offset, LAYOUT->FREQ_Y_BASELINE, !TRX.selected_vfo ? COLOR->FREQ_MHZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_mhz = mhz;
	}

	char buff[50] = "";
	if (redraw || (LCD_last_showed_freq_khz != khz))
	{
		addSymbols(buff, LCD_freq_string_khz, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_KHZ, LAYOUT->FREQ_Y_BASELINE, !TRX.selected_vfo ? COLOR->FREQ_KHZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_khz = khz;
	}
	if (redraw || (LCD_last_showed_freq_hz != hz) || TRX.ChannelMode)
	{
		addSymbols(buff, LCD_freq_string_hz, 3, "0", false);
		int_fast8_t band = -1;
		int_fast8_t channel = -1;
		if(TRX.ChannelMode)
		{
			band = getBandFromFreq(display_freq, false);
			if(!TRX.selected_vfo)
				channel = getChannelbyFreq(display_freq, false);
			else
				channel = getChannelbyFreq(display_freq, true);
		}
		if(TRX.ChannelMode && BANDS[band].channelsCount > 0)
		{
			sprintf(buff, "CH:-");
			if(band != -1 && channel != -1)
				sprintf(buff, "CH:%s", (char *)BANDS[band].channels[channel].name);
			addSymbols(buff, buff, 6, " ", true);
			LCDDriver_printText(buff, LAYOUT->FREQ_X_OFFSET_HZ, LAYOUT->FREQ_Y_BASELINE_SMALL - 18, !TRX.selected_vfo ? COLOR->FREQ_KHZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, 2);
		}
		else
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_HZ, LAYOUT->FREQ_Y_BASELINE_SMALL, !TRX.selected_vfo ? COLOR->FREQ_HZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, LAYOUT->FREQ_SMALL_FONT);
		LCD_last_showed_freq_hz = hz;
	}

#if (defined(LAY_800x480))
	display_freq = TRX.VFO_B.Freq;
	if (TRX.Transverter_Enabled)
		display_freq += (uint64_t)TRX.Transverter_Offset_Mhz * 1000 * 1000;
	
	LCD_last_showed_freq_B = display_freq;
	uint16_t hz_B = (LCD_last_showed_freq_B % 1000);
	uint16_t khz_B = ((LCD_last_showed_freq_B / 1000) % 1000);
	uint16_t mhz_B = ((LCD_last_showed_freq_B / 1000000) % 1000);
	uint16_t ghz_B = ((LCD_last_showed_freq_B / 1000000000) % 1000);
	if(display_freq >= 1000000000) //>= 1GHZ
	{
		hz_B = khz_B;
		khz_B = mhz_B;
		mhz_B = ghz_B;
	}
	
	uint16_t mhz_x_offset_B = 0;
	if (mhz_B >= 100)
		mhz_x_offset_B = LAYOUT->FREQ_B_X_OFFSET_100;
	else if (mhz_B >= 10)
		mhz_x_offset_B = LAYOUT->FREQ_B_X_OFFSET_10;
	else
		mhz_x_offset_B = LAYOUT->FREQ_B_X_OFFSET_1;

	if (redraw)
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_B_LEFT_MARGIN, LAYOUT->FREQ_B_Y_TOP, LCD_WIDTH - LAYOUT->FREQ_B_LEFT_MARGIN - LAYOUT->FREQ_B_RIGHT_MARGIN, LAYOUT->FREQ_B_BLOCK_HEIGHT, BG_COLOR);

	if ((mhz_x_offset_B - LAYOUT->FREQ_B_LEFT_MARGIN) > 0)
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_B_LEFT_MARGIN, LAYOUT->FREQ_B_Y_TOP, mhz_x_offset_B - LAYOUT->FREQ_B_LEFT_MARGIN, LAYOUT->FREQ_B_BLOCK_HEIGHT, BG_COLOR);

	// add spaces to output the frequency
	sprintf(LCD_freq_string_hz_B, "%d", hz_B);
	sprintf(LCD_freq_string_khz_B, "%d", khz_B);
	sprintf(LCD_freq_string_mhz_B, "%d", mhz_B);

	if (redraw || (LCD_last_showed_freq_mhz_B != mhz_B))
	{
		LCDDriver_printTextFont(LCD_freq_string_mhz_B, mhz_x_offset_B, LAYOUT->FREQ_B_Y_BASELINE, TRX.selected_vfo ? COLOR->FREQ_B_MHZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, LAYOUT->FREQ_B_FONT);
		LCD_last_showed_freq_mhz_B = mhz_B;
	}

	if (redraw || (LCD_last_showed_freq_khz_B != khz_B))
	{
		addSymbols(buff, LCD_freq_string_khz_B, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_B_X_OFFSET_KHZ, LAYOUT->FREQ_B_Y_BASELINE, TRX.selected_vfo ? COLOR->FREQ_B_KHZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, LAYOUT->FREQ_B_FONT);
		LCD_last_showed_freq_khz_B = khz_B;
	}
	if (redraw || (LCD_last_showed_freq_hz_B != hz_B) || TRX.ChannelMode)
	{
		addSymbols(buff, LCD_freq_string_hz_B, 3, "0", false);
		int_fast8_t band = -1;
		int_fast8_t channel = -1;
		if(TRX.ChannelMode)
		{
			band = getBandFromFreq(LCD_last_showed_freq_B, false);
			if(TRX.selected_vfo)
				channel = getChannelbyFreq(LCD_last_showed_freq_B, false);
			else
				channel = getChannelbyFreq(LCD_last_showed_freq_B, true);
		}
		if(TRX.ChannelMode && BANDS[band].channelsCount > 0)
		{
			sprintf(buff, "CH:-");
			if(band != -1 && channel != -1)
				sprintf(buff, "CH:%s", (char *)BANDS[band].channels[channel].name);
			addSymbols(buff, buff, 6, " ", true);
			LCDDriver_printText(buff, LAYOUT->FREQ_B_X_OFFSET_HZ, LAYOUT->FREQ_B_Y_BASELINE_SMALL - 18, TRX.selected_vfo ? COLOR->FREQ_B_KHZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, 2);
		}
		else
			LCDDriver_printTextFont(buff, LAYOUT->FREQ_B_X_OFFSET_HZ, LAYOUT->FREQ_B_Y_BASELINE_SMALL, TRX.selected_vfo ? COLOR->FREQ_B_HZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, LAYOUT->FREQ_B_SMALL_FONT);
		LCD_last_showed_freq_hz_B = hz_B;
	}
#endif

	if(redraw)
	{
		//Frequency delimiters
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X1_OFFSET, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET, !TRX.selected_vfo ? COLOR->FREQ_KHZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, LAYOUT->FREQ_FONT);
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X2_OFFSET, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET, !TRX.selected_vfo ? COLOR->FREQ_HZ : COLOR->FREQ_A_INACTIVE, BG_COLOR, LAYOUT->FREQ_FONT);
		#if (defined(LAY_800x480))
		LCDDriver_printTextFont(".", LAYOUT->FREQ_B_DELIMITER_X1_OFFSET, LAYOUT->FREQ_B_Y_BASELINE + LAYOUT->FREQ_B_DELIMITER_Y_OFFSET, TRX.selected_vfo ? COLOR->FREQ_B_KHZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, LAYOUT->FREQ_B_FONT);
		LCDDriver_printTextFont(".", LAYOUT->FREQ_B_DELIMITER_X2_OFFSET, LAYOUT->FREQ_B_Y_BASELINE + LAYOUT->FREQ_B_DELIMITER_Y_OFFSET, TRX.selected_vfo ? COLOR->FREQ_B_HZ : COLOR->FREQ_B_INACTIVE, BG_COLOR, LAYOUT->FREQ_B_FONT);
		#endif
	}
	
	NeedSaveSettings = true;

	LCD_UpdateQuery.FreqInfo = false;
	if (redraw)
		LCD_UpdateQuery.FreqInfoRedraw = false;

	LCD_busy = false;
}

static void LCD_drawSMeter(void)
{
	//analog version
	if (LAYOUT->STATUS_SMETER_ANALOG)
	{
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_BAR_X_OFFSET - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET, &image_data_meter, COLOR_BLACK, BG_COLOR);
		return;
	}

	// Labels on the scale
	const float32_t step = LAYOUT->STATUS_SMETER_WIDTH / 15.0f;
	LCDDriver_printText("S", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 0.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("1", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 1.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("3", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 3.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("5", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 5.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("7", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 7.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("9", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+20", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 11.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+40", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 13.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+60", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 15.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	for (uint8_t i = 0; i <= 15; i++)
	{
		uint16_t color = COLOR->STATUS_BAR_LEFT;
		if (i >= 9)
			color = COLOR->STATUS_BAR_RIGHT;
		if ((i % 2) != 0 || i == 0)
		{
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i) - 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -4, color);
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -6, color);
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i) + 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -4, color);
		}
		else
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -3, color);
	}

	// S-meter frame
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_LEFT);
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_RIGHT);
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_LEFT);
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_RIGHT);
}

static void LCD_displayStatusInfoGUI(bool redraw)
{
	// display RX / TX and s-meter
	if (LCD_systemMenuOpened || LCD_window.opened)
		return;
	if (LCD_busy)
	{
		if (redraw)
			LCD_UpdateQuery.StatusInfoGUIRedraw = true;
		else
			LCD_UpdateQuery.StatusInfoGUI = true;
		return;
	}
	LCD_busy = true;

	if (redraw)
	{
		LCDDriver_Fill_RectWH(0, LAYOUT->STATUS_Y_OFFSET, LCD_WIDTH, LAYOUT->STATUS_HEIGHT, BG_COLOR);
		if (LAYOUT->STATUS_SMETER_ANALOG)
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET, LAYOUT->STATUS_SMETER_WIDTH + 12, LAYOUT->STATUS_SMETER_ANALOG_HEIGHT, BG_COLOR);
	}

	if (TRX_on_TX())
	{
		if (TRX_Tune)
			LCDDriver_printTextFont("TU", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TU, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
		else
			LCDDriver_printTextFont("TX", LAYOUT->STATUS_TXRX_X_OFFSET + 1, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TX, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);

		// frame of the SWR meter
		const float32_t step = LAYOUT->STATUS_PMETER_WIDTH / 16.0f;
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_RIGHT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_RIGHT);

		for (uint8_t i = 0; i <= 16; i++)
		{
			uint16_t color = COLOR->STATUS_BAR_LEFT;
			if (i > 9)
				color = COLOR->STATUS_BAR_RIGHT;
			if ((i % 2) == 0)
				LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -10, color);
			else
				LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -5, color);
		}

		LCDDriver_printText("SWR:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("FWD:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_TX_LABELS_MARGIN_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("REF:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_TX_LABELS_MARGIN_X * 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		// frame of the ALC meter
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_AMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_AMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_printText("ALC:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	}
	else
	{
		LCD_UpdateQuery.StatusInfoBar = true;
		LCDDriver_printTextFont("RX", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_RX, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
	}

	//VFO indicator
	if (!TRX.selected_vfo) //VFO-A
	{
		if (!TRX.Dual_RX)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
		else if (TRX.Dual_RX_Type == VFO_A_AND_B)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A&B", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
		else
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A+B", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
	}
	else //VFO-B
	{
		if (!TRX.Dual_RX)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
		else if (TRX.Dual_RX_Type == VFO_A_AND_B)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B&A", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
		else
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B+A", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
	}

	//Mode indicator
#if (defined(LAY_800x480))
	printInfo(LAYOUT->STATUS_MODE_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT, (char *)MODE_DESCR[TRX.VFO_A.Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true);
	printInfoSmall(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT, (char *)MODE_DESCR[TRX.VFO_B.Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true);
#else
	printInfo(LAYOUT->STATUS_MODE_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT, (char *)MODE_DESCR[CurrentVFO->Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true);
#endif
	//Redraw TextBar
	if (NeedProcessDecoder)
	{
		LCDDriver_Fill_RectWH(0, LCD_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET - LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, LAYOUT->FFT_PRINT_SIZE, LAYOUT->FFT_CWDECODER_OFFSET, BG_COLOR);
		LCD_UpdateQuery.TextBar = true;
	}

	//ANT indicator
	printInfoSmall(LAYOUT->STATUS_ANT_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET), LAYOUT->STATUS_ANT_BLOCK_WIDTH, LAYOUT->STATUS_ANT_BLOCK_HEIGHT, (TRX.ANT ? "ANT2" : "ANT1"), BG_COLOR, COLOR->STATUS_RX, COLOR->STATUS_RX, true);

	//WIFI indicator
	if (WIFI_connected)
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, &IMAGES_wifi_active, COLOR_BLACK, BG_COLOR);
	else
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, &IMAGES_wifi_inactive, COLOR_BLACK, BG_COLOR);

	//SD indicator
	if (SD_Present)
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_SD_ICON_X, LAYOUT->STATUS_SD_ICON_Y, &IMAGES_sd_active, COLOR_BLACK, BG_COLOR);
	else
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_SD_ICON_X, LAYOUT->STATUS_SD_ICON_Y, &IMAGES_sd_inactive, COLOR_BLACK, BG_COLOR);

	/////BW trapezoid
	LCDDriver_Fill_RectWH(LAYOUT->BW_TRAPEZ_POS_X, LAYOUT->BW_TRAPEZ_POS_Y, LAYOUT->BW_TRAPEZ_WIDTH, LAYOUT->BW_TRAPEZ_HEIGHT, BG_COLOR); //clear back
#define bw_trapez_margin 10
	uint16_t bw_trapez_top_line_width = (uint16_t)(LAYOUT->BW_TRAPEZ_WIDTH * 0.9f - bw_trapez_margin * 2);
	//border
	LCDDriver_drawFastHLine(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_top_line_width / 2, LAYOUT->BW_TRAPEZ_POS_Y, bw_trapez_top_line_width, COLOR->BW_TRAPEZ_BORDER);																																 //top
	LCDDriver_drawFastHLine(LAYOUT->BW_TRAPEZ_POS_X, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT, LAYOUT->BW_TRAPEZ_WIDTH, COLOR->BW_TRAPEZ_BORDER);																																									 //bottom
	LCDDriver_drawLine(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_top_line_width / 2 - bw_trapez_margin, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_top_line_width / 2, LAYOUT->BW_TRAPEZ_POS_Y, COLOR->BW_TRAPEZ_BORDER); //left
	LCDDriver_drawLine(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_top_line_width / 2 + bw_trapez_margin, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_top_line_width / 2, LAYOUT->BW_TRAPEZ_POS_Y, COLOR->BW_TRAPEZ_BORDER); //right
	//bw fill
	float32_t bw_trapez_bw_left_width = 1.0f;
	float32_t bw_trapez_bw_right_width = 1.0f;
	float32_t bw_trapez_bw_hpf_margin = 0.0f;
	switch (CurrentVFO->Mode)
	{
	case TRX_MODE_LSB:
		bw_trapez_bw_hpf_margin = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_HPF_Filter;
	case TRX_MODE_DIGI_L:
		if (TRX_on_TX())
			bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_LPF_TX_Filter;
		else
			bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_LPF_RX_Filter;
		bw_trapez_bw_right_width = 0.0f;
		break;
	case TRX_MODE_USB:
		bw_trapez_bw_hpf_margin = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_HPF_Filter;
	case TRX_MODE_DIGI_U:
		bw_trapez_bw_left_width = 0.0f;
		if (TRX_on_TX())
			bw_trapez_bw_right_width = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_LPF_TX_Filter;
		else
			bw_trapez_bw_right_width = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_LPF_RX_Filter;
		break;
	case TRX_MODE_CW:
		bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_CW * TRX.CW_LPF_Filter;
		bw_trapez_bw_right_width = bw_trapez_bw_left_width;
		break;
	case TRX_MODE_NFM:
		if (TRX_on_TX())
			bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_NFM * TRX.FM_LPF_TX_Filter;
		else
			bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_NFM * TRX.FM_LPF_RX_Filter;
		bw_trapez_bw_right_width = bw_trapez_bw_left_width;
		break;
	case TRX_MODE_AM:
		if (TRX_on_TX())
			bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_AM * TRX.AM_LPF_TX_Filter;
		else
			bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_AM * TRX.AM_LPF_RX_Filter;
		bw_trapez_bw_right_width = bw_trapez_bw_left_width;
		break;
	case TRX_MODE_WFM:
		bw_trapez_bw_left_width = 1.0f;
		bw_trapez_bw_right_width = 1.0f;
		break;
	}
	uint16_t bw_trapez_left_width = (uint16_t)(bw_trapez_top_line_width / 2 * bw_trapez_bw_left_width);
	uint16_t bw_trapez_right_width = (uint16_t)(bw_trapez_top_line_width / 2 * bw_trapez_bw_right_width);
	uint16_t bw_trapez_bw_hpf_margin_width = (uint16_t)(bw_trapez_top_line_width / 2 * bw_trapez_bw_hpf_margin);
	uint16_t bw_trapez_bw_hpf_margin_width_offset = 0;
	if (bw_trapez_bw_hpf_margin_width > bw_trapez_margin)
		bw_trapez_bw_hpf_margin_width_offset = bw_trapez_bw_hpf_margin_width - bw_trapez_margin;
	if (bw_trapez_left_width > 0) //left wing
	{
		LCDDriver_Fill_RectWH(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, bw_trapez_left_width - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_HEIGHT - 2, COLOR->BW_TRAPEZ_FILL);
		LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width - bw_trapez_margin + 1, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, COLOR->BW_TRAPEZ_FILL);
		if (bw_trapez_bw_hpf_margin_width > 0)
			LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_bw_hpf_margin_width_offset, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, COLOR->BW_TRAPEZ_FILL);
	}
	if (bw_trapez_bw_right_width > 0) //right wing
	{
		LCDDriver_Fill_RectWH(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, bw_trapez_right_width - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_HEIGHT - 2, COLOR->BW_TRAPEZ_FILL);
		LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_right_width + bw_trapez_margin - 1, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_right_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_right_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, COLOR->BW_TRAPEZ_FILL);
		if (bw_trapez_bw_hpf_margin_width > 0)
			LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width_offset, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, COLOR->BW_TRAPEZ_FILL);
	}
	//shift stripe
	if (!TRX.ShiftEnabled || LCD_bw_trapez_stripe_pos == 0)
		LCD_bw_trapez_stripe_pos = LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2;
	LCDDriver_Fill_RectWH(LCD_bw_trapez_stripe_pos - 1, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT / 2, 3, LAYOUT->BW_TRAPEZ_HEIGHT / 2, COLOR->BW_TRAPEZ_STRIPE);
	/////END BW trapezoid

	LCD_UpdateQuery.StatusInfoGUI = false;
	if (redraw)
		LCD_UpdateQuery.StatusInfoGUIRedraw = false;
	LCD_busy = false;
}

static int32_t LCD_GetSMeterValPosition(float32_t dbm)
{
	int32_t width = LAYOUT->STATUS_SMETER_WIDTH - 2;
	float32_t TRX_s_meter = 0;
	if (!LAYOUT->STATUS_SMETER_ANALOG) //digital version
	{
		TRX_s_meter = (127.0f + dbm); // 127dbm - S0, 6dBm - 1S div
		if (CurrentVFO->Freq >= 144000000)
			TRX_s_meter = (147.0f + dbm); // 147dbm - S0 for frequencies above 144mhz

		if (TRX_s_meter < 54.01f) // first 9 points of meter is 6 dB each
			TRX_s_meter = (width / 15.0f) * (TRX_s_meter / 6.0f);
		else // the remaining 6 points, 10 dB each
			TRX_s_meter = ((width / 15.0f) * 9.0f) + ((TRX_s_meter - 54.0f) / 10.0f) * (width / 15.0f);

		TRX_s_meter += 1.0f;
		if (TRX_s_meter > width)
			TRX_s_meter = width;
		if (TRX_s_meter < 1.0f)
			TRX_s_meter = 1.0f;
	}
	else //analog meter version
	{
		TRX_s_meter = (127.0f + dbm); // 127dbm - S0, 6dBm - 1S div
		if (CurrentVFO->Freq >= 144000000)
			TRX_s_meter = (147.0f + dbm); // 147dbm - S0 for frequencies above 144mhz

		if (TRX_s_meter < 54.01f) // first 9 points of meter is 6 dB each
			TRX_s_meter = (width / 17.0f) * (TRX_s_meter / 6.0f);
		else // the remaining points, 10 dB each
			TRX_s_meter = ((width / 17.0f) * 9.0f) + ((TRX_s_meter - 54.0f) / 10.0f) * (width / 14.0f);

		//ugly corrections :/
		if (dbm > -5.0f) // > S9+60
			TRX_s_meter -= 2.0f;
		if (dbm > -15.0f) // > S9+50
			TRX_s_meter += 3.0f;
		if (dbm > -25.0f) // > S9+40
			TRX_s_meter += 2.0f;
		if (dbm > -35.0f) // > S9+30
			TRX_s_meter += 2.0f;
		if (dbm > -45.0f) // > S9+20
			TRX_s_meter += 2.0f;
		if (dbm > -75.0f) // > S8
			TRX_s_meter -= 2.0f;
		if (dbm < -87.0f) // < S7
			TRX_s_meter += 2.0f;
		if (dbm < -100.0f) // < S5
			TRX_s_meter += 2.0f;
		if (dbm < -104.0f) // < S4
			TRX_s_meter += 2.0f;
		/*if(dbm < -117.0f) // < S2
			TRX_s_meter -= 7.0f;*/
		if (dbm < -122.0f) // < S1
			TRX_s_meter += 2.0f;

		if (TRX_s_meter > width + 60.0f)
			TRX_s_meter = width + 60.0f;
		if (TRX_s_meter < -30.0f)
			TRX_s_meter = -30.0f;
	}
	return TRX_s_meter;
}

static void LCD_PrintMeterArrow(int16_t target_pixel_x)
{
	if (target_pixel_x < 0)
		target_pixel_x = 0;
	if (target_pixel_x > 220)
		target_pixel_x = 220;
	float32_t x0 = LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH / 2 + 2;
	float32_t y0 = LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_SMETER_ANALOG_HEIGHT + 140;
	float32_t x1 = LAYOUT->STATUS_BAR_X_OFFSET + target_pixel_x;
	float32_t y1 = LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 1;

	//length cut
	const uint32_t max_length = 220;
	float32_t x_diff = 0;
	float32_t y_diff = 0;
	float32_t length = sqrtf((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
	if (length > max_length)
	{
		float32_t coeff = (float32_t)max_length / length;
		x_diff = (x1 - x0) * coeff;
		y_diff = (y1 - y0) * coeff;
		x1 = x0 + x_diff;
		y1 = y0 + y_diff;
	}
	//right cut
	uint16_t tryes = 0;
	while ((x1 > LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH) && tryes < 100)
	{
		x_diff = (x1 - x0) * 0.99f;
		y_diff = (y1 - y0) * 0.99f;
		x1 = x0 + x_diff;
		y1 = y0 + y_diff;
		tryes++;
	}
	//left cut
	tryes = 0;
	while ((x1 < LAYOUT->STATUS_BAR_X_OFFSET) && tryes < 100)
	{
		x_diff = (x1 - x0) * 0.99f;
		y_diff = (y1 - y0) * 0.99f;
		x1 = x0 + x_diff;
		y1 = y0 + y_diff;
		tryes++;
	}
	//start cut
	tryes = 0;
	while ((y0 > LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_SMETER_ANALOG_HEIGHT) && tryes < 150)
	{
		x_diff = (x1 - x0) * 0.99f;
		y_diff = (y1 - y0) * 0.99f;
		x0 = x1 - x_diff;
		y0 = y1 - y_diff;
		tryes++;
	}

	//draw
	if (x1 < x0)
	{
		LCDDriver_drawLine(x0, y0, x1, y1, COLOR->STATUS_SMETER_STRIPE);
		LCDDriver_drawLine(x0 + 1, y0, x1 + 1, y1, COLOR->STATUS_SMETER_STRIPE);
	}
	else
	{
		LCDDriver_drawLine(x0, y0, x1, y1, COLOR->STATUS_SMETER_STRIPE);
		LCDDriver_drawLine(x0 - 1, y0, x1 - 1, y1, COLOR->STATUS_SMETER_STRIPE);
	}
}

static float32_t LCD_SWR2DBM_meter(float32_t swr)
{
	if (swr < 1.0f)
		swr = 1.0f;
	if (swr > 8.0f)
		swr = 8.0f;

	float32_t swr_to_dbm = -115.0f;
	if (swr <= 1.5f)
		swr_to_dbm += (swr - 1.0f) * 40.0f;
	if (swr > 1.5f && swr <= 2.0f)
		swr_to_dbm += 0.5f * 40.0f + (swr - 1.5f) * 20.0f;
	if (swr > 2.0f && swr <= 3.0f)
		swr_to_dbm += 0.5f * 40.0f + 0.5f * 20.0f + (swr - 2.0f) * 10.0f;
	if (swr > 3.0f)
		swr_to_dbm += 0.5f * 40.0f + 0.5f * 20.0f + 1.0f * 20.0f + (swr - 3.0f) * 10.0f;

	return swr_to_dbm;
}

static void LCD_displayStatusInfoBar(bool redraw)
{
	// S-meter and other information
	if (LCD_systemMenuOpened || LCD_window.opened)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.StatusInfoBar = true;
		return;
	}
	LCD_busy = true;
	char ctmp[50];

	if (!TRX_on_TX())
	{
		float32_t s_width = 0.0f;

		if (CurrentVFO->Mode == TRX_MODE_CW)
			s_width = LCD_last_s_meter * 0.5f + LCD_GetSMeterValPosition(TRX_RX_dBm) * 0.5f; // smooth CW faster!
		else
			s_width = LCD_last_s_meter * 0.75f + LCD_GetSMeterValPosition(TRX_RX_dBm) * 0.25f; // smooth the movement of the S-meter

		//digital s-meter version
		static uint32_t last_s_meter_draw_time = 0;
		if ((redraw || (LCD_last_s_meter != s_width) || (HAL_GetTick() - last_s_meter_draw_time) > 500) && !LAYOUT->STATUS_SMETER_ANALOG)
		{
			last_s_meter_draw_time = HAL_GetTick();
			//clear old bar
			if ((LCD_last_s_meter - s_width) > 0)
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, (uint16_t)(LCD_last_s_meter - s_width + 1), LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR);
			//and stripe
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)LCD_last_s_meter, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT, BG_COLOR);
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT, COLOR->STATUS_SMETER_STRIPE);

			// bar
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, (uint16_t)s_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);

			// peak
			static uint16_t smeter_peak_x = 0;
			static uint32_t smeter_peak_settime = 0;
			if (smeter_peak_x > s_width)
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + smeter_peak_x, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 2, LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR); //clear old peak
			if (smeter_peak_x > 0 && ((HAL_GetTick() - smeter_peak_settime) > LAYOUT->STATUS_SMETER_PEAK_HOLDTIME))
				smeter_peak_x--;
			if (s_width > smeter_peak_x)
			{
				smeter_peak_x = (uint16_t)s_width;
				smeter_peak_settime = HAL_GetTick();
			}
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + smeter_peak_x, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 2, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER_PEAK);

			// redraw s-meter gui and stripe
			LCD_drawSMeter();
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT, COLOR->STATUS_SMETER_STRIPE);

			LCD_last_s_meter = s_width;
		}

		//analog s-meter version
		if ((redraw || (LCD_last_s_meter != s_width)) && LAYOUT->STATUS_SMETER_ANALOG)
		{
			// redraw s-meter gui and line
			LCD_drawSMeter();
			LCD_PrintMeterArrow(s_width);

			LCD_last_s_meter = s_width;
		}

		//print dBm value
		sprintf(ctmp, "%ddBm", (int16_t)TRX_RX_dBm);
		addSymbols(ctmp, ctmp, 7, " ", true);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET, COLOR->STATUS_LABEL_DBM, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		//print s-meter value
		static float32_t TRX_RX_dBm_averaging = -120.0f;
		TRX_RX_dBm_averaging = 0.97f * TRX_RX_dBm_averaging + 0.03f * TRX_RX_dBm;
		if (TRX_RX_dBm > TRX_RX_dBm_averaging)
			TRX_RX_dBm_averaging = TRX_RX_dBm;

		if (CurrentVFO->Freq < 144000000)
		{
			if (TRX_RX_dBm_averaging <= -118.0f)
				sprintf(ctmp, "S1");
			else if (TRX_RX_dBm_averaging <= -112.0f)
				sprintf(ctmp, "S2");
			else if (TRX_RX_dBm_averaging <= -106.0f)
				sprintf(ctmp, "S3");
			else if (TRX_RX_dBm_averaging <= -100.0f)
				sprintf(ctmp, "S4");
			else if (TRX_RX_dBm_averaging <= -94.0f)
				sprintf(ctmp, "S5");
			else if (TRX_RX_dBm_averaging <= -88.0f)
				sprintf(ctmp, "S6");
			else if (TRX_RX_dBm_averaging <= -82.0f)
				sprintf(ctmp, "S7");
			else if (TRX_RX_dBm_averaging <= -76.0f)
				sprintf(ctmp, "S8");
			else if (TRX_RX_dBm_averaging <= -68.0f)
				sprintf(ctmp, "S9");
			else if (TRX_RX_dBm_averaging <= -58.0f)
				sprintf(ctmp, "S9+10");
			else if (TRX_RX_dBm_averaging <= -48.0f)
				sprintf(ctmp, "S9+20");
			else if (TRX_RX_dBm_averaging <= -38.0f)
				sprintf(ctmp, "S9+30");
			else if (TRX_RX_dBm_averaging <= -28.0f)
				sprintf(ctmp, "S9+40");
			else
				sprintf(ctmp, "S9+60");
		}
		else
		{
			if (TRX_RX_dBm_averaging <= -138.0f)
				sprintf(ctmp, "S1");
			else if (TRX_RX_dBm_averaging <= -132.0f)
				sprintf(ctmp, "S2");
			else if (TRX_RX_dBm_averaging <= -126.0f)
				sprintf(ctmp, "S3");
			else if (TRX_RX_dBm_averaging <= -120.0f)
				sprintf(ctmp, "S4");
			else if (TRX_RX_dBm_averaging <= -114.0f)
				sprintf(ctmp, "S5");
			else if (TRX_RX_dBm_averaging <= -108.0f)
				sprintf(ctmp, "S6");
			else if (TRX_RX_dBm_averaging <= -102.0f)
				sprintf(ctmp, "S7");
			else if (TRX_RX_dBm_averaging <= -96.0f)
				sprintf(ctmp, "S8");
			else if (TRX_RX_dBm_averaging <= -88.0f)
				sprintf(ctmp, "S9");
			else if (TRX_RX_dBm_averaging <= -78.0f)
				sprintf(ctmp, "S9+10");
			else if (TRX_RX_dBm_averaging <= -68.0f)
				sprintf(ctmp, "S9+20");
			else if (TRX_RX_dBm_averaging <= -58.0f)
				sprintf(ctmp, "S9+30");
			else if (TRX_RX_dBm_averaging <= -48.0f)
				sprintf(ctmp, "S9+40");
			else
				sprintf(ctmp, "S9+60");
		}

		addSymbols(ctmp, ctmp, 6, " ", true);
		LCDDriver_printTextFont(ctmp, LAYOUT->STATUS_LABEL_S_VAL_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_S_VAL_Y_OFFSET, COLOR->STATUS_LABEL_S_VAL, BG_COLOR, LAYOUT->STATUS_LABEL_S_VAL_FONT);
	}
	else
	{
		if (!LAYOUT->STATUS_SMETER_ANALOG)
		{
			//SWR
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_TX_LABELS_SWR_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH, LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
			sprintf(ctmp, "%.1f", (double)TRX_SWR);
			LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_SWR_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_RED, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

			//FWD
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_TX_LABELS_FWD_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH, LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
			sprintf(ctmp, "%.1fW", (double)TRX_PWR_Forward);
			LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_FWD_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_RED, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

			//REF
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_TX_LABELS_REF_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH, LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
			sprintf(ctmp, "%.1fW", (double)TRX_PWR_Backward);
			LCDDriver_printText(ctmp, LAYOUT->STATUS_TX_LABELS_REF_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_RED, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

			//SWR Meter
			float32_t fwd_power = TRX_PWR_Forward;
			if (fwd_power > CALIBRATE.MAX_RF_POWER)
				fwd_power = CALIBRATE.MAX_RF_POWER;
			uint16_t ref_width = (uint16_t)(TRX_PWR_Backward * (LAYOUT->STATUS_PMETER_WIDTH - 2) / CALIBRATE.MAX_RF_POWER);
			uint16_t fwd_width = (uint16_t)(fwd_power * (LAYOUT->STATUS_PMETER_WIDTH - 2) / CALIBRATE.MAX_RF_POWER);
			uint16_t est_width = (uint16_t)((CALIBRATE.MAX_RF_POWER - fwd_power) * (LAYOUT->STATUS_PMETER_WIDTH - 2) / CALIBRATE.MAX_RF_POWER);
			if (ref_width > fwd_width)
				ref_width = fwd_width;
			fwd_width -= ref_width;
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, fwd_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + 1 + fwd_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, ref_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_BAR_RIGHT);
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + 1 + fwd_width + ref_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, est_width, LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR);

			//ALC
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_TX_ALC_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH, LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
			uint8_t alc_level = (uint8_t)(TRX_ALC * 100.0f);
			sprintf(ctmp, "%d%%", alc_level);
			LCDDriver_printText(ctmp, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_TX_ALC_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
			uint16_t alc_level_width = LAYOUT->STATUS_AMETER_WIDTH * alc_level / 100;
			if (alc_level_width > LAYOUT->STATUS_AMETER_WIDTH)
				alc_level_width = LAYOUT->STATUS_AMETER_WIDTH;
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, alc_level_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);
			if (alc_level < 100)
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + alc_level_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_SMETER_TOP_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, LAYOUT->STATUS_AMETER_WIDTH - alc_level_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_LABEL_NOTCH);
		}
		else //analog meter version
		{
			//SWR
			sprintf(ctmp, "%.1f", (double)TRX_SWR);
			LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET - 5, COLOR->STATUS_LABEL_DBM, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

			//FWD
			sprintf(ctmp, "%.1fW", (double)TRX_PWR_Forward);
			LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET + 5, COLOR->STATUS_LABEL_DBM, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

			LCD_drawSMeter();
			LCD_PrintMeterArrow(LCD_GetSMeterValPosition(LCD_SWR2DBM_meter(TRX_SWR)));
		}
	}

	//Info labels
	char buff[32] = "";
	//BW HPF-LPF
	if (CurrentVFO->Mode == TRX_MODE_CW)
		sprintf(buff, "BW:%d", TRX.CW_LPF_Filter);
	else if ((CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U))
	{
		if (TRX_on_TX())
			sprintf(buff, "BW:%d", TRX.SSB_LPF_TX_Filter);
		else
			sprintf(buff, "BW:%d", TRX.SSB_LPF_RX_Filter);
	}
	else if ((CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB))
	{
		if (TRX_on_TX())
			sprintf(buff, "BW:%d-%d", TRX.SSB_HPF_Filter, TRX.SSB_LPF_TX_Filter);
		else
			sprintf(buff, "BW:%d-%d", TRX.SSB_HPF_Filter, TRX.SSB_LPF_RX_Filter);
	}
	else if ((CurrentVFO->Mode == TRX_MODE_AM))
	{
		if (TRX_on_TX())
			sprintf(buff, "BW:%d", TRX.AM_LPF_TX_Filter);
		else
			sprintf(buff, "BW:%d", TRX.AM_LPF_RX_Filter);
	}
	else if (CurrentVFO->Mode == TRX_MODE_NFM)
	{
		if (TRX_on_TX())
			sprintf(buff, "BW:%d", TRX.FM_LPF_TX_Filter);
		else
			sprintf(buff, "BW:%d", TRX.FM_LPF_RX_Filter);
	}
	else
		sprintf(buff, "BW:FULL");
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_BW_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_BW_Y_OFFSET, COLOR->STATUS_LABEL_BW, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	//RIT
	if (TRX.CLAR)
		sprintf(buff, "RIT:CLAR");
	else if (TRX.ShiftEnabled)
		sprintf(buff, "SHIFT:%d", TRX_SHIFT);
	else
		sprintf(buff, "RIT:OFF");
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_RIT_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_RIT_Y_OFFSET, COLOR->STATUS_LABEL_RIT, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
//THERMAL
#if (defined(LAY_800x480))
	sprintf(buff, "PA:%02d^oC MB:%02d^oC", (int16_t)TRX_RF_Temperature, (int16_t)TRX_STM32_TEMPERATURE);
#else
	sprintf(buff, "PA:%02d^o MB:%02d^o", (int16_t)TRX_RF_Temperature, (int16_t)TRX_STM32_TEMPERATURE);
#endif
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_THERM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_THERM_Y_OFFSET, COLOR->STATUS_LABEL_THERM, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	//NOTCH
	if (CurrentVFO->AutoNotchFilter)
		sprintf(buff, "NOTCH:AUTO");
	else if (CurrentVFO->ManualNotchFilter)
		sprintf(buff, "NOTCH:%uhz", CurrentVFO->NotchFC);
	else
		sprintf(buff, "NOTCH:OFF");
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_NOTCH_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_NOTCH_Y_OFFSET, COLOR->STATUS_LABEL_NOTCH, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	//FFT BW
	uint8_t fft_zoom = TRX.FFT_Zoom;
	if (CurrentVFO->Mode == TRX_MODE_CW)
		fft_zoom = TRX.FFT_ZoomCW;

	sprintf(buff, "%dkHz x%d", fft_current_spectrum_width_hz / 1000, fft_zoom);
	addSymbols(buff, buff, 10, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_FFT_BW_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_FFT_BW_Y_OFFSET, COLOR->STATUS_LABELS_BW, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

#if (defined(LAY_800x480))
	//CPU
	sprintf(buff, "CPU:%d%% FPS:%d  ", (uint32_t)CPU_LOAD.Load, FFT_FPS_Last);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_CPU_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_CPU_Y_OFFSET, COLOR->STATUS_LABEL_THERM, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	//AUTOGAIN
	LCDDriver_printText("AUTOGAIN", LAYOUT->STATUS_LABEL_AUTOGAIN_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_AUTOGAIN_Y_OFFSET, TRX.AutoGain ? COLOR->STATUS_LABEL_ACTIVE : COLOR->STATUS_LABEL_INACTIVE, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	//LOCK
	LCDDriver_printText("LOCK", LAYOUT->STATUS_LABEL_LOCK_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_LOCK_Y_OFFSET, TRX.Locked ? COLOR_RED : COLOR->STATUS_LABEL_INACTIVE, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
#endif

	//SD REC
	if (SD_RecordInProcess)
		LCDDriver_printText("REC", LAYOUT->STATUS_LABEL_REC_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_REC_Y_OFFSET, COLOR_RED, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	else
		LCDDriver_printText("   ", LAYOUT->STATUS_LABEL_REC_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_REC_Y_OFFSET, COLOR_RED, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

	//ERRORS LABELS
	LCDDriver_Fill_RectWH(LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, LAYOUT->STATUS_ERR_WIDTH, LAYOUT->STATUS_ERR_HEIGHT, BG_COLOR);
	if (TRX_ADC_OTR && !TRX_on_TX() && !TRX.ADC_SHDN)
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	else if (TRX_ADC_MAXAMPLITUDE > (ADC_FULL_SCALE * 0.499f) || TRX_ADC_MINAMPLITUDE < -(ADC_FULL_SCALE * 0.499f))
	{
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		TRX_ADC_OTR = true;
	}
	if (APROC_IFGain_Overflow)
		LCDDriver_printText("IFO", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	if (TRX_DAC_OTR)
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	if (WM8731_Buffer_underrun && !TRX_on_TX())
		LCDDriver_printText("WBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	if (FPGA_Buffer_underrun && TRX_on_TX())
		LCDDriver_printText("FBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	if (RX_USB_AUDIO_underrun)
		LCDDriver_printText("UBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	if (SD_underrun)
		LCDDriver_printText("SDF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

	Time = RTC->TR;
	Hours = ((Time >> 20) & 0x03) * 10 + ((Time >> 16) & 0x0f);
	Minutes = ((Time >> 12) & 0x07) * 10 + ((Time >> 8) & 0x0f);
	Seconds = ((Time >> 4) & 0x07) * 10 + ((Time >> 0) & 0x0f);

	if (redraw || (Hours != Last_showed_Hours))
	{
		sprintf(ctmp, "%d", Hours);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printTextFont(ctmp, LAYOUT->CLOCK_POS_HRS_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		LCDDriver_printTextFont(":", LCDDriver_GetCurrentXOffset(), LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		Last_showed_Hours = Hours;
	}
	if (redraw || (Minutes != Last_showed_Minutes))
	{
		sprintf(ctmp, "%d", Minutes);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printTextFont(ctmp, LAYOUT->CLOCK_POS_MIN_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		LCDDriver_printTextFont(":", LCDDriver_GetCurrentXOffset(), LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		Last_showed_Minutes = Minutes;
	}
	if (redraw || (Seconds != Last_showed_Seconds))
	{
		sprintf(ctmp, "%d", Seconds);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printTextFont(ctmp, LAYOUT->CLOCK_POS_SEC_X, LAYOUT->CLOCK_POS_Y, COLOR->CLOCK, BG_COLOR, LAYOUT->CLOCK_FONT);
		Last_showed_Seconds = Seconds;
	}

	LCD_UpdateQuery.StatusInfoBar = false;
	if (redraw)
		LCD_UpdateQuery.StatusInfoBarRedraw = false;
	LCD_busy = false;
}

static void LCD_displayTextBar(void)
{
	// display the text under the waterfall
	if (LCD_systemMenuOpened || LCD_window.opened)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.TextBar = true;
		return;
	}
	LCD_busy = true;

	if (TRX.CWDecoderEnabled && (CurrentVFO->Mode == TRX_MODE_CW || CurrentVFO->Mode == TRX_MODE_LOOPBACK))
	{
		char ctmp[70];
		sprintf(ctmp, "WPM:%d %s", CW_Decoder_WPM, (char *)&CW_Decoder_Text);
		LCDDriver_printText(ctmp, 2, (LCD_HEIGHT - LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET + 1), COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	}
	else if (NeedProcessDecoder && CurrentVFO->Mode == TRX_MODE_WFM)
	{
		LCDDriver_printText(RDS_Decoder_Text, 2, (LCD_HEIGHT - LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET + 1), COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	}

	LCD_UpdateQuery.TextBar = false;
	LCD_busy = false;
}

void LCD_redraw(bool do_now)
{
	TouchpadButton_handlers_count = 0;
	LCD_UpdateQuery.Background = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	LCD_UpdateQuery.TopButtonsRedraw = true;
	LCD_UpdateQuery.BottomButtonsRedraw = true;
	LCD_UpdateQuery.SystemMenuRedraw = true;
	LCD_UpdateQuery.TextBar = true;
	LCD_last_s_meter = 0;
	LCD_last_showed_freq = 0;
	Last_showed_Hours = 255;
	Last_showed_Minutes = 255;
	Last_showed_Seconds = 255;
	LCD_last_showed_freq_mhz = 9999;
	LCD_last_showed_freq_khz = 9999;
	LCD_last_showed_freq_hz = 9999;
	NeedWTFRedraw = true;
	if (do_now)
		LCD_doEvents();
}

bool LCD_doEvents(void)
{
	if (LCD_UpdateQuery.SystemMenuRedraw && LCD_showInfo_opened)
		LCD_busy = false;
	if (LCD_busy)
		return false;

	if (LCD_UpdateQuery.Background)
	{
		LCD_busy = true;
		LCDDriver_Fill(BG_COLOR);
		LCD_UpdateQuery.Background = false;
		LCD_busy = false;
	}
	if (LCD_UpdateQuery.BottomButtons)
		LCD_displayBottomButtons(false);
	if (LCD_UpdateQuery.BottomButtonsRedraw)
		LCD_displayBottomButtons(true);
	if (LCD_UpdateQuery.TopButtons)
		LCD_displayTopButtons(false);
	if (LCD_UpdateQuery.TopButtonsRedraw)
		LCD_displayTopButtons(true);
	if (LCD_UpdateQuery.FreqInfo)
		LCD_displayFreqInfo(false);
	if (LCD_UpdateQuery.FreqInfoRedraw)
		LCD_displayFreqInfo(true);
	if (LCD_UpdateQuery.StatusInfoGUIRedraw)
		LCD_displayStatusInfoGUI(true);
	if (LCD_UpdateQuery.StatusInfoGUI)
		LCD_displayStatusInfoGUI(false);
	if (LCD_UpdateQuery.StatusInfoBar)
		LCD_displayStatusInfoBar(false);
	if (LCD_UpdateQuery.StatusInfoBarRedraw)
		LCD_displayStatusInfoBar(true);
	if (LCD_UpdateQuery.SystemMenu)
		SYSMENU_drawSystemMenu(false);
	if (LCD_UpdateQuery.SystemMenuRedraw)
		SYSMENU_drawSystemMenu(true);
	if (LCD_UpdateQuery.SystemMenuCurrent)
	{
		SYSMENU_redrawCurrentItem();
		LCD_UpdateQuery.SystemMenuCurrent = false;
	}
	if (LCD_UpdateQuery.TextBar)
		LCD_displayTextBar();
	if (LCD_UpdateQuery.Tooltip)
		LCD_printTooltip();
	return true;
}

static void printInfoSmall(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t inactive_color, bool active)
{
	uint16_t x1, y1, w, h;
	LCDDriver_Fill_RectWH(x, y, width, height, back_color);
	LCDDriver_getTextBounds(text, x, y, &x1, &y1, &w, &h, (GFXfont *)&FreeSans7pt7b);
	//sendToDebug_str(text); sendToDebug_str(" "); sendToDebug_uint16(w, false);
	LCDDriver_printTextFont(text, x + (width - w) / 2, y + (height / 2) + h / 2 - 1, active ? text_color : inactive_color, back_color, (GFXfont *)&FreeSans7pt7b);
}

static void printInfo(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, uint16_t back_color, uint16_t text_color, uint16_t inactive_color, bool active)
{
	uint16_t x1, y1, w, h;
	LCDDriver_Fill_RectWH(x, y, width, height, back_color);
	LCDDriver_getTextBounds(text, x, y, &x1, &y1, &w, &h, (GFXfont *)&FreeSans9pt7b);
	//sendToDebug_str(text); sendToDebug_str(" "); sendToDebug_uint16(w, false);
	LCDDriver_printTextFont(text, x + (width - w) / 2, y + (height / 2) + h / 2 - 1, active ? text_color : inactive_color, back_color, (GFXfont *)&FreeSans9pt7b);
}

#if (defined(LAY_800x480))
static void printButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, bool active, bool show_lighter, bool in_window, uint32_t parameter, void (*clickHandler)(uint32_t parameter), void (*holdHandler)(uint32_t parameter), uint16_t active_color, uint16_t inactive_color)
{
	uint16_t x1_text, y1_text, w_text, h_text;
	if (in_window)
	{
		x += LCD_window.x;
		y += LCD_window.y;
	}
	uint16_t x_act = x + LAYOUT->BUTTON_PADDING;
	uint16_t y_act = y + LAYOUT->BUTTON_PADDING;
	uint16_t w_act = width - LAYOUT->BUTTON_PADDING * 2;
	uint16_t h_act = height - LAYOUT->BUTTON_PADDING * 2;
	//LCDDriver_Fill_RectWH(x, y, width, height, BG_COLOR); //background
	LCDDriver_Fill_RectWH(x_act, y_act, w_act, h_act, COLOR->BUTTON_BACK);										  //button body
	LCDDriver_drawRectXY(x_act, y_act, x_act + w_act, y_act + h_act, COLOR->BUTTON_BORDER);						  //border
	LCDDriver_getTextBounds(text, x_act, y_act, &x1_text, &y1_text, &w_text, &h_text, (GFXfont *)&FreeSans9pt7b); //get text bounds
	if (show_lighter)
	{
		LCDDriver_printTextFont(text, x_act + (w_act - w_text) / 2, y_act + (h_act * 2 / 5) + h_text / 2 - 1, active ? active_color : inactive_color, COLOR->BUTTON_BACK, &FreeSans9pt7b); //text
		uint16_t lighter_width = (uint16_t)((float32_t)w_act * LAYOUT->BUTTON_LIGHTER_WIDTH);
		LCDDriver_Fill_RectWH(x_act + ((w_act - lighter_width) / 2), y_act + h_act * 3 / 4, lighter_width, LAYOUT->BUTTON_LIGHTER_HEIGHT, active ? COLOR->BUTTON_LIGHTER_ACTIVE : COLOR->BUTTON_LIGHTER_INACTIVE); //lighter
	}
	else
		LCDDriver_printTextFont(text, x_act + (w_act - w_text) / 2, y_act + (h_act / 2) + h_text / 2 - 1, active ? active_color : inactive_color, COLOR->BUTTON_BACK, &FreeSans9pt7b); //text
	//add handler
	if (in_window)
	{
		LCD_window.buttons[LCD_window.buttons_count].x1 = x;
		LCD_window.buttons[LCD_window.buttons_count].y1 = y;
		LCD_window.buttons[LCD_window.buttons_count].x2 = x + width;
		LCD_window.buttons[LCD_window.buttons_count].y2 = y + height;
		LCD_window.buttons[LCD_window.buttons_count].parameter = parameter;
		LCD_window.buttons[LCD_window.buttons_count].clickHandler = clickHandler;
		LCD_window.buttons[LCD_window.buttons_count].holdHandler = holdHandler;
		LCD_window.buttons_count++;
	}
	else
	{
		bool exist = false;
		for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++)
		{
			if (TouchpadButton_handlers[i].x1 == x && TouchpadButton_handlers[i].y1 == y)
			{
				exist = true;
				break;
			}
		}
		if (!exist)
		{
			TouchpadButton_handlers[TouchpadButton_handlers_count].x1 = x;
			TouchpadButton_handlers[TouchpadButton_handlers_count].y1 = y;
			TouchpadButton_handlers[TouchpadButton_handlers_count].x2 = x + width;
			TouchpadButton_handlers[TouchpadButton_handlers_count].y2 = y + height;
			TouchpadButton_handlers[TouchpadButton_handlers_count].parameter = parameter;
			TouchpadButton_handlers[TouchpadButton_handlers_count].clickHandler = clickHandler;
			TouchpadButton_handlers[TouchpadButton_handlers_count].holdHandler = holdHandler;
			TouchpadButton_handlers_count++;
		}
	}
}
#endif

void LCD_showError(char text[], bool redraw)
{
	LCD_busy = true;
	if (!LCD_inited)
		LCD_Init();

	LCDDriver_Fill(COLOR_RED);
	uint16_t x1, y1, w, h;
	LCDDriver_getTextBounds(text, 0, 0, &x1, &y1, &w, &h, (GFXfont *)&FreeSans12pt7b);
	LCDDriver_printTextFont(text, LCD_WIDTH / 2 - w / 2, LCD_HEIGHT / 2 - h / 2, COLOR_WHITE, COLOR_RED, (GFXfont *)&FreeSans12pt7b);
	if (redraw)
		HAL_Delay(2000);
	LCD_busy = false;
	if (redraw)
		LCD_redraw(false);
}

void LCD_showInfo(char text[], bool autohide)
{
	LCD_showInfo_opened = true;
	LCD_busy = true;
	if (!LCD_inited)
		LCD_Init();
	println((char*)text);
	LCDDriver_Fill(BG_COLOR);
	uint16_t x1, y1, w, h;
	LCDDriver_getTextBounds(text, 0, 0, &x1, &y1, &w, &h, (GFXfont *)&FreeSans12pt7b);
	LCDDriver_printTextFont(text, LCD_WIDTH / 2 - w / 2, LCD_HEIGHT / 2 - h / 2, COLOR->CLOCK, BG_COLOR, (GFXfont *)&FreeSans12pt7b);
	if (autohide)
	{
		HAL_Delay(2000);
		LCD_showInfo_opened = false;
		LCD_busy = false;
		LCD_redraw(false);
	}
}

void LCD_processTouch(uint16_t x, uint16_t y)
{
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	if (TRX.Locked)
		return;
	if (LCD_systemMenuOpened)
	{
		SYSMENU_eventCloseAllSystemMenu();
		LCD_redraw(false);
		return;
	}
	//windows
	//outline touch (window close)
	if (LCD_window.opened && (y <= LCD_window.y || y >= (LCD_window.y + LCD_window.h) || x <= LCD_window.x || x >= (LCD_window.x + LCD_window.w)))
	{
		LCD_closeWindow();
		return;
	}
	//window buttons
	for (uint8_t i = 0; i < LCD_window.buttons_count; i++)
	{
		if ((LCD_window.buttons[i].x1 <= x) && (LCD_window.buttons[i].y1 <= y) && (LCD_window.buttons[i].x2 >= x) && (LCD_window.buttons[i].y2 >= y))
		{
			if (LCD_window.buttons[i].clickHandler != NULL)
				LCD_window.buttons[i].clickHandler(LCD_window.buttons[i].parameter);
			return;
		}
	}
	if (LCD_window.opened)
		return;
	//main mode click
	if (y >= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET - 20) && y <= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET + LAYOUT->STATUS_MODE_BLOCK_HEIGHT + 20) && x >= (LAYOUT->STATUS_MODE_X_OFFSET - 20) && x <= LAYOUT->STATUS_MODE_X_OFFSET + LAYOUT->STATUS_MODE_BLOCK_WIDTH + 20)
	{
		LCD_showModeWindow(false);
		return;
	}
	//sec mode click
	if (y >= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET - 20) && y <= (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET + LAYOUT->STATUS_MODE_BLOCK_HEIGHT + 20) && x >= (LAYOUT->STATUS_MODE_B_X_OFFSET - 20) && x <= (LAYOUT->STATUS_MODE_B_X_OFFSET + LAYOUT->STATUS_MODE_BLOCK_WIDTH + 20))
	{
		LCD_showModeWindow(true);
		return;
	}
	//bw click
	if (y >= LAYOUT->BW_TRAPEZ_POS_Y && y <= LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT && x >= LAYOUT->BW_TRAPEZ_POS_X && x <= LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH)
	{
		LCD_showBWWindow();
		return;
	}
	//main freq click
	if (y >= LAYOUT->FREQ_Y_TOP && y <= LAYOUT->FREQ_Y_TOP + LAYOUT->FREQ_BLOCK_HEIGHT && x >= LAYOUT->FREQ_LEFT_MARGIN && x <= LAYOUT->FREQ_LEFT_MARGIN + LAYOUT->FREQ_WIDTH)
	{
		if (!TRX.selected_vfo) //vfo-a
			LCD_showBandWindow(false);
		else
			FRONTPANEL_BUTTONHANDLER_AsB(0);
		return;
	}
	//sec freq click
	if (y >= LAYOUT->FREQ_B_Y_TOP && y <= LAYOUT->FREQ_B_Y_TOP + LAYOUT->FREQ_B_BLOCK_HEIGHT && x >= LAYOUT->FREQ_B_LEFT_MARGIN && x <= LAYOUT->FREQ_B_LEFT_MARGIN + LAYOUT->FREQ_B_WIDTH)
	{
		if (TRX.selected_vfo) //vfo-b
			LCD_showBandWindow(true);
		else
			FRONTPANEL_BUTTONHANDLER_AsB(0);
		return;
	}
	//buttons
	for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++)
	{
		if ((TouchpadButton_handlers[i].x1 <= x) && (TouchpadButton_handlers[i].y1 <= y) && (TouchpadButton_handlers[i].x2 >= x) && (TouchpadButton_handlers[i].y2 >= y))
		{
			if (TouchpadButton_handlers[i].clickHandler != NULL)
				TouchpadButton_handlers[i].clickHandler(TouchpadButton_handlers[i].parameter);
			return;
		}
	}
	//fft/wtf tap
	if (((LAYOUT->FFT_FFTWTF_POS_Y + 50) <= y) && (LAYOUT->FFT_PRINT_SIZE >= x) && ((LAYOUT->FFT_FFTWTF_POS_Y + LAYOUT->FFT_FFTWTF_BOTTOM - 50) >= y))
	{
		//frequency tap
		uint32_t newfreq = getFreqOnFFTPosition(x);
		newfreq = newfreq / 500 * 500;
		TRX_setFrequencySlowly(newfreq);
		LCD_UpdateQuery.FreqInfo = true;
		return;
	}
#endif
}

void LCD_processHoldTouch(uint16_t x, uint16_t y)
{
#if (defined(HAS_TOUCHPAD))
	if (TRX.Locked || LCD_window.opened)
		return;
	if (LCD_systemMenuOpened)
	{
		SYSMENU_eventCloseAllSystemMenu();
		LCD_redraw(false);
		return;
	}
	for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++)
	{
		if ((TouchpadButton_handlers[i].x1 <= x) && (TouchpadButton_handlers[i].y1 <= y) && (TouchpadButton_handlers[i].x2 >= x) && (TouchpadButton_handlers[i].y2 >= y))
		{
			if (TouchpadButton_handlers[i].holdHandler != NULL)
				TouchpadButton_handlers[i].holdHandler(TouchpadButton_handlers[i].parameter);
			return;
		}
	}
#endif
}

bool LCD_processSwipeTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
#pragma unused(dy)
#if (defined(HAS_TOUCHPAD))
	if (TRX.Locked)
		return false;
	if (LCD_systemMenuOpened || LCD_window.opened)
		return false;
	//fft/wtf swipe
	if (((LAYOUT->FFT_FFTWTF_POS_Y + 50) <= y) && (LAYOUT->FFT_PRINT_SIZE >= x) && ((LAYOUT->FFT_FFTWTF_POS_Y + FFT_AND_WTF_HEIGHT - 50) >= y))
	{
		const uint8_t slowler = 4;
		uint32_t newfreq = getFreqOnFFTPosition(LAYOUT->FFT_PRINT_SIZE / 2 - dx / slowler);
		if (TRX.Fast)
			newfreq = newfreq / TRX.FRQ_FAST_STEP * TRX.FRQ_FAST_STEP;
		else
			newfreq = newfreq / TRX.FRQ_STEP * TRX.FRQ_STEP;
		TRX_setFrequency(newfreq, CurrentVFO);
		LCD_UpdateQuery.FreqInfo = true;
		return true;
	}
	//bottom buttons
	if ((LAYOUT->FFT_FFTWTF_POS_Y + FFT_AND_WTF_HEIGHT - 50) < y)
	{
		if (dx < -50)
		{
			FRONTPANEL_funcbuttons_page++;
			if (FRONTPANEL_funcbuttons_page >= FUNCBUTTONS_PAGES)
				FRONTPANEL_funcbuttons_page = 0;
			LCD_UpdateQuery.BottomButtons = true;
			LCD_UpdateQuery.TopButtons = true;
			return true; //stop
		}
		if (dx > 50)
		{
			if (FRONTPANEL_funcbuttons_page == 0)
				FRONTPANEL_funcbuttons_page = FUNCBUTTONS_PAGES - 1;
			else
				FRONTPANEL_funcbuttons_page--;
			LCD_UpdateQuery.BottomButtons = true;
			LCD_UpdateQuery.TopButtons = true;
			return true; //stop
		}
	}
#endif
	return false;
}

void LCD_showTooltip(char text[])
{
	Tooltip_DiplayStartTime = HAL_GetTick();
	strcpy(Tooltip_string, text);
	Tooltip_first_draw = true;
	if (LCD_UpdateQuery.Tooltip) //redraw old tooltip
		LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.Tooltip = true;
	println((char *)text);
}

static void LCD_printTooltip(void)
{
	LCD_UpdateQuery.Tooltip = true;
	if (LCD_busy)
		return;
	if (LCD_systemMenuOpened || LCD_window.opened)
	{
		LCD_UpdateQuery.Tooltip = false;
		return;
	}
	LCD_busy = true;

	uint16_t x1, y1, w, h;
	LCDDriver_getTextBounds(Tooltip_string, LAYOUT->TOOLTIP_POS_X, LAYOUT->TOOLTIP_POS_Y, &x1, &y1, &w, &h, (GFXfont *)&FreeSans12pt7b);
	if (Tooltip_first_draw)
	{
		LCDDriver_Fill_RectWH(LAYOUT->TOOLTIP_POS_X - w / 2, LAYOUT->TOOLTIP_POS_Y, w + LAYOUT->TOOLTIP_MARGIN * 2, h + LAYOUT->TOOLTIP_MARGIN * 2, COLOR->TOOLTIP_BACK);
		LCDDriver_drawRectXY(LAYOUT->TOOLTIP_POS_X - w / 2, LAYOUT->TOOLTIP_POS_Y, LAYOUT->TOOLTIP_POS_X - w / 2 + w + LAYOUT->TOOLTIP_MARGIN * 2, LAYOUT->TOOLTIP_POS_Y + h + LAYOUT->TOOLTIP_MARGIN * 2, COLOR->TOOLTIP_BORD);
		Tooltip_first_draw = false;
	}
	LCDDriver_printTextFont(Tooltip_string, LAYOUT->TOOLTIP_POS_X - w / 2 + LAYOUT->TOOLTIP_MARGIN, LAYOUT->TOOLTIP_POS_Y + LAYOUT->TOOLTIP_MARGIN + h, COLOR->TOOLTIP_FORE, COLOR->TOOLTIP_BACK, (GFXfont *)&FreeSans12pt7b);

	LCD_busy = false;
	if ((HAL_GetTick() - Tooltip_DiplayStartTime) > LAYOUT->TOOLTIP_TIMEOUT)
	{
		LCD_UpdateQuery.Tooltip = false;
		LCD_UpdateQuery.FreqInfoRedraw = true;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBarRedraw = true;
	}
}

void LCD_openWindow(uint16_t w, uint16_t h)
{
#if (defined(HAS_TOUCHPAD))
	LCD_busy = true;
	LCD_window.opened = true;
	LCDDriver_fadeScreen(0.2f);
	uint16_t x = LCD_WIDTH / 2 - w / 2;
	uint16_t y = LCD_HEIGHT / 2 - h / 2;
	LCD_window.y = y;
	LCD_window.x = x;
	LCD_window.w = w;
	LCD_window.h = h;
	LCDDriver_drawRoundedRectWH(x, y, w, h, COLOR->WINDOWS_BORDER, 5, false);
	LCDDriver_drawRoundedRectWH(x + 1, y + 1, w - 2, h - 2, COLOR->WINDOWS_BG, 5, true);
	LCD_busy = false;

	LCD_window.buttons_count = 0;
#endif
}

void LCD_closeWindow(void)
{
#if (defined(HAS_TOUCHPAD))
	LCD_window.opened = false;
	LCD_window.buttons_count = 0;
	LCD_redraw(false);
#endif
}

static void LCD_showBandWindow(bool secondary_vfo)
{
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	const uint8_t buttons_in_line = 6;
	uint8_t selectable_bands_count = 0;
	uint8_t unselectable_bands_count = 0;
	for (uint8_t i = 0; i < BANDS_COUNT; i++)
		if (BANDS[i].selectable)
			selectable_bands_count++;
		else
			unselectable_bands_count++;
	const uint8_t buttons_lines_selectable = ceil((float32_t)selectable_bands_count / (float32_t)buttons_in_line);
	const uint8_t buttons_lines_unselectable = ceil((float32_t)unselectable_bands_count / (float32_t)buttons_in_line);
	const uint8_t divider_height = 30;
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * (buttons_lines_selectable + buttons_lines_unselectable) + divider_height + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines_selectable + buttons_lines_unselectable + 1);
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;
	int8_t curband = getBandFromFreq(TRX.VFO_A.Freq, true);
	if (secondary_vfo)
		curband = getBandFromFreq(TRX.VFO_B.Freq, true);

	//selectable bands first
	uint8_t yi = 0;
	uint8_t xi = 0;
	for (uint8_t bindx = 0; bindx < BANDS_COUNT; bindx++)
	{
		if (!BANDS[bindx].selectable)
			continue;

		if (!secondary_vfo)
			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, (char *)BANDS[bindx].name, (curband == bindx), true, true, bindx, FRONTPANEL_BUTTONHANDLER_SETBAND, FRONTPANEL_BUTTONHANDLER_SETBAND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
		else
			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, (char *)BANDS[bindx].name, (curband == bindx), true, true, bindx, FRONTPANEL_BUTTONHANDLER_SETSECBAND, FRONTPANEL_BUTTONHANDLER_SETSECBAND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);

		xi++;
		if (xi >= buttons_in_line)
		{
			yi++;
			xi = 0;
		}
	}
	//divider
	if(xi != 0)
		yi++;
	LCDDriver_drawFastHLine(LCD_WIDTH / 2 - window_width / 2, LCD_window.y + LAYOUT->WINDOWS_BUTTON_MARGIN + divider_height / 3 + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), window_width, COLOR->WINDOWS_BORDER);
	//unselectable bands next (broadcast)
	xi = 0;
	for (uint8_t bindx = 0; bindx < BANDS_COUNT; bindx++)
	{
		if (BANDS[bindx].selectable)
			continue;

		if (!secondary_vfo)
			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_MARGIN + divider_height + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, (char *)BANDS[bindx].name, (curband == bindx), true, true, bindx, FRONTPANEL_BUTTONHANDLER_SETBAND, FRONTPANEL_BUTTONHANDLER_SETBAND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
		else
			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_MARGIN + divider_height + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, (char *)BANDS[bindx].name, (curband == bindx), true, true, bindx, FRONTPANEL_BUTTONHANDLER_SETSECBAND, FRONTPANEL_BUTTONHANDLER_SETSECBAND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);

		xi++;
		if (xi >= buttons_in_line)
		{
			yi++;
			xi = 0;
		}
	}

	LCD_busy = false;
#endif
}

static void LCD_showModeWindow(bool secondary_vfo)
{
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	const uint8_t buttons_in_line = 4;
	const uint8_t buttons_lines = ceil((float32_t)TRX_MODE_COUNT / (float32_t)buttons_in_line);
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;
	int8_t curmode = TRX.VFO_A.Mode;
	if (secondary_vfo)
		curmode = TRX.VFO_B.Mode;
	for (uint8_t yi = 0; yi < buttons_lines; yi++)
	{
		for (uint8_t xi = 0; xi < buttons_in_line; xi++)
		{
			uint8_t index = yi * buttons_in_line + xi;
			if (index < TRX_MODE_COUNT)
			{
				if (!secondary_vfo)
					printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, (char *)MODE_DESCR[index], (curmode == index), true, true, index, FRONTPANEL_BUTTONHANDLER_SETMODE, FRONTPANEL_BUTTONHANDLER_SETMODE, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
				else
					printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, (char *)MODE_DESCR[index], (curmode == index), true, true, index, FRONTPANEL_BUTTONHANDLER_SETSECMODE, FRONTPANEL_BUTTONHANDLER_SETSECMODE, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
			}
		}
	}
	LCD_busy = false;
#endif
}

static void LCD_showBWWindow(void)
{
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))

	uint8_t filters_count = 0;
	uint32_t cur_width = CurrentVFO->LPF_RX_Filter_Width;
	if (TRX_on_TX())
		cur_width = CurrentVFO->LPF_TX_Filter_Width;
	if (CurrentVFO->Mode == TRX_MODE_CW)
		filters_count = CW_LPF_COUNT;
	if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U)
		filters_count = SSB_LPF_COUNT;
	if (CurrentVFO->Mode == TRX_MODE_AM)
		filters_count = AM_LPF_COUNT;
	if (CurrentVFO->Mode == TRX_MODE_NFM)
		filters_count = NFM_LPF_COUNT;
	if (filters_count == 0)
		return;

	const uint8_t buttons_in_line = 6;
	const uint8_t buttons_lines = ceil((float32_t)filters_count / (float32_t)buttons_in_line);
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;
	for (uint8_t yi = 0; yi < buttons_lines; yi++)
	{
		for (uint8_t xi = 0; xi < buttons_in_line; xi++)
		{
			uint8_t index = yi * buttons_in_line + xi;
			uint32_t width = 0;
			if (CurrentVFO->Mode == TRX_MODE_CW)
				width = AUTIO_FILTERS_LPF_CW_LIST[index];
			if (CurrentVFO->Mode == TRX_MODE_LSB || CurrentVFO->Mode == TRX_MODE_USB || CurrentVFO->Mode == TRX_MODE_DIGI_L || CurrentVFO->Mode == TRX_MODE_DIGI_U)
				width = AUTIO_FILTERS_LPF_SSB_LIST[index];
			if (CurrentVFO->Mode == TRX_MODE_AM)
				width = AUTIO_FILTERS_LPF_AM_LIST[index];
			if (CurrentVFO->Mode == TRX_MODE_NFM)
				width = AUTIO_FILTERS_LPF_NFM_LIST[index];
			char str[16];
			sprintf(str, "%d", width);
			if (index < filters_count)
			{
				if (TRX_on_TX())
					printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, str, (width == cur_width), true, true, width, FRONTPANEL_BUTTONHANDLER_SET_TX_BW, FRONTPANEL_BUTTONHANDLER_SET_TX_BW, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
				else
					printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, str, (width == cur_width), true, true, width, FRONTPANEL_BUTTONHANDLER_SET_RX_BW, FRONTPANEL_BUTTONHANDLER_SET_RX_BW, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
			}
		}
	}
	LCD_busy = false;
#endif
}

void LCD_showRFPowerWindow(void)
{
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	const uint8_t buttons_in_line = 5;
	const uint8_t buttons_lines = 2;
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	while (LCD_busy)
		;
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;
	for (uint8_t yi = 0; yi < buttons_lines; yi++)
	{
		for (uint8_t xi = 0; xi < buttons_in_line; xi++)
		{
			uint8_t index = 0 + (yi * buttons_in_line + xi) * 10;
			if (index > 50)
				index += 10;
			char str[8];
			sprintf(str, "%d%%", index);
			printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + xi * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_MARGIN + yi * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, str, (TRX.RF_Power == index), true, true, index, FRONTPANEL_BUTTONHANDLER_SETRF_POWER, FRONTPANEL_BUTTONHANDLER_SETRF_POWER, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
		}
	}
	LCD_busy = false;
#endif
}

void LCD_showManualFreqWindow(bool secondary_vfo)
{
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	manualFreqEnter = 0;
	const uint8_t buttons_in_line = 7;
	const uint8_t buttons_top_offset = 150;
	#define buttons_count 14 //1,2,3,4,5,6,7,8,9,0,mhz,khz,hz
	char* buttons[buttons_count] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "<-", "MHz", "KHz", "Hz"};
	const uint8_t buttons_lines = ceil((float32_t)buttons_count / (float32_t)buttons_in_line);
	uint16_t window_width = LAYOUT->WINDOWS_BUTTON_WIDTH * buttons_in_line + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_in_line + 1);
	uint16_t window_height = LAYOUT->WINDOWS_BUTTON_HEIGHT * buttons_lines + buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN * (buttons_lines + 1);
	LCD_openWindow(window_width, window_height);
	LCD_busy = true;

	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "1", false, true, true, 1, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "2", false, true, true, 2, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "3", false, true, true, 3, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 3 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "4", false, true, true, 4, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 4 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "5", false, true, true, 5, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 5 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "6", false, true, true, 6, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 6 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "7", false, true, true, 7, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
		
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 0 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "8", false, true, true, 8, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "9", false, true, true, 9, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 2 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "0", false, true, true, 0, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 3 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "<-", false, true, true, 11, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 4 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "MHz", false, true, true, 12, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 5 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "KHz", false, true, true, 13, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 6 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), buttons_top_offset + LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_HEIGHT + LAYOUT->WINDOWS_BUTTON_MARGIN), LAYOUT->WINDOWS_BUTTON_WIDTH, LAYOUT->WINDOWS_BUTTON_HEIGHT, "Hz", false, true, true, 14, LCD_ManualFreqButtonHandler, LCD_ManualFreqButtonHandler, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);

	LCD_ManualFreqButtonHandler(0);
	LCD_busy = false;
#endif
}

void LCD_ManualFreqButtonHandler(uint32_t parameter)
{
#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	char buff[50] = {0};
	uint32_t newfreq = 0;
	if(parameter < 10)
	{
		if(manualFreqEnter < 100000000)
			manualFreqEnter *= 10;
		else
		{
			manualFreqEnter /= 10;
			manualFreqEnter *= 10;
		}
		
		manualFreqEnter += parameter;
	}
	if(parameter == 11)
		manualFreqEnter /= 10;
	if(parameter == 12) //mhz
	{
		newfreq = manualFreqEnter * 1000000;
		LCD_closeWindow();
	}
	if(parameter == 13) //khz
	{
		newfreq = manualFreqEnter * 1000;
		LCD_closeWindow();
	}
	if(parameter == 14) //hz
	{
		newfreq = manualFreqEnter * 1;
		LCD_closeWindow();
	}
	if(newfreq > 0)
	{
		TRX_setFrequency(newfreq, CurrentVFO);
		
		int8_t band = getBandFromFreq(newfreq, true);
		if(band != -1)
		{
			TRX_setMode(TRX.BANDS_SAVED_SETTINGS[band].Mode, CurrentVFO);
			if(TRX.SAMPLERATE_MAIN != TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE)
			{
				TRX.SAMPLERATE_MAIN = TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE;
				FFT_Init();
				NeedReinitAudioFilters = true;
			}
			TRX.LNA = TRX.BANDS_SAVED_SETTINGS[band].LNA;
			TRX.ATT = TRX.BANDS_SAVED_SETTINGS[band].ATT;
			TRX.ATT_DB = TRX.BANDS_SAVED_SETTINGS[band].ATT_DB;
			TRX.ANT = TRX.BANDS_SAVED_SETTINGS[band].ANT;
			TRX.ADC_Driver = TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver;
			CurrentVFO->FM_SQL_threshold = TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold;
			TRX.ADC_PGA = TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA;
			CurrentVFO->DNR_Type = TRX.BANDS_SAVED_SETTINGS[band].DNR_Type;
			CurrentVFO->AGC = TRX.BANDS_SAVED_SETTINGS[band].AGC;
			CurrentVFO->SQL = TRX.BANDS_SAVED_SETTINGS[band].SQL;
			TRX.SQL_shadow = CurrentVFO->SQL;
			TRX.FM_SQL_threshold_shadow = CurrentVFO->FM_SQL_threshold;
		}
		TRX_Temporary_Stop_BandMap = false;
	}
		
	sprintf(buff, "%u", manualFreqEnter);
	printButton(LAYOUT->WINDOWS_BUTTON_MARGIN + 1 * (LAYOUT->WINDOWS_BUTTON_WIDTH + LAYOUT->WINDOWS_BUTTON_MARGIN), 50, LAYOUT->WINDOWS_BUTTON_WIDTH * 5 + LAYOUT->WINDOWS_BUTTON_MARGIN * 4, LAYOUT->WINDOWS_BUTTON_HEIGHT, buff, false, false, true, 0, NULL, NULL, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT);
#endif
}
