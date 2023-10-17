#include "fonts.h"
#include "lcd_driver.h"
#include "screen_layout.h"

#if (defined(LAY_480x320))

extern "C" constexpr STRUCT_LAYOUT_THEME LAYOUT_THEMES[LAYOUT_THEMES_COUNT] = {
#ifndef FRONTPANEL_LITE
    // Default
    {
        // Top row of status buttons
        .TOPBUTTONS_X1 = 0,
        .TOPBUTTONS_X2 = (LCD_WIDTH - 1),
        .TOPBUTTONS_Y1 = 1,
        .TOPBUTTONS_Y2 = 56,
        .TOPBUTTONS_WIDTH = 72,
        .TOPBUTTONS_HEIGHT = 22,
        .TOPBUTTONS_TB_MARGIN = 2,
        .TOPBUTTONS_LR_MARGIN = 2,
        .TOPBUTTONS_PRE_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_X1 + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_PRE_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_ATT_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_PRE_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_ATT_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_PGA_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_ATT_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_PGA_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_DRV_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_PGA_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_DRV_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_AGC_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_DRV_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_AGC_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_SECOND_LINE_Y = (uint16_t)(LAYOUT_THEMES[0].TOPBUTTONS_Y1 + LAYOUT_THEMES[0].TOPBUTTONS_HEIGHT + LAYOUT_THEMES[0].TOPBUTTONS_TB_MARGIN),
        .TOPBUTTONS_DNR_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_X1 + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_DNR_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        .TOPBUTTONS_NB_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_DNR_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_NB_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        .TOPBUTTONS_FAST_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_NB_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_FAST_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        .TOPBUTTONS_MUTE_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_FAST_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_MUTE_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        .TOPBUTTONS_LOCK_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_MUTE_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_LOCK_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        // Clock
        .CLOCK_POS_Y = 17,
        .CLOCK_POS_HRS_X = (LCD_WIDTH - 75),
        .CLOCK_POS_MIN_X = (uint16_t)(LAYOUT_THEMES[0].CLOCK_POS_HRS_X + 25),
        .CLOCK_POS_SEC_X = (uint16_t)(LAYOUT_THEMES[0].CLOCK_POS_MIN_X + 25),
        .CLOCK_FONT = &FreeSans9pt7b,
        // WIFI
        .STATUS_WIFI_ICON_X = (LCD_WIDTH - 93),
        .STATUS_WIFI_ICON_Y = 3,
        // SD
        .STATUS_SD_ICON_X = (LCD_WIDTH - 93 - 16),
        .STATUS_SD_ICON_Y = 3,
        // FAN
        .STATUS_FAN_ICON_X = (LCD_WIDTH - 93 - 16),
        .STATUS_FAN_ICON_Y = 3,
        // Frequency output
        .FREQ_A_LEFT = 0,
        .FREQ_X_OFFSET_100 = 37,
        .FREQ_X_OFFSET_10 = 73,
        .FREQ_X_OFFSET_1 = 113,
        .FREQ_X_OFFSET_KHZ = 170,
        .FREQ_X_OFFSET_HZ = 306,
        .FREQ_HEIGHT = 51,
        .FREQ_WIDTH = 370,
        .FREQ_TOP_OFFSET = 3,
        .FREQ_LEFT_MARGIN = 37,
        .FREQ_RIGHT_MARGIN = (uint16_t)(LCD_WIDTH - LAYOUT_THEMES[0].FREQ_LEFT_MARGIN - LAYOUT_THEMES[0].FREQ_WIDTH),
        .FREQ_BOTTOM_OFFSET = 8,
        .FREQ_BLOCK_HEIGHT = (uint16_t)(LAYOUT_THEMES[0].FREQ_HEIGHT + LAYOUT_THEMES[0].FREQ_TOP_OFFSET + LAYOUT_THEMES[0].FREQ_BOTTOM_OFFSET),
        .FREQ_Y_TOP = LAYOUT_THEMES[0].TOPBUTTONS_Y2,
        .FREQ_Y_BASELINE = (uint16_t)(LAYOUT_THEMES[0].TOPBUTTONS_Y2 + LAYOUT_THEMES[0].FREQ_HEIGHT + LAYOUT_THEMES[0].FREQ_TOP_OFFSET),
        .FREQ_Y_BASELINE_SMALL = (uint16_t)(LAYOUT_THEMES[0].FREQ_Y_BASELINE - 2),
        .FREQ_FONT = &FreeSans36pt7b,
        .FREQ_SMALL_FONT = &Quito32pt7b,
        .FREQ_CH_FONT = &FreeSans24pt7b,
        .FREQ_CH_B_FONT = &FreeSans24pt7b,
        .FREQ_DELIMITER_Y_OFFSET = 0,
        .FREQ_DELIMITER_X1_OFFSET = 151,
        .FREQ_DELIMITER_X2_OFFSET = 285,
        // Display statuses under frequency
        .STATUS_Y_OFFSET = (uint16_t)(LAYOUT_THEMES[0].FREQ_Y_TOP + LAYOUT_THEMES[0].FREQ_BLOCK_HEIGHT + 1),
        .STATUS_HEIGHT = 44,
        .STATUS_BAR_X_OFFSET = 60,
        .STATUS_BAR_Y_OFFSET = 16,
        .STATUS_BAR_HEIGHT = 10,
        .STATUS_TXRX_X_OFFSET = 3,
        .STATUS_TXRX_Y_OFFSET = -50,
        .STATUS_TXRX_FONT = &FreeSans9pt7b,
        .STATUS_VFO_X_OFFSET = 0,
        .STATUS_VFO_Y_OFFSET = -43,
        .STATUS_VFO_BLOCK_WIDTH = 37,
        .STATUS_VFO_BLOCK_HEIGHT = 22,
        .STATUS_ANT_X_OFFSET = 0,
        .STATUS_ANT_Y_OFFSET = -23,
        .STATUS_ANT_BLOCK_WIDTH = 37,
        .STATUS_ANT_BLOCK_HEIGHT = 22,
        .STATUS_TX_LABELS_OFFSET_X = 5,
        .STATUS_TX_LABELS_MARGIN_X = 55,
        .STATUS_SMETER_ANALOG = false,
        .STATUS_SMETER_TOP_OFFSET = 0,
        .STATUS_SMETER_ANALOG_HEIGHT = 0,
        .STATUS_SMETER_WIDTH = 400,
        .STATUS_SMETER_MARKER_HEIGHT = 30,
        .STATUS_PMETER_WIDTH = 300,
        .STATUS_AMETER_WIDTH = 90,
        .STATUS_ALC_BAR_X_OFFSET = 10,
        .STATUS_LABELS_OFFSET_Y = 0,
        .STATUS_LABELS_FONT_SIZE = 1,
        .STATUS_LABEL_S_VAL_X_OFFSET = 12,
        .STATUS_LABEL_S_VAL_Y_OFFSET = 25,
        .STATUS_LABEL_S_VAL_FONT = &FreeSans7pt7b,
        .STATUS_LABEL_DBM_X_OFFSET = 5,
        .STATUS_LABEL_DBM_Y_OFFSET = 36,
        .STATUS_LABEL_BW_X_OFFSET = 60,
        .STATUS_LABEL_BW_Y_OFFSET = 36,
        .STATUS_LABEL_RIT_X_OFFSET = 160,
        .STATUS_LABEL_RIT_Y_OFFSET = 36,
        .STATUS_LABEL_THERM_X_OFFSET = 235,
        .STATUS_LABEL_THERM_Y_OFFSET = 36,
        .STATUS_LABEL_NOTCH_X_OFFSET = 335,
        .STATUS_LABEL_NOTCH_Y_OFFSET = 36,
        .STATUS_LABEL_FFT_BW_X_OFFSET = 420,
        .STATUS_LABEL_FFT_BW_Y_OFFSET = 36,
        .STATUS_LABEL_REC_X_OFFSET = 130,
        .STATUS_LABEL_REC_Y_OFFSET = 36,
        .STATUS_SMETER_PEAK_HOLDTIME = 1000,
        .STATUS_SMETER_TXLABELS_MARGIN = 55,
        .STATUS_SMETER_TXLABELS_PADDING = 23,
        .STATUS_TX_LABELS_VAL_WIDTH = 25,
        .STATUS_TX_LABELS_VAL_HEIGHT = 8,
        .STATUS_TX_LABELS_SWR_X = (uint16_t)(LAYOUT_THEMES[0].STATUS_BAR_X_OFFSET + LAYOUT_THEMES[0].STATUS_TX_LABELS_OFFSET_X + LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_PADDING),
        .STATUS_TX_LABELS_FWD_X = (uint16_t)(LAYOUT_THEMES[0].STATUS_BAR_X_OFFSET + LAYOUT_THEMES[0].STATUS_TX_LABELS_OFFSET_X + LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_MARGIN +
                                             LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_PADDING),
        .STATUS_TX_LABELS_REF_X = (uint16_t)(LAYOUT_THEMES[0].STATUS_BAR_X_OFFSET + LAYOUT_THEMES[0].STATUS_TX_LABELS_OFFSET_X + LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_MARGIN * 2 +
                                             LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_PADDING),
        .STATUS_TX_ALC_X_OFFSET = 40,
        .STATUS_MODE_X_OFFSET = (uint16_t)(LCD_WIDTH - LAYOUT_THEMES[0].FREQ_RIGHT_MARGIN + 10),
        .STATUS_MODE_Y_OFFSET = -42,
        .STATUS_MODE_BLOCK_WIDTH = 48,
        .STATUS_MODE_BLOCK_HEIGHT = 22,
        .STATUS_ERR_OFFSET_X = 435,
        .STATUS_ERR_OFFSET_Y = 65,
        .STATUS_ERR_WIDTH = 20,
        .STATUS_ERR_HEIGHT = 8,
        .TEXTBAR_FONT = 2,
        // FFT and waterfall CHECK MAX_FFT_HEIGHT AND MAX_WTF_HEIGHT defines !!!
        .FFT_HEIGHT_STYLE1 = 50,
        .WTF_HEIGHT_STYLE1 = 102,
        .FFT_HEIGHT_STYLE2 = 76,
        .WTF_HEIGHT_STYLE2 = 76,
        .FFT_HEIGHT_STYLE3 = 102,
        .WTF_HEIGHT_STYLE3 = 50,
        .FFT_HEIGHT_STYLE4 = 122,
        .WTF_HEIGHT_STYLE4 = 30,
        .FFT_HEIGHT_STYLE5 = 132,
        .WTF_HEIGHT_STYLE5 = 20,
        .FFT_PRINT_SIZE = LCD_WIDTH,
        .FFT_CWDECODER_OFFSET = 17,
        .FFT_FFTWTF_POS_Y = (uint16_t)(LCD_HEIGHT - LAYOUT_THEMES[0].FFT_HEIGHT_STYLE1 - LAYOUT_THEMES[0].WTF_HEIGHT_STYLE1),
        .FFT_FFTWTF_BOTTOM = (uint16_t)(LAYOUT_THEMES[0].FFT_FFTWTF_POS_Y + LAYOUT_THEMES[0].FFT_HEIGHT_STYLE1 + LAYOUT_THEMES[0].WTF_HEIGHT_STYLE1),
        .FFT_FREQLABELS_HEIGHT = 0,
        // System menu
        .SYSMENU_FONT_SIZE = 2,
        .SYSMENU_X1 = 5,
        .SYSMENU_X2 = 400,
        .SYSMENU_W = 458,
        .SYSMENU_ITEM_HEIGHT = 18,
        .SYSMENU_MAX_ITEMS_ON_PAGE = (uint16_t)(LCD_HEIGHT / LAYOUT_THEMES[0].SYSMENU_ITEM_HEIGHT),
        // Stuff
        .GREETINGS_X = (LCD_WIDTH / 2 - 5),
        .GREETINGS_Y = 25,
        // Tooltip
        .TOOLTIP_FONT = (const GFXfont *)&FreeSans12pt7b,
        .TOOLTIP_TIMEOUT = 1500,
        .TOOLTIP_MARGIN = 5,
        .TOOLTIP_POS_X = (LCD_WIDTH / 2),
        .TOOLTIP_POS_Y = 70,
        // BW Trapezoid
        .BW_TRAPEZ_POS_X = 380,
        .BW_TRAPEZ_POS_Y = 25 + 10,
        .BW_TRAPEZ_HEIGHT = 25 - 10,
        .BW_TRAPEZ_WIDTH = 96,
    },
    // 7-segment digitals
    {
        .TOPBUTTONS_X1 = LAYOUT_THEMES[0].TOPBUTTONS_X1,
        .TOPBUTTONS_X2 = LAYOUT_THEMES[0].TOPBUTTONS_X2,
        .TOPBUTTONS_Y1 = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_WIDTH = LAYOUT_THEMES[0].TOPBUTTONS_WIDTH,
        .TOPBUTTONS_HEIGHT = LAYOUT_THEMES[0].TOPBUTTONS_HEIGHT,
        .TOPBUTTONS_PRE_X = LAYOUT_THEMES[0].TOPBUTTONS_PRE_X,
        .TOPBUTTONS_PRE_Y = LAYOUT_THEMES[0].TOPBUTTONS_PRE_Y,
        .TOPBUTTONS_ATT_X = LAYOUT_THEMES[0].TOPBUTTONS_ATT_X,
        .TOPBUTTONS_ATT_Y = LAYOUT_THEMES[0].TOPBUTTONS_ATT_Y,
        .TOPBUTTONS_PGA_X = LAYOUT_THEMES[0].TOPBUTTONS_PGA_X,
        .TOPBUTTONS_PGA_Y = LAYOUT_THEMES[0].TOPBUTTONS_PGA_Y,
        .TOPBUTTONS_DRV_X = LAYOUT_THEMES[0].TOPBUTTONS_DRV_X,
        .TOPBUTTONS_DRV_Y = LAYOUT_THEMES[0].TOPBUTTONS_DRV_Y,
        .TOPBUTTONS_AGC_X = LAYOUT_THEMES[0].TOPBUTTONS_AGC_X,
        .TOPBUTTONS_AGC_Y = LAYOUT_THEMES[0].TOPBUTTONS_AGC_Y,
        .TOPBUTTONS_DNR_X = LAYOUT_THEMES[0].TOPBUTTONS_DNR_X,
        .TOPBUTTONS_DNR_Y = LAYOUT_THEMES[0].TOPBUTTONS_DNR_Y,
        .TOPBUTTONS_NB_X = LAYOUT_THEMES[0].TOPBUTTONS_NB_X,
        .TOPBUTTONS_NB_Y = LAYOUT_THEMES[0].TOPBUTTONS_NB_Y,
        .TOPBUTTONS_FAST_X = LAYOUT_THEMES[0].TOPBUTTONS_FAST_X,
        .TOPBUTTONS_FAST_Y = LAYOUT_THEMES[0].TOPBUTTONS_FAST_Y,
        .TOPBUTTONS_MUTE_X = LAYOUT_THEMES[0].TOPBUTTONS_MUTE_X,
        .TOPBUTTONS_MUTE_Y = LAYOUT_THEMES[0].TOPBUTTONS_MUTE_Y,
        .TOPBUTTONS_LOCK_X = LAYOUT_THEMES[0].TOPBUTTONS_LOCK_X,
        .TOPBUTTONS_LOCK_Y = LAYOUT_THEMES[0].TOPBUTTONS_LOCK_Y,
        // clock
        .CLOCK_POS_Y = LAYOUT_THEMES[0].CLOCK_POS_Y,
        .CLOCK_POS_HRS_X = LAYOUT_THEMES[0].CLOCK_POS_HRS_X,
        .CLOCK_POS_MIN_X = LAYOUT_THEMES[0].CLOCK_POS_MIN_X,
        .CLOCK_POS_SEC_X = LAYOUT_THEMES[0].CLOCK_POS_SEC_X,
        .CLOCK_FONT = LAYOUT_THEMES[0].CLOCK_FONT,
        // WIFI
        .STATUS_WIFI_ICON_X = LAYOUT_THEMES[0].STATUS_WIFI_ICON_X,
        .STATUS_WIFI_ICON_Y = LAYOUT_THEMES[0].STATUS_WIFI_ICON_Y,
        // SD
        .STATUS_SD_ICON_X = LAYOUT_THEMES[0].STATUS_SD_ICON_X,
        .STATUS_SD_ICON_Y = LAYOUT_THEMES[0].STATUS_SD_ICON_Y,
        // FAN
        .STATUS_FAN_ICON_X = LAYOUT_THEMES[0].STATUS_FAN_ICON_X,
        .STATUS_FAN_ICON_Y = LAYOUT_THEMES[0].STATUS_FAN_ICON_Y,
        // frequency output VFO-A
        .FREQ_A_LEFT = LAYOUT_THEMES[0].FREQ_A_LEFT,
        .FREQ_X_OFFSET_100 = LAYOUT_THEMES[0].FREQ_X_OFFSET_100,
        .FREQ_X_OFFSET_10 = LAYOUT_THEMES[0].FREQ_X_OFFSET_10,
        .FREQ_X_OFFSET_1 = (uint16_t)(LAYOUT_THEMES[0].FREQ_X_OFFSET_1 - 4),
        .FREQ_X_OFFSET_KHZ = LAYOUT_THEMES[0].FREQ_X_OFFSET_KHZ,
        .FREQ_X_OFFSET_HZ = LAYOUT_THEMES[0].FREQ_X_OFFSET_HZ,
        .FREQ_HEIGHT = LAYOUT_THEMES[0].FREQ_HEIGHT,
        .FREQ_WIDTH = LAYOUT_THEMES[0].FREQ_WIDTH,
        .FREQ_TOP_OFFSET = LAYOUT_THEMES[0].FREQ_TOP_OFFSET,
        .FREQ_LEFT_MARGIN = LAYOUT_THEMES[0].FREQ_LEFT_MARGIN,
        .FREQ_RIGHT_MARGIN = LAYOUT_THEMES[0].FREQ_RIGHT_MARGIN,
        .FREQ_BOTTOM_OFFSET = LAYOUT_THEMES[0].FREQ_BOTTOM_OFFSET,
        .FREQ_BLOCK_HEIGHT = LAYOUT_THEMES[0].FREQ_BLOCK_HEIGHT,
        .FREQ_Y_TOP = LAYOUT_THEMES[0].FREQ_Y_TOP,
        .FREQ_Y_BASELINE = LAYOUT_THEMES[0].FREQ_Y_BASELINE,
        .FREQ_Y_BASELINE_SMALL = LAYOUT_THEMES[0].FREQ_Y_BASELINE_SMALL,
        .FREQ_FONT = &DS_DIGIT36pt7b,
        .FREQ_SMALL_FONT = &DS_DIGIT32pt7b,
        .FREQ_CH_FONT = &FreeSans24pt7b,
        .FREQ_CH_B_FONT = &FreeSans24pt7b,
        .FREQ_DELIMITER_Y_OFFSET = LAYOUT_THEMES[0].FREQ_DELIMITER_Y_OFFSET,
        .FREQ_DELIMITER_X1_OFFSET = LAYOUT_THEMES[0].FREQ_DELIMITER_X1_OFFSET,
        .FREQ_DELIMITER_X2_OFFSET = LAYOUT_THEMES[0].FREQ_DELIMITER_X2_OFFSET,
        // display statuses under frequency
        .STATUS_Y_OFFSET = LAYOUT_THEMES[0].STATUS_Y_OFFSET,
        .STATUS_HEIGHT = LAYOUT_THEMES[0].STATUS_HEIGHT,
        .STATUS_BAR_X_OFFSET = LAYOUT_THEMES[0].STATUS_BAR_X_OFFSET,
        .STATUS_BAR_Y_OFFSET = LAYOUT_THEMES[0].STATUS_BAR_Y_OFFSET,
        .STATUS_BAR_HEIGHT = LAYOUT_THEMES[0].STATUS_BAR_HEIGHT,
        .STATUS_TXRX_X_OFFSET = LAYOUT_THEMES[0].STATUS_TXRX_X_OFFSET,
        .STATUS_TXRX_Y_OFFSET = LAYOUT_THEMES[0].STATUS_TXRX_Y_OFFSET,
        .STATUS_TXRX_FONT = LAYOUT_THEMES[0].STATUS_TXRX_FONT,
        .STATUS_VFO_X_OFFSET = LAYOUT_THEMES[0].STATUS_VFO_X_OFFSET,
        .STATUS_VFO_Y_OFFSET = LAYOUT_THEMES[0].STATUS_VFO_Y_OFFSET,
        .STATUS_VFO_BLOCK_WIDTH = LAYOUT_THEMES[0].STATUS_VFO_BLOCK_WIDTH,
        .STATUS_VFO_BLOCK_HEIGHT = LAYOUT_THEMES[0].STATUS_VFO_BLOCK_HEIGHT,
        .STATUS_ANT_X_OFFSET = LAYOUT_THEMES[0].STATUS_ANT_X_OFFSET,
        .STATUS_ANT_Y_OFFSET = LAYOUT_THEMES[0].STATUS_ANT_Y_OFFSET,
        .STATUS_ANT_BLOCK_WIDTH = LAYOUT_THEMES[0].STATUS_ANT_BLOCK_WIDTH,
        .STATUS_ANT_BLOCK_HEIGHT = LAYOUT_THEMES[0].STATUS_ANT_BLOCK_HEIGHT,
        .STATUS_TX_LABELS_OFFSET_X = LAYOUT_THEMES[0].STATUS_TX_LABELS_OFFSET_X,
        .STATUS_TX_LABELS_MARGIN_X = LAYOUT_THEMES[0].STATUS_TX_LABELS_MARGIN_X,
        .STATUS_SMETER_ANALOG = LAYOUT_THEMES[0].STATUS_SMETER_ANALOG,
        .STATUS_SMETER_TOP_OFFSET = LAYOUT_THEMES[0].STATUS_SMETER_TOP_OFFSET,
        .STATUS_SMETER_ANALOG_HEIGHT = LAYOUT_THEMES[0].STATUS_SMETER_ANALOG_HEIGHT,
        .STATUS_SMETER_WIDTH = LAYOUT_THEMES[0].STATUS_SMETER_WIDTH,
        .STATUS_SMETER_MARKER_HEIGHT = LAYOUT_THEMES[0].STATUS_SMETER_MARKER_HEIGHT,
        .STATUS_PMETER_WIDTH = LAYOUT_THEMES[0].STATUS_PMETER_WIDTH,
        .STATUS_AMETER_WIDTH = LAYOUT_THEMES[0].STATUS_AMETER_WIDTH,
        .STATUS_ALC_BAR_X_OFFSET = LAYOUT_THEMES[0].STATUS_ALC_BAR_X_OFFSET,
        .STATUS_LABELS_OFFSET_Y = LAYOUT_THEMES[0].STATUS_LABELS_OFFSET_Y,
        .STATUS_LABELS_FONT_SIZE = LAYOUT_THEMES[0].STATUS_LABELS_FONT_SIZE,
        .STATUS_LABEL_S_VAL_X_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_S_VAL_X_OFFSET,
        .STATUS_LABEL_S_VAL_Y_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_S_VAL_Y_OFFSET,
        .STATUS_LABEL_S_VAL_FONT = LAYOUT_THEMES[0].STATUS_LABEL_S_VAL_FONT,
        .STATUS_LABEL_DBM_X_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_DBM_X_OFFSET,
        .STATUS_LABEL_DBM_Y_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_DBM_Y_OFFSET,
        .STATUS_LABEL_BW_X_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_BW_X_OFFSET,
        .STATUS_LABEL_BW_Y_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_BW_Y_OFFSET,
        .STATUS_LABEL_RIT_X_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_RIT_X_OFFSET,
        .STATUS_LABEL_RIT_Y_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_RIT_Y_OFFSET,
        .STATUS_LABEL_THERM_X_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_THERM_X_OFFSET,
        .STATUS_LABEL_THERM_Y_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_THERM_Y_OFFSET,
        .STATUS_LABEL_NOTCH_X_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_NOTCH_X_OFFSET,
        .STATUS_LABEL_NOTCH_Y_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_NOTCH_Y_OFFSET,
        .STATUS_LABEL_FFT_BW_X_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_FFT_BW_X_OFFSET,
        .STATUS_LABEL_FFT_BW_Y_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_FFT_BW_Y_OFFSET,
        .STATUS_LABEL_REC_X_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_REC_X_OFFSET,
        .STATUS_LABEL_REC_Y_OFFSET = LAYOUT_THEMES[0].STATUS_LABEL_REC_Y_OFFSET,
        .STATUS_SMETER_PEAK_HOLDTIME = LAYOUT_THEMES[0].STATUS_SMETER_PEAK_HOLDTIME,
        .STATUS_SMETER_TXLABELS_MARGIN = LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_MARGIN,
        .STATUS_SMETER_TXLABELS_PADDING = LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_PADDING,
        .STATUS_TX_LABELS_VAL_WIDTH = LAYOUT_THEMES[0].STATUS_TX_LABELS_VAL_WIDTH,
        .STATUS_TX_LABELS_VAL_HEIGHT = LAYOUT_THEMES[0].STATUS_TX_LABELS_VAL_HEIGHT,
        .STATUS_TX_LABELS_SWR_X = LAYOUT_THEMES[0].STATUS_TX_LABELS_SWR_X,
        .STATUS_TX_LABELS_FWD_X = LAYOUT_THEMES[0].STATUS_TX_LABELS_FWD_X,
        .STATUS_TX_LABELS_REF_X = LAYOUT_THEMES[0].STATUS_TX_LABELS_REF_X,
        .STATUS_TX_ALC_X_OFFSET = LAYOUT_THEMES[0].STATUS_TX_ALC_X_OFFSET,
        .STATUS_MODE_X_OFFSET = LAYOUT_THEMES[0].STATUS_MODE_X_OFFSET,
        .STATUS_MODE_Y_OFFSET = LAYOUT_THEMES[0].STATUS_MODE_Y_OFFSET,
        .STATUS_MODE_BLOCK_WIDTH = LAYOUT_THEMES[0].STATUS_MODE_BLOCK_WIDTH,
        .STATUS_MODE_BLOCK_HEIGHT = LAYOUT_THEMES[0].STATUS_MODE_BLOCK_HEIGHT,
        .STATUS_ERR_OFFSET_X = LAYOUT_THEMES[0].STATUS_ERR_OFFSET_X,
        .STATUS_ERR_OFFSET_Y = LAYOUT_THEMES[0].STATUS_ERR_OFFSET_Y,
        .STATUS_ERR_WIDTH = LAYOUT_THEMES[0].STATUS_ERR_WIDTH,
        .STATUS_ERR_HEIGHT = LAYOUT_THEMES[0].STATUS_ERR_HEIGHT,
        // text bar under wtf
        .TEXTBAR_FONT = LAYOUT_THEMES[0].TEXTBAR_FONT,
        // bottom buttons
        .BOTTOM_BUTTONS_BLOCK_HEIGHT = LAYOUT_THEMES[0].BOTTOM_BUTTONS_BLOCK_HEIGHT,
        .BOTTOM_BUTTONS_BLOCK_TOP = LAYOUT_THEMES[0].BOTTOM_BUTTONS_BLOCK_TOP,
        .BOTTOM_BUTTONS_COUNT = LAYOUT_THEMES[0].BOTTOM_BUTTONS_COUNT,
        .BOTTOM_BUTTONS_ONE_WIDTH = LAYOUT_THEMES[0].BOTTOM_BUTTONS_ONE_WIDTH,
        // FFT and waterfall
        .FFT_HEIGHT_STYLE1 = LAYOUT_THEMES[0].FFT_HEIGHT_STYLE1,
        .WTF_HEIGHT_STYLE1 = LAYOUT_THEMES[0].WTF_HEIGHT_STYLE1,
        .FFT_HEIGHT_STYLE2 = LAYOUT_THEMES[0].FFT_HEIGHT_STYLE2,
        .WTF_HEIGHT_STYLE2 = LAYOUT_THEMES[0].WTF_HEIGHT_STYLE2,
        .FFT_HEIGHT_STYLE3 = LAYOUT_THEMES[0].FFT_HEIGHT_STYLE3,
        .WTF_HEIGHT_STYLE3 = LAYOUT_THEMES[0].WTF_HEIGHT_STYLE3,
        .FFT_HEIGHT_STYLE4 = LAYOUT_THEMES[0].FFT_HEIGHT_STYLE4,
        .WTF_HEIGHT_STYLE4 = LAYOUT_THEMES[0].WTF_HEIGHT_STYLE4,
        .FFT_HEIGHT_STYLE5 = LAYOUT_THEMES[0].FFT_HEIGHT_STYLE5,
        .WTF_HEIGHT_STYLE5 = LAYOUT_THEMES[0].WTF_HEIGHT_STYLE5,
        .FFT_PRINT_SIZE = LAYOUT_THEMES[0].FFT_PRINT_SIZE,
        .FFT_CWDECODER_OFFSET = LAYOUT_THEMES[0].FFT_CWDECODER_OFFSET,
        .FFT_FFTWTF_POS_Y = LAYOUT_THEMES[0].FFT_FFTWTF_POS_Y,
        .FFT_FFTWTF_BOTTOM = LAYOUT_THEMES[0].FFT_FFTWTF_BOTTOM,
        .FFT_FREQLABELS_HEIGHT = LAYOUT_THEMES[0].FFT_FREQLABELS_HEIGHT,
        // system menu
        .SYSMENU_FONT_SIZE = LAYOUT_THEMES[0].SYSMENU_FONT_SIZE,
        .SYSMENU_X1 = LAYOUT_THEMES[0].SYSMENU_X1,
        .SYSMENU_X2 = LAYOUT_THEMES[0].SYSMENU_X2,
        .SYSMENU_W = LAYOUT_THEMES[0].SYSMENU_W,
        .SYSMENU_ITEM_HEIGHT = LAYOUT_THEMES[0].SYSMENU_ITEM_HEIGHT,
        .SYSMENU_MAX_ITEMS_ON_PAGE = LAYOUT_THEMES[0].SYSMENU_MAX_ITEMS_ON_PAGE,
        // Tooltip
        .TOOLTIP_FONT = LAYOUT_THEMES[0].TOOLTIP_FONT,
        .TOOLTIP_TIMEOUT = LAYOUT_THEMES[0].TOOLTIP_TIMEOUT,
        .TOOLTIP_MARGIN = LAYOUT_THEMES[0].TOOLTIP_MARGIN,
        .TOOLTIP_POS_X = LAYOUT_THEMES[0].TOOLTIP_POS_X,
        .TOOLTIP_POS_Y = LAYOUT_THEMES[0].TOOLTIP_POS_Y,
        // BW Trapezoid
        .BW_TRAPEZ_POS_X = LAYOUT_THEMES[0].BW_TRAPEZ_POS_X,
        .BW_TRAPEZ_POS_Y = LAYOUT_THEMES[0].BW_TRAPEZ_POS_Y,
        .BW_TRAPEZ_HEIGHT = LAYOUT_THEMES[0].BW_TRAPEZ_HEIGHT,
        .BW_TRAPEZ_WIDTH = LAYOUT_THEMES[0].BW_TRAPEZ_WIDTH,
    },
#endif
#ifdef FRONTPANEL_LITE
    // Default
    {
        // Top row of status buttons
        .TOPBUTTONS_X1 = 0,
        .TOPBUTTONS_X2 = (LCD_WIDTH - 1),
        .TOPBUTTONS_Y1 = 1,
        .TOPBUTTONS_Y2 = 56,
        .TOPBUTTONS_WIDTH = 72,
        .TOPBUTTONS_HEIGHT = 22,
        .TOPBUTTONS_TB_MARGIN = 2,
        .TOPBUTTONS_LR_MARGIN = 2,
        .TOPBUTTONS_PRE_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_X1 + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_PRE_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_ATT_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_PRE_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_ATT_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_PGA_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_ATT_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_PGA_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_DRV_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_PGA_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_DRV_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_AGC_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_DRV_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_AGC_Y = LAYOUT_THEMES[0].TOPBUTTONS_Y1,
        .TOPBUTTONS_SECOND_LINE_Y = (uint16_t)(LAYOUT_THEMES[0].TOPBUTTONS_Y1 + LAYOUT_THEMES[0].TOPBUTTONS_HEIGHT + LAYOUT_THEMES[0].TOPBUTTONS_TB_MARGIN),
        .TOPBUTTONS_DNR_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_X1 + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_DNR_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        .TOPBUTTONS_NB_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_DNR_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_NB_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        .TOPBUTTONS_FAST_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_NB_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_FAST_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        .TOPBUTTONS_MUTE_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_FAST_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_MUTE_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        .TOPBUTTONS_LOCK_X = (float32_t)(LAYOUT_THEMES[0].TOPBUTTONS_MUTE_X + LAYOUT_THEMES[0].TOPBUTTONS_WIDTH + LAYOUT_THEMES[0].TOPBUTTONS_LR_MARGIN),
        .TOPBUTTONS_LOCK_Y = LAYOUT_THEMES[0].TOPBUTTONS_SECOND_LINE_Y,
        // Clock
        .CLOCK_POS_Y = 17,
        .CLOCK_POS_HRS_X = (LCD_WIDTH - 75),
        .CLOCK_POS_MIN_X = (uint16_t)(LAYOUT_THEMES[0].CLOCK_POS_HRS_X + 25),
        .CLOCK_POS_SEC_X = (uint16_t)(LAYOUT_THEMES[0].CLOCK_POS_MIN_X + 25),
        .CLOCK_FONT = &FreeSans9pt7b,
        // WIFI
        .STATUS_WIFI_ICON_X = (LCD_WIDTH - 93),
        .STATUS_WIFI_ICON_Y = 3,
        // SD
        .STATUS_SD_ICON_X = (LCD_WIDTH - 93 - 16),
        .STATUS_SD_ICON_Y = 3,
        // FAN
        .STATUS_FAN_ICON_X = (LCD_WIDTH - 93 - 16),
        .STATUS_FAN_ICON_Y = 3,
        // Frequency output
        .FREQ_A_LEFT = 0,
        .FREQ_X_OFFSET_100 = 37,
        .FREQ_X_OFFSET_10 = 73,
        .FREQ_X_OFFSET_1 = 113,
        .FREQ_X_OFFSET_KHZ = 170,
        .FREQ_X_OFFSET_HZ = 306,
        .FREQ_HEIGHT = 51,
        .FREQ_WIDTH = 370,
        .FREQ_TOP_OFFSET = 3,
        .FREQ_LEFT_MARGIN = 37,
        .FREQ_RIGHT_MARGIN = (uint16_t)(LCD_WIDTH - LAYOUT_THEMES[0].FREQ_LEFT_MARGIN - LAYOUT_THEMES[0].FREQ_WIDTH),
        .FREQ_BOTTOM_OFFSET = 8,
        .FREQ_BLOCK_HEIGHT = (uint16_t)(LAYOUT_THEMES[0].FREQ_HEIGHT + LAYOUT_THEMES[0].FREQ_TOP_OFFSET + LAYOUT_THEMES[0].FREQ_BOTTOM_OFFSET),
        .FREQ_Y_TOP = LAYOUT_THEMES[0].TOPBUTTONS_Y2,
        .FREQ_Y_BASELINE = (uint16_t)(LAYOUT_THEMES[0].TOPBUTTONS_Y2 + LAYOUT_THEMES[0].FREQ_HEIGHT + LAYOUT_THEMES[0].FREQ_TOP_OFFSET),
        .FREQ_Y_BASELINE_SMALL = (uint16_t)(LAYOUT_THEMES[0].FREQ_Y_BASELINE - 2),
        .FREQ_FONT = &FreeSans36pt7b,
        .FREQ_SMALL_FONT = &FreeSans32pt7b,
        .FREQ_CH_FONT = &FreeSans24pt7b,
        .FREQ_CH_B_FONT = &FreeSans24pt7b,
        .FREQ_DELIMITER_Y_OFFSET = 0,
        .FREQ_DELIMITER_X1_OFFSET = 151,
        .FREQ_DELIMITER_X2_OFFSET = 285,
        // Display statuses under frequency
        .STATUS_Y_OFFSET = (uint16_t)(LAYOUT_THEMES[0].FREQ_Y_TOP + LAYOUT_THEMES[0].FREQ_BLOCK_HEIGHT + 1),
        .STATUS_HEIGHT = 44,
        .STATUS_BAR_X_OFFSET = 60,
        .STATUS_BAR_Y_OFFSET = 16,
        .STATUS_BAR_HEIGHT = 10,
        .STATUS_TXRX_X_OFFSET = 2, // RX offset from the left edge of the screen (in pixels)
        .STATUS_TXRX_Y_OFFSET = -50,
        .STATUS_TXRX_FONT = &FreeSans9pt7b,
        .STATUS_VFO_X_OFFSET = 0,
        .STATUS_VFO_Y_OFFSET = -43,
        .STATUS_VFO_BLOCK_WIDTH = 37,
        .STATUS_VFO_BLOCK_HEIGHT = 22,
        .STATUS_ANT_X_OFFSET = 0,
        .STATUS_ANT_Y_OFFSET = -23,
        .STATUS_ANT_BLOCK_WIDTH = 37,
        .STATUS_ANT_BLOCK_HEIGHT = 22,
        .STATUS_TX_LABELS_OFFSET_X = 5,
        .STATUS_TX_LABELS_MARGIN_X = 55,
        .STATUS_SMETER_ANALOG = false,
        .STATUS_SMETER_TOP_OFFSET = 0,
        .STATUS_SMETER_ANALOG_HEIGHT = 0,
        .STATUS_SMETER_WIDTH = 400,
        .STATUS_SMETER_MARKER_HEIGHT = 30,
        .STATUS_PMETER_WIDTH = 300,
        .STATUS_AMETER_WIDTH = 90,
        .STATUS_ALC_BAR_X_OFFSET = 10,
        .STATUS_LABELS_OFFSET_Y = 0,
        .STATUS_LABELS_FONT_SIZE = 1,
        .STATUS_LABEL_S_VAL_X_OFFSET = 12,
        .STATUS_LABEL_S_VAL_Y_OFFSET = 25,
        .STATUS_LABEL_S_VAL_FONT = &FreeSans7pt7b,
        .STATUS_LABEL_DBM_X_OFFSET = 30,
        .STATUS_LABEL_DBM_Y_OFFSET = -65,
        .STATUS_LABEL_BW_X_OFFSET = 400,
        .STATUS_LABEL_BW_Y_OFFSET = -65,
        .STATUS_LABEL_RIT_X_OFFSET = 160,
        .STATUS_LABEL_RIT_Y_OFFSET = 36,
        .STATUS_LABEL_THERM_X_OFFSET = 235,
        .STATUS_LABEL_THERM_Y_OFFSET = 36,
        .STATUS_LABEL_NOTCH_X_OFFSET = 335,
        .STATUS_LABEL_NOTCH_Y_OFFSET = 36,
        .STATUS_LABEL_FFT_BW_X_OFFSET = 420,
        .STATUS_LABEL_FFT_BW_Y_OFFSET = 36,
        // Information panel
        .STATUS_INFOA_X_OFFSET = 1,
        .STATUS_INFOB_X_OFFSET = 81,
        .STATUS_INFOC_X_OFFSET = 161,
        .STATUS_INFOD_X_OFFSET = 241,
        .STATUS_INFOE_X_OFFSET = 321,
        .STATUS_INFOF_X_OFFSET = 401,
        .STATUS_INFO_Y_OFFSET = 149,
        .STATUS_INFO_WIDTH = 74,
        .STATUS_INFO_HEIGHT = 12,
        .STATUS_LABEL_REC_X_OFFSET = 130,
        .STATUS_LABEL_REC_Y_OFFSET = 36,
        .STATUS_SMETER_PEAK_HOLDTIME = 1000,
        .STATUS_SMETER_TXLABELS_MARGIN = 55,
        .STATUS_SMETER_TXLABELS_PADDING = 23,
        .STATUS_TX_LABELS_VAL_WIDTH = 25,
        .STATUS_TX_LABELS_VAL_HEIGHT = 8,
        .STATUS_TX_LABELS_SWR_X = (uint16_t)(LAYOUT_THEMES[0].STATUS_BAR_X_OFFSET + LAYOUT_THEMES[0].STATUS_TX_LABELS_OFFSET_X + LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_PADDING),
        .STATUS_TX_LABELS_FWD_X = (uint16_t)(LAYOUT_THEMES[0].STATUS_BAR_X_OFFSET + LAYOUT_THEMES[0].STATUS_TX_LABELS_OFFSET_X + LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_MARGIN +
                                             LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_PADDING),
        .STATUS_TX_LABELS_REF_X = (uint16_t)(LAYOUT_THEMES[0].STATUS_BAR_X_OFFSET + LAYOUT_THEMES[0].STATUS_TX_LABELS_OFFSET_X + LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_MARGIN * 2 +
                                             LAYOUT_THEMES[0].STATUS_SMETER_TXLABELS_PADDING),
        .STATUS_TX_ALC_X_OFFSET = 40,
        .STATUS_MODE_X_OFFSET = (uint16_t)(LCD_WIDTH - LAYOUT_THEMES[0].FREQ_RIGHT_MARGIN + 10),
        .STATUS_MODE_Y_OFFSET = -42,
        .STATUS_MODE_BLOCK_WIDTH = 50,
        .STATUS_MODE_BLOCK_HEIGHT = 22,
        .STATUS_ERR_OFFSET_X = 435,
        .STATUS_ERR_OFFSET_Y = 65,
        .STATUS_ERR_WIDTH = 20,
        .STATUS_ERR_HEIGHT = 8,
        .TEXTBAR_FONT = 2,
        // bottom buttons
        .BOTTOM_BUTTONS_BLOCK_HEIGHT = 22,
        .BOTTOM_BUTTONS_BLOCK_TOP = (uint16_t)(LCD_HEIGHT - LAYOUT_THEMES[0].BOTTOM_BUTTONS_BLOCK_HEIGHT),
        .BOTTOM_BUTTONS_COUNT = FUNCBUTTONS_ON_PAGE,
        .BOTTOM_BUTTONS_ONE_WIDTH = (uint16_t)(LCD_WIDTH / LAYOUT_THEMES[0].BOTTOM_BUTTONS_COUNT),
        // FFT and waterfall CHECK MAX_FFT_HEIGHT AND MAX_WTF_HEIGHT defines !!!
        .FFT_HEIGHT_STYLE1 = 40,
        .WTF_HEIGHT_STYLE1 = 90,
        .FFT_HEIGHT_STYLE2 = 50,
        .WTF_HEIGHT_STYLE2 = 80,
        .FFT_HEIGHT_STYLE3 = 65,
        .WTF_HEIGHT_STYLE3 = 65,
        .FFT_HEIGHT_STYLE4 = 80,
        .WTF_HEIGHT_STYLE4 = 50,
        .FFT_HEIGHT_STYLE5 = 90,
        .WTF_HEIGHT_STYLE5 = 40,
        .FFT_PRINT_SIZE = LCD_WIDTH,
        .FFT_CWDECODER_OFFSET = 17,
        .FFT_FFTWTF_POS_Y = (uint16_t)(LCD_HEIGHT - LAYOUT_THEMES[0].FFT_HEIGHT_STYLE1 - LAYOUT_THEMES[0].WTF_HEIGHT_STYLE1 - LAYOUT_THEMES[0].BOTTOM_BUTTONS_BLOCK_HEIGHT),
        .FFT_FFTWTF_BOTTOM = (uint16_t)(LAYOUT_THEMES[0].FFT_FFTWTF_POS_Y + LAYOUT_THEMES[0].FFT_HEIGHT_STYLE1 + LAYOUT_THEMES[0].WTF_HEIGHT_STYLE1),
        .FFT_FREQLABELS_HEIGHT = 0,
        // System menu
        .SYSMENU_FONT_SIZE = 2,
        .SYSMENU_X1 = 5,
        .SYSMENU_X2 = 400,
        .SYSMENU_W = 458,
        .SYSMENU_ITEM_HEIGHT = 18,
        .SYSMENU_MAX_ITEMS_ON_PAGE = (uint16_t)(LCD_HEIGHT / LAYOUT_THEMES[0].SYSMENU_ITEM_HEIGHT),
        // Stuff
        .GREETINGS_X = (LCD_WIDTH / 2 - 5),
        .GREETINGS_Y = 25,
        // Tooltip
        .TOOLTIP_FONT = (const GFXfont *)&FreeSans12pt7b,
        .TOOLTIP_TIMEOUT = 1500,
        .TOOLTIP_MARGIN = 5,
        .TOOLTIP_POS_X = (LCD_WIDTH / 2),
        .TOOLTIP_POS_Y = 70,
        // BW Trapezoid
        .BW_TRAPEZ_POS_X = 380,
        .BW_TRAPEZ_POS_Y = 25,
        .BW_TRAPEZ_HEIGHT = 25,
        .BW_TRAPEZ_WIDTH = 96,
        // Touch buttons layout
        .BUTTON_PADDING = 1,
        .BUTTON_LIGHTER_WIDTH = 0.4f,
        .BUTTON_LIGHTER_HEIGHT = 4,
    },
#endif
};

#endif
