#include "system_menu.h"
#include "lcd.h"
#include "settings.h"
#include "audio_filters.h"
#include "bootloader.h"
#include "functions.h"
#include "wifi.h"
#include "spec_analyzer.h"
#include "fonts.h"
#include "agc.h"
#include "screen_layout.h"

static void SYSMENU_HANDL_TRX_RFPower(int8_t direction);
static void SYSMENU_HANDL_TRX_BandMap(int8_t direction);
static void SYSMENU_HANDL_TRX_AutoGain(int8_t direction);
static void SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE(int8_t direction);
static void SYSMENU_HANDL_TRX_LPFFilter(int8_t direction);
static void SYSMENU_HANDL_TRX_BPFFilter(int8_t direction);
static void SYSMENU_HANDL_TRX_MICIN(int8_t direction);
static void SYSMENU_HANDL_TRX_LINEIN(int8_t direction);
static void SYSMENU_HANDL_TRX_USBIN(int8_t direction);
static void SYSMENU_HANDL_TRX_ENCODER_SLOW_RATE(int8_t direction);
static void SYSMENU_HANDL_TRX_DEBUG_CONSOLE(int8_t direction);

static void SYSMENU_HANDL_AUDIO_IFGain(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_THRES(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_AVERAGE(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_MINMAL(int8_t direction);
static void SYSMENU_HANDL_AUDIO_SSB_HPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_SSB_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_CW_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_CW_HPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AM_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FM_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_AGCSpeed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_AGCSpeed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FMSquelch(int8_t direction);

static void SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ(int8_t direction);
static void SYSMENU_HANDL_CW_Decoder(int8_t direction);
static void SYSMENU_HANDL_CW_SelfHear(int8_t direction);
static void SYSMENU_HANDL_CW_Keyer(int8_t direction);
static void SYSMENU_HANDL_CW_Keyer_WPM(int8_t direction);
static void SYSMENU_HANDL_CW_Key_timeout(int8_t direction);

static void SYSMENU_HANDL_LCD_Brightness(int8_t direction);
static void SYSMENU_HANDL_LCD_SMeter_Style(int8_t direction);
static void SYSMENU_HANDL_LCD_Standby_Time(int8_t direction);

static void SYSMENU_HANDL_FFT_Enabled(int8_t direction);
static void SYSMENU_HANDL_FFT_Averaging(int8_t direction);
static void SYSMENU_HANDL_FFT_Window(int8_t direction);
static void SYSMENU_HANDL_FFT_Zoom(int8_t direction);
static void SYSMENU_HANDL_FFT_Style(int8_t direction);

static void SYSMENU_HANDL_ADC_PGA(int8_t direction);
static void SYSMENU_HANDL_ADC_RAND(int8_t direction);
static void SYSMENU_HANDL_ADC_SHDN(int8_t direction);
static void SYSMENU_HANDL_ADC_DITH(int8_t direction);
static void SYSMENU_HANDL_ADC_DRIVER(int8_t direction);

static void SYSMENU_HANDL_WIFI_Enabled(int8_t direction);
static void SYSMENU_HANDL_WIFI_SelectAP(int8_t direction);
static void SYSMENU_HANDL_WIFI_SetAPpassword(int8_t direction);
static void SYSMENU_HANDL_WIFI_Timezone(int8_t direction);

static void SYSMENU_HANDL_SETTIME(int8_t direction);
static void SYSMENU_HANDL_Bootloader(int8_t direction);

static void SYSMENU_HANDL_SPECTRUM_Begin(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_End(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_Start(int8_t direction);

static void SYSMENU_HANDL_CALIB_CIC_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_DAC_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_0(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_1(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_2(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_3(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_4(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_5(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_6(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_7(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_8(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_9(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_10(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_11(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_12(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_13(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_14(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_15(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_16(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_17(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_18(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_19(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_20(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_21(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_22(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_23(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_24(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_25(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_26(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_27(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_28(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_29(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_30(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_31(int8_t direction);
static void SYSMENU_HANDL_CALIB_S_METER(int8_t direction);
static void SYSMENU_HANDL_CALIB_ADC_OFFSET(int8_t direction);
static void SYSMENU_HANDL_CALIB_ATT_DB(int8_t direction);
static void SYSMENU_HANDL_CALIB_LNA_GAIN(int8_t direction);
static void SYSMENU_HANDL_CALIB_LPF_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_0_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_0_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_1_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_1_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_2_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_2_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_3_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_3_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_4_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_4_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_5_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_5_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_6_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_6_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_HPF_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_SWR_TRANS_RATE(int8_t direction);
	
static void SYSMENU_HANDL_TRXMENU(int8_t direction);
static void SYSMENU_HANDL_AUDIOMENU(int8_t direction);
static void SYSMENU_HANDL_CWMENU(int8_t direction);
static void SYSMENU_HANDL_LCDMENU(int8_t direction);
static void SYSMENU_HANDL_FFTMENU(int8_t direction);
static void SYSMENU_HANDL_ADCMENU(int8_t direction);
static void SYSMENU_HANDL_WIFIMENU(int8_t direction);
static void SYSMENU_HANDL_SPECTRUMMENU(int8_t direction);
static void SYSMENU_HANDL_CALIBRATIONMENU(int8_t direction);

static struct sysmenu_item_handler sysmenu_handlers[] =
{
	{"TRX Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_TRXMENU},
	{"AUDIO Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_AUDIOMENU},
	{"CW Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_CWMENU},
	{"LCD Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_LCDMENU},
	{"FFT Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_FFTMENU},
	{"ADC/DAC Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_ADCMENU},
	{"WIFI Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_WIFIMENU},
	{"Set Clock Time", SYSMENU_RUN, 0, SYSMENU_HANDL_SETTIME},
	{"Flash update", SYSMENU_RUN, 0, SYSMENU_HANDL_Bootloader},
	{"Spectrum Analyzer", SYSMENU_MENU, 0, SYSMENU_HANDL_SPECTRUMMENU},
	{"Calibration", SYSMENU_HIDDEN_MENU, 0, SYSMENU_HANDL_CALIBRATIONMENU},
};
static uint8_t sysmenu_item_count = sizeof(sysmenu_handlers) / sizeof(sysmenu_handlers[0]);

static struct sysmenu_item_handler sysmenu_trx_handlers[] =
{
	{"RF Power", SYSMENU_UINT8, (uint32_t *)&TRX.RF_Power, SYSMENU_HANDL_TRX_RFPower},
	{"Band Map", SYSMENU_BOOLEAN, (uint32_t *)&TRX.BandMapEnabled, SYSMENU_HANDL_TRX_BandMap},
	{"AutoGainer", SYSMENU_BOOLEAN, (uint32_t *)&TRX.AutoGain, SYSMENU_HANDL_TRX_AutoGain},
	{"LPF Filter", SYSMENU_BOOLEAN, (uint32_t *)&TRX.LPF, SYSMENU_HANDL_TRX_LPFFilter},
	{"BPF Filter", SYSMENU_BOOLEAN, (uint32_t *)&TRX.BPF, SYSMENU_HANDL_TRX_BPFFilter},
	{"Encoder slow rate", SYSMENU_UINT8, (uint32_t *)&TRX.ENCODER_SLOW_RATE, SYSMENU_HANDL_TRX_ENCODER_SLOW_RATE},
	{"Two Signal TUNE", SYSMENU_BOOLEAN, (uint32_t *)&TRX.TWO_SIGNAL_TUNE, SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE},
	{"DEBUG Console", SYSMENU_BOOLEAN, (uint32_t *)&TRX.Debug_Console, SYSMENU_HANDL_TRX_DEBUG_CONSOLE},
	{"MIC IN", SYSMENU_BOOLEAN, (uint32_t *)&TRX.InputType_MIC, SYSMENU_HANDL_TRX_MICIN},
	{"LINE IN", SYSMENU_BOOLEAN, (uint32_t *)&TRX.InputType_LINE, SYSMENU_HANDL_TRX_LINEIN},
	{"USB IN", SYSMENU_BOOLEAN, (uint32_t *)&TRX.InputType_USB, SYSMENU_HANDL_TRX_USBIN},
};
static uint8_t sysmenu_trx_item_count = sizeof(sysmenu_trx_handlers) / sizeof(sysmenu_trx_handlers[0]);

static struct sysmenu_item_handler sysmenu_audio_handlers[] =
{
	{"IF Gain, dB", SYSMENU_UINT8, (uint32_t *)&TRX.IF_Gain, SYSMENU_HANDL_AUDIO_IFGain},
	{"DNR Threshold", SYSMENU_UINT8, (uint32_t *)&TRX.DNR_SNR_THRESHOLD, SYSMENU_HANDL_AUDIO_DNR_THRES},
	{"DNR Average", SYSMENU_UINT8, (uint32_t *)&TRX.DNR_AVERAGE, SYSMENU_HANDL_AUDIO_DNR_AVERAGE},
	{"DNR Minimal", SYSMENU_UINT8, (uint32_t *)&TRX.DNR_MINIMAL, SYSMENU_HANDL_AUDIO_DNR_MINMAL},
	{"SSB HPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.SSB_HPF_Filter, SYSMENU_HANDL_AUDIO_SSB_HPF_pass},
	{"SSB LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.SSB_LPF_Filter, SYSMENU_HANDL_AUDIO_SSB_LPF_pass},
	{"CW HPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.CW_HPF_Filter, SYSMENU_HANDL_AUDIO_CW_HPF_pass},		
	{"CW LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.CW_LPF_Filter, SYSMENU_HANDL_AUDIO_CW_LPF_pass},
	{"AM LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.AM_LPF_Filter, SYSMENU_HANDL_AUDIO_AM_LPF_pass},		
	{"FM LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.FM_LPF_Filter, SYSMENU_HANDL_AUDIO_FM_LPF_pass},
	{"FM Squelch", SYSMENU_UINT8, (uint32_t *)&TRX.FM_SQL_threshold, SYSMENU_HANDL_AUDIO_FMSquelch},
	{"RX AGC Speed", SYSMENU_UINT8, (uint32_t *)&TRX.RX_AGC_speed, SYSMENU_HANDL_AUDIO_RX_AGCSpeed},
	{"TX AGC Speed", SYSMENU_UINT8, (uint32_t *)&TRX.TX_AGC_speed, SYSMENU_HANDL_AUDIO_TX_AGCSpeed},
};
static uint8_t sysmenu_audio_item_count = sizeof(sysmenu_audio_handlers) / sizeof(sysmenu_audio_handlers[0]);

static struct sysmenu_item_handler sysmenu_cw_handlers[] =
{
	{"CW Key timeout", SYSMENU_UINT16, (uint32_t *)&TRX.Key_timeout, SYSMENU_HANDL_CW_Key_timeout},
	{"CW Generator shift", SYSMENU_UINT16, (uint32_t *)&TRX.CW_GENERATOR_SHIFT_HZ, SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ},
	{"CW Self Hear", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CW_SelfHear, SYSMENU_HANDL_CW_SelfHear},
	{"CW Keyer", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CW_KEYER, SYSMENU_HANDL_CW_Keyer},
	{"CW Keyer WPM", SYSMENU_UINT8, (uint32_t *)&TRX.CW_KEYER_WPM, SYSMENU_HANDL_CW_Keyer_WPM},
	{"CW Decoder", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CWDecoder, SYSMENU_HANDL_CW_Decoder},
};
static uint8_t sysmenu_cw_item_count = sizeof(sysmenu_cw_handlers) / sizeof(sysmenu_cw_handlers[0]);

static struct sysmenu_item_handler sysmenu_lcd_handlers[] =
{
	{"LCD Brightness", SYSMENU_UINT8, (uint32_t *)&TRX.LCD_Brightness, SYSMENU_HANDL_LCD_Brightness},
	{"LCD Sleep Time", SYSMENU_UINT8, (uint32_t *)&TRX.Standby_Time, SYSMENU_HANDL_LCD_Standby_Time},
	{"S-METER Line", SYSMENU_BOOLEAN, (uint32_t *)&TRX.S_METER_Style, SYSMENU_HANDL_LCD_SMeter_Style},
};
static uint8_t sysmenu_lcd_item_count = sizeof(sysmenu_lcd_handlers) / sizeof(sysmenu_lcd_handlers[0]);

static struct sysmenu_item_handler sysmenu_fft_handlers[] =
{
	{"FFT Zoom", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Zoom, SYSMENU_HANDL_FFT_Zoom},
	{"FFT Style", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Style, SYSMENU_HANDL_FFT_Style},
	{"FFT Enabled", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Enabled, SYSMENU_HANDL_FFT_Enabled},
	{"FFT Averaging", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Averaging, SYSMENU_HANDL_FFT_Averaging},
	{"FFT Window", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Window, SYSMENU_HANDL_FFT_Window},
};
static uint8_t sysmenu_fft_item_count = sizeof(sysmenu_fft_handlers) / sizeof(sysmenu_fft_handlers[0]);

static struct sysmenu_item_handler sysmenu_adc_handlers[] =
{
	{"ADC Driver", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_Driver, SYSMENU_HANDL_ADC_DRIVER},
	{"ADC Preamp", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_PGA, SYSMENU_HANDL_ADC_PGA},
	{"ADC Dither", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_DITH, SYSMENU_HANDL_ADC_DITH},
	{"ADC Randomizer", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_RAND, SYSMENU_HANDL_ADC_RAND},
	{"ADC Shutdown", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_SHDN, SYSMENU_HANDL_ADC_SHDN},
};
static uint8_t sysmenu_adc_item_count = sizeof(sysmenu_adc_handlers) / sizeof(sysmenu_adc_handlers[0]);

static struct sysmenu_item_handler sysmenu_wifi_handlers[] =
{
	{"WIFI Enabled", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WIFI_Enabled, SYSMENU_HANDL_WIFI_Enabled},
	{"WIFI Select AP", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SelectAP},
	{"WIFI Set AP Pass", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SetAPpassword},
	{"WIFI Timezone", SYSMENU_INT8, (uint32_t *)&TRX.WIFI_TIMEZONE, SYSMENU_HANDL_WIFI_Timezone},
};
static uint8_t sysmenu_wifi_item_count = sizeof(sysmenu_wifi_handlers) / sizeof(sysmenu_wifi_handlers[0]);

static struct sysmenu_item_handler sysmenu_spectrum_handlers[] =
{
	{"Spectrum START", SYSMENU_RUN, 0, SYSMENU_HANDL_SPECTRUM_Start},
	{"Begin, 10kHz", SYSMENU_UINT16, (uint32_t *)&TRX.SPEC_Begin, SYSMENU_HANDL_SPECTRUM_Begin},
	{"End, 10kHz", SYSMENU_UINT16, (uint32_t *)&TRX.SPEC_End, SYSMENU_HANDL_SPECTRUM_End},
};
static uint8_t sysmenu_spectrum_item_count = sizeof(sysmenu_spectrum_handlers) / sizeof(sysmenu_spectrum_handlers[0]);

static struct sysmenu_item_handler sysmenu_calibration_handlers[] =
{
	{"CIC Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.CIC_GAINER_val, SYSMENU_HANDL_CALIB_CIC_SHIFT},
	{"CICCOMP Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.CICFIR_GAINER_val, SYSMENU_HANDL_CALIB_CICCOMP_SHIFT},
	{"TX CICCOMP Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.TXCICFIR_GAINER_val, SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT},
	{"DAC Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.DAC_GAINER_val, SYSMENU_HANDL_CALIB_DAC_SHIFT},
	{"RF GAIN 0", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[0], SYSMENU_HANDL_CALIB_RF_GAIN_0},
	{"RF GAIN 1", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[1], SYSMENU_HANDL_CALIB_RF_GAIN_1},
	{"RF GAIN 2", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[2], SYSMENU_HANDL_CALIB_RF_GAIN_2},
	{"RF GAIN 3", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[3], SYSMENU_HANDL_CALIB_RF_GAIN_3},
	{"RF GAIN 4", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[4], SYSMENU_HANDL_CALIB_RF_GAIN_4},
	{"RF GAIN 5", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[5], SYSMENU_HANDL_CALIB_RF_GAIN_5},
	{"RF GAIN 6", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[6], SYSMENU_HANDL_CALIB_RF_GAIN_6},
	{"RF GAIN 7", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[7], SYSMENU_HANDL_CALIB_RF_GAIN_7},
	{"RF GAIN 8", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[8], SYSMENU_HANDL_CALIB_RF_GAIN_8},
	{"RF GAIN 9", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[9], SYSMENU_HANDL_CALIB_RF_GAIN_9},
	{"RF GAIN 10", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[10], SYSMENU_HANDL_CALIB_RF_GAIN_10},
	{"RF GAIN 11", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[11], SYSMENU_HANDL_CALIB_RF_GAIN_11},
	{"RF GAIN 12", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[12], SYSMENU_HANDL_CALIB_RF_GAIN_12},
	{"RF GAIN 13", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[13], SYSMENU_HANDL_CALIB_RF_GAIN_13},
	{"RF GAIN 14", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[14], SYSMENU_HANDL_CALIB_RF_GAIN_14},
	{"RF GAIN 15", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[15], SYSMENU_HANDL_CALIB_RF_GAIN_15},
	{"RF GAIN 16", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[16], SYSMENU_HANDL_CALIB_RF_GAIN_16},
	{"RF GAIN 17", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[17], SYSMENU_HANDL_CALIB_RF_GAIN_17},
	{"RF GAIN 18", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[18], SYSMENU_HANDL_CALIB_RF_GAIN_18},
	{"RF GAIN 19", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[19], SYSMENU_HANDL_CALIB_RF_GAIN_19},
	{"RF GAIN 20", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[20], SYSMENU_HANDL_CALIB_RF_GAIN_20},
	{"RF GAIN 21", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[21], SYSMENU_HANDL_CALIB_RF_GAIN_21},
	{"RF GAIN 22", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[22], SYSMENU_HANDL_CALIB_RF_GAIN_22},
	{"RF GAIN 23", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[23], SYSMENU_HANDL_CALIB_RF_GAIN_23},
	{"RF GAIN 24", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[24], SYSMENU_HANDL_CALIB_RF_GAIN_24},
	{"RF GAIN 25", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[25], SYSMENU_HANDL_CALIB_RF_GAIN_25},
	{"RF GAIN 26", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[26], SYSMENU_HANDL_CALIB_RF_GAIN_26},
	{"RF GAIN 27", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[27], SYSMENU_HANDL_CALIB_RF_GAIN_27},
	{"RF GAIN 28", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[28], SYSMENU_HANDL_CALIB_RF_GAIN_28},
	{"RF GAIN 29", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[29], SYSMENU_HANDL_CALIB_RF_GAIN_29},
	{"RF GAIN 30", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[30], SYSMENU_HANDL_CALIB_RF_GAIN_30},
	{"RF GAIN 31+", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power[31], SYSMENU_HANDL_CALIB_RF_GAIN_31},
	{"S METER", SYSMENU_INT16, (uint32_t *)&CALIBRATE.smeter_calibration, SYSMENU_HANDL_CALIB_S_METER},
	{"ADC OFFSET", SYSMENU_INT16, (uint32_t *)&CALIBRATE.adc_offset, SYSMENU_HANDL_CALIB_ADC_OFFSET},
	{"ATT DB", SYSMENU_INT16, (uint32_t *)&CALIBRATE.att_db, SYSMENU_HANDL_CALIB_ATT_DB},
	{"LNA GAIN DB", SYSMENU_INT16, (uint32_t *)&CALIBRATE.lna_gain_db, SYSMENU_HANDL_CALIB_LNA_GAIN},
	{"LPF END", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.LPF_END, SYSMENU_HANDL_CALIB_LPF_END},
	{"BPF 0 START", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_0_START, SYSMENU_HANDL_CALIB_BPF_0_START},
	{"BPF 0 END", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_0_END, SYSMENU_HANDL_CALIB_BPF_0_END},
	{"BPF 1 START", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_1_START, SYSMENU_HANDL_CALIB_BPF_1_START},
	{"BPF 1 END", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_1_END, SYSMENU_HANDL_CALIB_BPF_1_END},
	{"BPF 2 START", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_2_START, SYSMENU_HANDL_CALIB_BPF_2_START},
	{"BPF 2 END", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_2_END, SYSMENU_HANDL_CALIB_BPF_2_END},
	{"BPF 3 START", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_3_START, SYSMENU_HANDL_CALIB_BPF_3_START},
	{"BPF 3 END", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_3_END, SYSMENU_HANDL_CALIB_BPF_3_END},
	{"BPF 4 START", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_4_START, SYSMENU_HANDL_CALIB_BPF_4_START},
	{"BPF 4 END", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_4_END, SYSMENU_HANDL_CALIB_BPF_4_END},
	{"BPF 5 START", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_5_START, SYSMENU_HANDL_CALIB_BPF_5_START},
	{"BPF 5 END", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_5_END, SYSMENU_HANDL_CALIB_BPF_5_END},
	{"BPF 6 START", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_6_START, SYSMENU_HANDL_CALIB_BPF_6_START},
	{"BPF 6 END", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_6_END, SYSMENU_HANDL_CALIB_BPF_6_END},
	{"HPF START", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_7_HPF, SYSMENU_HANDL_CALIB_HPF_START},
	{"SWR TRANS RATE", SYSMENU_FLOAT32, (uint32_t *)&CALIBRATE.swr_trans_rate_shadow, SYSMENU_HANDL_CALIB_SWR_TRANS_RATE},
};
static uint8_t sysmenu_calibration_item_count = sizeof(sysmenu_calibration_handlers) / sizeof(sysmenu_calibration_handlers[0]);

//COMMON MENU
static void drawSystemMenuElement(char *title, SystemMenuType type, uint32_t *value, bool onlyVal);
static void redrawCurrentItem(void);
static void SYSMENU_WIFI_DrawSelectAPMenu(bool full_redraw);
static void SYSMENU_WIFI_SelectAPMenuMove(int8_t dir);
static void SYSMENU_WIFI_DrawAPpasswordMenu(bool full_redraw);
static void SYSMENU_WIFI_RotatePasswordChar(int8_t dir);

static struct sysmenu_item_handler *sysmenu_handlers_selected = &sysmenu_handlers[0];
static uint8_t *sysmenu_item_count_selected = &sysmenu_item_count;
static uint8_t systemMenuIndex = 0;
static uint8_t systemMenuRootIndex = 0;
static uint16_t sysmenu_y = 5;
static uint8_t sysmenu_i = 0;
static bool sysmenu_onroot = true;
bool sysmenu_hiddenmenu_enabled = false;
static const uint8_t max_items_on_page = LCD_HEIGHT / LAY_SYSMENU_ITEM_HEIGHT;

//WIFI
static bool sysmenu_wifi_selectap_menu_opened = false;
static bool sysmenu_wifi_setAPpassword_menu_opened = false;
static uint16_t sysmenu_wifi_rescan_interval = 0;
static uint8_t sysmenu_wifi_selected_ap_index = 0;
static uint8_t sysmenu_wifi_selected_ap_password_char_index = 0;

//SPEC analyser
bool sysmenu_spectrum_opened = false;
static uint32_t sysmenu_spectrum_lastfreq = 0;

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wunused-parameter"

//TRX MENU

static void SYSMENU_HANDL_TRXMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_trx_handlers[0];
	sysmenu_item_count_selected = &sysmenu_trx_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

void SYSMENU_RFPOWER_HOTKEY(void)
{
	sysmenu_handlers_selected = &sysmenu_trx_handlers[0];
	sysmenu_item_count_selected = &sysmenu_trx_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_TRX_BandMap(int8_t direction)
{
	if (direction > 0)
		TRX.BandMapEnabled = true;
	if (direction < 0)
		TRX.BandMapEnabled = false;
}

static void SYSMENU_HANDL_TRX_AutoGain(int8_t direction)
{
	if (direction > 0)
		TRX.AutoGain = true;
	if (direction < 0)
		TRX.AutoGain = false;
	FPGA_NeedSendParams = true;
}

static void SYSMENU_HANDL_TRX_LPFFilter(int8_t direction)
{
	if (direction > 0)
		TRX.LPF = true;
	if (direction < 0)
		TRX.LPF = false;
}

static void SYSMENU_HANDL_TRX_BPFFilter(int8_t direction)
{
	if (direction > 0)
		TRX.BPF = true;
	if (direction < 0)
		TRX.BPF = false;
}

static void SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE(int8_t direction)
{
	if (direction > 0)
		TRX.TWO_SIGNAL_TUNE = true;
	if (direction < 0)
		TRX.TWO_SIGNAL_TUNE = false;
}

static void SYSMENU_HANDL_TRX_RFPower(int8_t direction)
{
	TRX.RF_Power += direction;
	if (TRX.RF_Power < 1)
		TRX.RF_Power = 1;
	if (TRX.RF_Power > 100)
		TRX.RF_Power = 100;
}

static void SYSMENU_HANDL_TRX_MICIN(int8_t direction)
{
	if (direction > 0)
		TRX.InputType_MIC = true;
	if (direction < 0)
		TRX.InputType_MIC = false;
	TRX.InputType_LINE = false;
	TRX.InputType_USB = false;
	drawSystemMenu(false);
	TRX_Restart_Mode();
}

static void SYSMENU_HANDL_TRX_LINEIN(int8_t direction)
{
	if (direction > 0)
		TRX.InputType_LINE = true;
	if (direction < 0)
		TRX.InputType_LINE = false;
	TRX.InputType_MIC = false;
	TRX.InputType_USB = false;
	drawSystemMenu(false);
	TRX_Restart_Mode();
}

static void SYSMENU_HANDL_TRX_USBIN(int8_t direction)
{
	if (direction > 0)
		TRX.InputType_USB = true;
	if (direction < 0)
		TRX.InputType_USB = false;
	TRX.InputType_MIC = false;
	TRX.InputType_LINE = false;
	drawSystemMenu(false);
	TRX_Restart_Mode();
}

static void SYSMENU_HANDL_TRX_ENCODER_SLOW_RATE(int8_t direction)
{
	TRX.ENCODER_SLOW_RATE += direction;
	if (TRX.ENCODER_SLOW_RATE < 1)
		TRX.ENCODER_SLOW_RATE = 1;
	if (TRX.ENCODER_SLOW_RATE > 100)
		TRX.ENCODER_SLOW_RATE = 100;
}

static void SYSMENU_HANDL_TRX_DEBUG_CONSOLE(int8_t direction)
{
	if (direction > 0)
		TRX.Debug_Console = true;
	if (direction < 0)
		TRX.Debug_Console = false;
}

//AUDIO MENU

static void SYSMENU_HANDL_AUDIOMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = &sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

void SYSMENU_AUDIO_SSB_HOTKEY(void)
{
	sysmenu_handlers_selected = &sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = &sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 1;
	drawSystemMenu(true);
}

void SYSMENU_AUDIO_CW_HOTKEY(void)
{
	sysmenu_handlers_selected = &sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = &sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 3;
	drawSystemMenu(true);
}

void SYSMENU_AUDIO_AM_HOTKEY(void)
{
	sysmenu_handlers_selected = &sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = &sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 4;
	drawSystemMenu(true);
}

void SYSMENU_AUDIO_FM_HOTKEY(void)
{
	sysmenu_handlers_selected = &sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = &sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 6;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_AUDIO_IFGain(int8_t direction)
{
	TRX.IF_Gain += direction;
	if (TRX.IF_Gain < 1)
		TRX.IF_Gain = 1;
	if (TRX.IF_Gain > 80)
		TRX.IF_Gain = 80;
}

static void SYSMENU_HANDL_AUDIO_DNR_THRES(int8_t direction)
{
	TRX.DNR_SNR_THRESHOLD += direction;
	if (TRX.DNR_SNR_THRESHOLD < 1)
		TRX.DNR_SNR_THRESHOLD = 1;
	if (TRX.DNR_SNR_THRESHOLD > 20)
		TRX.DNR_SNR_THRESHOLD = 20;
}

static void SYSMENU_HANDL_AUDIO_DNR_AVERAGE(int8_t direction)
{
	TRX.DNR_AVERAGE += direction;
	if (TRX.DNR_AVERAGE < 1)
		TRX.DNR_AVERAGE = 1;
	if (TRX.DNR_AVERAGE > 200)
		TRX.DNR_AVERAGE = 200;
}

static void SYSMENU_HANDL_AUDIO_DNR_MINMAL(int8_t direction)
{
	TRX.DNR_MINIMAL += direction;
	if (TRX.DNR_MINIMAL < 1)
		TRX.DNR_MINIMAL = 1;
	if (TRX.DNR_MINIMAL > 200)
		TRX.DNR_MINIMAL = 200;
}

static void SYSMENU_HANDL_AUDIO_RX_AGCSpeed(int8_t direction)
{
	TRX.RX_AGC_speed += direction;
	if (TRX.RX_AGC_speed < 1)
		TRX.RX_AGC_speed = 1;
	if (TRX.RX_AGC_speed > 20)
		TRX.RX_AGC_speed = 20;
	InitAGC();
}

static void SYSMENU_HANDL_AUDIO_TX_AGCSpeed(int8_t direction)
{
	TRX.TX_AGC_speed += direction;
	if (TRX.TX_AGC_speed < 1)
		TRX.TX_AGC_speed = 1;
	if (TRX.TX_AGC_speed > 50)
		TRX.TX_AGC_speed = 50;
	InitAGC();
}

static void SYSMENU_HANDL_AUDIO_FMSquelch(int8_t direction)
{
	if (direction < 0 && TRX.FM_SQL_threshold == 0)
		return;
	TRX.FM_SQL_threshold += direction;
	if (TRX.FM_SQL_threshold > 10)
		TRX.FM_SQL_threshold = 10;
	
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if(band>0)
		TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold = TRX.FM_SQL_threshold;
}

static void SYSMENU_HANDL_AUDIO_SSB_HPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.SSB_HPF_Filter == 0)
			TRX.SSB_HPF_Filter = 60;
		else if (TRX.SSB_HPF_Filter == 60)
			TRX.SSB_HPF_Filter = 100;
		else if (TRX.SSB_HPF_Filter == 100)
			TRX.SSB_HPF_Filter = 200;
		else if (TRX.SSB_HPF_Filter == 200)
			TRX.SSB_HPF_Filter = 300;
		else if (TRX.SSB_HPF_Filter == 300)
			TRX.SSB_HPF_Filter = 400;
		else if (TRX.SSB_HPF_Filter == 400)
			TRX.SSB_HPF_Filter = 500;
	}
	else
	{
		if (TRX.SSB_HPF_Filter == 60)
			TRX.SSB_HPF_Filter = 0;
		else if (TRX.SSB_HPF_Filter == 100)
			TRX.SSB_HPF_Filter = 60;
		else if (TRX.SSB_HPF_Filter == 200)
			TRX.SSB_HPF_Filter = 100;
		else if (TRX.SSB_HPF_Filter == 300)
			TRX.SSB_HPF_Filter = 200;
		else if (TRX.SSB_HPF_Filter == 400)
			TRX.SSB_HPF_Filter = 300;
		else if (TRX.SSB_HPF_Filter == 500)
			TRX.SSB_HPF_Filter = 400;
	}
	ReinitAudioFilters();
}

static void SYSMENU_HANDL_AUDIO_CW_HPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.CW_HPF_Filter == 0)
			TRX.CW_HPF_Filter = 60;
		else if (TRX.CW_HPF_Filter == 60)
			TRX.CW_HPF_Filter = 100;
	}
	else
	{
		if (TRX.CW_HPF_Filter == 60)
			TRX.CW_HPF_Filter = 0;
		else if (TRX.CW_HPF_Filter == 100)
			TRX.CW_HPF_Filter = 60;
	}
	ReinitAudioFilters();
}

static void SYSMENU_HANDL_AUDIO_CW_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.CW_LPF_Filter == 300)
			TRX.CW_LPF_Filter = 500;
		else if (TRX.CW_LPF_Filter == 500)
			TRX.CW_LPF_Filter = 700;
		else if (TRX.CW_LPF_Filter == 700)
			TRX.CW_LPF_Filter = 1000;
		else if (TRX.CW_LPF_Filter == 1400)
			TRX.CW_LPF_Filter = 1600;
		else if (TRX.CW_LPF_Filter == 1600)
			TRX.CW_LPF_Filter = 1800;
		else if (TRX.CW_LPF_Filter == 1800)
			TRX.CW_LPF_Filter = 2100;
	}
	else
	{
		if (TRX.CW_LPF_Filter == 500)
			TRX.CW_LPF_Filter = 300;
		else if (TRX.CW_LPF_Filter == 700)
			TRX.CW_LPF_Filter = 500;
		else if (TRX.CW_LPF_Filter == 1000)
			TRX.CW_LPF_Filter = 700;
		else if (TRX.CW_LPF_Filter == 1400)
			TRX.CW_LPF_Filter = 1000;
		else if (TRX.CW_LPF_Filter == 1600)
			TRX.CW_LPF_Filter = 1400;
		else if (TRX.CW_LPF_Filter == 1800)
			TRX.CW_LPF_Filter = 1600;
		else if (TRX.CW_LPF_Filter == 2100)
			TRX.CW_LPF_Filter = 1800;
	}

	ReinitAudioFilters();
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_SSB_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.SSB_LPF_Filter == 0)
			TRX.SSB_LPF_Filter = 1400;
		
		if (TRX.SSB_LPF_Filter == 1400)
			TRX.SSB_LPF_Filter = 1600;
		else if (TRX.SSB_LPF_Filter == 1600)
			TRX.SSB_LPF_Filter = 1800;
		else if (TRX.SSB_LPF_Filter == 1800)
			TRX.SSB_LPF_Filter = 2100;
		else if (TRX.SSB_LPF_Filter == 2100)
			TRX.SSB_LPF_Filter = 2300;
		else if (TRX.SSB_LPF_Filter == 2300)
			TRX.SSB_LPF_Filter = 2500;
		else if (TRX.SSB_LPF_Filter == 2500)
			TRX.SSB_LPF_Filter = 2700;
		else if (TRX.SSB_LPF_Filter == 2700)
			TRX.SSB_LPF_Filter = 2900;
		else if (TRX.SSB_LPF_Filter == 2900)
			TRX.SSB_LPF_Filter = 3000;
		else if (TRX.SSB_LPF_Filter == 3000)
			TRX.SSB_LPF_Filter = 3200;
		else if (TRX.SSB_LPF_Filter == 3200)
			TRX.SSB_LPF_Filter = 3400;
	}
	else
	{
		if (TRX.SSB_LPF_Filter == 1600)
			TRX.SSB_LPF_Filter = 1400;
		else if (TRX.SSB_LPF_Filter == 1800)
			TRX.SSB_LPF_Filter = 1600;
		else if (TRX.SSB_LPF_Filter == 2100)
			TRX.SSB_LPF_Filter = 1800;
		else if (TRX.SSB_LPF_Filter == 2300)
			TRX.SSB_LPF_Filter = 2100;
		else if (TRX.SSB_LPF_Filter == 2500)
			TRX.SSB_LPF_Filter = 2300;
		else if (TRX.SSB_LPF_Filter == 2700)
			TRX.SSB_LPF_Filter = 2500;
		else if (TRX.SSB_LPF_Filter == 2900)
			TRX.SSB_LPF_Filter = 2700;
		else if (TRX.SSB_LPF_Filter == 3000)
			TRX.SSB_LPF_Filter = 2900;
		else if (TRX.SSB_LPF_Filter == 3200)
			TRX.SSB_LPF_Filter = 3000;
		else if (TRX.SSB_LPF_Filter == 3400)
			TRX.SSB_LPF_Filter = 3200;
	}

	ReinitAudioFilters();
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_AM_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.AM_LPF_Filter == 2100)
			TRX.AM_LPF_Filter = 2300;
		else if (TRX.AM_LPF_Filter == 2300)
			TRX.AM_LPF_Filter = 2500;
		else if (TRX.AM_LPF_Filter == 2500)
			TRX.AM_LPF_Filter = 2700;
		else if (TRX.AM_LPF_Filter == 2700)
			TRX.AM_LPF_Filter = 2900;
		else if (TRX.AM_LPF_Filter == 2900)
			TRX.AM_LPF_Filter = 3000;
		else if (TRX.AM_LPF_Filter == 3000)
			TRX.AM_LPF_Filter = 3200;
		else if (TRX.AM_LPF_Filter == 3200)
			TRX.AM_LPF_Filter = 3400;
		else if (TRX.AM_LPF_Filter == 3400)
			TRX.AM_LPF_Filter = 3600;
		else if (TRX.AM_LPF_Filter == 3600)
			TRX.AM_LPF_Filter = 3800;
		else if (TRX.AM_LPF_Filter == 3800)
			TRX.AM_LPF_Filter = 4000;
		else if (TRX.AM_LPF_Filter == 4000)
			TRX.AM_LPF_Filter = 4500;
		else if (TRX.AM_LPF_Filter == 4500)
			TRX.AM_LPF_Filter = 5000;
		else if (TRX.AM_LPF_Filter == 5000)
			TRX.AM_LPF_Filter = 6000;
		else if (TRX.AM_LPF_Filter == 6000)
			TRX.AM_LPF_Filter = 7000;
	}
	else
	{
		if (TRX.AM_LPF_Filter == 2300)
			TRX.AM_LPF_Filter = 2100;
		else if (TRX.AM_LPF_Filter == 2500)
			TRX.AM_LPF_Filter = 2300;
		else if (TRX.AM_LPF_Filter == 2700)
			TRX.AM_LPF_Filter = 2500;
		else if (TRX.AM_LPF_Filter == 2900)
			TRX.AM_LPF_Filter = 2700;
		else if (TRX.AM_LPF_Filter == 3000)
			TRX.AM_LPF_Filter = 2900;
		else if (TRX.AM_LPF_Filter == 3200)
			TRX.AM_LPF_Filter = 3000;
		else if (TRX.AM_LPF_Filter == 3400)
			TRX.AM_LPF_Filter = 3200;
		else if (TRX.AM_LPF_Filter == 3600)
			TRX.AM_LPF_Filter = 3400;
		else if (TRX.AM_LPF_Filter == 3800)
			TRX.AM_LPF_Filter = 3400;
		else if (TRX.AM_LPF_Filter == 4000)
			TRX.AM_LPF_Filter = 3800;
		else if (TRX.AM_LPF_Filter == 4500)
			TRX.AM_LPF_Filter = 3800;
		else if (TRX.AM_LPF_Filter == 5000)
			TRX.AM_LPF_Filter = 4500;
		else if (TRX.AM_LPF_Filter == 6000)
			TRX.AM_LPF_Filter = 5000;
		else if (TRX.AM_LPF_Filter == 7000)
			TRX.AM_LPF_Filter = 6000;
	}

	ReinitAudioFilters();
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_FM_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.FM_LPF_Filter == 5000)
			TRX.FM_LPF_Filter = 6000;
		else if (TRX.FM_LPF_Filter == 6000)
			TRX.FM_LPF_Filter = 7000;
		else if (TRX.FM_LPF_Filter == 7000)
			TRX.FM_LPF_Filter = 8000;
		else if (TRX.FM_LPF_Filter == 8000)
			TRX.FM_LPF_Filter = 9000;
		else if (TRX.FM_LPF_Filter == 9000)
			TRX.FM_LPF_Filter = 10000;
		else if (TRX.FM_LPF_Filter == 10000)
			TRX.FM_LPF_Filter = 15000;
		else if (TRX.FM_LPF_Filter == 15000)
			TRX.FM_LPF_Filter = 20000;
	}
	else
	{
		if (TRX.FM_LPF_Filter == 6000)
			TRX.FM_LPF_Filter = 5000;
		else if (TRX.FM_LPF_Filter == 7000)
			TRX.FM_LPF_Filter = 6000;
		else if (TRX.FM_LPF_Filter == 8000)
			TRX.FM_LPF_Filter = 7000;
		else if (TRX.FM_LPF_Filter == 9000)
			TRX.FM_LPF_Filter = 8000;
		else if (TRX.FM_LPF_Filter == 10000)
			TRX.FM_LPF_Filter = 9000;
		else if (TRX.FM_LPF_Filter == 15000)
			TRX.FM_LPF_Filter = 10000;
		else if (TRX.FM_LPF_Filter == 20000)
			TRX.FM_LPF_Filter = 5000;
	}

	ReinitAudioFilters();
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

//CW MENU

static void SYSMENU_HANDL_CWMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_cw_handlers[0];
	sysmenu_item_count_selected = &sysmenu_cw_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

void SYSMENU_CW_WPM_HOTKEY(void)
{
	sysmenu_handlers_selected = &sysmenu_cw_handlers[0];
	sysmenu_item_count_selected = &sysmenu_cw_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 4;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_CW_Decoder(int8_t direction)
{
	if (direction > 0)
		TRX.CWDecoder = true;
	if (direction < 0)
		TRX.CWDecoder = false;
}

static void SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ(int8_t direction)
{
	TRX.CW_GENERATOR_SHIFT_HZ += direction * 100;
	if (TRX.CW_GENERATOR_SHIFT_HZ < 100)
		TRX.CW_GENERATOR_SHIFT_HZ = 100;
	if (TRX.CW_GENERATOR_SHIFT_HZ > 10000)
		TRX.CW_GENERATOR_SHIFT_HZ = 10000;
}

static void SYSMENU_HANDL_CW_Key_timeout(int8_t direction)
{
	if (TRX.Key_timeout > 0 || direction > 0)
		TRX.Key_timeout += direction * 50;
	if (TRX.Key_timeout > 5000)
		TRX.Key_timeout = 5000;
}

static void SYSMENU_HANDL_CW_Keyer(int8_t direction)
{
	if (direction > 0)
		TRX.CW_KEYER = true;
	if (direction < 0)
		TRX.CW_KEYER = false;
}

static void SYSMENU_HANDL_CW_Keyer_WPM(int8_t direction)
{
	TRX.CW_KEYER_WPM += direction;
	if (TRX.CW_KEYER_WPM < 1)
		TRX.CW_KEYER_WPM = 1;
	if (TRX.CW_KEYER_WPM > 500)
		TRX.CW_KEYER_WPM = 500;
}

static void SYSMENU_HANDL_CW_SelfHear(int8_t direction)
{
	if (direction > 0)
		TRX.CW_SelfHear = true;
	if (direction < 0)
		TRX.CW_SelfHear = false;
}

//LCD MENU

static void SYSMENU_HANDL_LCDMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_lcd_handlers[0];
	sysmenu_item_count_selected = &sysmenu_lcd_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_LCD_Brightness(int8_t direction)
{
	TRX.LCD_Brightness += direction;
	if (TRX.LCD_Brightness < 1)
		TRX.LCD_Brightness = 1;
	if (TRX.LCD_Brightness > 100)
		TRX.LCD_Brightness = 100;
	LCDDriver_setBrightness(TRX.LCD_Brightness);
}

static void SYSMENU_HANDL_LCD_Standby_Time(int8_t direction)
{
	if (TRX.Standby_Time > 0 || direction > 0)
		TRX.Standby_Time += direction;
	if (TRX.Standby_Time > 250)
		TRX.Standby_Time = 250;
}

static void SYSMENU_HANDL_LCD_SMeter_Style(int8_t direction)
{
	if (direction > 0)
		TRX.S_METER_Style = true;
	if (direction < 0)
		TRX.S_METER_Style = false;
}

//FFT MENU

static void SYSMENU_HANDL_FFTMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_fft_handlers[0];
	sysmenu_item_count_selected = &sysmenu_fft_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_FFT_Enabled(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Enabled = true;
	if (direction < 0)
		TRX.FFT_Enabled = false;
}

static void SYSMENU_HANDL_FFT_Averaging(int8_t direction)
{
	TRX.FFT_Averaging += direction;
	if (TRX.FFT_Averaging < 1)
		TRX.FFT_Averaging = 1;
	if (TRX.FFT_Averaging > 10)
		TRX.FFT_Averaging = 10;
}

static void SYSMENU_HANDL_FFT_Window(int8_t direction)
{
	TRX.FFT_Window += direction;
	if (TRX.FFT_Window < 1)
		TRX.FFT_Window = 1;
	if (TRX.FFT_Window > 3)
		TRX.FFT_Window = 3;
	FFT_Init();
}

static void SYSMENU_HANDL_FFT_Zoom(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.FFT_Zoom == 1)
			TRX.FFT_Zoom = 2;
		else if (TRX.FFT_Zoom == 2)
			TRX.FFT_Zoom = 4;
		else if (TRX.FFT_Zoom == 4)
			TRX.FFT_Zoom = 8;
		else if (TRX.FFT_Zoom == 8)
			TRX.FFT_Zoom = 16;
	}
	else
	{
		if (TRX.FFT_Zoom == 2)
			TRX.FFT_Zoom = 1;
		else if (TRX.FFT_Zoom == 4)
			TRX.FFT_Zoom = 2;
		else if (TRX.FFT_Zoom == 8)
			TRX.FFT_Zoom = 4;
		else if (TRX.FFT_Zoom == 16)
			TRX.FFT_Zoom = 8;
	}
	FFT_Init();
}

static void SYSMENU_HANDL_FFT_Style(int8_t direction)
{
	TRX.FFT_Style += direction;
	if (TRX.FFT_Style < 1)
		TRX.FFT_Style = 1;
	if (TRX.FFT_Style > 4)
		TRX.FFT_Style = 4;
	FFT_Init();
}

//ADC/DAC MENU

static void SYSMENU_HANDL_ADCMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_adc_handlers[0];
	sysmenu_item_count_selected = &sysmenu_adc_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_ADC_DRIVER(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_Driver = true;
	if (direction < 0)
		TRX.ADC_Driver = false;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if(band>0)
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
	FPGA_NeedSendParams = true;
}

static void SYSMENU_HANDL_ADC_PGA(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_PGA = true;
	if (direction < 0)
		TRX.ADC_PGA = false;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if(band>0)
		TRX.BANDS_SAVED_SETTINGS[band].ADC_PGA = TRX.ADC_PGA;
	FPGA_NeedSendParams = true;
}

static void SYSMENU_HANDL_ADC_RAND(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_RAND = true;
	if (direction < 0)
		TRX.ADC_RAND = false;
	FPGA_NeedSendParams = true;
}

static void SYSMENU_HANDL_ADC_SHDN(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_SHDN = true;
	if (direction < 0)
		TRX.ADC_SHDN = false;
	FPGA_NeedSendParams = true;
}

static void SYSMENU_HANDL_ADC_DITH(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_DITH = true;
	if (direction < 0)
		TRX.ADC_DITH = false;
	FPGA_NeedSendParams = true;
}

//WIFI MENU

static void SYSMENU_HANDL_WIFIMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_wifi_handlers[0];
	sysmenu_item_count_selected = &sysmenu_wifi_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_WIFI_DrawSelectAPMenuCallback(void)
{
	sysmenu_wifi_rescan_interval = 0;
}

static void SYSMENU_WIFI_DrawSelectAPMenu(bool full_redraw)
{
	if (full_redraw || sysmenu_wifi_rescan_interval == 0)
	{
		LCDDriver_Fill(COLOR_BLACK);
		LCDDriver_printText("AP Found:", 5, 5, COLOR_WHITE, COLOR_BLACK, 2);
		for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
			LCDDriver_printText((char *)WIFI_FoundedAP[i], 10, 33 + i * 24, COLOR_GREEN, COLOR_BLACK, 2);
		LCDDriver_drawFastHLine(0, 49 + sysmenu_wifi_selected_ap_index * 24, LAY_SYSMENU_W, COLOR_WHITE);
		WIFI_ListAP(SYSMENU_WIFI_DrawSelectAPMenuCallback);
	}
	sysmenu_wifi_rescan_interval++;
	if (sysmenu_wifi_rescan_interval > 30)
		sysmenu_wifi_rescan_interval = 0;
	LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_WIFI_SelectAPMenuMove(int8_t dir)
{
	if (dir < 0 && sysmenu_wifi_selected_ap_index > 0)
		sysmenu_wifi_selected_ap_index--;
	if (dir > 0 && sysmenu_wifi_selected_ap_index < WIFI_FOUNDED_AP_MAXCOUNT)
		sysmenu_wifi_selected_ap_index++;
	SYSMENU_WIFI_DrawSelectAPMenu(true);
	if (dir == 0)
	{
		strcpy(TRX.WIFI_AP, (char *)&WIFI_FoundedAP[sysmenu_wifi_selected_ap_index]);
		WIFI_InitStateIndex = 0;
		WIFI_State = WIFI_INITED;
		sysmenu_wifi_selectap_menu_opened = false;
		systemMenuIndex = 0;
		drawSystemMenu(true);
	}
}

static void SYSMENU_WIFI_DrawAPpasswordMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(COLOR_BLACK);
		LCDDriver_printText("AP Password:", 5, 5, COLOR_WHITE, COLOR_BLACK, 2);
	}

	LCDDriver_printText(TRX.WIFI_PASSWORD, 10, 37, COLOR_GREEN, COLOR_BLACK, 2);
	LCDDriver_drawFastHLine(8 + sysmenu_wifi_selected_ap_password_char_index * 12, 54, 12, COLOR_RED);
}

static void SYSMENU_WIFI_RotatePasswordChar(int8_t dir)
{
	bool full_redraw = false;
	if (TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;
	TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] += dir;

	//не показываем спецсимволы
	if (TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] >= 1 && TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir > 0)
		TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] = 33;
	if (TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] >= 1 && TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir < 0)
		TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] >= 127)
		TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (TRX.WIFI_PASSWORD[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		SYSMENU_WIFI_DrawAPpasswordMenu(true);
	else
		SYSMENU_WIFI_DrawAPpasswordMenu(false);
}

static void SYSMENU_HANDL_WIFI_Enabled(int8_t direction)
{
	if (direction > 0)
		TRX.WIFI_Enabled = true;
	if (direction < 0)
		TRX.WIFI_Enabled = false;
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
	if (TRX.WIFI_TIMEZONE < -12)
		TRX.WIFI_TIMEZONE = -12;
	if (TRX.WIFI_TIMEZONE > 12)
		TRX.WIFI_TIMEZONE = 12;
	WIFI_State = WIFI_INITED;
}

//SET TIME MENU

static void SYSMENU_HANDL_SETTIME(int8_t direction)
{
	if (!LCD_timeMenuOpened)
		LCDDriver_Fill(COLOR_BLACK);
	LCD_timeMenuOpened = true;
	static uint8_t Hours;
	static uint8_t Minutes;
	static uint8_t Seconds;
	static uint32_t Time;
	char ctmp[50];
	Time = RTC->TR;
	Hours = ((Time >> 20) & 0x03) * 10 + ((Time >> 16) & 0x0f);
	Minutes = ((Time >> 12) & 0x07) * 10 + ((Time >> 8) & 0x0f);
	Seconds = ((Time >> 4) & 0x07) * 10 + ((Time >> 0) & 0x0f);
	sprintf(ctmp, "%d", Hours);
	addSymbols(ctmp, ctmp, 2, "0", false);
	LCDDriver_printText(ctmp, 76, 100, COLOR_BUTTON_TEXT, TimeMenuSelection == 0 ? COLOR_WHITE : COLOR_BLACK, 3);
	LCDDriver_printText(":", 124, 100, COLOR_BUTTON_TEXT, COLOR_BLACK, 3);
	sprintf(ctmp, "%d", Minutes);
	addSymbols(ctmp, ctmp, 2, "0", false);
	LCDDriver_printText(ctmp, 148, 100, COLOR_BUTTON_TEXT, TimeMenuSelection == 1 ? COLOR_WHITE : COLOR_BLACK, 3);
	LCDDriver_printText(":", 194, 100, COLOR_BUTTON_TEXT, COLOR_BLACK, 3);
	sprintf(ctmp, "%d", Seconds);
	addSymbols(ctmp, ctmp, 2, "0", false);
	LCDDriver_printText(ctmp, 220, 100, COLOR_BUTTON_TEXT, TimeMenuSelection == 2 ? COLOR_WHITE : COLOR_BLACK, 3);
}

//FLASH MENU

static void SYSMENU_HANDL_Bootloader(int8_t direction)
{
	WM8731_CleanBuffer();
	JumpToBootloader();
}

//SPECTRUM MENU

static void SYSMENU_HANDL_SPECTRUMMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_spectrum_handlers[0];
	sysmenu_item_count_selected = &sysmenu_spectrum_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_SPECTRUM_Begin(int8_t direction)
{
	TRX.SPEC_Begin += direction;
	if (TRX.SPEC_Begin < 1)
		TRX.SPEC_Begin = 1;
}

static void SYSMENU_HANDL_SPECTRUM_End(int8_t direction)
{
	TRX.SPEC_End += direction;
	if (TRX.SPEC_End < 1)
		TRX.SPEC_End = 1;
}

static void SYSMENU_HANDL_SPECTRUM_Start(int8_t direction)
{
	sysmenu_spectrum_lastfreq = CurrentVFO()->Freq;
	sysmenu_spectrum_opened = true;
	SPEC_Start();
	drawSystemMenu(true);
}

//CALIBRATION MENU

static void SYSMENU_HANDL_CALIBRATIONMENU(int8_t direction)
{
	sysmenu_handlers_selected = &sysmenu_calibration_handlers[0];
	sysmenu_item_count_selected = &sysmenu_calibration_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	drawSystemMenu(true);
}

static void SYSMENU_HANDL_CALIB_CIC_SHIFT(int8_t direction)
{
	CALIBRATE.CIC_GAINER_val += direction;
	if (CALIBRATE.CIC_GAINER_val < 32)
		CALIBRATE.CIC_GAINER_val = 32;
	if (CALIBRATE.CIC_GAINER_val > 88)
		CALIBRATE.CIC_GAINER_val = 88;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_CICCOMP_SHIFT(int8_t direction)
{
	CALIBRATE.CICFIR_GAINER_val += direction;
	if (CALIBRATE.CICFIR_GAINER_val < 32)
		CALIBRATE.CICFIR_GAINER_val = 32;
	if (CALIBRATE.CICFIR_GAINER_val > 64)
		CALIBRATE.CICFIR_GAINER_val = 64;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT(int8_t direction)
{
	CALIBRATE.TXCICFIR_GAINER_val += direction;
	if (CALIBRATE.TXCICFIR_GAINER_val < 16)
		CALIBRATE.TXCICFIR_GAINER_val = 16;
	if (CALIBRATE.TXCICFIR_GAINER_val > 64)
		CALIBRATE.TXCICFIR_GAINER_val = 64;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_DAC_SHIFT(int8_t direction)
{
	CALIBRATE.DAC_GAINER_val += direction;
	if (CALIBRATE.DAC_GAINER_val < 14)
		CALIBRATE.DAC_GAINER_val = 14;
	if (CALIBRATE.DAC_GAINER_val > 32)
		CALIBRATE.DAC_GAINER_val = 32;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN(uint8_t id, int8_t direction)
{
	if (CALIBRATE.rf_out_power[id] > 0)
		CALIBRATE.rf_out_power[id] += direction;
	if (CALIBRATE.rf_out_power[id] == 0 && direction > 0)
		CALIBRATE.rf_out_power[id] += direction;
	if (CALIBRATE.rf_out_power[id] > 100)
		CALIBRATE.rf_out_power[id] = 100;
	
	if(id==0)
		TRX_setFrequency(500000, CurrentVFO());
	else
		TRX_setFrequency(id*1000000, CurrentVFO());
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_0(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(0, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_1(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(1, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_2(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(2, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_3(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(3, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_4(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(4, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_5(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(5, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_6(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(6, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_7(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(7, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_8(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(8, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_9(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(9, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_10(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(10, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_11(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(11, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_12(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(12, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_13(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(13, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_14(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(14, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_15(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(15, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_16(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(16, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_17(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(17, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_18(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(18, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_19(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(19, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_20(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(20, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_21(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(21, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_22(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(22, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_23(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(23, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_24(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(24, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_25(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(25, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_26(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(26, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_27(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(27, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_28(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(28, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_29(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(29, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_30(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(30, direction);
}
static void SYSMENU_HANDL_CALIB_RF_GAIN_31(int8_t direction)
{
	SYSMENU_HANDL_CALIB_RF_GAIN(31, direction);
}


static void SYSMENU_HANDL_CALIB_S_METER(int8_t direction)
{
	CALIBRATE.smeter_calibration += direction;
	if (CALIBRATE.smeter_calibration < -50)
		CALIBRATE.smeter_calibration = -50;
	if (CALIBRATE.smeter_calibration > 50)
		CALIBRATE.smeter_calibration = 50;
	NeedSaveCalibration = true;
}
	
static void SYSMENU_HANDL_CALIB_ADC_OFFSET(int8_t direction)
{
	CALIBRATE.adc_offset += direction;
	if (CALIBRATE.adc_offset < -500)
		CALIBRATE.adc_offset = -500;
	if (CALIBRATE.adc_offset > 500)
		CALIBRATE.adc_offset = 500;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_ATT_DB(int8_t direction)
{
	CALIBRATE.att_db += direction;
	if (CALIBRATE.att_db < -50)
		CALIBRATE.att_db = -50;
	if (CALIBRATE.att_db > 0)
		CALIBRATE.att_db = 0;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_LNA_GAIN(int8_t direction)
{
	CALIBRATE.lna_gain_db += direction;
	if (CALIBRATE.lna_gain_db < 0)
		CALIBRATE.lna_gain_db = 0;
	if (CALIBRATE.lna_gain_db > 100)
		CALIBRATE.lna_gain_db = 100;
	NeedSaveCalibration = true;
}

#pragma GCC diagnostic ignored "-Wsign-conversion"
static void SYSMENU_HANDL_CALIB_LPF_END(int8_t direction)
{
	CALIBRATE.LPF_END += direction*100000;
	if (CALIBRATE.LPF_END < 1)
		CALIBRATE.LPF_END = 1;
	if (CALIBRATE.LPF_END > 999999999)
		CALIBRATE.LPF_END = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_0_START(int8_t direction)
{
	CALIBRATE.BPF_0_START += direction*100000;
	if (CALIBRATE.BPF_0_START < 1)
		CALIBRATE.BPF_0_START = 1;
	if (CALIBRATE.BPF_0_START > 999999999)
		CALIBRATE.BPF_0_START = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_0_END(int8_t direction)
{
	CALIBRATE.BPF_0_END += direction*100000;
	if (CALIBRATE.BPF_0_END < 1)
		CALIBRATE.BPF_0_END = 1;
	if (CALIBRATE.BPF_0_END > 999999999)
		CALIBRATE.BPF_0_END = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_1_START(int8_t direction)
{
	CALIBRATE.BPF_1_START += direction*100000;
	if (CALIBRATE.BPF_1_START < 1)
		CALIBRATE.BPF_1_START = 1;
	if (CALIBRATE.BPF_1_START > 999999999)
		CALIBRATE.BPF_1_START = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_1_END(int8_t direction)
{
	CALIBRATE.BPF_1_END += direction*100000;
	if (CALIBRATE.BPF_1_END < 1)
		CALIBRATE.BPF_1_END = 1;
	if (CALIBRATE.BPF_1_END > 999999999)
		CALIBRATE.BPF_1_END = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_2_START(int8_t direction)
{
	CALIBRATE.BPF_2_START += direction*100000;
	if (CALIBRATE.BPF_2_START < 1)
		CALIBRATE.BPF_2_START = 1;
	if (CALIBRATE.BPF_2_START > 999999999)
		CALIBRATE.BPF_2_START = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_2_END(int8_t direction)
{
	CALIBRATE.BPF_2_END += direction*100000;
	if (CALIBRATE.BPF_2_END < 1)
		CALIBRATE.BPF_2_END = 1;
	if (CALIBRATE.BPF_2_END > 999999999)
		CALIBRATE.BPF_2_END = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_3_START(int8_t direction)
{
	CALIBRATE.BPF_3_START += direction*100000;
	if (CALIBRATE.BPF_3_START < 1)
		CALIBRATE.BPF_3_START = 1;
	if (CALIBRATE.BPF_3_START > 999999999)
		CALIBRATE.BPF_3_START = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_3_END(int8_t direction)
{
	CALIBRATE.BPF_3_END += direction*100000;
	if (CALIBRATE.BPF_3_END < 1)
		CALIBRATE.BPF_3_END = 1;
	if (CALIBRATE.BPF_3_END > 999999999)
		CALIBRATE.BPF_3_END = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_4_START(int8_t direction)
{
	CALIBRATE.BPF_4_START += direction*100000;
	if (CALIBRATE.BPF_4_START < 1)
		CALIBRATE.BPF_4_START = 1;
	if (CALIBRATE.BPF_4_START > 999999999)
		CALIBRATE.BPF_4_START = 999999999;
	NeedSaveCalibration = true;
}


static void SYSMENU_HANDL_CALIB_BPF_4_END(int8_t direction)
{
	CALIBRATE.BPF_4_END += direction*100000;
	if (CALIBRATE.BPF_4_END < 1)
		CALIBRATE.BPF_4_END = 1;
	if (CALIBRATE.BPF_4_END > 999999999)
		CALIBRATE.BPF_4_END = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_5_START(int8_t direction)
{
	CALIBRATE.BPF_5_START += direction*100000;
	if (CALIBRATE.BPF_5_START < 1)
		CALIBRATE.BPF_5_START = 1;
	if (CALIBRATE.BPF_5_START > 999999999)
		CALIBRATE.BPF_5_START = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_5_END(int8_t direction)
{
	CALIBRATE.BPF_5_END += direction*100000;
	if (CALIBRATE.BPF_5_END < 1)
		CALIBRATE.BPF_5_END = 1;
	if (CALIBRATE.BPF_5_END > 999999999)
		CALIBRATE.BPF_5_END = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_6_START(int8_t direction)
{
	CALIBRATE.BPF_6_START += direction*100000;
	if (CALIBRATE.BPF_6_START < 1)
		CALIBRATE.BPF_6_START = 1;
	if (CALIBRATE.BPF_6_START > 999999999)
		CALIBRATE.BPF_6_START = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_BPF_6_END(int8_t direction)
{
	CALIBRATE.BPF_6_END += direction*100000;
	if (CALIBRATE.BPF_6_END < 1)
		CALIBRATE.BPF_6_END = 1;
	if (CALIBRATE.BPF_6_END > 999999999)
		CALIBRATE.BPF_6_END = 999999999;
	NeedSaveCalibration = true;
}

static void SYSMENU_HANDL_CALIB_HPF_START(int8_t direction)
{
	CALIBRATE.BPF_7_HPF += direction*100000;
	if (CALIBRATE.BPF_7_HPF < 1)
		CALIBRATE.BPF_7_HPF = 1;
	if (CALIBRATE.BPF_7_HPF > 999999999)
		CALIBRATE.BPF_7_HPF = 999999999;
	NeedSaveCalibration = true;
}
#pragma GCC diagnostic pop

static void SYSMENU_HANDL_CALIB_SWR_TRANS_RATE(int8_t direction)
{
	CALIBRATE.swr_trans_rate += (float32_t)direction*0.1f;
	if (CALIBRATE.swr_trans_rate < 1.0f)
		CALIBRATE.swr_trans_rate = 1.0f;
	if (CALIBRATE.swr_trans_rate > 50.0f)
		CALIBRATE.swr_trans_rate = 50.0f;
	CALIBRATE.swr_trans_rate_shadow = (int32_t)(roundf(CALIBRATE.swr_trans_rate * 100.0f));
	NeedSaveCalibration = true;
}

//COMMON MENU FUNCTIONS
void drawSystemMenu(bool draw_background)
{
	if (LCD_busy)
	{
		LCD_UpdateQuery.SystemMenu = true;
		return;
	}
	if (!LCD_systemMenuOpened)
		return;
	if (LCD_timeMenuOpened)
	{
		SYSMENU_HANDL_SETTIME(0);
		return;
	}
	if (sysmenu_wifi_selectap_menu_opened)
	{
		SYSMENU_WIFI_DrawSelectAPMenu(false);
		return;
	}
	if (sysmenu_wifi_setAPpassword_menu_opened)
	{
		SYSMENU_WIFI_DrawAPpasswordMenu(false);
		return;
	}
	if (sysmenu_spectrum_opened)
	{
		SPEC_Draw();
		return;
	}
	LCD_busy = true;

	sysmenu_i = 0;
	sysmenu_y = 5;

	if (draw_background)
		LCDDriver_Fill(COLOR_BLACK);

	uint8_t current_selected_page = systemMenuIndex / max_items_on_page;
	if(current_selected_page * max_items_on_page > *sysmenu_item_count_selected)
		current_selected_page = 0;
	
	for (uint8_t m = 0; m < *sysmenu_item_count_selected; m++)
	{
		uint8_t current_page = m / max_items_on_page;
		if(current_selected_page == current_page)
			drawSystemMenuElement(sysmenu_handlers_selected[m].title, sysmenu_handlers_selected[m].type, sysmenu_handlers_selected[m].value, false);
	}
	
	LCD_UpdateQuery.SystemMenu = false;
	LCD_busy = false;
}

void eventRotateSystemMenu(int8_t direction)
{
	if (sysmenu_wifi_selectap_menu_opened)
	{
		SYSMENU_WIFI_SelectAPMenuMove(0);
		return;
	}
	if (sysmenu_wifi_setAPpassword_menu_opened)
	{
		SYSMENU_WIFI_RotatePasswordChar(direction);
		return;
	}
	sysmenu_handlers_selected[systemMenuIndex].menuHandler(direction);
	if (sysmenu_handlers_selected[systemMenuIndex].type != SYSMENU_RUN)
		redrawCurrentItem();
}

void eventCloseSystemMenu(void)
{
	if (sysmenu_wifi_selectap_menu_opened)
	{
		sysmenu_wifi_selectap_menu_opened = false;
		systemMenuIndex = 0;
		drawSystemMenu(true);
		WIFI_InitStateIndex = 0;
		WIFI_State = WIFI_INITED;
	}
	else if (sysmenu_wifi_setAPpassword_menu_opened)
	{
		sysmenu_wifi_setAPpassword_menu_opened = false;
		systemMenuIndex = 0;
		drawSystemMenu(true);
		WIFI_InitStateIndex = 0;
		WIFI_State = WIFI_INITED;
	}
	else if (sysmenu_spectrum_opened)
	{
		sysmenu_spectrum_opened = false;
		systemMenuIndex = 0;
		drawSystemMenu(true);
	}
	else
	{
		if (sysmenu_onroot)
		{
			LCD_systemMenuOpened = false;
			LCD_UpdateQuery.Background = true;
			LCD_redraw();
		}
		else
		{
			sysmenu_handlers_selected = &sysmenu_handlers[0];
			sysmenu_item_count_selected = &sysmenu_item_count;
			sysmenu_onroot = true;
			systemMenuIndex = systemMenuRootIndex;
			drawSystemMenu(true);
		}
	}
	NeedSaveSettings = true;
}

void eventSecRotateSystemMenu(int8_t direction)
{
	//wifi select AP menu
	if (sysmenu_wifi_selectap_menu_opened)
	{
		if (direction < 0)
			SYSMENU_WIFI_SelectAPMenuMove(-1);
		else
			SYSMENU_WIFI_SelectAPMenuMove(1);
		return;
	}
	//wifi set password menu
	if (sysmenu_wifi_setAPpassword_menu_opened)
	{
		if (direction < 0 && sysmenu_wifi_selected_ap_password_char_index > 0)
		{
			sysmenu_wifi_selected_ap_password_char_index--;
			SYSMENU_WIFI_DrawAPpasswordMenu(true);
		}
		else if (sysmenu_wifi_selected_ap_password_char_index < (MAX_WIFIPASS_LENGTH - 1))
		{
			sysmenu_wifi_selected_ap_password_char_index++;
			SYSMENU_WIFI_DrawAPpasswordMenu(true);
		}
		return;
	}
	//spectrum analyzer
	if (sysmenu_spectrum_opened)
	{
		sysmenu_spectrum_opened = false;
		LCDDriver_Fill(COLOR_BLACK);
		drawSystemMenu(true);
		TRX_setFrequency(sysmenu_spectrum_lastfreq, CurrentVFO());
		return;
	}
	//time menu
	if (LCD_timeMenuOpened)
	{
		LCDDriver_Fill(COLOR_BLACK);
		if (direction < 0)
		{
			TimeMenuSelection--;
			if (TimeMenuSelection > 2)
				TimeMenuSelection = 2;
		}
		else
		{
			TimeMenuSelection++;
			if (TimeMenuSelection == 3)
				TimeMenuSelection = 0;
		}
		LCD_UpdateQuery.SystemMenu = true;
		return;
	}
	//other
	uint8_t current_page = systemMenuIndex / max_items_on_page;
	LCDDriver_drawFastHLine(0, (5 + (systemMenuIndex - current_page * max_items_on_page) * LAY_SYSMENU_ITEM_HEIGHT) + 17, LAY_SYSMENU_W, COLOR_BLACK);
	if (direction < 0)
	{
		if (systemMenuIndex > 0)
			systemMenuIndex--;
		else
			systemMenuIndex = *sysmenu_item_count_selected - 1;
	}
	else
	{
		if (systemMenuIndex < (*sysmenu_item_count_selected - 1))
		{
			systemMenuIndex++;
		}
		else
			systemMenuIndex = 0;
	}
	redrawCurrentItem();
	if(sysmenu_onroot)
		systemMenuRootIndex = systemMenuIndex;
	uint8_t new_page = systemMenuIndex / max_items_on_page;

	if(current_page!=new_page)
		drawSystemMenu(true);
}

static void redrawCurrentItem(void)
{
	uint8_t current_page = systemMenuIndex / max_items_on_page;
	sysmenu_i = (systemMenuIndex - current_page * max_items_on_page);
	sysmenu_y = 5 + (systemMenuIndex - current_page * max_items_on_page) * LAY_SYSMENU_ITEM_HEIGHT;
	drawSystemMenuElement(sysmenu_handlers_selected[systemMenuIndex].title, sysmenu_handlers_selected[systemMenuIndex].type, sysmenu_handlers_selected[systemMenuIndex].value, true);
}

static void drawSystemMenuElement(char *title, SystemMenuType type, uint32_t *value, bool onlyVal)
{
	if(!sysmenu_hiddenmenu_enabled && type==SYSMENU_HIDDEN_MENU)
		return;
	
	char ctmp[10];
	if (!onlyVal)
	{
		LCDDriver_Fill_RectXY(0, sysmenu_y, LAY_SYSMENU_W, sysmenu_y + 17, COLOR_BLACK);
		LCDDriver_printText(title, LAY_SYSMENU_X1, sysmenu_y, COLOR_WHITE, COLOR_BLACK, 2);
	}
	
	uint16_t x_pos = LAY_SYSMENU_X2;
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
		x_pos = LAY_SYSMENU_X2_BIGINT;
		break;
	case SYSMENU_INT8:
		sprintf(ctmp, "%d", (int8_t)*value);
		break;
	case SYSMENU_INT16:
		sprintf(ctmp, "%d", (int16_t)*value);
		break;
	case SYSMENU_INT32:
		sprintf(ctmp, "%d", (int32_t)*value);
		x_pos = LAY_SYSMENU_X2_BIGINT;
		break;
	case SYSMENU_FLOAT32:
		sprintf(ctmp, "%.2f", (double)((float32_t)((int32_t)*value)/100.0f));
		x_pos = LAY_SYSMENU_X2_BIGINT;
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
	case SYSMENU_HIDDEN_MENU:
		sprintf(ctmp, "!!!");
		break;
	}
	
	if (onlyVal)
		LCDDriver_Fill_RectWH(x_pos, sysmenu_y, 5 * 12, 13, COLOR_BLACK);
	LCDDriver_printText(ctmp, x_pos, sysmenu_y, COLOR_WHITE, COLOR_BLACK, 2);
	
	uint8_t current_selected_page = systemMenuIndex / max_items_on_page;
	if (systemMenuIndex == sysmenu_i + current_selected_page * max_items_on_page)
		LCDDriver_drawFastHLine(0, sysmenu_y + 17, LAY_SYSMENU_W, COLOR_WHITE);
	sysmenu_i++;
	sysmenu_y += LAY_SYSMENU_ITEM_HEIGHT;
}
