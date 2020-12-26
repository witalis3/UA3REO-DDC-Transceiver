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
#include "front_unit.h"
#include "screen_layout.h"
#include "images.h"
#include "wifi.h"
#include "fft.h"

volatile bool LCD_busy = false;
volatile DEF_LCD_UpdateQuery LCD_UpdateQuery = {false};
volatile bool LCD_systemMenuOpened = false;
uint16_t LCD_bw_trapez_stripe_pos = 0;
STRUCT_COLOR_THEME *COLOR = &COLOR_THEMES[0];
STRUCT_LAYOUT_THEME *LAYOUT = &LAYOUT_THEMES[0];

static char LCD_freq_string_hz[6] = {0};
static char LCD_freq_string_khz[6] = {0};
static char LCD_freq_string_mhz[6] = {0};
static uint32_t LCD_last_showed_freq = 0;
static uint16_t LCD_last_showed_freq_mhz = 9999;
static uint16_t LCD_last_showed_freq_khz = 9999;
static uint16_t LCD_last_showed_freq_hz = 9999;
#if (defined(LAY_800x480))
static char LCD_freq_string_hz_B[6] = {0};
static char LCD_freq_string_khz_B[6] = {0};
static char LCD_freq_string_mhz_B[6] = {0};
static uint32_t LCD_last_showed_freq_B = 0;
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
#if (defined(LAY_800x480))
static void printButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, bool active, bool show_lighter, void (*clickHandler)(void), void (*holdHandler)(void));
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
	if (LCD_systemMenuOpened)
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
	printButton(LAYOUT->TOPBUTTONS_PRE_X, LAYOUT->TOPBUTTONS_PRE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PRE", TRX.LNA, true, FRONTPANEL_BUTTONHANDLER_PRE, NULL);
	char buff[64] = {0};
	sprintf(buff, "ATT%d", (uint8_t)TRX.ATT_DB);
	if (TRX.ATT_DB == 0)
		sprintf(buff, "ATT");
	printButton(LAYOUT->TOPBUTTONS_ATT_X, LAYOUT->TOPBUTTONS_ATT_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, buff, TRX.ATT, true, FRONTPANEL_BUTTONHANDLER_ATT, FRONTPANEL_BUTTONHANDLER_ATTHOLD);
	printButton(LAYOUT->TOPBUTTONS_PGA_X, LAYOUT->TOPBUTTONS_PGA_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "PGA", TRX.ADC_PGA, true, FRONTPANEL_BUTTONHANDLER_PGA_ONLY, NULL);
	printButton(LAYOUT->TOPBUTTONS_DRV_X, LAYOUT->TOPBUTTONS_DRV_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DRV", TRX.ADC_Driver, true, FRONTPANEL_BUTTONHANDLER_DRV_ONLY, NULL);
	printButton(LAYOUT->TOPBUTTONS_FAST_X, LAYOUT->TOPBUTTONS_FAST_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "FAST", TRX.Fast, true, FRONTPANEL_BUTTONHANDLER_FAST, NULL);
	printButton(LAYOUT->TOPBUTTONS_AGC_X, LAYOUT->TOPBUTTONS_AGC_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "AGC", CurrentVFO()->AGC, true, FRONTPANEL_BUTTONHANDLER_AGC, FRONTPANEL_BUTTONHANDLER_AGC_SPEED);
	printButton(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR", CurrentVFO()->DNR, true, FRONTPANEL_BUTTONHANDLER_DNR, NULL);
	printButton(LAYOUT->TOPBUTTONS_NB_X, LAYOUT->TOPBUTTONS_NB_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NB", TRX.NOISE_BLANKER, true, FRONTPANEL_BUTTONHANDLER_NB, NULL);
	printButton(LAYOUT->TOPBUTTONS_NOTCH_X, LAYOUT->TOPBUTTONS_NOTCH_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "NOTCH", (CurrentVFO()->AutoNotchFilter || CurrentVFO()->ManualNotchFilter), true, FRONTPANEL_BUTTONHANDLER_NOTCH, FRONTPANEL_BUTTONHANDLER_NOTCH_MANUAL);
	printButton(LAYOUT->TOPBUTTONS_MUTE_X, LAYOUT->TOPBUTTONS_MUTE_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "MUTE", TRX_Mute, true, FRONTPANEL_BUTTONHANDLER_MUTE, NULL);
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
	printInfo(LAYOUT->TOPBUTTONS_AGC_X, LAYOUT->TOPBUTTONS_AGC_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "AGC", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, CurrentVFO()->AGC);
	printInfo(LAYOUT->TOPBUTTONS_DNR_X, LAYOUT->TOPBUTTONS_DNR_Y, LAYOUT->TOPBUTTONS_WIDTH, LAYOUT->TOPBUTTONS_HEIGHT, "DNR", COLOR->BUTTON_BACKGROUND, COLOR->BUTTON_TEXT, COLOR->BUTTON_INACTIVE_TEXT, CurrentVFO()->DNR);
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
	if (LCD_systemMenuOpened)
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

	printButton(LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH * 0, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "A / B", true, false, FRONTPANEL_BUTTONHANDLER_AsB, FRONTPANEL_BUTTONHANDLER_DOUBLE);
	printButton(LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH * 1, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "B=A", true, false, FRONTPANEL_BUTTONHANDLER_ArB, FRONTPANEL_BUTTONHANDLER_DOUBLEMODE);
	printButton(LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH * 2, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "TUNE", true, false, FRONTPANEL_BUTTONHANDLER_TUNE, NULL);
	printButton(LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH * 3, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "POWER", true, false, FRONTPANEL_BUTTONHANDLER_RF_POWER, NULL);
	printButton(LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH * 4, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "ANT", true, false, FRONTPANEL_BUTTONHANDLER_ANT, NULL);
	printButton(LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH * 5, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "BW", true, false, FRONTPANEL_BUTTONHANDLER_BW, FRONTPANEL_BUTTONHANDLER_HPF);
	printButton(LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH * 6, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "SERVICE", true, false, FRONTPANEL_BUTTONHANDLER_SERVICES, NULL);
	printButton(LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH * 7, LAYOUT->BOTTOM_BUTTONS_BLOCK_TOP, LAYOUT->BOTTOM_BUTTONS_ONE_WIDTH, LAYOUT->BOTTOM_BUTTONS_BLOCK_HEIGHT, "MENU", true, false, FRONTPANEL_BUTTONHANDLER_MENU, FRONTPANEL_BUTTONHANDLER_LOCK);
#endif

	LCD_UpdateQuery.BottomButtons = false;
	if (redraw)
		LCD_UpdateQuery.BottomButtonsRedraw = false;
	LCD_busy = false;
}

static void LCD_displayFreqInfo(bool redraw)
{ // display the frequency on the screen
	if (LCD_systemMenuOpened)
		return;
	if (!redraw && (LCD_last_showed_freq == CurrentVFO()->Freq)
#if (defined(LAY_800x480))
		&& (LCD_last_showed_freq_B == SecondaryVFO()->Freq)
#endif
	)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.FreqInfo = true;
		return;
	}
	LCD_busy = true;

	uint32_t display_freq = CurrentVFO()->Freq;
	if (TRX.Transverter_Enabled)
		display_freq += TRX.Transverter_Offset_Mhz * 1000 * 1000;
	if (display_freq > 999999999)
		display_freq = 999999999;
	LCD_last_showed_freq = display_freq;

	uint16_t mhz_x_offset = 0;
	if (display_freq >= 100000000)
		mhz_x_offset = LAYOUT->FREQ_X_OFFSET_100;
	else if (display_freq >= 10000000)
		mhz_x_offset = LAYOUT->FREQ_X_OFFSET_10;
	else
		mhz_x_offset = LAYOUT->FREQ_X_OFFSET_1;

	if (redraw)
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_Y_TOP, LCD_WIDTH - LAYOUT->FREQ_LEFT_MARGIN - LAYOUT->FREQ_RIGHT_MARGIN, LAYOUT->FREQ_BLOCK_HEIGHT, BG_COLOR);

	if ((mhz_x_offset - LAYOUT->FREQ_LEFT_MARGIN) > 0)
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_Y_BASELINE - LAYOUT->FREQ_HEIGHT, mhz_x_offset - LAYOUT->FREQ_LEFT_MARGIN, LAYOUT->FREQ_HEIGHT, BG_COLOR);

	// add spaces to output the frequency
	uint16_t hz = (display_freq % 1000);
	uint16_t khz = ((display_freq / 1000) % 1000);
	uint16_t mhz = ((display_freq / 1000000) % 1000000);
	sprintf(LCD_freq_string_hz, "%d", hz);
	sprintf(LCD_freq_string_khz, "%d", khz);
	sprintf(LCD_freq_string_mhz, "%d", mhz);

	if (redraw || (LCD_last_showed_freq_mhz != mhz))
	{
		LCDDriver_printTextFont(LCD_freq_string_mhz, mhz_x_offset, LAYOUT->FREQ_Y_BASELINE, COLOR->FREQ_MHZ, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_mhz = mhz;
	}

	char buff[50] = "";
	if (redraw || (LCD_last_showed_freq_khz != khz))
	{
		addSymbols(buff, LCD_freq_string_khz, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_KHZ, LAYOUT->FREQ_Y_BASELINE, COLOR->FREQ_KHZ, BG_COLOR, LAYOUT->FREQ_FONT);
		LCD_last_showed_freq_khz = khz;
	}
	if (redraw || (LCD_last_showed_freq_hz != hz))
	{
		addSymbols(buff, LCD_freq_string_hz, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_X_OFFSET_HZ, LAYOUT->FREQ_Y_BASELINE_SMALL, COLOR->FREQ_HZ, BG_COLOR, LAYOUT->FREQ_SMALL_FONT);
		LCD_last_showed_freq_hz = hz;
	}

#if (defined(LAY_800x480))
	uint16_t mhz_x_offset_B = 0;
	LCD_last_showed_freq_B = SecondaryVFO()->Freq;
	if (SecondaryVFO()->Freq >= 100000000)
		mhz_x_offset_B = LAYOUT->FREQ_B_X_OFFSET_100;
	else if (SecondaryVFO()->Freq >= 10000000)
		mhz_x_offset_B = LAYOUT->FREQ_B_X_OFFSET_10;
	else
		mhz_x_offset_B = LAYOUT->FREQ_B_X_OFFSET_1;

	if (redraw)
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_B_LEFT_MARGIN, LAYOUT->FREQ_B_Y_TOP, LCD_WIDTH - LAYOUT->FREQ_B_LEFT_MARGIN - LAYOUT->FREQ_B_RIGHT_MARGIN, LAYOUT->FREQ_B_BLOCK_HEIGHT, BG_COLOR);

	if ((mhz_x_offset_B - LAYOUT->FREQ_B_LEFT_MARGIN) > 0)
		LCDDriver_Fill_RectWH(LAYOUT->FREQ_B_LEFT_MARGIN, LAYOUT->FREQ_B_Y_BASELINE - LAYOUT->FREQ_B_HEIGHT, mhz_x_offset_B - LAYOUT->FREQ_B_LEFT_MARGIN, LAYOUT->FREQ_B_HEIGHT, BG_COLOR);

	// add spaces to output the frequency
	uint16_t hz_B = (SecondaryVFO()->Freq % 1000);
	uint16_t khz_B = ((SecondaryVFO()->Freq / 1000) % 1000);
	uint16_t mhz_B = ((SecondaryVFO()->Freq / 1000000) % 1000000);
	sprintf(LCD_freq_string_hz_B, "%d", hz_B);
	sprintf(LCD_freq_string_khz_B, "%d", khz_B);
	sprintf(LCD_freq_string_mhz_B, "%d", mhz_B);

	if (redraw || (LCD_last_showed_freq_mhz_B != mhz_B))
	{
		LCDDriver_printTextFont(LCD_freq_string_mhz_B, mhz_x_offset_B, LAYOUT->FREQ_B_Y_BASELINE, COLOR->FREQ_B_MHZ, BG_COLOR, LAYOUT->FREQ_B_FONT);
		LCD_last_showed_freq_mhz_B = mhz_B;
	}

	if (redraw || (LCD_last_showed_freq_khz_B != khz_B))
	{
		addSymbols(buff, LCD_freq_string_khz_B, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_B_X_OFFSET_KHZ, LAYOUT->FREQ_B_Y_BASELINE, COLOR->FREQ_B_KHZ, BG_COLOR, LAYOUT->FREQ_B_FONT);
		LCD_last_showed_freq_khz_B = khz_B;
	}
	if (redraw || (LCD_last_showed_freq_hz_B != hz_B))
	{
		addSymbols(buff, LCD_freq_string_hz, 3, "0", false);
		LCDDriver_printTextFont(buff, LAYOUT->FREQ_B_X_OFFSET_HZ, LAYOUT->FREQ_B_Y_BASELINE_SMALL, COLOR->FREQ_B_HZ, BG_COLOR, LAYOUT->FREQ_B_SMALL_FONT);
		LCD_last_showed_freq_hz_B = hz_B;
	}
#endif

	NeedSaveSettings = true;

	LCD_UpdateQuery.FreqInfo = false;
	if (redraw)
		LCD_UpdateQuery.FreqInfoRedraw = false;

	LCD_busy = false;
}

static void LCD_drawSMeter(void)
{
	// Labels on the scale
	const float32_t step = LAYOUT->STATUS_SMETER_WIDTH / 15.0f;
	LCDDriver_printText("S", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 0.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("1", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 1.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("3", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 3.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("5", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 5.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("7", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 7.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("9", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+20", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 11.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+40", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 13.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	LCDDriver_printText("+60", LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 15.0f) - 10, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	for (uint8_t i = 0; i <= 15; i++)
	{
		uint16_t color = COLOR->STATUS_BAR_LEFT;
		if (i >= 9)
			color = COLOR->STATUS_BAR_RIGHT;
		if ((i % 2) != 0 || i == 0)
		{
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i) - 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -4, color);
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -6, color);
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i) + 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -4, color);
		}
		else
			LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -3, color);
	}

	// S-meter frame
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_LEFT);
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_RIGHT);
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_LEFT);
	LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f) - 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_SMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_RIGHT);
}

static void LCD_displayStatusInfoGUI(bool redraw)
{
	// display RX / TX and s-meter
	if (LCD_systemMenuOpened)
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
		LCDDriver_Fill_RectWH(0, LAYOUT->STATUS_Y_OFFSET, LCD_WIDTH, LAYOUT->STATUS_HEIGHT, BG_COLOR);

	if (TRX_on_TX())
	{
		if (TRX_Tune)
			LCDDriver_printTextFont("TU", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TX, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
		else
			LCDDriver_printTextFont("TX", LAYOUT->STATUS_TXRX_X_OFFSET + 1, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_TX, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);

		// frame of the SWR meter
		const float32_t step = LAYOUT->STATUS_PMETER_WIDTH / 16.0f;
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_RIGHT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * 9.0f), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_RIGHT);

		for (uint8_t i = 0; i <= 16; i++)
		{
			uint16_t color = COLOR->STATUS_BAR_LEFT;
			if (i > 9)
				color = COLOR->STATUS_BAR_RIGHT;
			if ((i % 2) == 0)
				LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -10, color);
			else
				LCDDriver_drawFastVLine(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)(step * i), LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, -5, color);
		}

		LCDDriver_printText("SWR:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("FWD:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_TX_LABELS_MARGIN_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		LCDDriver_printText("REF:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_TX_LABELS_MARGIN_X * 2, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		// frame of the ALC meter
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_AMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_drawRectXY(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_AMETER_WIDTH, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + LAYOUT->STATUS_BAR_HEIGHT + 1, COLOR->STATUS_BAR_LEFT);
		LCDDriver_printText("ALC:", LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_LABELS_TX, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	}
	else
	{
		LCD_UpdateQuery.StatusInfoBar = true;
		LCDDriver_printTextFont("RX", LAYOUT->STATUS_TXRX_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_TXRX_Y_OFFSET), COLOR->STATUS_RX, BG_COLOR, LAYOUT->STATUS_TXRX_FONT);
		//Frequency delimiters
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X1_OFFSET, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET, COLOR->FREQ_KHZ, BG_COLOR, LAYOUT->FREQ_FONT);
		LCDDriver_printTextFont(".", LAYOUT->FREQ_DELIMITER_X2_OFFSET, LAYOUT->FREQ_Y_BASELINE + LAYOUT->FREQ_DELIMITER_Y_OFFSET, COLOR->FREQ_HZ, BG_COLOR, LAYOUT->FREQ_FONT);
#if (defined(LAY_800x480))
		LCDDriver_printTextFont(".", LAYOUT->FREQ_B_DELIMITER_X1_OFFSET, LAYOUT->FREQ_B_Y_BASELINE + LAYOUT->FREQ_B_DELIMITER_Y_OFFSET, COLOR->FREQ_B_KHZ, BG_COLOR, LAYOUT->FREQ_B_FONT);
		LCDDriver_printTextFont(".", LAYOUT->FREQ_B_DELIMITER_X2_OFFSET, LAYOUT->FREQ_B_Y_BASELINE + LAYOUT->FREQ_B_DELIMITER_Y_OFFSET, COLOR->FREQ_B_HZ, BG_COLOR, LAYOUT->FREQ_B_FONT);
#endif
	}

	//VFO indicator
	if (!TRX.current_vfo) //VFO-A
	{
		if (!TRX.Dual_RX)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
		else if (TRX.Dual_RX_Type == VFO_A_AND_B)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A&B", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
		else if (TRX.Dual_RX_Type == VFO_A_PLUS_B)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "A+B", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
	}
	else //VFO-B
	{
		if (!TRX.Dual_RX)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
		else if (TRX.Dual_RX_Type == VFO_A_AND_B)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B&A", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
		else if (TRX.Dual_RX_Type == VFO_A_PLUS_B)
			printInfo(LAYOUT->STATUS_VFO_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_VFO_Y_OFFSET), LAYOUT->STATUS_VFO_BLOCK_WIDTH, LAYOUT->STATUS_VFO_BLOCK_HEIGHT, "B+A", COLOR->STATUS_VFO_BG, COLOR->STATUS_VFO, COLOR->STATUS_VFO, true);
	}

	//Mode indicator
	printInfo(LAYOUT->STATUS_MODE_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT, (char *)MODE_DESCR[CurrentVFO()->Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true);
#if (defined(LAY_800x480))
	printInfo(LAYOUT->STATUS_MODE_B_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_MODE_B_Y_OFFSET), LAYOUT->STATUS_MODE_BLOCK_WIDTH, LAYOUT->STATUS_MODE_BLOCK_HEIGHT, (char *)MODE_DESCR[SecondaryVFO()->Mode], BG_COLOR, COLOR->STATUS_MODE, COLOR->STATUS_MODE, true);
#endif
	//Redraw CW decoder
	if (TRX.CWDecoder && (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U || CurrentVFO()->Mode == TRX_MODE_LOOPBACK))
	{
		LCDDriver_Fill_RectWH(0, LCD_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET, LAYOUT->FFT_PRINT_SIZE, LAYOUT->FFT_CWDECODER_OFFSET, BG_COLOR);
		LCD_UpdateQuery.TextBar = true;
	}

	//ANT indicator
	printInfoSmall(LAYOUT->STATUS_ANT_X_OFFSET, (LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_ANT_Y_OFFSET), LAYOUT->STATUS_ANT_BLOCK_WIDTH, LAYOUT->STATUS_ANT_BLOCK_HEIGHT, (TRX.ANT ? "ANT2" : "ANT1"), BG_COLOR, COLOR->STATUS_RX, COLOR->STATUS_RX, true);

	//WIFI indicator
	if (WIFI_connected)
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, &IMAGES_wifi_active, COLOR_BLACK, BG_COLOR);
	else
		LCDDriver_printImage_RLECompressed(LAYOUT->STATUS_WIFI_ICON_X, LAYOUT->STATUS_WIFI_ICON_Y, &IMAGES_wifi_inactive, COLOR_BLACK, BG_COLOR);

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
	switch (CurrentVFO()->Mode)
	{
	case TRX_MODE_LSB:
		bw_trapez_bw_hpf_margin = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_HPF_Filter;
	case TRX_MODE_DIGI_L:
		bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_LPF_Filter;
		bw_trapez_bw_right_width = 0.0f;
		break;
	case TRX_MODE_CW_L:
		bw_trapez_bw_hpf_margin = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.CW_HPF_Filter * 4.0f;
		bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_CW * TRX.CW_LPF_Filter;
		bw_trapez_bw_right_width = 0.0f;
		break;
	case TRX_MODE_USB:
		bw_trapez_bw_hpf_margin = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_HPF_Filter;
	case TRX_MODE_DIGI_U:
		bw_trapez_bw_left_width = 0.0f;
		bw_trapez_bw_right_width = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.SSB_LPF_Filter;
		break;
	case TRX_MODE_CW_U:
		bw_trapez_bw_left_width = 0.0f;
		bw_trapez_bw_right_width = 1.0f / (float32_t)MAX_LPF_WIDTH_CW * TRX.CW_LPF_Filter;
		bw_trapez_bw_hpf_margin = 1.0f / (float32_t)MAX_LPF_WIDTH_SSB * TRX.CW_HPF_Filter * 4.0f;
		break;
	case TRX_MODE_NFM:
		bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_NFM * TRX.FM_LPF_Filter;
		bw_trapez_bw_right_width = 1.0f / (float32_t)MAX_LPF_WIDTH_NFM * TRX.FM_LPF_Filter;
		break;
	case TRX_MODE_AM:
		bw_trapez_bw_left_width = 1.0f / (float32_t)MAX_LPF_WIDTH_AM * TRX.AM_LPF_Filter;
		bw_trapez_bw_right_width = 1.0f / (float32_t)MAX_LPF_WIDTH_AM * TRX.AM_LPF_Filter;
		break;
	case TRX_MODE_WFM:
		bw_trapez_bw_left_width = 1.0f;
		bw_trapez_bw_right_width = 1.0f;
		break;
	}
	uint16_t bw_trapez_left_width = (uint16_t)(bw_trapez_top_line_width / 2 * bw_trapez_bw_left_width);
	uint16_t bw_trapez_right_width = (uint16_t)(bw_trapez_top_line_width / 2 * bw_trapez_bw_right_width);
	uint16_t bw_trapez_bw_hpf_margin_width = (uint16_t)(bw_trapez_top_line_width / 2 * bw_trapez_bw_hpf_margin);
	if (bw_trapez_left_width > 0) //left wing
	{
		LCDDriver_Fill_RectWH(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, bw_trapez_left_width - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_HEIGHT - 2, COLOR->BW_TRAPEZ_FILL);
		LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width - bw_trapez_margin + 1, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_left_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, COLOR->BW_TRAPEZ_FILL);
		if (bw_trapez_bw_hpf_margin_width > 0)
			LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, COLOR->BW_TRAPEZ_FILL);
	}
	if (bw_trapez_bw_right_width > 0) //right wing
	{
		LCDDriver_Fill_RectWH(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, bw_trapez_right_width - bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_HEIGHT - 2, COLOR->BW_TRAPEZ_FILL);
		LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_right_width + bw_trapez_margin - 1, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_right_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_right_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, COLOR->BW_TRAPEZ_FILL);
		if (bw_trapez_bw_hpf_margin_width > 0)
			LCDDriver_Fill_Triangle(LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2 + bw_trapez_bw_hpf_margin_width, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT - 1, COLOR->BW_TRAPEZ_FILL);
	}
	//shift stripe
	if (!TRX.ShiftEnabled)
		LCD_bw_trapez_stripe_pos = LAYOUT->BW_TRAPEZ_POS_X + LAYOUT->BW_TRAPEZ_WIDTH / 2;
	LCDDriver_Fill_RectWH(LCD_bw_trapez_stripe_pos - 1, LAYOUT->BW_TRAPEZ_POS_Y + LAYOUT->BW_TRAPEZ_HEIGHT / 2, 3, LAYOUT->BW_TRAPEZ_HEIGHT / 2, COLOR->BW_TRAPEZ_STRIPE);
	/////END BW trapezoid

	LCD_UpdateQuery.StatusInfoGUI = false;
	if (redraw)
		LCD_UpdateQuery.StatusInfoGUIRedraw = false;
	LCD_busy = false;
}

static void LCD_displayStatusInfoBar(bool redraw)
{
	// S-meter and other information
	if (LCD_systemMenuOpened)
		return;
	if (LCD_busy)
	{
		LCD_UpdateQuery.StatusInfoBar = true;
		return;
	}
	LCD_busy = true;
	char ctmp[50];
	const int width = LAYOUT->STATUS_SMETER_WIDTH - 2;

	if (!TRX_on_TX())
	{
		float32_t TRX_s_meter = (127.0f + TRX_RX_dBm); // 127dbm - S0, 6dBm - 1S div
		if (CurrentVFO()->Freq >= 144000000)
			TRX_s_meter = (147.0f + TRX_RX_dBm); // 147dbm - S0 for frequencies above 144mhz

		if (TRX_s_meter < 54.01f) // first 9 points of meter is 6 dB each
			TRX_s_meter = (width / 15.0f) * (TRX_s_meter / 6.0f);
		else // the remaining 6 points, 10 dB each
			TRX_s_meter = ((width / 15.0f) * 9.0f) + ((TRX_s_meter - 54.0f) / 10.0f) * (width / 15.0f);

		TRX_s_meter += 1.0f;
		if (TRX_s_meter > width)
			TRX_s_meter = width;
		if (TRX_s_meter < 1.0f)
			TRX_s_meter = 1.0f;

		float32_t s_width = LCD_last_s_meter * 0.75f + TRX_s_meter * 0.25f; // smooth the movement of the S-meter

		if (redraw || (LCD_last_s_meter != s_width))
		{
			//clear old bar
			if ((LCD_last_s_meter - s_width) > 0)
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, (uint16_t)(LCD_last_s_meter - s_width + 1), LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR);
			//and stripe
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)LCD_last_s_meter, LAYOUT->STATUS_Y_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT, BG_COLOR);
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT, COLOR->STATUS_SMETER_STRIPE);

			// bar
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, (uint16_t)s_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);

			// peak
			static uint16_t smeter_peak_x = 0;
			static uint32_t smeter_peak_settime = 0;
			if (smeter_peak_x > s_width)
				LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + smeter_peak_x, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 2, LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR); //clear old peak
			if (smeter_peak_x > 0 && ((HAL_GetTick() - smeter_peak_settime) > LAYOUT->STATUS_SMETER_PEAK_HOLDTIME))
				smeter_peak_x--;
			if (s_width > smeter_peak_x)
			{
				smeter_peak_x = (uint16_t)s_width;
				smeter_peak_settime = HAL_GetTick();
			}
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + smeter_peak_x, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, 2, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER_PEAK);

			// redraw s-meter gui and stripe
			LCD_drawSMeter();
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + (uint16_t)s_width, LAYOUT->STATUS_Y_OFFSET + 5, 2, LAYOUT->STATUS_SMETER_MARKER_HEIGHT, COLOR->STATUS_SMETER_STRIPE);

			LCD_last_s_meter = s_width;
		}

		//print dBm value
		sprintf(ctmp, "%ddBm", TRX_RX_dBm);
		addSymbols(ctmp, ctmp, 7, " ", true);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_LABEL_DBM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_DBM_Y_OFFSET, COLOR->STATUS_LABEL_DBM, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		//print s-meter value
		static float32_t TRX_RX_dBm_averaging = -120.0f;
		TRX_RX_dBm_averaging = 0.97f * TRX_RX_dBm_averaging + 0.03f * TRX_RX_dBm;
		if (TRX_RX_dBm > TRX_RX_dBm_averaging)
			TRX_RX_dBm_averaging = TRX_RX_dBm;

		if (CurrentVFO()->Freq < 144000000)
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
		//SWR
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_SMETER_TXLABELS_PADDING, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH, LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
		sprintf(ctmp, "%.1f", (double)TRX_SWR);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_SMETER_TXLABELS_PADDING, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_RED, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		//FWD
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_SMETER_TXLABELS_MARGIN + LAYOUT->STATUS_SMETER_TXLABELS_PADDING, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH, LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
		sprintf(ctmp, "%.1fW", (double)TRX_PWR_Forward);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_SMETER_TXLABELS_MARGIN + LAYOUT->STATUS_SMETER_TXLABELS_PADDING, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_RED, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		//REF
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_SMETER_TXLABELS_MARGIN * 2 + LAYOUT->STATUS_SMETER_TXLABELS_PADDING, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH, LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
		sprintf(ctmp, "%.1fW", (double)TRX_PWR_Backward);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_TX_LABELS_OFFSET_X + LAYOUT->STATUS_SMETER_TXLABELS_MARGIN * 2 + LAYOUT->STATUS_SMETER_TXLABELS_PADDING, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR_RED, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

		//SWR Meter
		float32_t fwd_power = TRX_PWR_Forward;
		if (fwd_power > MAX_RF_POWER)
			fwd_power = MAX_RF_POWER;
		uint16_t ref_width = (uint16_t)(TRX_PWR_Backward * (LAYOUT->STATUS_PMETER_WIDTH - 2) / MAX_RF_POWER);
		uint16_t fwd_width = (uint16_t)(fwd_power * (LAYOUT->STATUS_PMETER_WIDTH - 2) / MAX_RF_POWER);
		uint16_t est_width = (uint16_t)((MAX_RF_POWER - fwd_power) * (LAYOUT->STATUS_PMETER_WIDTH - 2) / MAX_RF_POWER);
		if (ref_width > fwd_width)
			ref_width = fwd_width;
		fwd_width -= ref_width;
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + 1, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, fwd_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + 1 + fwd_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, ref_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_BAR_RIGHT);
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + 1 + fwd_width + ref_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, est_width, LAYOUT->STATUS_BAR_HEIGHT - 3, BG_COLOR);

		//ALC
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_TX_ALC_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, LAYOUT->STATUS_TX_LABELS_VAL_WIDTH, LAYOUT->STATUS_TX_LABELS_VAL_HEIGHT, BG_COLOR);
		uint8_t alc_level = (uint8_t)(TRX_ALC * 100.0f);
		sprintf(ctmp, "%d%%", alc_level);
		LCDDriver_printText(ctmp, LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_TX_ALC_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABELS_OFFSET_Y, COLOR->STATUS_BAR_LABELS, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		uint16_t alc_level_width = LAYOUT->STATUS_AMETER_WIDTH * alc_level / 100;
		if (alc_level_width > LAYOUT->STATUS_AMETER_WIDTH)
			alc_level_width = LAYOUT->STATUS_AMETER_WIDTH;
		LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, alc_level_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_SMETER);
		if (alc_level < 100)
			LCDDriver_Fill_RectWH(LAYOUT->STATUS_BAR_X_OFFSET + LAYOUT->STATUS_PMETER_WIDTH + LAYOUT->STATUS_ALC_BAR_X_OFFSET + alc_level_width, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_BAR_Y_OFFSET + 2, LAYOUT->STATUS_AMETER_WIDTH - alc_level_width, LAYOUT->STATUS_BAR_HEIGHT - 3, COLOR->STATUS_LABEL_NOTCH);
	}

	//Info labels
	char buff[32] = "";
	//BW HPF-LPF
	if ((CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U))
		sprintf(buff, "BW:%d-%d", TRX.CW_HPF_Filter, TRX.CW_LPF_Filter);
	else if ((CurrentVFO()->Mode == TRX_MODE_DIGI_L || CurrentVFO()->Mode == TRX_MODE_DIGI_U))
		sprintf(buff, "BW:%d", TRX.SSB_LPF_Filter);
	else if ((CurrentVFO()->Mode == TRX_MODE_LSB || CurrentVFO()->Mode == TRX_MODE_USB))
		sprintf(buff, "BW:%d-%d", TRX.SSB_HPF_Filter, TRX.SSB_LPF_Filter);
	else if ((CurrentVFO()->Mode == TRX_MODE_AM))
		sprintf(buff, "BW:%d", TRX.AM_LPF_Filter);
	else if ((CurrentVFO()->Mode == TRX_MODE_NFM) || (CurrentVFO()->Mode == TRX_MODE_WFM))
		sprintf(buff, "BW:%d", TRX.FM_LPF_Filter);
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
	sprintf(buff, "PA:%d^o MB:%d^o", (int16_t)TRX_RF_Temperature, (int16_t)TRX_STM32_TEMPERATURE);
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_THERM_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_THERM_Y_OFFSET, COLOR->STATUS_LABEL_THERM, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	//NOTCH
	if (CurrentVFO()->AutoNotchFilter)
		sprintf(buff, "NOTCH:AUTO");
	else if (CurrentVFO()->ManualNotchFilter)
		sprintf(buff, "NOTCH:%uhz", CurrentVFO()->NotchFC);
	else
		sprintf(buff, "NOTCH:OFF");
	addSymbols(buff, buff, 12, " ", true);
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_NOTCH_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_NOTCH_Y_OFFSET, COLOR->STATUS_LABEL_NOTCH, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	//FFT BW
	uint8_t fft_zoom = TRX.FFT_Zoom;
	if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
		fft_zoom = TRX.FFT_ZoomCW;
	if (fft_zoom == 1)
		sprintf(buff, "FFT:96kHz");
	else if (fft_zoom == 2)
		sprintf(buff, "FFT:48kHz");
	else if (fft_zoom == 4)
		sprintf(buff, "FFT:24kHz");
	else if (fft_zoom == 8)
		sprintf(buff, "FFT:12kHz");
	else if (fft_zoom == 16)
		sprintf(buff, "FFT:6kHz ");
	LCDDriver_printText(buff, LAYOUT->STATUS_LABEL_FFT_BW_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_FFT_BW_Y_OFFSET, COLOR->STATUS_LABELS_BW, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

#if (defined(LAY_800x480))
	//LOCK
	LCDDriver_printText("LOCK", LAYOUT->STATUS_LABEL_LOCK_X_OFFSET, LAYOUT->STATUS_Y_OFFSET + LAYOUT->STATUS_LABEL_LOCK_Y_OFFSET, TRX.Locked ? COLOR->STATUS_LABEL_ACTIVE : COLOR->STATUS_LABEL_INACTIVE, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
#endif

	//ERRORS LABELS
	LCDDriver_Fill_RectWH(LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, LAYOUT->STATUS_ERR_WIDTH, LAYOUT->STATUS_ERR_HEIGHT, BG_COLOR);
	if (TRX_ADC_OTR && !TRX_on_TX())
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	else if (TRX_ADC_MAXAMPLITUDE > (ADC_FULL_SCALE * 0.499f) || TRX_ADC_MINAMPLITUDE < -(ADC_FULL_SCALE * 0.499f))
	{
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
		TRX_ADC_OTR = true;
	}
	if (TRX_DAC_OTR)
		LCDDriver_printText("OVR", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	if (WM8731_Buffer_underrun && !TRX_on_TX())
		LCDDriver_printText("WBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	if (FPGA_Buffer_underrun && TRX_on_TX())
		LCDDriver_printText("FBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);
	if (RX_USB_AUDIO_underrun)
		LCDDriver_printText("UBF", LAYOUT->STATUS_ERR_OFFSET_X, LAYOUT->STATUS_ERR_OFFSET_Y, COLOR->STATUS_ERR, BG_COLOR, LAYOUT->STATUS_LABELS_FONT_SIZE);

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
		LCDDriver_printText(ctmp, 0, LCD_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET + 1, COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
		LCDDriver_printText((char *)&CW_Decoder_Text, LAYOUT->TEXTBAR_TEXT_X_OFFSET, LCD_HEIGHT - LAYOUT->FFT_CWDECODER_OFFSET + 1, COLOR->CLOCK, BG_COLOR, LAYOUT->TEXTBAR_FONT);
	}

	LCD_UpdateQuery.TextBar = false;
	LCD_busy = false;
}

void LCD_redraw(void)
{
	TouchpadButton_handlers_count = 0;
	LCD_UpdateQuery.Background = true;
	LCD_UpdateQuery.FreqInfoRedraw = true;
	LCD_UpdateQuery.StatusInfoBarRedraw = true;
	LCD_UpdateQuery.StatusInfoGUIRedraw = true;
	LCD_UpdateQuery.TopButtonsRedraw = true;
	LCD_UpdateQuery.BottomButtonsRedraw = true;
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
	//LCD_doEvents();
}

void LCD_doEvents(void)
{
	if (LCD_busy)
		return;

	if (LCD_UpdateQuery.Background)
	{
		LCD_busy = true;
		LCDDriver_Fill(BG_COLOR);
		LCD_UpdateQuery.Background = false;
		LCD_busy = false;
	}
	if (LCD_UpdateQuery.TopButtons)
		LCD_displayTopButtons(false);
	if (LCD_UpdateQuery.TopButtonsRedraw)
		LCD_displayTopButtons(true);
	if (LCD_UpdateQuery.BottomButtons)
		LCD_displayBottomButtons(false);
	if (LCD_UpdateQuery.BottomButtonsRedraw)
		LCD_displayBottomButtons(true);
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
	if (LCD_UpdateQuery.SystemMenuCurrent)
	{
		SYSMENU_redrawCurrentItem();
		LCD_UpdateQuery.SystemMenuCurrent = false;
	}
	if (LCD_UpdateQuery.TextBar)
		LCD_displayTextBar();
	if (LCD_UpdateQuery.Tooltip)
		LCD_printTooltip();
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
static void printButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *text, bool active, bool show_lighter, void (*clickHandler)(void), void (*holdHandler)(void))
{
	uint16_t x1_text, y1_text, w_text, h_text;
	uint16_t x_act = x + LAYOUT->BUTTON_PADDING;
	uint16_t y_act = y + LAYOUT->BUTTON_PADDING;
	uint16_t w_act = width - LAYOUT->BUTTON_PADDING * 2;
	uint16_t h_act = height - LAYOUT->BUTTON_PADDING * 2;
	//LCDDriver_Fill_RectWH(x, y, width, height, BG_COLOR); //backgroud
	LCDDriver_Fill_RectWH(x_act, y_act, w_act, h_act, COLOR->BUTTON_BACK);										  //button body
	LCDDriver_drawRectXY(x_act, y_act, x_act + w_act, y_act + h_act, COLOR->BUTTON_BORDER);						  //border
	LCDDriver_getTextBounds(text, x_act, y_act, &x1_text, &y1_text, &w_text, &h_text, (GFXfont *)&FreeSans9pt7b); //get text bounds
	if (show_lighter)
	{
		LCDDriver_printTextFont(text, x_act + (w_act - w_text) / 2, y_act + (h_act * 2 / 5) + h_text / 2 - 1, active ? COLOR->BUTTON_TEXT : COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BACK, &FreeSans9pt7b); //text
		uint16_t lighter_width = (uint16_t)((float32_t)w_act * LAYOUT->BUTTON_LIGHTER_WIDTH);
		LCDDriver_Fill_RectWH(x_act + ((w_act - lighter_width) / 2), y_act + h_act * 3 / 4, lighter_width, LAYOUT->BUTTON_LIGHTER_HEIGHT, active ? COLOR->BUTTON_LIGHTER_ACTIVE : COLOR->BUTTON_LIGHTER_INACTIVE); //lighter
	}
	else
		LCDDriver_printTextFont(text, x_act + (w_act - w_text) / 2, y_act + (h_act / 2) + h_text / 2 - 1, active ? COLOR->BUTTON_TEXT : COLOR->BUTTON_INACTIVE_TEXT, COLOR->BUTTON_BACK, &FreeSans9pt7b); //text
	//add handler
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
		TouchpadButton_handlers[TouchpadButton_handlers_count].clickHandler = clickHandler;
		TouchpadButton_handlers[TouchpadButton_handlers_count].holdHandler = holdHandler;
		TouchpadButton_handlers_count++;
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
		LCD_redraw();
}

void LCD_showInfo(char text[], bool autohide)
{
	LCD_busy = true;
	if (!LCD_inited)
		LCD_Init();

	LCDDriver_Fill(BG_COLOR);
	uint16_t x1, y1, w, h;
	LCDDriver_getTextBounds(text, 0, 0, &x1, &y1, &w, &h, (GFXfont *)&FreeSans12pt7b);
	LCDDriver_printTextFont(text, LCD_WIDTH / 2 - w / 2, LCD_HEIGHT / 2 - h / 2, COLOR->CLOCK, BG_COLOR, (GFXfont *)&FreeSans12pt7b);
	if (autohide)
	{
		HAL_Delay(2000);
		LCD_busy = false;
		LCD_redraw();
	}
}

void LCD_processTouch(uint16_t x, uint16_t y)
{
	if (TRX.Locked)
		return;
	if (LCD_systemMenuOpened)
	{
		SYSMENU_eventCloseAllSystemMenu();
		LCD_redraw();
		return;
	}
	//buttons
	for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++)
	{
		if ((TouchpadButton_handlers[i].x1 <= x) && (TouchpadButton_handlers[i].y1 <= y) && (TouchpadButton_handlers[i].x2 >= x) && (TouchpadButton_handlers[i].y2 >= y))
		{
			if (TouchpadButton_handlers[i].clickHandler != NULL)
				TouchpadButton_handlers[i].clickHandler();
			return;
		}
	}
	//fft/wtf tap
	if (((LAYOUT->FFT_FFTWTF_POS_Y + 50) <= y) && (LAYOUT->FFT_PRINT_SIZE >= x) && ((LAYOUT->FFT_FFTWTF_POS_Y + LAYOUT->FFT_FFTWTF_BOTTOM - 50) >= y))
	{
		//frequency tap
		uint32_t newfreq = getFreqOnFFTPosition(x);
		if (TRX.Fast)
			newfreq = newfreq / TRX.FRQ_FAST_STEP * TRX.FRQ_FAST_STEP;
		else
			newfreq = newfreq / TRX.FRQ_STEP * TRX.FRQ_STEP;
		TRX_setFrequency(newfreq, CurrentVFO());
		LCD_UpdateQuery.FreqInfo = true;
	}
}

void LCD_processHoldTouch(uint16_t x, uint16_t y)
{
	if (TRX.Locked)
		return;
	if (LCD_systemMenuOpened)
	{
		SYSMENU_eventCloseAllSystemMenu();
		LCD_redraw();
		return;
	}
	for (uint8_t i = 0; i < TouchpadButton_handlers_count; i++)
	{
		if ((TouchpadButton_handlers[i].x1 <= x) && (TouchpadButton_handlers[i].y1 <= y) && (TouchpadButton_handlers[i].x2 >= x) && (TouchpadButton_handlers[i].y2 >= y))
		{
			if (TouchpadButton_handlers[i].holdHandler != NULL)
				TouchpadButton_handlers[i].holdHandler();
			return;
		}
	}
}

bool LCD_processSwipeTouch(uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
	#pragma unused(dy)
	if (TRX.Locked)
		return false;
	if (LCD_systemMenuOpened)
		return false;
	//fft/wtf swipe
	if (((LAYOUT->FFT_FFTWTF_POS_Y + 50) <= y) && (LAYOUT->FFT_PRINT_SIZE >= x) && ((LAYOUT->FFT_FFTWTF_POS_Y + LAYOUT->FFT_FFTWTF_BOTTOM - 50) >= y))
	{
		const uint8_t slowler = 4;
		uint32_t newfreq = getFreqOnFFTPosition(LAYOUT->FFT_PRINT_SIZE / 2 - dx / slowler);
		if (TRX.Fast)
			newfreq = newfreq / TRX.FRQ_FAST_STEP * TRX.FRQ_FAST_STEP;
		else
			newfreq = newfreq / TRX.FRQ_STEP * TRX.FRQ_STEP;
		TRX_setFrequency(newfreq, CurrentVFO());
		LCD_UpdateQuery.FreqInfo = true;
		return true;
	}
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
}

static void LCD_printTooltip(void)
{
	LCD_UpdateQuery.Tooltip = true;
	if (LCD_busy)
		return;
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
	}
}
