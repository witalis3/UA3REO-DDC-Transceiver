#include "system_menu.h"
#include "lcd.h"
#include "settings.h"
#include "audio_filters.h"
#include "bootloader.h"
#include "functions.h"
#include "wifi.h"
#include "spec_analyzer.h"
#include "swr_analyzer.h"
#include "fonts.h"
#include "agc.h"
#include "screen_layout.h"
#include "noise_blanker.h"
#include "bands.h"
#include "sd.h"
#include "wspr.h"
#include "stm32h7xx_it.h"
#include "usbd_ua3reo.h"
#include "filemanager.h"

static void SYSMENU_HANDL_TRX_RFPower(int8_t direction);
static void SYSMENU_HANDL_TRX_BandMap(int8_t direction);
static void SYSMENU_HANDL_TRX_AutoGain(int8_t direction);
static void SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE(int8_t direction);
static void SYSMENU_HANDL_TRX_RFFilters(int8_t direction);
static void SYSMENU_HANDL_TRX_INPUT_TYPE(int8_t direction);
static void SYSMENU_HANDL_TRX_SHIFT_INTERVAL(int8_t direction);
static void SYSMENU_HANDL_TRX_SAMPLERATE_MAIN(int8_t direction);
static void SYSMENU_HANDL_TRX_SAMPLERATE_WFM(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_FAST_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_ENC_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_ENC_FAST_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_ENC_ACCELERATE(int8_t direction);
static void SYSMENU_HANDL_TRX_ATT_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_DEBUG_TYPE(int8_t direction);
static void SYSMENU_HANDL_TRX_SetCallsign(int8_t direction);
static void SYSMENU_HANDL_TRX_SetLocator(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_ENABLE(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_OFFSET(int8_t direction);

static void SYSMENU_HANDL_AUDIO_IFGain(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AGC_GAIN_TARGET(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_Gain(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_THRES(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_AVERAGE(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_MINMAL(int8_t direction);
static void SYSMENU_HANDL_AUDIO_SSB_HPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_SSB_LPF_RX_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_SSB_LPF_TX_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_CW_LPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_CW_HPF_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AM_LPF_RX_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AM_LPF_TX_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FM_LPF_RX_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FM_LPF_TX_pass(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_LOW(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_MID(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_HIG(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_REVERBER(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_EQ_LOW(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_EQ_MID(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_EQ_HIG(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_AGC_SSB_Speed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_AGC_CW_Speed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_CompressorSpeed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_CompressorMaxGain(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FMSquelch(int8_t direction);
static void SYSMENU_HANDL_AUDIO_VAD_Squelch(int8_t direction);
static void SYSMENU_HANDL_AUDIO_Beeper(int8_t direction);

static void SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ(int8_t direction);
static void SYSMENU_HANDL_CW_Decoder(int8_t direction);
static void SYSMENU_HANDL_CW_SelfHear(int8_t direction);
static void SYSMENU_HANDL_CW_Keyer(int8_t direction);
static void SYSMENU_HANDL_CW_Keyer_WPM(int8_t direction);
static void SYSMENU_HANDL_CW_Key_timeout(int8_t direction);
static void SYSMENU_HANDL_CW_GaussFilter(int8_t direction);

static void SYSMENU_HANDL_SCREEN_FFT_Enabled(int8_t direction);
static void SYSMENU_HANDL_SCREEN_COLOR_THEME(int8_t direction);
static void SYSMENU_HANDL_SCREEN_LAYOUT_THEME(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Averaging(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Window(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Zoom(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_ZoomCW(int8_t direction);
static void SYSMENU_HANDL_SCREEN_LCD_Brightness(int8_t direction);
static void SYSMENU_HANDL_SCREEN_WTF_Moving(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Height(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Style(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Color(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Grid(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Background(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Speed(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Sensitivity(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Compressor(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Lens(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_HoldPeaks(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_3D(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Automatic(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_ManualBottom(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_ManualTop(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON1(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON2(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON3(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON4(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON5(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON6(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON7(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON8(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON9(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON10(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON11(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON12(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON13(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON14(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON15(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON16(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON17(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON18(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON19(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON20(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON21(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON22(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON23(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON24(int8_t direction);

static void SYSMENU_HANDL_ADC_PGA(int8_t direction);
static void SYSMENU_HANDL_ADC_RAND(int8_t direction);
static void SYSMENU_HANDL_ADC_SHDN(int8_t direction);
static void SYSMENU_HANDL_ADC_DITH(int8_t direction);
static void SYSMENU_HANDL_ADC_DRIVER(int8_t direction);

static void SYSMENU_HANDL_WIFI_Enabled(int8_t direction);
static void SYSMENU_HANDL_WIFI_SelectAP1(int8_t direction);
static void SYSMENU_HANDL_WIFI_SetAP1password(int8_t direction);
static void SYSMENU_HANDL_WIFI_SelectAP2(int8_t direction);
static void SYSMENU_HANDL_WIFI_SetAP2password(int8_t direction);
static void SYSMENU_HANDL_WIFI_SelectAP3(int8_t direction);
static void SYSMENU_HANDL_WIFI_SetAP3password(int8_t direction);
static void SYSMENU_HANDL_WIFI_Timezone(int8_t direction);
static void SYSMENU_HANDL_WIFI_CAT_Server(int8_t direction);
static void SYSMENU_HANDL_WIFI_UpdateFW(int8_t direction);

static void SYSMENU_HANDL_SD_Format(int8_t direction);
static void SYSMENU_HANDL_SD_ExportSettings(int8_t direction);
static void SYSMENU_HANDL_SD_ImportSettings(int8_t direction);
static void SYSMENU_HANDL_SD_USB(int8_t direction);

static void SYSMENU_HANDL_SETTIME(int8_t direction);
static void SYSMENU_HANDL_Bootloader(int8_t direction);
static void SYSMENU_HANDL_SYSINFO(int8_t direction);

static void SYSMENU_HANDL_CALIB_ENCODER_SLOW_RATE(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_INVERT(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER2_INVERT(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_DEBOUNCE(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER2_DEBOUNCE(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_ON_FALLING(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_ACCELERATION(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_48K_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_96K_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_192K_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_384K_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_DAC_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_2200M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_160M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_80M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_40M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_30M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_20M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_17M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_15M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_12M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_10M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_6M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_2M(int8_t direction);
static void SYSMENU_HANDL_CALIB_S_METER(int8_t direction);
static void SYSMENU_HANDL_CALIB_ADC_OFFSET(int8_t direction);
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
static void SYSMENU_HANDL_CALIB_SWR_FWD_RATE(int8_t direction);
static void SYSMENU_HANDL_CALIB_SWR_REF_RATE(int8_t direction);
static void SYSMENU_HANDL_CALIB_VCXO(int8_t direction);
static void SYSMENU_HANDL_CALIB_FW_AD8307_SLP(int8_t direction);  //Tisho
static void SYSMENU_HANDL_CALIB_FW_AD8307_OFFS(int8_t direction); //Tisho
static void SYSMENU_HANDL_CALIB_BW_AD8307_SLP(int8_t direction);  //Tisho
static void SYSMENU_HANDL_CALIB_BW_AD8307_OFFS(int8_t direction); //Tisho
static void SYSMENU_HANDL_CALIB_FAN_MEDIUM_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_FAN_MEDIUM_STOP(int8_t direction);
static void SYSMENU_HANDL_CALIB_FAN_FULL_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_TRX_MAX_RF_TEMP(int8_t direction);
static void SYSMENU_HANDL_CALIB_TRX_MAX_SWR(int8_t direction);
static void SYSMENU_HANDL_CALIB_FM_DEVIATION_SCALE(int8_t direction);
static void SYSMENU_HANDL_CALIB_TUNE_MAX_POWER(int8_t direction);

static void SYSMENU_HANDL_SPECTRUM_Begin(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_Start(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_End(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_TopDBM(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_BottomDBM(int8_t direction);

static void SYSMENU_HANDL_WSPR_Start(int8_t direction);
static void SYSMENU_HANDL_WSPR_FREQ_OFFSET(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND160(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND80(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND40(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND30(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND20(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND17(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND15(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND12(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND10(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND6(int8_t direction);
static void SYSMENU_HANDL_WSPR_BAND2(int8_t direction);

static void SYSMENU_HANDL_TRXMENU(int8_t direction);
static void SYSMENU_HANDL_AUDIOMENU(int8_t direction);
static void SYSMENU_HANDL_CWMENU(int8_t direction);
static void SYSMENU_HANDL_LCDMENU(int8_t direction);
static void SYSMENU_HANDL_ADCMENU(int8_t direction);
static void SYSMENU_HANDL_WIFIMENU(int8_t direction);
static void SYSMENU_HANDL_SDMENU(int8_t direction);
static void SYSMENU_HANDL_CALIBRATIONMENU(int8_t direction);

static void SYSMENU_HANDL_SPECTRUMMENU(int8_t direction);
static void SYSMENU_HANDL_SWR_BAND_START(int8_t direction);
static void SYSMENU_HANDL_SWR_HF_START(int8_t direction);
static void SYSMENU_HANDL_RDA_STATS(int8_t direction);
static void SYSMENU_HANDL_PROPAGINATION(int8_t direction);
static void SYSMENU_HANDL_WSPRMENU(int8_t direction);
static void SYSMENU_HANDL_FILEMANAGER(int8_t direction);
static void SYSMENU_HANDL_SWR_Tandem_Ctrl(int8_t direction); //Tisho

const static struct sysmenu_item_handler sysmenu_handlers[] =
	{
		{"TRX Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_TRXMENU},
		{"AUDIO Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_AUDIOMENU},
		{"CW Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_CWMENU},
		{"SCREEN Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_LCDMENU},
		{"ADC/DAC Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_ADCMENU},
		{"WIFI Settings", SYSMENU_MENU, 0, SYSMENU_HANDL_WIFIMENU},
		{"SD Card", SYSMENU_MENU, 0, SYSMENU_HANDL_SDMENU},
		{"Set Clock Time", SYSMENU_RUN, 0, SYSMENU_HANDL_SETTIME},
		{"Flash update", SYSMENU_RUN, 0, SYSMENU_HANDL_Bootloader},
		{"System info", SYSMENU_RUN, 0, SYSMENU_HANDL_SYSINFO},
		{"Calibration", SYSMENU_HIDDEN_MENU, 0, SYSMENU_HANDL_CALIBRATIONMENU},
};
const static uint8_t sysmenu_item_count = sizeof(sysmenu_handlers) / sizeof(sysmenu_handlers[0]);

const static struct sysmenu_item_handler sysmenu_trx_handlers[] =
	{
		{"RF Power", SYSMENU_UINT8, (uint32_t *)&TRX.RF_Power, SYSMENU_HANDL_TRX_RFPower},
		{"Band Map", SYSMENU_BOOLEAN, (uint32_t *)&TRX.BandMapEnabled, SYSMENU_HANDL_TRX_BandMap},
		{"AutoGainer", SYSMENU_BOOLEAN, (uint32_t *)&TRX.AutoGain, SYSMENU_HANDL_TRX_AutoGain},
		{"RF_Filters", SYSMENU_BOOLEAN, (uint32_t *)&TRX.RF_Filters, SYSMENU_HANDL_TRX_RFFilters},
		{"Two Signal TUNE", SYSMENU_BOOLEAN, (uint32_t *)&TRX.TWO_SIGNAL_TUNE, SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE},
		{"Shift Interval", SYSMENU_UINT16, (uint32_t *)&TRX.SHIFT_INTERVAL, SYSMENU_HANDL_TRX_SHIFT_INTERVAL},
		{"TRX Samplerate", SYSMENU_ENUM, (uint32_t *)&TRX.SAMPLERATE_MAIN, SYSMENU_HANDL_TRX_SAMPLERATE_MAIN, {"48khz", "96khz", "192khz", "384khz"}},
		{"WFM Samplerate", SYSMENU_ENUM, (uint32_t *)&TRX.SAMPLERATE_WFM, SYSMENU_HANDL_TRX_SAMPLERATE_WFM, {"48khz", "96khz", "192khz", "384khz"}},
		{"Freq Step", SYSMENU_UINT16, (uint32_t *)&TRX.FRQ_STEP, SYSMENU_HANDL_TRX_FRQ_STEP},
		{"Freq Step FAST", SYSMENU_UINT16, (uint32_t *)&TRX.FRQ_FAST_STEP, SYSMENU_HANDL_TRX_FRQ_FAST_STEP},
		{"Freq Step ENC2", SYSMENU_UINT16, (uint32_t *)&TRX.FRQ_ENC_STEP, SYSMENU_HANDL_TRX_FRQ_ENC_STEP},
		{"Freq Step ENC2 FAST", SYSMENU_UINT32R, (uint32_t *)&TRX.FRQ_ENC_FAST_STEP, SYSMENU_HANDL_TRX_FRQ_ENC_FAST_STEP},
		{"Encoder Accelerate", SYSMENU_BOOLEAN, (uint32_t *)&TRX.Encoder_Accelerate, SYSMENU_HANDL_TRX_ENC_ACCELERATE},
		{"Att step, dB", SYSMENU_UINT8, (uint32_t *)&TRX.ATT_STEP, SYSMENU_HANDL_TRX_ATT_STEP},
		{"DEBUG Console", SYSMENU_ENUM, (uint32_t *)&TRX.Debug_Type, SYSMENU_HANDL_TRX_DEBUG_TYPE, {"OFF", "SYSTEM", "WIFI", "BUTTONS", "TOUCH"}},
		{"Input Type", SYSMENU_ENUM, (uint32_t *)&TRX.InputType, SYSMENU_HANDL_TRX_INPUT_TYPE, {"MIC", "LINE", "USB"}},
		{"Callsign", SYSMENU_RUN, 0, SYSMENU_HANDL_TRX_SetCallsign},
		{"Locator", SYSMENU_RUN, 0, SYSMENU_HANDL_TRX_SetLocator},
		{"Transverter Enable", SYSMENU_BOOLEAN, (uint32_t *)&TRX.Transverter_Enabled, SYSMENU_HANDL_TRX_TRANSV_ENABLE},
		{"Transverter Offset, mHz", SYSMENU_UINT16, (uint32_t *)&TRX.Transverter_Offset_Mhz, SYSMENU_HANDL_TRX_TRANSV_OFFSET},
};
const static uint8_t sysmenu_trx_item_count = sizeof(sysmenu_trx_handlers) / sizeof(sysmenu_trx_handlers[0]);

const static struct sysmenu_item_handler sysmenu_audio_handlers[] =
	{
		{"IF Gain, dB", SYSMENU_UINT8, (uint32_t *)&TRX.IF_Gain, SYSMENU_HANDL_AUDIO_IFGain},
		{"AGC Gain target, LKFS", SYSMENU_INT8, (uint32_t *)&TRX.AGC_GAIN_TARGET, SYSMENU_HANDL_AUDIO_AGC_GAIN_TARGET},
		{"Mic Gain", SYSMENU_UINT8, (uint32_t *)&TRX.MIC_GAIN, SYSMENU_HANDL_AUDIO_MIC_Gain},
		{"DNR Threshold", SYSMENU_UINT8, (uint32_t *)&TRX.DNR_SNR_THRESHOLD, SYSMENU_HANDL_AUDIO_DNR_THRES},
		{"DNR Average", SYSMENU_UINT8, (uint32_t *)&TRX.DNR_AVERAGE, SYSMENU_HANDL_AUDIO_DNR_AVERAGE},
		{"DNR Minimal", SYSMENU_UINT8, (uint32_t *)&TRX.DNR_MINIMAL, SYSMENU_HANDL_AUDIO_DNR_MINMAL},
		{"SSB HPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.SSB_HPF_Filter, SYSMENU_HANDL_AUDIO_SSB_HPF_pass},
		{"SSB LPF RX Pass", SYSMENU_UINT16, (uint32_t *)&TRX.SSB_LPF_RX_Filter, SYSMENU_HANDL_AUDIO_SSB_LPF_RX_pass},
		{"SSB LPF TX Pass", SYSMENU_UINT16, (uint32_t *)&TRX.SSB_LPF_TX_Filter, SYSMENU_HANDL_AUDIO_SSB_LPF_TX_pass},
		{"CW HPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.CW_HPF_Filter, SYSMENU_HANDL_AUDIO_CW_HPF_pass},
		{"CW LPF Pass", SYSMENU_UINT16, (uint32_t *)&TRX.CW_LPF_Filter, SYSMENU_HANDL_AUDIO_CW_LPF_pass},
		{"AM LPF RX Pass", SYSMENU_UINT16, (uint32_t *)&TRX.AM_LPF_RX_Filter, SYSMENU_HANDL_AUDIO_AM_LPF_RX_pass},
		{"AM LPF TX Pass", SYSMENU_UINT16, (uint32_t *)&TRX.AM_LPF_TX_Filter, SYSMENU_HANDL_AUDIO_AM_LPF_TX_pass},
		{"FM LPF RX Pass", SYSMENU_UINT16, (uint32_t *)&TRX.FM_LPF_RX_Filter, SYSMENU_HANDL_AUDIO_FM_LPF_RX_pass},
		{"FM LPF TX Pass", SYSMENU_UINT16, (uint32_t *)&TRX.FM_LPF_TX_Filter, SYSMENU_HANDL_AUDIO_FM_LPF_TX_pass},
		{"FM Squelch", SYSMENU_UINT8, (uint32_t *)&TRX.FM_SQL_threshold, SYSMENU_HANDL_AUDIO_FMSquelch},
		{"VAD Squelch", SYSMENU_BOOLEAN, (uint32_t *)&TRX.VAD_Squelch, SYSMENU_HANDL_AUDIO_VAD_Squelch},
		{"MIC EQ Low", SYSMENU_INT8, (uint32_t *)&TRX.MIC_EQ_LOW, SYSMENU_HANDL_AUDIO_MIC_EQ_LOW},
		{"MIC EQ Mid", SYSMENU_INT8, (uint32_t *)&TRX.MIC_EQ_MID, SYSMENU_HANDL_AUDIO_MIC_EQ_MID},
		{"MIC EQ High", SYSMENU_INT8, (uint32_t *)&TRX.MIC_EQ_HIG, SYSMENU_HANDL_AUDIO_MIC_EQ_HIG},
		{"MIC Reverber", SYSMENU_UINT8, (uint32_t *)&TRX.MIC_REVERBER, SYSMENU_HANDL_AUDIO_MIC_REVERBER},
		{"RX EQ Low", SYSMENU_INT8, (uint32_t *)&TRX.RX_EQ_LOW, SYSMENU_HANDL_AUDIO_RX_EQ_LOW},
		{"RX EQ Mid", SYSMENU_INT8, (uint32_t *)&TRX.RX_EQ_MID, SYSMENU_HANDL_AUDIO_RX_EQ_MID},
		{"RX EQ High", SYSMENU_INT8, (uint32_t *)&TRX.RX_EQ_HIG, SYSMENU_HANDL_AUDIO_RX_EQ_HIG},
		{"RX AGC SSB Speed", SYSMENU_UINT8, (uint32_t *)&TRX.RX_AGC_SSB_speed, SYSMENU_HANDL_AUDIO_RX_AGC_SSB_Speed},
		{"RX AGC CW Speed", SYSMENU_UINT8, (uint32_t *)&TRX.RX_AGC_CW_speed, SYSMENU_HANDL_AUDIO_RX_AGC_CW_Speed},
		{"TX Compressor Speed", SYSMENU_UINT8, (uint32_t *)&TRX.TX_Compressor_speed, SYSMENU_HANDL_AUDIO_TX_CompressorSpeed},
		{"TX Compressor MaxGain", SYSMENU_UINT8, (uint32_t *)&TRX.TX_Compressor_maxgain, SYSMENU_HANDL_AUDIO_TX_CompressorMaxGain},
		{"Beeper", SYSMENU_BOOLEAN, (uint32_t *)&TRX.Beeper, SYSMENU_HANDL_AUDIO_Beeper},
};
const static uint8_t sysmenu_audio_item_count = sizeof(sysmenu_audio_handlers) / sizeof(sysmenu_audio_handlers[0]);

const static struct sysmenu_item_handler sysmenu_cw_handlers[] =
	{
		{"CW Key timeout", SYSMENU_UINT16, (uint32_t *)&TRX.CW_Key_timeout, SYSMENU_HANDL_CW_Key_timeout},
		{"CW Generator shift", SYSMENU_UINT16, (uint32_t *)&TRX.CW_GENERATOR_SHIFT_HZ, SYSMENU_HANDL_CW_GENERATOR_SHIFT_HZ},
		{"CW Self Hear", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CW_SelfHear, SYSMENU_HANDL_CW_SelfHear},
		{"CW Keyer", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CW_KEYER, SYSMENU_HANDL_CW_Keyer},
		{"CW Keyer WPM", SYSMENU_UINT8, (uint32_t *)&TRX.CW_KEYER_WPM, SYSMENU_HANDL_CW_Keyer_WPM},
		{"CW Gauss filter", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CW_GaussFilter, SYSMENU_HANDL_CW_GaussFilter},
		{"CW Decoder", SYSMENU_BOOLEAN, (uint32_t *)&TRX.CWDecoder, SYSMENU_HANDL_CW_Decoder},
};
const static uint8_t sysmenu_cw_item_count = sizeof(sysmenu_cw_handlers) / sizeof(sysmenu_cw_handlers[0]);

const static struct sysmenu_item_handler sysmenu_screen_handlers[] =
	{
		{"FFT Zoom", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Zoom, SYSMENU_HANDL_SCREEN_FFT_Zoom},
		{"FFT Zoom CW", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_ZoomCW, SYSMENU_HANDL_SCREEN_FFT_ZoomCW},
#ifdef HAS_BRIGHTNESS_CONTROL
		{"LCD Brightness", SYSMENU_UINT8, (uint32_t *)&TRX.LCD_Brightness, SYSMENU_HANDL_SCREEN_LCD_Brightness},
#endif
		{"Color Theme", SYSMENU_ENUMR, (uint32_t *)&TRX.ColorThemeId, SYSMENU_HANDL_SCREEN_COLOR_THEME, {"Black", "White", "Colored"}},
#ifdef LAY_480x320
		{"Layout Theme", SYSMENU_UINT8, (uint32_t *)&TRX.LayoutThemeId, SYSMENU_HANDL_SCREEN_LAYOUT_THEME, {"Default", "7 Segment"}},
#endif
#ifdef LAY_800x480
		{"Layout Theme", SYSMENU_ENUMR, (uint32_t *)&TRX.LayoutThemeId, SYSMENU_HANDL_SCREEN_LAYOUT_THEME, {"Default", "Analog", "7 Segment"}},
#endif
		{"FFT Speed", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Speed, SYSMENU_HANDL_SCREEN_FFT_Speed},
		{"FFT Automatic", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Automatic, SYSMENU_HANDL_SCREEN_FFT_Automatic},
		{"FFT Sensitivity", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Sensitivity, SYSMENU_HANDL_SCREEN_FFT_Sensitivity},
		{"FFT Manual Bottom, dBm", SYSMENU_INT16, (uint32_t *)&TRX.FFT_ManualBottom, SYSMENU_HANDL_SCREEN_FFT_ManualBottom},
		{"FFT Manual Top, dBm", SYSMENU_INT16, (uint32_t *)&TRX.FFT_ManualTop, SYSMENU_HANDL_SCREEN_FFT_ManualTop},
		{"FFT Height", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Height, SYSMENU_HANDL_SCREEN_FFT_Height},
		{"FFT Style", SYSMENU_ENUMR, (uint32_t *)&TRX.FFT_Style, SYSMENU_HANDL_SCREEN_FFT_Style, {"", "Gradient", "Fill", "Dots", "Contour"}},
		{"FFT Color", SYSMENU_ENUMR, (uint32_t *)&TRX.FFT_Color, SYSMENU_HANDL_SCREEN_FFT_Color, {"", "Blu>Y>R", "Bla>Y>R", "Bla>Y>G", "Bla>R", "Bla>G", "Bla>Blu", "Bla>W"}},
		{"FFT Grid", SYSMENU_ENUM, (uint32_t *)&TRX.FFT_Grid, SYSMENU_HANDL_SCREEN_FFT_Grid, {"NO", "Top", "All", "Bott"}},
		{"FFT Background", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Background, SYSMENU_HANDL_SCREEN_FFT_Background},
		{"FFT Lens", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Lens, SYSMENU_HANDL_SCREEN_FFT_Lens},
		{"FFT Hold Peaks", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_HoldPeaks, SYSMENU_HANDL_SCREEN_FFT_HoldPeaks},
		{"FFT 3D Mode", SYSMENU_ENUM, (uint32_t *)&TRX.FFT_3D, SYSMENU_HANDL_SCREEN_FFT_3D, {"NO", "Lines", "Dots"}},
		{"FFT Enabled", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Enabled, SYSMENU_HANDL_SCREEN_FFT_Enabled},
		{"WTF Moving", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WTF_Moving, SYSMENU_HANDL_SCREEN_WTF_Moving},
		{"FFT Compressor", SYSMENU_BOOLEAN, (uint32_t *)&TRX.FFT_Compressor, SYSMENU_HANDL_SCREEN_FFT_Compressor},
		{"FFT Averaging", SYSMENU_UINT8, (uint32_t *)&TRX.FFT_Averaging, SYSMENU_HANDL_SCREEN_FFT_Averaging},
		{"FFT Window", SYSMENU_ENUMR, (uint32_t *)&TRX.FFT_Window, SYSMENU_HANDL_SCREEN_FFT_Window, {"", "Dolph", "Blackman", "Nutall", "BlNutall", "Hann", "Hamming", "No"}},
#ifdef HRDW_HAS_FUNCBUTTONS
#if FUNCBUTTONS_COUNT == 24
		{"Func button 1", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[0], SYSMENU_HANDL_SCREEN_FUNC_BUTTON1},
		{"Func button 2", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[1], SYSMENU_HANDL_SCREEN_FUNC_BUTTON2},
		{"Func button 3", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[2], SYSMENU_HANDL_SCREEN_FUNC_BUTTON3},
		{"Func button 4", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[3], SYSMENU_HANDL_SCREEN_FUNC_BUTTON4},
		{"Func button 5", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[4], SYSMENU_HANDL_SCREEN_FUNC_BUTTON5},
		{"Func button 6", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[5], SYSMENU_HANDL_SCREEN_FUNC_BUTTON6},
		{"Func button 7", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[6], SYSMENU_HANDL_SCREEN_FUNC_BUTTON7},
		{"Func button 8", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[7], SYSMENU_HANDL_SCREEN_FUNC_BUTTON8},
		{"Func button 9", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[8], SYSMENU_HANDL_SCREEN_FUNC_BUTTON9},
		{"Func button 10", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[9], SYSMENU_HANDL_SCREEN_FUNC_BUTTON10},
		{"Func button 11", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[10], SYSMENU_HANDL_SCREEN_FUNC_BUTTON11},
		{"Func button 12", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[11], SYSMENU_HANDL_SCREEN_FUNC_BUTTON12},
		{"Func button 13", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[12], SYSMENU_HANDL_SCREEN_FUNC_BUTTON13},
		{"Func button 14", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[13], SYSMENU_HANDL_SCREEN_FUNC_BUTTON14},
		{"Func button 15", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[14], SYSMENU_HANDL_SCREEN_FUNC_BUTTON15},
		{"Func button 16", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[15], SYSMENU_HANDL_SCREEN_FUNC_BUTTON16},
		{"Func button 17", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[16], SYSMENU_HANDL_SCREEN_FUNC_BUTTON17},
		{"Func button 18", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[17], SYSMENU_HANDL_SCREEN_FUNC_BUTTON18},
		{"Func button 19", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[18], SYSMENU_HANDL_SCREEN_FUNC_BUTTON19},
		{"Func button 20", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[19], SYSMENU_HANDL_SCREEN_FUNC_BUTTON20},
		{"Func button 21", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[20], SYSMENU_HANDL_SCREEN_FUNC_BUTTON21},
		{"Func button 22", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[21], SYSMENU_HANDL_SCREEN_FUNC_BUTTON22},
		{"Func button 23", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[22], SYSMENU_HANDL_SCREEN_FUNC_BUTTON23},
		{"Func button 24", SYSMENU_FUNCBUTTON, (uint32_t *)&TRX.FuncButtons[23], SYSMENU_HANDL_SCREEN_FUNC_BUTTON24},
#endif
#endif
};
const static uint8_t sysmenu_screen_item_count = sizeof(sysmenu_screen_handlers) / sizeof(sysmenu_screen_handlers[0]);

const static struct sysmenu_item_handler sysmenu_adc_handlers[] =
	{
		{"ADC Driver", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_Driver, SYSMENU_HANDL_ADC_DRIVER},
		{"ADC Preamp", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_PGA, SYSMENU_HANDL_ADC_PGA},
		{"ADC Dither", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_DITH, SYSMENU_HANDL_ADC_DITH},
		{"ADC Randomizer", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_RAND, SYSMENU_HANDL_ADC_RAND},
		{"ADC Shutdown", SYSMENU_BOOLEAN, (uint32_t *)&TRX.ADC_SHDN, SYSMENU_HANDL_ADC_SHDN},
};
const static uint8_t sysmenu_adc_item_count = sizeof(sysmenu_adc_handlers) / sizeof(sysmenu_adc_handlers[0]);

const static struct sysmenu_item_handler sysmenu_wifi_handlers[] =
	{
		{"WIFI Enabled", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WIFI_Enabled, SYSMENU_HANDL_WIFI_Enabled},
		{"WIFI Network 1", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SelectAP1},
		{"WIFI Network 1 Pass", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SetAP1password},
		{"WIFI Network 2", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SelectAP2},
		{"WIFI Network 2 Pass", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SetAP2password},
		{"WIFI Network 3", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SelectAP3},
		{"WIFI Network 3 Pass", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_SetAP3password},
		{"WIFI Timezone", SYSMENU_INT8, (uint32_t *)&TRX.WIFI_TIMEZONE, SYSMENU_HANDL_WIFI_Timezone},
		{"WIFI CAT Server", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WIFI_CAT_SERVER, SYSMENU_HANDL_WIFI_CAT_Server},
		{"WIFI Update ESP firmware", SYSMENU_RUN, 0, SYSMENU_HANDL_WIFI_UpdateFW},
		{"", SYSMENU_INFOLINE, 0, 0},
		{"NET:", SYSMENU_INFOLINE, 0, 0},
		{WIFI_AP, SYSMENU_INFOLINE, 0, 0},
		{"", SYSMENU_INFOLINE, 0, 0},
		{"IP:", SYSMENU_INFOLINE, 0, 0},
		{WIFI_IP, SYSMENU_INFOLINE, 0, 0},
};
const static uint8_t sysmenu_wifi_item_count = sizeof(sysmenu_wifi_handlers) / sizeof(sysmenu_wifi_handlers[0]);

const static struct sysmenu_item_handler sysmenu_sd_handlers[] =
	{
		{"USB SD Card Reader", SYSMENU_BOOLEAN, (uint32_t *)&SD_USBCardReader, SYSMENU_HANDL_SD_USB},
		{"Export Settings to SD card", SYSMENU_RUN, 0, SYSMENU_HANDL_SD_ExportSettings},
		{"Import Settings from SD card", SYSMENU_RUN, 0, SYSMENU_HANDL_SD_ImportSettings},
		{"Format SD card", SYSMENU_RUN, 0, SYSMENU_HANDL_SD_Format},
};
const static uint8_t sysmenu_sd_item_count = sizeof(sysmenu_sd_handlers) / sizeof(sysmenu_sd_handlers[0]);

const static struct sysmenu_item_handler sysmenu_calibration_handlers[] =
	{
		{"Encoder invert", SYSMENU_BOOLEAN, (uint32_t *)&CALIBRATE.ENCODER_INVERT, SYSMENU_HANDL_CALIB_ENCODER_INVERT},
		{"Encoder2 invert", SYSMENU_BOOLEAN, (uint32_t *)&CALIBRATE.ENCODER2_INVERT, SYSMENU_HANDL_CALIB_ENCODER2_INVERT},
		{"Encoder debounce", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.ENCODER_DEBOUNCE, SYSMENU_HANDL_CALIB_ENCODER_DEBOUNCE},
		{"Encoder2 debounce", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.ENCODER2_DEBOUNCE, SYSMENU_HANDL_CALIB_ENCODER2_DEBOUNCE},
		{"Encoder slow rate", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.ENCODER_SLOW_RATE, SYSMENU_HANDL_CALIB_ENCODER_SLOW_RATE},
		{"Encoder on falling", SYSMENU_BOOLEAN, (uint32_t *)&CALIBRATE.ENCODER_ON_FALLING, SYSMENU_HANDL_CALIB_ENCODER_ON_FALLING},
		{"Encoder acceleration", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.ENCODER_ACCELERATION, SYSMENU_HANDL_CALIB_ENCODER_ACCELERATION},
		{"CICCOMP 48K Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.CICFIR_GAINER_48K_val, SYSMENU_HANDL_CALIB_CICCOMP_48K_SHIFT},
		{"CICCOMP 96K Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.CICFIR_GAINER_96K_val, SYSMENU_HANDL_CALIB_CICCOMP_96K_SHIFT},
		{"CICCOMP 192K Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.CICFIR_GAINER_192K_val, SYSMENU_HANDL_CALIB_CICCOMP_192K_SHIFT},
		{"CICCOMP 384K Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.CICFIR_GAINER_384K_val, SYSMENU_HANDL_CALIB_CICCOMP_384K_SHIFT},
		{"TX CICCOMP Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.TXCICFIR_GAINER_val, SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT},
		{"DAC Shift", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.DAC_GAINER_val, SYSMENU_HANDL_CALIB_DAC_SHIFT},
		{"RF GAIN 2200m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_2200m, SYSMENU_HANDL_CALIB_RF_GAIN_2200M},
		{"RF GAIN 160m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_160m, SYSMENU_HANDL_CALIB_RF_GAIN_160M},
		{"RF GAIN 80m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_80m, SYSMENU_HANDL_CALIB_RF_GAIN_80M},
		{"RF GAIN 40m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_40m, SYSMENU_HANDL_CALIB_RF_GAIN_40M},
		{"RF GAIN 30m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_30m, SYSMENU_HANDL_CALIB_RF_GAIN_30M},
		{"RF GAIN 20m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_20m, SYSMENU_HANDL_CALIB_RF_GAIN_20M},
		{"RF GAIN 17m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_17m, SYSMENU_HANDL_CALIB_RF_GAIN_17M},
		{"RF GAIN 15m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_15m, SYSMENU_HANDL_CALIB_RF_GAIN_15M},
		{"RF GAIN 12m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_12m, SYSMENU_HANDL_CALIB_RF_GAIN_12M},
		{"RF GAIN 10m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_10m, SYSMENU_HANDL_CALIB_RF_GAIN_10M},
		{"RF GAIN 6m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_6m, SYSMENU_HANDL_CALIB_RF_GAIN_6M},
		{"RF GAIN 2m", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.rf_out_power_2m, SYSMENU_HANDL_CALIB_RF_GAIN_2M},
		{"S METER", SYSMENU_INT16, (uint32_t *)&CALIBRATE.smeter_calibration, SYSMENU_HANDL_CALIB_S_METER},
		{"ADC OFFSET", SYSMENU_INT16, (uint32_t *)&CALIBRATE.adc_offset, SYSMENU_HANDL_CALIB_ADC_OFFSET},
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
		{"HPF START", SYSMENU_UINT32, (uint32_t *)&CALIBRATE.BPF_HPF, SYSMENU_HANDL_CALIB_HPF_START},
		{"SWR FWD RATE", SYSMENU_FLOAT32, (uint32_t *)&CALIBRATE.SWR_FWD_Calibration, SYSMENU_HANDL_CALIB_SWR_FWD_RATE},
		{"SWR REF RATE", SYSMENU_FLOAT32, (uint32_t *)&CALIBRATE.SWR_REF_Calibration, SYSMENU_HANDL_CALIB_SWR_REF_RATE},
		{"VCXO Correction", SYSMENU_INT8, (uint32_t *)&CALIBRATE.VCXO_correction, SYSMENU_HANDL_CALIB_VCXO},
		{"FW_AD8307_Slope (mv/dB)", SYSMENU_FLOAT32, (uint32_t *)&CALIBRATE.FW_AD8307_SLP, SYSMENU_HANDL_CALIB_FW_AD8307_SLP},
		{"FW_AD8307_Offset (mV)", SYSMENU_FLOAT32, (uint32_t *)&CALIBRATE.FW_AD8307_OFFS, SYSMENU_HANDL_CALIB_FW_AD8307_OFFS},
		{"BW_AD8307_Slope (mv/dB)", SYSMENU_FLOAT32, (uint32_t *)&CALIBRATE.BW_AD8307_SLP, SYSMENU_HANDL_CALIB_BW_AD8307_SLP},
		{"BW_AD8307_Offset (mV)", SYSMENU_FLOAT32, (uint32_t *)&CALIBRATE.BW_AD8307_OFFS, SYSMENU_HANDL_CALIB_BW_AD8307_OFFS},
		{"FAN Medium start", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.FAN_MEDIUM_START, SYSMENU_HANDL_CALIB_FAN_MEDIUM_START},
		{"FAN Medium stop", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.FAN_MEDIUM_STOP, SYSMENU_HANDL_CALIB_FAN_MEDIUM_STOP},
		{"FAN Full start", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.FAN_FULL_START, SYSMENU_HANDL_CALIB_FAN_FULL_START},
		{"MAX RF Temp", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.TRX_MAX_RF_TEMP, SYSMENU_HANDL_CALIB_TRX_MAX_RF_TEMP},
		{"MAX SWR", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.TRX_MAX_SWR, SYSMENU_HANDL_CALIB_TRX_MAX_SWR},
		{"FM Deviation Scale", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.FM_DEVIATION_SCALE, SYSMENU_HANDL_CALIB_FM_DEVIATION_SCALE},
		{"TUNE Max Power", SYSMENU_UINT8, (uint32_t *)&CALIBRATE.TUNE_MAX_POWER, SYSMENU_HANDL_CALIB_TUNE_MAX_POWER},
};
const static uint8_t sysmenu_calibration_item_count = sizeof(sysmenu_calibration_handlers) / sizeof(sysmenu_calibration_handlers[0]);

const static struct sysmenu_item_handler sysmenu_spectrum_handlers[] =
	{
		{"Spectrum START", SYSMENU_RUN, 0, SYSMENU_HANDL_SPECTRUM_Start},
		{"Begin, kHz", SYSMENU_UINT32, (uint32_t *)&TRX.SPEC_Begin, SYSMENU_HANDL_SPECTRUM_Begin},
		{"End, kHz", SYSMENU_UINT32, (uint32_t *)&TRX.SPEC_End, SYSMENU_HANDL_SPECTRUM_End},
		{"Top, dBm", SYSMENU_INT16, (uint32_t *)&TRX.SPEC_TopDBM, SYSMENU_HANDL_SPECTRUM_TopDBM},
		{"Bottom, dBm", SYSMENU_INT16, (uint32_t *)&TRX.SPEC_BottomDBM, SYSMENU_HANDL_SPECTRUM_BottomDBM},
};
const static uint8_t sysmenu_spectrum_item_count = sizeof(sysmenu_spectrum_handlers) / sizeof(sysmenu_spectrum_handlers[0]);

const static struct sysmenu_item_handler sysmenu_wspr_handlers[] =
	{
		{"WSPR Beacon START", SYSMENU_RUN, 0, SYSMENU_HANDL_WSPR_Start},
		{"Freq offset", SYSMENU_INT16, (uint32_t *)&TRX.WSPR_FREQ_OFFSET, SYSMENU_HANDL_WSPR_FREQ_OFFSET},
		{"BAND 160m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_160, SYSMENU_HANDL_WSPR_BAND160},
		{"BAND 80m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_80, SYSMENU_HANDL_WSPR_BAND80},
		{"BAND 40m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_40, SYSMENU_HANDL_WSPR_BAND40},
		{"BAND 30m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_30, SYSMENU_HANDL_WSPR_BAND30},
		{"BAND 20m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_20, SYSMENU_HANDL_WSPR_BAND20},
		{"BAND 17m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_17, SYSMENU_HANDL_WSPR_BAND17},
		{"BAND 15m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_15, SYSMENU_HANDL_WSPR_BAND15},
		{"BAND 12m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_12, SYSMENU_HANDL_WSPR_BAND12},
		{"BAND 10m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_10, SYSMENU_HANDL_WSPR_BAND10},
		{"BAND 6m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_6, SYSMENU_HANDL_WSPR_BAND6},
		{"BAND 2m", SYSMENU_BOOLEAN, (uint32_t *)&TRX.WSPR_BANDS_2, SYSMENU_HANDL_WSPR_BAND2},
};
const static uint8_t sysmenu_wspr_item_count = sizeof(sysmenu_wspr_handlers) / sizeof(sysmenu_wspr_handlers[0]);

const static struct sysmenu_item_handler sysmenu_services_handlers[] =
	{
		{"Band SWR", SYSMENU_RUN, 0, SYSMENU_HANDL_SWR_BAND_START},
		{"HF SWR", SYSMENU_RUN, 0, SYSMENU_HANDL_SWR_HF_START},
		{"Spectrum Analyzer", SYSMENU_MENU, 0, SYSMENU_HANDL_SPECTRUMMENU},
		{"RDA Statistics", SYSMENU_RUN, 0, SYSMENU_HANDL_RDA_STATS},
		{"Propagination", SYSMENU_RUN, 0, SYSMENU_HANDL_PROPAGINATION},
		{"WSPR Beacon", SYSMENU_MENU, 0, SYSMENU_HANDL_WSPRMENU},
		{"File Manager", SYSMENU_RUN, 0, SYSMENU_HANDL_FILEMANAGER},
#ifdef SWR_AD8307_LOG
		{"SWR Tandem Match Contr.", SYSMENU_RUN, 0, SYSMENU_HANDL_SWR_Tandem_Ctrl}, //Tisho
#endif
};
const static uint8_t sysmenu_services_item_count = sizeof(sysmenu_services_handlers) / sizeof(sysmenu_services_handlers[0]);

//COMMON MENU
static void drawSystemMenuElement(char *title, SystemMenuType type, uint32_t *value, char enumerate[ENUM_MAX_COUNT][ENUM_MAX_LENGTH], bool onlyVal);
static void SYSMENU_WIFI_DrawSelectAP1Menu(bool full_redraw);
static void SYSMENU_WIFI_SelectAP1MenuMove(int8_t dir);
static void SYSMENU_WIFI_DrawAP1passwordMenu(bool full_redraw);
static void SYSMENU_WIFI_RotatePasswordChar1(int8_t dir);
static void SYSMENU_WIFI_DrawSelectAP2Menu(bool full_redraw);
static void SYSMENU_WIFI_SelectAP2MenuMove(int8_t dir);
static void SYSMENU_WIFI_DrawAP2passwordMenu(bool full_redraw);
static void SYSMENU_WIFI_RotatePasswordChar2(int8_t dir);
static void SYSMENU_WIFI_DrawSelectAP3Menu(bool full_redraw);
static void SYSMENU_WIFI_SelectAP3MenuMove(int8_t dir);
static void SYSMENU_WIFI_DrawAP3passwordMenu(bool full_redraw);
static void SYSMENU_WIFI_RotatePasswordChar3(int8_t dir);
static void SYSMENU_TRX_DrawCallsignMenu(bool full_redraw);
static void SYSMENU_TRX_RotateCallsignChar(int8_t dir);
static void SYSMENU_TRX_DrawLocatorMenu(bool full_redraw);
static void SYSMENU_TRX_RotateLocatorChar(int8_t dir);

static struct sysmenu_item_handler *sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_handlers[0];
static uint8_t *sysmenu_item_count_selected = (uint8_t*)&sysmenu_item_count;
static uint8_t systemMenuIndex = 0;
static uint8_t systemMenuRootIndex = 0;
static uint16_t sysmenu_y = 5;
static uint8_t sysmenu_i = 0;
static bool sysmenu_onroot = true;
bool SYSMENU_hiddenmenu_enabled = false;
static bool sysmenu_services_opened = false;
static bool sysmenu_infowindow_opened = false;
static bool sysmenu_sysinfo_opened = false;
static bool sysmenu_filemanager_opened = false;
static bool sysmenu_item_selected_by_enc2 = false;

//WIFI
static bool sysmenu_wifi_needupdate_ap = true;
static bool sysmenu_wifi_selectap1_menu_opened = false;
static bool sysmenu_wifi_selectap2_menu_opened = false;
static bool sysmenu_wifi_selectap3_menu_opened = false;
static bool sysmenu_wifi_setAP1password_menu_opened = false;
static bool sysmenu_wifi_setAP2password_menu_opened = false;
static bool sysmenu_wifi_setAP3password_menu_opened = false;
static bool sysmenu_trx_setCallsign_menu_opened = false;
static bool sysmenu_trx_setLocator_menu_opened = false;
static uint8_t sysmenu_wifi_selected_ap_index = 0;
static uint8_t sysmenu_wifi_selected_ap_password_char_index = 0;
static uint8_t sysmenu_trx_selected_callsign_char_index = 0;
static uint8_t sysmenu_trx_selected_locator_char_index = 0;

//Time menu
static bool sysmenu_timeMenuOpened = false;
static uint8_t TimeMenuSelection = 0;

//TRX MENU

static void SYSMENU_HANDL_TRXMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_trx_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_trx_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void SYSMENU_TRX_RFPOWER_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_trx_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_trx_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_redraw(false);
}

void SYSMENU_TRX_STEP_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_trx_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_trx_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 7;
	if (TRX.Fast)
		systemMenuIndex = 8;
	LCD_redraw(false);
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

static void SYSMENU_HANDL_TRX_RFFilters(int8_t direction)
{
	if (direction > 0)
		TRX.RF_Filters = true;
	if (direction < 0)
		TRX.RF_Filters = false;
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
	if (direction > 0 || TRX.RF_Power > 0)
		TRX.RF_Power += direction;
	if (TRX.RF_Power > 100)
		TRX.RF_Power = 100;
}

static void SYSMENU_HANDL_TRX_INPUT_TYPE(int8_t direction)
{
	if (direction > 0 || TRX.InputType > 0)
		TRX.InputType += direction;
	if (TRX.InputType > 2)
		TRX.InputType = 2;
	WM8731_TXRX_mode();
}

static void SYSMENU_HANDL_TRX_DEBUG_TYPE(int8_t direction)
{
	if (direction > 0 || TRX.Debug_Type > 0)
		TRX.Debug_Type += direction;
	if (TRX.Debug_Type > 4)
		TRX.Debug_Type = 4;
}

static void SYSMENU_HANDL_TRX_SAMPLERATE_MAIN(int8_t direction)
{
	if (direction > 0 || TRX.SAMPLERATE_MAIN > 0)
		TRX.SAMPLERATE_MAIN += direction;
	if (TRX.SAMPLERATE_MAIN > 3)
		TRX.SAMPLERATE_MAIN = 3;
	
	FFT_Init();
}

static void SYSMENU_HANDL_TRX_SAMPLERATE_WFM(int8_t direction)
{
	if (direction > 0 || TRX.SAMPLERATE_WFM > 0)
		TRX.SAMPLERATE_WFM += direction;
	if (TRX.SAMPLERATE_WFM > 3)
		TRX.SAMPLERATE_WFM = 3;
	
	FFT_Init();
}

static void SYSMENU_HANDL_TRX_SHIFT_INTERVAL(int8_t direction)
{
	TRX.SHIFT_INTERVAL += direction * 100;
	if (TRX.SHIFT_INTERVAL < 100)
		TRX.SHIFT_INTERVAL = 100;
	if (TRX.SHIFT_INTERVAL > 10000)
		TRX.SHIFT_INTERVAL = 10000;
}

static void SYSMENU_HANDL_TRX_FRQ_STEP(int8_t direction)
{
	const uint16_t freq_steps[] = {1, 10, 25, 50, 100};
	for (uint8_t i = 0; i < ARRLENTH(freq_steps); i++)
		if (TRX.FRQ_STEP == freq_steps[i])
		{
			if (direction < 0)
			{
				if (i > 0)
					TRX.FRQ_STEP = freq_steps[i - 1];
				else
					TRX.FRQ_STEP = freq_steps[0];
				return;
			}
			else
			{
				if (i < (ARRLENTH(freq_steps) - 1))
					TRX.FRQ_STEP = freq_steps[i + 1];
				else
					TRX.FRQ_STEP = freq_steps[ARRLENTH(freq_steps) - 1];
				return;
			}
		}
	TRX.FRQ_STEP = freq_steps[0];
}

static void SYSMENU_HANDL_TRX_FRQ_FAST_STEP(int8_t direction)
{
	const uint16_t freq_steps[] = {10, 25, 50, 100, 500, 1000};
	for (uint8_t i = 0; i < ARRLENTH(freq_steps); i++)
		if (TRX.FRQ_FAST_STEP == freq_steps[i])
		{
			if (direction < 0)
			{
				if (i > 0)
					TRX.FRQ_FAST_STEP = freq_steps[i - 1];
				else
					TRX.FRQ_FAST_STEP = freq_steps[0];
				return;
			}
			else
			{
				if (i < (ARRLENTH(freq_steps) - 1))
					TRX.FRQ_FAST_STEP = freq_steps[i + 1];
				else
					TRX.FRQ_FAST_STEP = freq_steps[ARRLENTH(freq_steps) - 1];
				return;
			}
		}
	TRX.FRQ_FAST_STEP = freq_steps[0];
}

static void SYSMENU_HANDL_TRX_FRQ_ENC_STEP(int8_t direction)
{
	const uint16_t freq_steps[] = {1000, 5000, 25000, 50000};
	for (uint8_t i = 0; i < ARRLENTH(freq_steps); i++)
		if (TRX.FRQ_ENC_STEP == freq_steps[i])
		{
			if (direction < 0)
			{
				if (i > 0)
					TRX.FRQ_ENC_STEP = freq_steps[i - 1];
				else
					TRX.FRQ_ENC_STEP = freq_steps[0];
				return;
			}
			else
			{
				if (i < (ARRLENTH(freq_steps) - 1))
					TRX.FRQ_ENC_STEP = freq_steps[i + 1];
				else
					TRX.FRQ_ENC_STEP = freq_steps[ARRLENTH(freq_steps) - 1];
				return;
			}
		}
	TRX.FRQ_ENC_STEP = freq_steps[0];
}

static void SYSMENU_HANDL_TRX_FRQ_ENC_FAST_STEP(int8_t direction)
{
	const uint32_t freq_steps[] = {5000, 25000, 50000, 100000, 500000};
	for (uint8_t i = 0; i < ARRLENTH(freq_steps); i++)
		if (TRX.FRQ_ENC_FAST_STEP == freq_steps[i])
		{
			if (direction < 0)
			{
				if (i > 0)
					TRX.FRQ_ENC_FAST_STEP = freq_steps[i - 1];
				else
					TRX.FRQ_ENC_FAST_STEP = freq_steps[0];
				return;
			}
			else
			{
				if (i < (ARRLENTH(freq_steps) - 1))
					TRX.FRQ_ENC_FAST_STEP = freq_steps[i + 1];
				else
					TRX.FRQ_ENC_FAST_STEP = freq_steps[ARRLENTH(freq_steps) - 1];
				return;
			}
		}
	TRX.FRQ_ENC_FAST_STEP = freq_steps[0];
}

static void SYSMENU_HANDL_TRX_ENC_ACCELERATE(int8_t direction)
{
	if (direction > 0)
		TRX.Encoder_Accelerate = true;
	if (direction < 0)
		TRX.Encoder_Accelerate = false;
}

static void SYSMENU_HANDL_TRX_ATT_STEP(int8_t direction)
{
	TRX.ATT_STEP += direction;
	if (TRX.ATT_STEP < 1)
		TRX.ATT_STEP = 1;
	if (TRX.ATT_STEP > 15)
		TRX.ATT_STEP = 15;
}

static void SYSMENU_TRX_DrawCallsignMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("CALLSIGN:", 5, 5, FG_COLOR, BG_COLOR, 2);
	}

	LCDDriver_printText(TRX.CALLSIGN, 10, 37, COLOR_GREEN, BG_COLOR, 2);
	LCDDriver_drawFastHLine(8 + sysmenu_trx_selected_callsign_char_index * 12, 54, 12, COLOR_RED);
}

static void SYSMENU_TRX_DrawLocatorMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("LOCATOR:", 5, 5, FG_COLOR, BG_COLOR, 2);
	}

	LCDDriver_printText(TRX.LOCATOR, 10, 37, COLOR_GREEN, BG_COLOR, 2);
	LCDDriver_drawFastHLine(8 + sysmenu_trx_selected_locator_char_index * 12, 54, 12, COLOR_RED);
}

static void SYSMENU_TRX_RotateCallsignChar(int8_t dir)
{
	bool full_redraw = false;
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] == 0)
		full_redraw = true;
	TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] += dir;

	// do not show special characters
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] >= 1 && TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] <= 32 && dir > 0)
		TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] = 33;
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] >= 1 && TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] <= 32 && dir < 0)
		TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] = 0;
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] >= 127)
		TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] = 0;
	if (TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		LCD_UpdateQuery.SystemMenuRedraw = true;
	else
		LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_TRX_RotateLocatorChar(int8_t dir)
{
	bool full_redraw = false;
	if (TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] == 0)
		full_redraw = true;
	TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] += dir;

	// do not show special characters
	if (TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] >= 1 && TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] <= 32 && dir > 0)
		TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] = 33;
	if (TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] >= 1 && TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] <= 32 && dir < 0)
		TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] = 0;
	if (TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] >= 127)
		TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] = 0;
	if (TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		LCD_UpdateQuery.SystemMenuRedraw = true;
	else
		LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_HANDL_TRX_SetCallsign(int8_t direction)
{
#pragma unused(direction)
	sysmenu_trx_setCallsign_menu_opened = true;
	SYSMENU_TRX_DrawCallsignMenu(true);
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_TRX_SetLocator(int8_t direction)
{
#pragma unused(direction)
	sysmenu_trx_setLocator_menu_opened = true;
	SYSMENU_TRX_DrawLocatorMenu(true);
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_TRX_TRANSV_ENABLE(int8_t direction)
{
	if (direction > 0)
		TRX.Transverter_Enabled = true;
	if (direction < 0)
		TRX.Transverter_Enabled = false;
}

static void SYSMENU_HANDL_TRX_TRANSV_OFFSET(int8_t direction)
{
	TRX.Transverter_Offset_Mhz += direction;
	if (TRX.Transverter_Offset_Mhz < 1)
		TRX.Transverter_Offset_Mhz = 1;
	if (TRX.Transverter_Offset_Mhz > 500)
		TRX.Transverter_Offset_Mhz = 500;
}

//AUDIO MENU

static void SYSMENU_HANDL_AUDIOMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void SYSMENU_AUDIO_BW_SSB_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 7;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_CW_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 10;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_AM_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 11;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_FM_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 13;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_HPF_SSB_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 6;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_HPF_CW_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	systemMenuIndex = 9;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_SQUELCH_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 15;
	LCD_redraw(false);
}

void SYSMENU_AUDIO_AGC_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_audio_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_audio_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 24;
	LCD_redraw(false);
}

static void SYSMENU_HANDL_AUDIO_IFGain(int8_t direction)
{
	TRX.IF_Gain += direction;
	if (TRX.IF_Gain < 1)
		TRX.IF_Gain = 1;
	if (TRX.IF_Gain > 80)
		TRX.IF_Gain = 80;
}

static void SYSMENU_HANDL_AUDIO_AGC_GAIN_TARGET(int8_t direction)
{
	TRX.AGC_GAIN_TARGET += direction;
	if (TRX.AGC_GAIN_TARGET < -80)
		TRX.AGC_GAIN_TARGET = -80;
	if (TRX.AGC_GAIN_TARGET > -10)
		TRX.AGC_GAIN_TARGET = -10;
}

static void SYSMENU_HANDL_AUDIO_MIC_Gain(int8_t direction)
{
	TRX.MIC_GAIN += direction;
	if (TRX.MIC_GAIN < 1)
		TRX.MIC_GAIN = 1;
	if (TRX.MIC_GAIN > 20)
		TRX.MIC_GAIN = 20;
}

static void SYSMENU_HANDL_AUDIO_DNR_THRES(int8_t direction)
{
	TRX.DNR_SNR_THRESHOLD += direction;
	if (TRX.DNR_SNR_THRESHOLD < 1)
		TRX.DNR_SNR_THRESHOLD = 1;
	if (TRX.DNR_SNR_THRESHOLD > 100)
		TRX.DNR_SNR_THRESHOLD = 100;
}

static void SYSMENU_HANDL_AUDIO_DNR_AVERAGE(int8_t direction)
{
	TRX.DNR_AVERAGE += direction;
	if (TRX.DNR_AVERAGE < 1)
		TRX.DNR_AVERAGE = 1;
	if (TRX.DNR_AVERAGE > 100)
		TRX.DNR_AVERAGE = 100;
}

static void SYSMENU_HANDL_AUDIO_DNR_MINMAL(int8_t direction)
{
	TRX.DNR_MINIMAL += direction;
	if (TRX.DNR_MINIMAL < 1)
		TRX.DNR_MINIMAL = 1;
	if (TRX.DNR_MINIMAL > 100)
		TRX.DNR_MINIMAL = 100;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_LOW(int8_t direction)
{
	TRX.MIC_EQ_LOW += direction;
	if (TRX.MIC_EQ_LOW < -10)
		TRX.MIC_EQ_LOW = -10;
	if (TRX.MIC_EQ_LOW > 10)
		TRX.MIC_EQ_LOW = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_MID(int8_t direction)
{
	TRX.MIC_EQ_MID += direction;
	if (TRX.MIC_EQ_MID < -10)
		TRX.MIC_EQ_MID = -10;
	if (TRX.MIC_EQ_MID > 10)
		TRX.MIC_EQ_MID = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_HIG(int8_t direction)
{
	TRX.MIC_EQ_HIG += direction;
	if (TRX.MIC_EQ_HIG < -10)
		TRX.MIC_EQ_HIG = -10;
	if (TRX.MIC_EQ_HIG > 10)
		TRX.MIC_EQ_HIG = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_REVERBER(int8_t direction)
{
	if (direction > 0 || TRX.MIC_REVERBER > 0)
		TRX.MIC_REVERBER += direction;
	if (TRX.MIC_REVERBER > (AUDIO_MAX_REVERBER_TAPS - 1))
		TRX.MIC_REVERBER = (AUDIO_MAX_REVERBER_TAPS - 1);
	NeedReinitReverber = true;
}

static void SYSMENU_HANDL_AUDIO_RX_EQ_LOW(int8_t direction)
{
	TRX.RX_EQ_LOW += direction;
	if (TRX.RX_EQ_LOW < -10)
		TRX.RX_EQ_LOW = -10;
	if (TRX.RX_EQ_LOW > 10)
		TRX.RX_EQ_LOW = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_RX_EQ_MID(int8_t direction)
{
	TRX.RX_EQ_MID += direction;
	if (TRX.RX_EQ_MID < -10)
		TRX.RX_EQ_MID = -10;
	if (TRX.RX_EQ_MID > 10)
		TRX.RX_EQ_MID = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_RX_EQ_HIG(int8_t direction)
{
	TRX.RX_EQ_HIG += direction;
	if (TRX.RX_EQ_HIG < -10)
		TRX.RX_EQ_HIG = -10;
	if (TRX.RX_EQ_HIG > 10)
		TRX.RX_EQ_HIG = 10;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_RX_AGC_SSB_Speed(int8_t direction)
{
	TRX.RX_AGC_SSB_speed += direction;
	if (TRX.RX_AGC_SSB_speed < 1)
		TRX.RX_AGC_SSB_speed = 1;
	if (TRX.RX_AGC_SSB_speed > 20)
		TRX.RX_AGC_SSB_speed = 20;
}

static void SYSMENU_HANDL_AUDIO_RX_AGC_CW_Speed(int8_t direction)
{
	TRX.RX_AGC_CW_speed += direction;
	if (TRX.RX_AGC_CW_speed < 1)
		TRX.RX_AGC_CW_speed = 1;
	if (TRX.RX_AGC_CW_speed > 20)
		TRX.RX_AGC_CW_speed = 20;
}

static void SYSMENU_HANDL_AUDIO_TX_CompressorSpeed(int8_t direction)
{
	TRX.TX_Compressor_speed += direction;
	if (TRX.TX_Compressor_speed < 1)
		TRX.TX_Compressor_speed = 1;
	if (TRX.TX_Compressor_speed > 10)
		TRX.TX_Compressor_speed = 10;
}

static void SYSMENU_HANDL_AUDIO_TX_CompressorMaxGain(int8_t direction)
{
	TRX.TX_Compressor_maxgain += direction;
	if (TRX.TX_Compressor_maxgain < 1)
		TRX.TX_Compressor_maxgain = 1;
	if (TRX.TX_Compressor_maxgain > 10)
		TRX.TX_Compressor_maxgain = 10;
}

static void SYSMENU_HANDL_AUDIO_FMSquelch(int8_t direction)
{
	if (direction < 0 && CurrentVFO()->FM_SQL_threshold == 0)
		return;
	CurrentVFO()->FM_SQL_threshold += direction;
	if (CurrentVFO()->FM_SQL_threshold > 10)
		CurrentVFO()->FM_SQL_threshold = 10;
	TRX.FM_SQL_threshold = CurrentVFO()->FM_SQL_threshold;
	
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold = CurrentVFO()->FM_SQL_threshold;
}

static void SYSMENU_HANDL_AUDIO_VAD_Squelch(int8_t direction)
{
	if (direction > 0)
		TRX.VAD_Squelch = true;
	if (direction < 0)
		TRX.VAD_Squelch = false;
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
		else if (TRX.SSB_HPF_Filter == 500)
			TRX.SSB_HPF_Filter = 600;
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
		else if (TRX.SSB_HPF_Filter == 600)
			TRX.SSB_HPF_Filter = 500;
	}
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_CW_HPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.CW_HPF_Filter == 0)
			TRX.CW_HPF_Filter = 60;
		else if (TRX.CW_HPF_Filter == 60)
			TRX.CW_HPF_Filter = 100;
		else if (TRX.CW_HPF_Filter == 100)
			TRX.CW_HPF_Filter = 200;
		else if (TRX.CW_HPF_Filter == 200)
			TRX.CW_HPF_Filter = 300;
		else if (TRX.CW_HPF_Filter == 300)
			TRX.CW_HPF_Filter = 400;
		else if (TRX.CW_HPF_Filter == 400)
			TRX.CW_HPF_Filter = 500;
		else if (TRX.CW_HPF_Filter == 500)
			TRX.CW_HPF_Filter = 600;
	}
	else
	{
		if (TRX.CW_HPF_Filter == 60)
			TRX.CW_HPF_Filter = 0;
		else if (TRX.CW_HPF_Filter == 100)
			TRX.CW_HPF_Filter = 60;
		else if (TRX.CW_HPF_Filter == 200)
			TRX.CW_HPF_Filter = 100;
		else if (TRX.CW_HPF_Filter == 300)
			TRX.CW_HPF_Filter = 200;
		else if (TRX.CW_HPF_Filter == 400)
			TRX.CW_HPF_Filter = 300;
		else if (TRX.CW_HPF_Filter == 500)
			TRX.CW_HPF_Filter = 400;
		else if (TRX.CW_HPF_Filter == 600)
			TRX.CW_HPF_Filter = 500;
	}
	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_CW_LPF_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.CW_LPF_Filter == 100)
			TRX.CW_LPF_Filter = 150;
		else if (TRX.CW_LPF_Filter == 150)
			TRX.CW_LPF_Filter = 200;
		else if (TRX.CW_LPF_Filter == 200)
			TRX.CW_LPF_Filter = 250;
		else if (TRX.CW_LPF_Filter == 250)
			TRX.CW_LPF_Filter = 300;
		else if (TRX.CW_LPF_Filter == 300)
			TRX.CW_LPF_Filter = 350;
		else if (TRX.CW_LPF_Filter == 350)
			TRX.CW_LPF_Filter = 400;
		else if (TRX.CW_LPF_Filter == 400)
			TRX.CW_LPF_Filter = 450;
		else if (TRX.CW_LPF_Filter == 450)
			TRX.CW_LPF_Filter = 500;
		else if (TRX.CW_LPF_Filter == 500)
			TRX.CW_LPF_Filter = 550;
		else if (TRX.CW_LPF_Filter == 550)
			TRX.CW_LPF_Filter = 600;
		else if (TRX.CW_LPF_Filter == 600)
			TRX.CW_LPF_Filter = 650;
		else if (TRX.CW_LPF_Filter == 650)
			TRX.CW_LPF_Filter = 700;
		else if (TRX.CW_LPF_Filter == 700)
			TRX.CW_LPF_Filter = 750;
		else if (TRX.CW_LPF_Filter == 750)
			TRX.CW_LPF_Filter = 800;
		else if (TRX.CW_LPF_Filter == 800)
			TRX.CW_LPF_Filter = 850;
		else if (TRX.CW_LPF_Filter == 850)
			TRX.CW_LPF_Filter = 900;
		else if (TRX.CW_LPF_Filter == 900)
			TRX.CW_LPF_Filter = 950;
		else if (TRX.CW_LPF_Filter == 950)
			TRX.CW_LPF_Filter = 1000;
	}
	else
	{
		if (TRX.CW_LPF_Filter == 1000)
			TRX.CW_LPF_Filter = 950;
		else if (TRX.CW_LPF_Filter == 950)
			TRX.CW_LPF_Filter = 900;
		else if (TRX.CW_LPF_Filter == 900)
			TRX.CW_LPF_Filter = 850;
		else if (TRX.CW_LPF_Filter == 850)
			TRX.CW_LPF_Filter = 800;
		else if (TRX.CW_LPF_Filter == 800)
			TRX.CW_LPF_Filter = 750;
		else if (TRX.CW_LPF_Filter == 750)
			TRX.CW_LPF_Filter = 700;
		else if (TRX.CW_LPF_Filter == 700)
			TRX.CW_LPF_Filter = 650;
		else if (TRX.CW_LPF_Filter == 650)
			TRX.CW_LPF_Filter = 600;
		else if (TRX.CW_LPF_Filter == 600)
			TRX.CW_LPF_Filter = 550;
		else if (TRX.CW_LPF_Filter == 550)
			TRX.CW_LPF_Filter = 500;
		else if (TRX.CW_LPF_Filter == 500)
			TRX.CW_LPF_Filter = 450;
		else if (TRX.CW_LPF_Filter == 450)
			TRX.CW_LPF_Filter = 400;
		else if (TRX.CW_LPF_Filter == 400)
			TRX.CW_LPF_Filter = 350;
		else if (TRX.CW_LPF_Filter == 350)
			TRX.CW_LPF_Filter = 300;
		else if (TRX.CW_LPF_Filter == 300)
			TRX.CW_LPF_Filter = 250;
		else if (TRX.CW_LPF_Filter == 250)
			TRX.CW_LPF_Filter = 200;
		else if (TRX.CW_LPF_Filter == 200)
			TRX.CW_LPF_Filter = 150;
		else if (TRX.CW_LPF_Filter == 150)
			TRX.CW_LPF_Filter = 100;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_SSB_LPF_RX_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.SSB_LPF_RX_Filter == 0)
			TRX.SSB_LPF_RX_Filter = 1400;

		if (TRX.SSB_LPF_RX_Filter == 1400)
			TRX.SSB_LPF_RX_Filter = 1600;
		else if (TRX.SSB_LPF_RX_Filter == 1600)
			TRX.SSB_LPF_RX_Filter = 1800;
		else if (TRX.SSB_LPF_RX_Filter == 1800)
			TRX.SSB_LPF_RX_Filter = 2100;
		else if (TRX.SSB_LPF_RX_Filter == 2100)
			TRX.SSB_LPF_RX_Filter = 2300;
		else if (TRX.SSB_LPF_RX_Filter == 2300)
			TRX.SSB_LPF_RX_Filter = 2500;
		else if (TRX.SSB_LPF_RX_Filter == 2500)
			TRX.SSB_LPF_RX_Filter = 2700;
		else if (TRX.SSB_LPF_RX_Filter == 2700)
			TRX.SSB_LPF_RX_Filter = 2900;
		else if (TRX.SSB_LPF_RX_Filter == 2900)
			TRX.SSB_LPF_RX_Filter = 3000;
		else if (TRX.SSB_LPF_RX_Filter == 3000)
			TRX.SSB_LPF_RX_Filter = 3200;
		else if (TRX.SSB_LPF_RX_Filter == 3200)
			TRX.SSB_LPF_RX_Filter = 3400;
	}
	else
	{
		if (TRX.SSB_LPF_RX_Filter == 1600)
			TRX.SSB_LPF_RX_Filter = 1400;
		else if (TRX.SSB_LPF_RX_Filter == 1800)
			TRX.SSB_LPF_RX_Filter = 1600;
		else if (TRX.SSB_LPF_RX_Filter == 2100)
			TRX.SSB_LPF_RX_Filter = 1800;
		else if (TRX.SSB_LPF_RX_Filter == 2300)
			TRX.SSB_LPF_RX_Filter = 2100;
		else if (TRX.SSB_LPF_RX_Filter == 2500)
			TRX.SSB_LPF_RX_Filter = 2300;
		else if (TRX.SSB_LPF_RX_Filter == 2700)
			TRX.SSB_LPF_RX_Filter = 2500;
		else if (TRX.SSB_LPF_RX_Filter == 2900)
			TRX.SSB_LPF_RX_Filter = 2700;
		else if (TRX.SSB_LPF_RX_Filter == 3000)
			TRX.SSB_LPF_RX_Filter = 2900;
		else if (TRX.SSB_LPF_RX_Filter == 3200)
			TRX.SSB_LPF_RX_Filter = 3000;
		else if (TRX.SSB_LPF_RX_Filter == 3400)
			TRX.SSB_LPF_RX_Filter = 3200;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_SSB_LPF_TX_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.SSB_LPF_TX_Filter == 0)
			TRX.SSB_LPF_TX_Filter = 1400;

		if (TRX.SSB_LPF_TX_Filter == 1400)
			TRX.SSB_LPF_TX_Filter = 1600;
		else if (TRX.SSB_LPF_TX_Filter == 1600)
			TRX.SSB_LPF_TX_Filter = 1800;
		else if (TRX.SSB_LPF_TX_Filter == 1800)
			TRX.SSB_LPF_TX_Filter = 2100;
		else if (TRX.SSB_LPF_TX_Filter == 2100)
			TRX.SSB_LPF_TX_Filter = 2300;
		else if (TRX.SSB_LPF_TX_Filter == 2300)
			TRX.SSB_LPF_TX_Filter = 2500;
		else if (TRX.SSB_LPF_TX_Filter == 2500)
			TRX.SSB_LPF_TX_Filter = 2700;
		else if (TRX.SSB_LPF_TX_Filter == 2700)
			TRX.SSB_LPF_TX_Filter = 2900;
		else if (TRX.SSB_LPF_TX_Filter == 2900)
			TRX.SSB_LPF_TX_Filter = 3000;
		else if (TRX.SSB_LPF_TX_Filter == 3000)
			TRX.SSB_LPF_TX_Filter = 3200;
		else if (TRX.SSB_LPF_TX_Filter == 3200)
			TRX.SSB_LPF_TX_Filter = 3400;
	}
	else
	{
		if (TRX.SSB_LPF_TX_Filter == 1600)
			TRX.SSB_LPF_TX_Filter = 1400;
		else if (TRX.SSB_LPF_TX_Filter == 1800)
			TRX.SSB_LPF_TX_Filter = 1600;
		else if (TRX.SSB_LPF_TX_Filter == 2100)
			TRX.SSB_LPF_TX_Filter = 1800;
		else if (TRX.SSB_LPF_TX_Filter == 2300)
			TRX.SSB_LPF_TX_Filter = 2100;
		else if (TRX.SSB_LPF_TX_Filter == 2500)
			TRX.SSB_LPF_TX_Filter = 2300;
		else if (TRX.SSB_LPF_TX_Filter == 2700)
			TRX.SSB_LPF_TX_Filter = 2500;
		else if (TRX.SSB_LPF_TX_Filter == 2900)
			TRX.SSB_LPF_TX_Filter = 2700;
		else if (TRX.SSB_LPF_TX_Filter == 3000)
			TRX.SSB_LPF_TX_Filter = 2900;
		else if (TRX.SSB_LPF_TX_Filter == 3200)
			TRX.SSB_LPF_TX_Filter = 3000;
		else if (TRX.SSB_LPF_TX_Filter == 3400)
			TRX.SSB_LPF_TX_Filter = 3200;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_AM_LPF_RX_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.AM_LPF_RX_Filter == 2100)
			TRX.AM_LPF_RX_Filter = 2300;
		else if (TRX.AM_LPF_RX_Filter == 2300)
			TRX.AM_LPF_RX_Filter = 2500;
		else if (TRX.AM_LPF_RX_Filter == 2500)
			TRX.AM_LPF_RX_Filter = 2700;
		else if (TRX.AM_LPF_RX_Filter == 2700)
			TRX.AM_LPF_RX_Filter = 2900;
		else if (TRX.AM_LPF_RX_Filter == 2900)
			TRX.AM_LPF_RX_Filter = 3000;
		else if (TRX.AM_LPF_RX_Filter == 3000)
			TRX.AM_LPF_RX_Filter = 3200;
		else if (TRX.AM_LPF_RX_Filter == 3200)
			TRX.AM_LPF_RX_Filter = 3400;
		else if (TRX.AM_LPF_RX_Filter == 3400)
			TRX.AM_LPF_RX_Filter = 3600;
		else if (TRX.AM_LPF_RX_Filter == 3600)
			TRX.AM_LPF_RX_Filter = 3800;
		else if (TRX.AM_LPF_RX_Filter == 3800)
			TRX.AM_LPF_RX_Filter = 4000;
		else if (TRX.AM_LPF_RX_Filter == 4000)
			TRX.AM_LPF_RX_Filter = 4500;
		else if (TRX.AM_LPF_RX_Filter == 4500)
			TRX.AM_LPF_RX_Filter = 5000;
		else if (TRX.AM_LPF_RX_Filter == 5000)
			TRX.AM_LPF_RX_Filter = 6000;
		else if (TRX.AM_LPF_RX_Filter == 6000)
			TRX.AM_LPF_RX_Filter = 7000;
		else if (TRX.AM_LPF_RX_Filter == 7000)
			TRX.AM_LPF_RX_Filter = 8000;
		else if (TRX.AM_LPF_RX_Filter == 8000)
			TRX.AM_LPF_RX_Filter = 9000;
		else if (TRX.AM_LPF_RX_Filter == 9000)
			TRX.AM_LPF_RX_Filter = 10000;
	}
	else
	{
		if (TRX.AM_LPF_RX_Filter == 2300)
			TRX.AM_LPF_RX_Filter = 2100;
		else if (TRX.AM_LPF_RX_Filter == 2500)
			TRX.AM_LPF_RX_Filter = 2300;
		else if (TRX.AM_LPF_RX_Filter == 2700)
			TRX.AM_LPF_RX_Filter = 2500;
		else if (TRX.AM_LPF_RX_Filter == 2900)
			TRX.AM_LPF_RX_Filter = 2700;
		else if (TRX.AM_LPF_RX_Filter == 3000)
			TRX.AM_LPF_RX_Filter = 2900;
		else if (TRX.AM_LPF_RX_Filter == 3200)
			TRX.AM_LPF_RX_Filter = 3000;
		else if (TRX.AM_LPF_RX_Filter == 3400)
			TRX.AM_LPF_RX_Filter = 3200;
		else if (TRX.AM_LPF_RX_Filter == 3600)
			TRX.AM_LPF_RX_Filter = 3400;
		else if (TRX.AM_LPF_RX_Filter == 3800)
			TRX.AM_LPF_RX_Filter = 3400;
		else if (TRX.AM_LPF_RX_Filter == 4000)
			TRX.AM_LPF_RX_Filter = 3800;
		else if (TRX.AM_LPF_RX_Filter == 4500)
			TRX.AM_LPF_RX_Filter = 3800;
		else if (TRX.AM_LPF_RX_Filter == 5000)
			TRX.AM_LPF_RX_Filter = 4500;
		else if (TRX.AM_LPF_RX_Filter == 6000)
			TRX.AM_LPF_RX_Filter = 5000;
		else if (TRX.AM_LPF_RX_Filter == 7000)
			TRX.AM_LPF_RX_Filter = 6000;
		else if (TRX.AM_LPF_RX_Filter == 8000)
			TRX.AM_LPF_RX_Filter = 7000;
		else if (TRX.AM_LPF_RX_Filter == 9000)
			TRX.AM_LPF_RX_Filter = 8000;
		else if (TRX.AM_LPF_RX_Filter == 10000)
			TRX.AM_LPF_RX_Filter = 9000;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_AM_LPF_TX_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.AM_LPF_TX_Filter == 2100)
			TRX.AM_LPF_TX_Filter = 2300;
		else if (TRX.AM_LPF_TX_Filter == 2300)
			TRX.AM_LPF_TX_Filter = 2500;
		else if (TRX.AM_LPF_TX_Filter == 2500)
			TRX.AM_LPF_TX_Filter = 2700;
		else if (TRX.AM_LPF_TX_Filter == 2700)
			TRX.AM_LPF_TX_Filter = 2900;
		else if (TRX.AM_LPF_TX_Filter == 2900)
			TRX.AM_LPF_TX_Filter = 3000;
		else if (TRX.AM_LPF_TX_Filter == 3000)
			TRX.AM_LPF_TX_Filter = 3200;
		else if (TRX.AM_LPF_TX_Filter == 3200)
			TRX.AM_LPF_TX_Filter = 3400;
		else if (TRX.AM_LPF_TX_Filter == 3400)
			TRX.AM_LPF_TX_Filter = 3600;
		else if (TRX.AM_LPF_TX_Filter == 3600)
			TRX.AM_LPF_TX_Filter = 3800;
		else if (TRX.AM_LPF_TX_Filter == 3800)
			TRX.AM_LPF_TX_Filter = 4000;
		else if (TRX.AM_LPF_TX_Filter == 4000)
			TRX.AM_LPF_TX_Filter = 4500;
		else if (TRX.AM_LPF_TX_Filter == 4500)
			TRX.AM_LPF_TX_Filter = 5000;
		else if (TRX.AM_LPF_TX_Filter == 5000)
			TRX.AM_LPF_TX_Filter = 6000;
		else if (TRX.AM_LPF_TX_Filter == 6000)
			TRX.AM_LPF_TX_Filter = 7000;
		else if (TRX.AM_LPF_TX_Filter == 7000)
			TRX.AM_LPF_TX_Filter = 8000;
		else if (TRX.AM_LPF_TX_Filter == 8000)
			TRX.AM_LPF_TX_Filter = 9000;
		else if (TRX.AM_LPF_TX_Filter == 9000)
			TRX.AM_LPF_TX_Filter = 10000;
	}
	else
	{
		if (TRX.AM_LPF_TX_Filter == 2300)
			TRX.AM_LPF_TX_Filter = 2100;
		else if (TRX.AM_LPF_TX_Filter == 2500)
			TRX.AM_LPF_TX_Filter = 2300;
		else if (TRX.AM_LPF_TX_Filter == 2700)
			TRX.AM_LPF_TX_Filter = 2500;
		else if (TRX.AM_LPF_TX_Filter == 2900)
			TRX.AM_LPF_TX_Filter = 2700;
		else if (TRX.AM_LPF_TX_Filter == 3000)
			TRX.AM_LPF_TX_Filter = 2900;
		else if (TRX.AM_LPF_TX_Filter == 3200)
			TRX.AM_LPF_TX_Filter = 3000;
		else if (TRX.AM_LPF_TX_Filter == 3400)
			TRX.AM_LPF_TX_Filter = 3200;
		else if (TRX.AM_LPF_TX_Filter == 3600)
			TRX.AM_LPF_TX_Filter = 3400;
		else if (TRX.AM_LPF_TX_Filter == 3800)
			TRX.AM_LPF_TX_Filter = 3400;
		else if (TRX.AM_LPF_TX_Filter == 4000)
			TRX.AM_LPF_TX_Filter = 3800;
		else if (TRX.AM_LPF_TX_Filter == 4500)
			TRX.AM_LPF_TX_Filter = 3800;
		else if (TRX.AM_LPF_TX_Filter == 5000)
			TRX.AM_LPF_TX_Filter = 4500;
		else if (TRX.AM_LPF_TX_Filter == 6000)
			TRX.AM_LPF_TX_Filter = 5000;
		else if (TRX.AM_LPF_TX_Filter == 7000)
			TRX.AM_LPF_TX_Filter = 6000;
		else if (TRX.AM_LPF_TX_Filter == 8000)
			TRX.AM_LPF_TX_Filter = 7000;
		else if (TRX.AM_LPF_TX_Filter == 9000)
			TRX.AM_LPF_TX_Filter = 8000;
		else if (TRX.AM_LPF_TX_Filter == 10000)
			TRX.AM_LPF_TX_Filter = 9000;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_FM_LPF_RX_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.FM_LPF_RX_Filter == 5000)
			TRX.FM_LPF_RX_Filter = 6000;
		else if (TRX.FM_LPF_RX_Filter == 6000)
			TRX.FM_LPF_RX_Filter = 7000;
		else if (TRX.FM_LPF_RX_Filter == 7000)
			TRX.FM_LPF_RX_Filter = 8000;
		else if (TRX.FM_LPF_RX_Filter == 8000)
			TRX.FM_LPF_RX_Filter = 9000;
		else if (TRX.FM_LPF_RX_Filter == 9000)
			TRX.FM_LPF_RX_Filter = 10000;
		else if (TRX.FM_LPF_RX_Filter == 10000)
			TRX.FM_LPF_RX_Filter = 15000;
		else if (TRX.FM_LPF_RX_Filter == 15000)
			TRX.FM_LPF_RX_Filter = 20000;
	}
	else
	{
		if (TRX.FM_LPF_RX_Filter == 6000)
			TRX.FM_LPF_RX_Filter = 5000;
		else if (TRX.FM_LPF_RX_Filter == 7000)
			TRX.FM_LPF_RX_Filter = 6000;
		else if (TRX.FM_LPF_RX_Filter == 8000)
			TRX.FM_LPF_RX_Filter = 7000;
		else if (TRX.FM_LPF_RX_Filter == 9000)
			TRX.FM_LPF_RX_Filter = 8000;
		else if (TRX.FM_LPF_RX_Filter == 10000)
			TRX.FM_LPF_RX_Filter = 9000;
		else if (TRX.FM_LPF_RX_Filter == 15000)
			TRX.FM_LPF_RX_Filter = 10000;
		else if (TRX.FM_LPF_RX_Filter == 20000)
			TRX.FM_LPF_RX_Filter = 15000;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_FM_LPF_TX_pass(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.FM_LPF_TX_Filter == 5000)
			TRX.FM_LPF_TX_Filter = 6000;
		else if (TRX.FM_LPF_TX_Filter == 6000)
			TRX.FM_LPF_TX_Filter = 7000;
		else if (TRX.FM_LPF_TX_Filter == 7000)
			TRX.FM_LPF_TX_Filter = 8000;
		else if (TRX.FM_LPF_TX_Filter == 8000)
			TRX.FM_LPF_TX_Filter = 9000;
		else if (TRX.FM_LPF_TX_Filter == 9000)
			TRX.FM_LPF_TX_Filter = 10000;
		else if (TRX.FM_LPF_TX_Filter == 10000)
			TRX.FM_LPF_TX_Filter = 15000;
		else if (TRX.FM_LPF_TX_Filter == 15000)
			TRX.FM_LPF_TX_Filter = 20000;
	}
	else
	{
		if (TRX.FM_LPF_TX_Filter == 6000)
			TRX.FM_LPF_TX_Filter = 5000;
		else if (TRX.FM_LPF_TX_Filter == 7000)
			TRX.FM_LPF_TX_Filter = 6000;
		else if (TRX.FM_LPF_TX_Filter == 8000)
			TRX.FM_LPF_TX_Filter = 7000;
		else if (TRX.FM_LPF_TX_Filter == 9000)
			TRX.FM_LPF_TX_Filter = 8000;
		else if (TRX.FM_LPF_TX_Filter == 10000)
			TRX.FM_LPF_TX_Filter = 9000;
		else if (TRX.FM_LPF_TX_Filter == 15000)
			TRX.FM_LPF_TX_Filter = 10000;
		else if (TRX.FM_LPF_TX_Filter == 20000)
			TRX.FM_LPF_TX_Filter = 15000;
	}

	TRX_setMode(SecondaryVFO()->Mode, SecondaryVFO());
	TRX_setMode(CurrentVFO()->Mode, CurrentVFO());
}

static void SYSMENU_HANDL_AUDIO_Beeper(int8_t direction)
{
	if (direction > 0)
		TRX.Beeper = true;
	if (direction < 0)
		TRX.Beeper = false;
}

//CW MENU

static void SYSMENU_HANDL_CWMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_cw_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_cw_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_redraw(false);
}

void SYSMENU_CW_WPM_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_cw_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_cw_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 4;
	LCD_redraw(false);
}

void SYSMENU_CW_KEYER_HOTKEY(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_cw_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_cw_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 3;
	LCD_redraw(false);
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
	TRX.CW_Key_timeout += direction * 50;
	if (TRX.CW_Key_timeout < 50)
		TRX.CW_Key_timeout = 50;
	if (TRX.CW_Key_timeout > 5000)
		TRX.CW_Key_timeout = 5000;
}

static void SYSMENU_HANDL_CW_Keyer(int8_t direction)
{
	if (direction > 0)
		TRX.CW_KEYER = true;
	if (direction < 0)
		TRX.CW_KEYER = false;
}

static void SYSMENU_HANDL_CW_GaussFilter(int8_t direction)
{
	if (direction > 0)
		TRX.CW_GaussFilter = true;
	if (direction < 0)
		TRX.CW_GaussFilter = false;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_CW_Keyer_WPM(int8_t direction)
{
	TRX.CW_KEYER_WPM += direction;
	if (TRX.CW_KEYER_WPM < 1)
		TRX.CW_KEYER_WPM = 1;
	if (TRX.CW_KEYER_WPM > 200)
		TRX.CW_KEYER_WPM = 200;
}

static void SYSMENU_HANDL_CW_SelfHear(int8_t direction)
{
	if (direction > 0)
		TRX.CW_SelfHear = true;
	if (direction < 0)
		TRX.CW_SelfHear = false;
}

//SCREEN MENU

static void SYSMENU_HANDL_LCDMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_screen_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_screen_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_SCREEN_FFT_Enabled(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Enabled = true;
	if (direction < 0)
		TRX.FFT_Enabled = false;
}

static void SYSMENU_HANDL_SCREEN_COLOR_THEME(int8_t direction)
{
	if (direction > 0 || TRX.ColorThemeId > 0)
		TRX.ColorThemeId += direction;
	if (TRX.ColorThemeId > (COLOR_THEMES_COUNT - 1))
		TRX.ColorThemeId = (COLOR_THEMES_COUNT - 1);

	COLOR = &COLOR_THEMES[TRX.ColorThemeId];
	FFT_Init();
	LCD_redraw(false);
}

static void SYSMENU_HANDL_SCREEN_LAYOUT_THEME(int8_t direction)
{
	if (direction > 0 || TRX.LayoutThemeId > 0)
		TRX.LayoutThemeId += direction;
	if (TRX.LayoutThemeId > (LAYOUT_THEMES_COUNT - 1))
		TRX.LayoutThemeId = (LAYOUT_THEMES_COUNT - 1);

	LAYOUT = &LAYOUT_THEMES[TRX.LayoutThemeId];
	FFT_Init();
	LCD_redraw(false);
}

static void SYSMENU_HANDL_SCREEN_FFT_Compressor(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Compressor = true;
	if (direction < 0)
		TRX.FFT_Compressor = false;
}

static void SYSMENU_HANDL_SCREEN_FFT_Averaging(int8_t direction)
{
	TRX.FFT_Averaging += direction;
	if (TRX.FFT_Averaging < 1)
		TRX.FFT_Averaging = 1;
	if (TRX.FFT_Averaging > FFT_MAX_MEANS)
		TRX.FFT_Averaging = FFT_MAX_MEANS;
}

static void SYSMENU_HANDL_SCREEN_FFT_Window(int8_t direction)
{
	TRX.FFT_Window += direction;
	if (TRX.FFT_Window < 1)
		TRX.FFT_Window = 1;
	if (TRX.FFT_Window > 7)
		TRX.FFT_Window = 7;
	FFT_PreInit();
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Zoom(int8_t direction)
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

static void SYSMENU_HANDL_SCREEN_FFT_ZoomCW(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.FFT_ZoomCW == 1)
			TRX.FFT_ZoomCW = 2;
		else if (TRX.FFT_ZoomCW == 2)
			TRX.FFT_ZoomCW = 4;
		else if (TRX.FFT_ZoomCW == 4)
			TRX.FFT_ZoomCW = 8;
		else if (TRX.FFT_ZoomCW == 8)
			TRX.FFT_ZoomCW = 16;
	}
	else
	{
		if (TRX.FFT_ZoomCW == 2)
			TRX.FFT_ZoomCW = 1;
		else if (TRX.FFT_ZoomCW == 4)
			TRX.FFT_ZoomCW = 2;
		else if (TRX.FFT_ZoomCW == 8)
			TRX.FFT_ZoomCW = 4;
		else if (TRX.FFT_ZoomCW == 16)
			TRX.FFT_ZoomCW = 8;
	}
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Height(int8_t direction)
{
	TRX.FFT_Height += direction;
	if (TRX.FFT_Height < 1)
		TRX.FFT_Height = 1;
	if (TRX.FFT_Height > 3)
		TRX.FFT_Height = 3;
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Style(int8_t direction)
{
	TRX.FFT_Style += direction;
	if (TRX.FFT_Style < 1)
		TRX.FFT_Style = 1;
	if (TRX.FFT_Style > 4)
		TRX.FFT_Style = 4;
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Color(int8_t direction)
{
	TRX.FFT_Color += direction;
	if (TRX.FFT_Color < 1)
		TRX.FFT_Color = 1;
	if (TRX.FFT_Color > 7)
		TRX.FFT_Color = 7;
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Grid(int8_t direction)
{
	TRX.FFT_Grid += direction;
	if (TRX.FFT_Grid < 0)
		TRX.FFT_Grid = 0;
	if (TRX.FFT_Grid > 3)
		TRX.FFT_Grid = 3;
	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_WTF_Moving(int8_t direction)
{
	if (direction > 0)
		TRX.WTF_Moving = true;
	if (direction < 0)
		TRX.WTF_Moving = false;
}

static void SYSMENU_HANDL_SCREEN_FFT_Background(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Background = true;
	if (direction < 0)
		TRX.FFT_Background = false;
}

static void SYSMENU_HANDL_SCREEN_FFT_Lens(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Lens = true;
	if (direction < 0)
		TRX.FFT_Lens = false;
}

static void SYSMENU_HANDL_SCREEN_FFT_HoldPeaks(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_HoldPeaks = true;
	if (direction < 0)
		TRX.FFT_HoldPeaks = false;
}

static void SYSMENU_HANDL_SCREEN_LCD_Brightness(int8_t direction)
{
	TRX.LCD_Brightness += direction;
	if (TRX.LCD_Brightness < 1)
		TRX.LCD_Brightness = 1;
	if (TRX.LCD_Brightness > 100)
		TRX.LCD_Brightness = 100;
	LCDDriver_setBrightness(TRX.LCD_Brightness);
}

static void SYSMENU_HANDL_SCREEN_FFT_3D(int8_t direction)
{
	TRX.FFT_3D += direction;
	if (TRX.FFT_3D > 2)
		TRX.FFT_3D = 2;
}

static void SYSMENU_HANDL_SCREEN_FFT_Automatic(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Automatic = true;
	if (direction < 0)
		TRX.FFT_Automatic = false;
}

static void SYSMENU_HANDL_SCREEN_FFT_ManualBottom(int8_t direction)
{
	TRX.FFT_ManualBottom += direction;
	if (TRX.FFT_ManualBottom < -150)
		TRX.FFT_ManualBottom = -150;
	if (TRX.FFT_ManualBottom > 50)
		TRX.FFT_ManualBottom = 50;
}

static void SYSMENU_HANDL_SCREEN_FFT_ManualTop(int8_t direction)
{
	TRX.FFT_ManualTop += direction;
	if (TRX.FFT_ManualTop < -150)
		TRX.FFT_ManualTop = -150;
	if (TRX.FFT_ManualTop > 50)
		TRX.FFT_ManualTop = 50;
}

static void SYSMENU_HANDL_SCREEN_FFT_Speed(int8_t direction)
{
	TRX.FFT_Speed += direction;
	if (TRX.FFT_Speed < 1)
		TRX.FFT_Speed = 1;
	if (TRX.FFT_Speed > 5)
		TRX.FFT_Speed = 5;
}

static void SYSMENU_HANDL_SCREEN_FFT_Sensitivity(int8_t direction)
{
	TRX.FFT_Sensitivity += direction;
	if (TRX.FFT_Sensitivity < FFT_MIN + 1)
		TRX.FFT_Sensitivity = FFT_MIN + 1;
	if (TRX.FFT_Sensitivity > FFT_MAX_TOP_SCALE)
		TRX.FFT_Sensitivity = FFT_MAX_TOP_SCALE;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON1(int8_t direction)
{
	if (TRX.FuncButtons[0] > 0 || direction > 0)
		TRX.FuncButtons[0] += direction;
	if (TRX.FuncButtons[0] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[0] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON2(int8_t direction)
{
	if (TRX.FuncButtons[1] > 0 || direction > 0)
		TRX.FuncButtons[1] += direction;
	if (TRX.FuncButtons[1] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[1] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON3(int8_t direction)
{
	if (TRX.FuncButtons[2] > 0 || direction > 0)
		TRX.FuncButtons[2] += direction;
	if (TRX.FuncButtons[2] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[2] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON4(int8_t direction)
{
	if (TRX.FuncButtons[3] > 0 || direction > 0)
		TRX.FuncButtons[3] += direction;
	if (TRX.FuncButtons[3] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[3] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON5(int8_t direction)
{
	if (TRX.FuncButtons[4] > 0 || direction > 0)
		TRX.FuncButtons[4] += direction;
	if (TRX.FuncButtons[4] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[4] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON6(int8_t direction)
{
	if (TRX.FuncButtons[5] > 0 || direction > 0)
		TRX.FuncButtons[5] += direction;
	if (TRX.FuncButtons[5] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[5] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON7(int8_t direction)
{
	if (TRX.FuncButtons[6] > 0 || direction > 0)
		TRX.FuncButtons[6] += direction;
	if (TRX.FuncButtons[6] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[6] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON8(int8_t direction)
{
	if (TRX.FuncButtons[7] > 0 || direction > 0)
		TRX.FuncButtons[7] += direction;
	if (TRX.FuncButtons[7] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[7] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON9(int8_t direction)
{
	if (TRX.FuncButtons[8] > 0 || direction > 0)
		TRX.FuncButtons[8] += direction;
	if (TRX.FuncButtons[8] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[8] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON10(int8_t direction)
{
	if (TRX.FuncButtons[9] > 0 || direction > 0)
		TRX.FuncButtons[9] += direction;
	if (TRX.FuncButtons[9] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[9] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON11(int8_t direction)
{
	if (TRX.FuncButtons[10] > 0 || direction > 0)
		TRX.FuncButtons[10] += direction;
	if (TRX.FuncButtons[10] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[10] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON12(int8_t direction)
{
	if (TRX.FuncButtons[11] > 0 || direction > 0)
		TRX.FuncButtons[11] += direction;
	if (TRX.FuncButtons[11] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[11] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON13(int8_t direction)
{
	if (TRX.FuncButtons[12] > 0 || direction > 0)
		TRX.FuncButtons[12] += direction;
	if (TRX.FuncButtons[12] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[12] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON14(int8_t direction)
{
	if (TRX.FuncButtons[13] > 0 || direction > 0)
		TRX.FuncButtons[13] += direction;
	if (TRX.FuncButtons[13] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[13] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON15(int8_t direction)
{
	if (TRX.FuncButtons[14] > 0 || direction > 0)
		TRX.FuncButtons[14] += direction;
	if (TRX.FuncButtons[14] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[14] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON16(int8_t direction)
{
	if (TRX.FuncButtons[15] > 0 || direction > 0)
		TRX.FuncButtons[15] += direction;
	if (TRX.FuncButtons[15] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[15] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON17(int8_t direction)
{
	if (TRX.FuncButtons[16] > 0 || direction > 0)
		TRX.FuncButtons[16] += direction;
	if (TRX.FuncButtons[16] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[16] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON18(int8_t direction)
{
	if (TRX.FuncButtons[17] > 0 || direction > 0)
		TRX.FuncButtons[17] += direction;
	if (TRX.FuncButtons[17] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[17] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON19(int8_t direction)
{
	if (TRX.FuncButtons[18] > 0 || direction > 0)
		TRX.FuncButtons[18] += direction;
	if (TRX.FuncButtons[18] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[18] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON20(int8_t direction)
{
	if (TRX.FuncButtons[19] > 0 || direction > 0)
		TRX.FuncButtons[19] += direction;
	if (TRX.FuncButtons[19] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[19] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON21(int8_t direction)
{
	if (TRX.FuncButtons[20] > 0 || direction > 0)
		TRX.FuncButtons[20] += direction;
	if (TRX.FuncButtons[20] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[20] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON22(int8_t direction)
{
	if (TRX.FuncButtons[21] > 0 || direction > 0)
		TRX.FuncButtons[21] += direction;
	if (TRX.FuncButtons[21] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[21] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON23(int8_t direction)
{
	if (TRX.FuncButtons[22] > 0 || direction > 0)
		TRX.FuncButtons[22] += direction;
	if (TRX.FuncButtons[22] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[22] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON24(int8_t direction)
{
	if (TRX.FuncButtons[23] > 0 || direction > 0)
		TRX.FuncButtons[23] += direction;
	if (TRX.FuncButtons[23] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[23] = FUNCBUTTONS_COUNT - 1;
}

//ADC/DAC MENU

static void SYSMENU_HANDL_ADCMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_adc_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_adc_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_ADC_DRIVER(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_Driver = true;
	if (direction < 0)
		TRX.ADC_Driver = false;
	int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
	if (band > 0)
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
	if (band > 0)
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
#pragma unused(direction)
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_wifi_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_wifi_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_WIFI_RedrawSelectAPMenu(void)
{
	LCD_UpdateQuery.SystemMenuRedraw = true;
}


static void SYSMENU_WIFI_DrawSelectAP1Menu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		uint16_t curr_x = 5;
		LCDDriver_printText("NET1 Found:", curr_x, 5, FG_COLOR, BG_COLOR, 2);
		curr_x += 28;
		LCDDriver_printText(">Refresh", 10, curr_x, COLOR_WHITE, BG_COLOR, 2);
		curr_x += 24;
		for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
			LCDDriver_printText((char *)WIFI_FoundedAP[i], 10, curr_x + i * 24, COLOR_GREEN, BG_COLOR, 2);
		LCDDriver_drawFastHLine(0, 49 + sysmenu_wifi_selected_ap_index * 24, LAYOUT->SYSMENU_W, FG_COLOR);
	}
	if(sysmenu_wifi_needupdate_ap)
	{
		sysmenu_wifi_needupdate_ap = false;
		WIFI_ListAP(SYSMENU_HANDL_WIFI_RedrawSelectAPMenu);
	}
}

static void SYSMENU_WIFI_DrawSelectAP2Menu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		uint16_t curr_x = 5;
		LCDDriver_printText("NET2 Found:", curr_x, 5, FG_COLOR, BG_COLOR, 2);
		curr_x += 28;
		LCDDriver_printText(">Refresh", 10, curr_x, COLOR_WHITE, BG_COLOR, 2);
		curr_x += 24;
		for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
			LCDDriver_printText((char *)WIFI_FoundedAP[i], 10, curr_x + i * 24, COLOR_GREEN, BG_COLOR, 2);
		LCDDriver_drawFastHLine(0, 49 + sysmenu_wifi_selected_ap_index * 24, LAYOUT->SYSMENU_W, FG_COLOR);
	}
	if(sysmenu_wifi_needupdate_ap)
	{
		sysmenu_wifi_needupdate_ap = false;
		WIFI_ListAP(SYSMENU_HANDL_WIFI_RedrawSelectAPMenu);
	}
}

static void SYSMENU_WIFI_DrawSelectAP3Menu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		uint16_t curr_x = 5;
		LCDDriver_printText("NET3 Found:", curr_x, 5, FG_COLOR, BG_COLOR, 2);
		curr_x += 28;
		LCDDriver_printText(">Refresh", 10, curr_x, COLOR_WHITE, BG_COLOR, 2);
		curr_x += 24;
		for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
			LCDDriver_printText((char *)WIFI_FoundedAP[i], 10, curr_x + i * 24, COLOR_GREEN, BG_COLOR, 2);
		LCDDriver_drawFastHLine(0, 49 + sysmenu_wifi_selected_ap_index * 24, LAYOUT->SYSMENU_W, FG_COLOR);
	}
	if(sysmenu_wifi_needupdate_ap)
	{
		sysmenu_wifi_needupdate_ap = false;
		WIFI_ListAP(SYSMENU_HANDL_WIFI_RedrawSelectAPMenu);
	}
}

static void SYSMENU_WIFI_SelectAP1MenuMove(int8_t dir)
{
	if (dir < 0 && sysmenu_wifi_selected_ap_index > 0)
		sysmenu_wifi_selected_ap_index--;
	if (dir > 0 && sysmenu_wifi_selected_ap_index < WIFI_FOUNDED_AP_MAXCOUNT)
		sysmenu_wifi_selected_ap_index++;
	SYSMENU_WIFI_DrawSelectAP1Menu(true);
	if (dir == 0)
	{
		if(sysmenu_wifi_selected_ap_index == 0)
		{
			sysmenu_wifi_needupdate_ap = true;
			dma_memset((void*)WIFI_FoundedAP, 0, sizeof(WIFI_FoundedAP));
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		else
		{
			strcpy(TRX.WIFI_AP1, (char *)&WIFI_FoundedAP[sysmenu_wifi_selected_ap_index - 1]);
			WIFI_State = WIFI_CONFIGURED;
			sysmenu_wifi_selectap1_menu_opened = false;
			systemMenuIndex = 0;
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
	}
}

static void SYSMENU_WIFI_SelectAP2MenuMove(int8_t dir)
{
	if (dir < 0 && sysmenu_wifi_selected_ap_index > 0)
		sysmenu_wifi_selected_ap_index--;
	if (dir > 0 && sysmenu_wifi_selected_ap_index < WIFI_FOUNDED_AP_MAXCOUNT)
		sysmenu_wifi_selected_ap_index++;
	SYSMENU_WIFI_DrawSelectAP2Menu(true);
	if (dir == 0)
	{
		if(sysmenu_wifi_selected_ap_index == 0)
		{
			sysmenu_wifi_needupdate_ap = true;
			dma_memset((void*)WIFI_FoundedAP, 0, sizeof(WIFI_FoundedAP));
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		else
		{
			strcpy(TRX.WIFI_AP2, (char *)&WIFI_FoundedAP[sysmenu_wifi_selected_ap_index - 1]);
			WIFI_State = WIFI_CONFIGURED;
			sysmenu_wifi_selectap2_menu_opened = false;
			systemMenuIndex = 0;
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
	}
}

static void SYSMENU_WIFI_SelectAP3MenuMove(int8_t dir)
{
	if (dir < 0 && sysmenu_wifi_selected_ap_index > 0)
		sysmenu_wifi_selected_ap_index--;
	if (dir > 0 && sysmenu_wifi_selected_ap_index < WIFI_FOUNDED_AP_MAXCOUNT)
		sysmenu_wifi_selected_ap_index++;
	SYSMENU_WIFI_DrawSelectAP3Menu(true);
	if (dir == 0)
	{
		if(sysmenu_wifi_selected_ap_index == 0)
		{
			sysmenu_wifi_needupdate_ap = true;
			dma_memset((void*)WIFI_FoundedAP, 0, sizeof(WIFI_FoundedAP));
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		else
		{
			strcpy(TRX.WIFI_AP3, (char *)&WIFI_FoundedAP[sysmenu_wifi_selected_ap_index - 1]);
			WIFI_State = WIFI_CONFIGURED;
			sysmenu_wifi_selectap3_menu_opened = false;
			systemMenuIndex = 0;
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
	}
}

static void SYSMENU_WIFI_DrawAP1passwordMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("NET1 Password:", 5, 5, FG_COLOR, BG_COLOR, 2);
	}

	LCDDriver_printText(TRX.WIFI_PASSWORD1, 10, 37, COLOR_GREEN, BG_COLOR, 2);
	LCDDriver_drawFastHLine(8 + sysmenu_wifi_selected_ap_password_char_index * 12, 54, 12, COLOR_RED);
}

static void SYSMENU_WIFI_DrawAP2passwordMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("NET2 Password:", 5, 5, FG_COLOR, BG_COLOR, 2);
	}

	LCDDriver_printText(TRX.WIFI_PASSWORD2, 10, 37, COLOR_GREEN, BG_COLOR, 2);
	LCDDriver_drawFastHLine(8 + sysmenu_wifi_selected_ap_password_char_index * 12, 54, 12, COLOR_RED);
}

static void SYSMENU_WIFI_DrawAP3passwordMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("NET3 Password:", 5, 5, FG_COLOR, BG_COLOR, 2);
	}

	LCDDriver_printText(TRX.WIFI_PASSWORD3, 10, 37, COLOR_GREEN, BG_COLOR, 2);
	LCDDriver_drawFastHLine(8 + sysmenu_wifi_selected_ap_password_char_index * 12, 54, 12, COLOR_RED);
}

static void SYSMENU_WIFI_RotatePasswordChar1(int8_t dir)
{
	bool full_redraw = false;
	if (TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;
	TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] += dir;

	// do not show special characters
	if (TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] >= 1 && TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir > 0)
		TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] = 33;
	if (TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] >= 1 && TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir < 0)
		TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] >= 127)
		TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (TRX.WIFI_PASSWORD1[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		LCD_UpdateQuery.SystemMenuRedraw = true;
	else
		LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_WIFI_RotatePasswordChar2(int8_t dir)
{
	bool full_redraw = false;
	if (TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;
	TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] += dir;

	// do not show special characters
	if (TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] >= 1 && TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir > 0)
		TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] = 33;
	if (TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] >= 1 && TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir < 0)
		TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] >= 127)
		TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (TRX.WIFI_PASSWORD2[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		LCD_UpdateQuery.SystemMenuRedraw = true;
	else
		LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_WIFI_RotatePasswordChar3(int8_t dir)
{
	bool full_redraw = false;
	if (TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;
	TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] += dir;

	// do not show special characters
	if (TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] >= 1 && TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir > 0)
		TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] = 33;
	if (TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] >= 1 && TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir < 0)
		TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] >= 127)
		TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (TRX.WIFI_PASSWORD3[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		LCD_UpdateQuery.SystemMenuRedraw = true;
	else
		LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_HANDL_WIFI_Enabled(int8_t direction)
{
	if (direction > 0)
		TRX.WIFI_Enabled = true;
	if (direction < 0)
		TRX.WIFI_Enabled = false;
}

static void SYSMENU_HANDL_WIFI_SelectAP1(int8_t direction)
{
#pragma unused(direction)
	sysmenu_wifi_needupdate_ap = true;
	sysmenu_wifi_selected_ap_index = 0;
	sysmenu_wifi_selectap1_menu_opened = true;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_WIFI_SelectAP2(int8_t direction)
{
#pragma unused(direction)
	sysmenu_wifi_needupdate_ap = true;
	sysmenu_wifi_selected_ap_index = 0;
	sysmenu_wifi_selectap2_menu_opened = true;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_WIFI_SelectAP3(int8_t direction)
{
#pragma unused(direction)
	sysmenu_wifi_needupdate_ap = true;
	sysmenu_wifi_selected_ap_index = 0;
	sysmenu_wifi_selectap3_menu_opened = true;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_WIFI_SetAP1password(int8_t direction)
{
#pragma unused(direction)
	sysmenu_wifi_selected_ap_password_char_index = 0;
	sysmenu_wifi_setAP1password_menu_opened = true;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_WIFI_SetAP2password(int8_t direction)
{
#pragma unused(direction)
	sysmenu_wifi_selected_ap_password_char_index = 0;
	sysmenu_wifi_setAP2password_menu_opened = true;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_WIFI_SetAP3password(int8_t direction)
{
#pragma unused(direction)
	sysmenu_wifi_selected_ap_password_char_index = 0;
	sysmenu_wifi_setAP3password_menu_opened = true;
	LCD_UpdateQuery.SystemMenuRedraw = true;
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

static void SYSMENU_HANDL_WIFI_CAT_Server(int8_t direction)
{
	if (direction > 0)
		TRX.WIFI_CAT_SERVER = true;
	if (direction < 0)
		TRX.WIFI_CAT_SERVER = false;
}

static void SYSMENU_HANDL_WIFI_UpdateFW(int8_t direction)
{
#pragma unused(direction)
	LCD_systemMenuOpened = false;
	LCD_redraw(false);
	LCD_doEvents();
	WIFI_UpdateFW(NULL);
	LCD_showTooltip("Started, see console");
}

//SD MENU

static void SYSMENU_HANDL_SDMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_sd_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_sd_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_SD_USB(int8_t direction)
{
	if (direction > 0 && SD_isIdle() && !LCD_busy)
	{
		TRX_Mute = true;
		SD_USBCardReader = true;
		USBD_Restart();
	}
	else
	{
		SD_USBCardReader = false;
		USBD_Restart();
		TRX_Mute = false;
	}
}

static void SYSMENU_HANDL_SD_ExportSettings(int8_t direction)
{
	if (direction > 0 && SD_isIdle() && !LCD_busy)
	{
		SD_doCommand(SDCOMM_EXPORT_SETTINGS, false);
	}
}

static void SYSMENU_HANDL_SD_ImportSettings(int8_t direction)
{
	if (direction > 0 && SD_isIdle() && !LCD_busy)
	{
		SD_doCommand(SDCOMM_IMPORT_SETTINGS, false);
		NeedSaveCalibration = true;
	}
}

static void SYSMENU_HANDL_SD_Format(int8_t direction)
{
	if (direction > 0 && SD_isIdle() && !LCD_busy)
	{
		SD_doCommand(SDCOMM_FORMAT, false);
	}
}

//SET TIME MENU

static void SYSMENU_HANDL_SETTIME(int8_t direction)
{
	if (!sysmenu_timeMenuOpened)
		LCDDriver_Fill(BG_COLOR);
	sysmenu_timeMenuOpened = true;
	static uint8_t Hours;
	static uint8_t Minutes;
	static uint8_t Seconds;
	static uint32_t Time;
	char ctmp[50];
	Time = RTC->TR;
	Hours = ((Time >> 20) & 0x03) * 10 + ((Time >> 16) & 0x0f);
	Minutes = ((Time >> 12) & 0x07) * 10 + ((Time >> 8) & 0x0f);
	Seconds = ((Time >> 4) & 0x07) * 10 + ((Time >> 0) & 0x0f);
	if (direction != 0)
	{
		if (TimeMenuSelection == 0)
		{
			if (Hours == 0 && direction < 0)
			{
				LCD_UpdateQuery.SystemMenuRedraw = true;
				return;
			}
			Hours = (uint8_t)(Hours + direction);
		}
		if (TimeMenuSelection == 1)
		{
			if (Minutes == 0 && direction < 0)
			{
				LCD_UpdateQuery.SystemMenuRedraw = true;
				return;
			}
			Minutes = (uint8_t)(Minutes + direction);
		}
		if (TimeMenuSelection == 2)
		{
			if (Seconds == 0 && direction < 0)
				return;
			Seconds = (uint8_t)(Seconds + direction);
		}
		if (Hours >= 24)
			Hours = 0;
		if (Minutes >= 60)
			Minutes = 0;
		if (Seconds >= 60)
			Seconds = 0;
		RTC_TimeTypeDef sTime;
		sTime.TimeFormat = RTC_HOURFORMAT12_PM;
		sTime.SubSeconds = 0;
		sTime.SecondFraction = 0;
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_SET;
		sTime.Hours = Hours;
		sTime.Minutes = Minutes;
		sTime.Seconds = Seconds;
		BKPSRAM_Enable();
		HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	if(direction == 0)
	{
		sprintf(ctmp, "%d", Hours);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printText(ctmp, 76, 100, COLOR->BUTTON_TEXT, TimeMenuSelection == 0 ? FG_COLOR : BG_COLOR, 3);
		LCDDriver_printText(":", 124, 100, COLOR->BUTTON_TEXT, BG_COLOR, 3);
		sprintf(ctmp, "%d", Minutes);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printText(ctmp, 148, 100, COLOR->BUTTON_TEXT, TimeMenuSelection == 1 ? FG_COLOR : BG_COLOR, 3);
		LCDDriver_printText(":", 194, 100, COLOR->BUTTON_TEXT, BG_COLOR, 3);
		sprintf(ctmp, "%d", Seconds);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printText(ctmp, 220, 100, COLOR->BUTTON_TEXT, TimeMenuSelection == 2 ? FG_COLOR : BG_COLOR, 3);
	}
}

//FLASH MENU

static void SYSMENU_HANDL_Bootloader(int8_t direction)
{
#pragma unused(direction)
	WM8731_CleanBuffer();
	TRX_NeedGoToBootloader = true;
	TRX_Inited = false;
	LCD_busy = true;
}

//SYSTEM INFO
static void SYSMENU_HANDL_SYSINFO(int8_t direction)
{
	sysmenu_infowindow_opened = true;
	sysmenu_sysinfo_opened = true;
	if (direction != 0)
		LCDDriver_Fill(BG_COLOR);
#define y_offs 20
	uint16_t y = 10;
	char out[80];
	sprintf(out, "STM32 FW ver: %s", version_string);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, 2);
	y += y_offs;
	sprintf(out, "FPGA FW ver: %d.%d.%d", FPGA_FW_Version[2], FPGA_FW_Version[1], FPGA_FW_Version[0]);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, 2);
	y += y_offs;
	sprintf(out, "WIFI IP: %s", WIFI_IP);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, 2);
	y += y_offs;
	sprintf(out, "FPGA SAMPLES: %d     ", dbg_FPGA_samples);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, 2);
	y += y_offs;
	sprintf(out, "STM32 VOLTAGE: %f     ", TRX_STM32_VREF);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, 2);
	y += y_offs;
	sprintf(out, "IQ PHASE: %f     ", TRX_IQ_phase_error);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, 2);
	y += y_offs;
	sprintf(out, "ADC MIN/MAX: %d/%d     ", TRX_ADC_MINAMPLITUDE, TRX_ADC_MAXAMPLITUDE);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, 2);
	y += y_offs;
	sprintf(out, "VCXO ERROR: %d     ", TRX_VCXO_ERROR);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, 2);
	y += y_offs;

	LCD_UpdateQuery.SystemMenu = true;
}

//CALIBRATION MENU

static void SYSMENU_HANDL_CALIB_ENCODER_INVERT(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENCODER_INVERT = true;
	if (direction < 0)
		CALIBRATE.ENCODER_INVERT = false;
}

static void SYSMENU_HANDL_CALIB_ENCODER2_INVERT(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENCODER2_INVERT = true;
	if (direction < 0)
		CALIBRATE.ENCODER2_INVERT = false;
}

static void SYSMENU_HANDL_CALIB_ENCODER_DEBOUNCE(int8_t direction)
{
	if (CALIBRATE.ENCODER_DEBOUNCE > 0 || direction > 0)
		CALIBRATE.ENCODER_DEBOUNCE += direction;
	if (CALIBRATE.ENCODER_DEBOUNCE > 250)
		CALIBRATE.ENCODER_DEBOUNCE = 250;
}

static void SYSMENU_HANDL_CALIB_ENCODER2_DEBOUNCE(int8_t direction)
{
	if (CALIBRATE.ENCODER2_DEBOUNCE > 0 || direction > 0)
		CALIBRATE.ENCODER2_DEBOUNCE += direction;
	if (CALIBRATE.ENCODER2_DEBOUNCE > 250)
		CALIBRATE.ENCODER2_DEBOUNCE = 250;
}

static void SYSMENU_HANDL_CALIB_ENCODER_SLOW_RATE(int8_t direction)
{
	CALIBRATE.ENCODER_SLOW_RATE += direction;
	if (CALIBRATE.ENCODER_SLOW_RATE < 1)
		CALIBRATE.ENCODER_SLOW_RATE = 1;
	if (CALIBRATE.ENCODER_SLOW_RATE > 100)
		CALIBRATE.ENCODER_SLOW_RATE = 100;
}

static void SYSMENU_HANDL_CALIB_ENCODER_ON_FALLING(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENCODER_ON_FALLING = true;
	if (direction < 0)
		CALIBRATE.ENCODER_ON_FALLING = false;
}

static void SYSMENU_HANDL_CALIB_ENCODER_ACCELERATION(int8_t direction)
{
	CALIBRATE.ENCODER_ACCELERATION += direction;
	if (CALIBRATE.ENCODER_ACCELERATION < 1)
		CALIBRATE.ENCODER_ACCELERATION = 1;
	if (CALIBRATE.ENCODER_ACCELERATION > 250)
		CALIBRATE.ENCODER_ACCELERATION = 250;
}

static void SYSMENU_HANDL_CALIBRATIONMENU(int8_t direction)
{
#pragma unused(direction)
	if (!SYSMENU_hiddenmenu_enabled)
		return;
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_calibration_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_calibration_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_SWR_Tandem_Ctrl(int8_t direction) //Tisho
{
#pragma unused(direction)
	if (SYSMENU_TDM_CTRL_opened)
	{
	}
	else
	{
		SYSMENU_TDM_CTRL_opened = true;
		TDM_Voltages_Start();
		//		drawSystemMenu(true);
	}
}

static void SYSMENU_HANDL_CALIB_CICCOMP_48K_SHIFT(int8_t direction)
{
	CALIBRATE.CICFIR_GAINER_48K_val += direction;
	if (CALIBRATE.CICFIR_GAINER_48K_val < 32)
		CALIBRATE.CICFIR_GAINER_48K_val = 32;
	if (CALIBRATE.CICFIR_GAINER_48K_val > 61)
		CALIBRATE.CICFIR_GAINER_48K_val = 61;
}

static void SYSMENU_HANDL_CALIB_CICCOMP_96K_SHIFT(int8_t direction)
{
	CALIBRATE.CICFIR_GAINER_96K_val += direction;
	if (CALIBRATE.CICFIR_GAINER_96K_val < 32)
		CALIBRATE.CICFIR_GAINER_96K_val = 32;
	if (CALIBRATE.CICFIR_GAINER_96K_val > 61)
		CALIBRATE.CICFIR_GAINER_96K_val = 61;
}

static void SYSMENU_HANDL_CALIB_CICCOMP_192K_SHIFT(int8_t direction)
{
	CALIBRATE.CICFIR_GAINER_192K_val += direction;
	if (CALIBRATE.CICFIR_GAINER_192K_val < 32)
		CALIBRATE.CICFIR_GAINER_192K_val = 32;
	if (CALIBRATE.CICFIR_GAINER_192K_val > 61)
		CALIBRATE.CICFIR_GAINER_192K_val = 61;
}

static void SYSMENU_HANDL_CALIB_CICCOMP_384K_SHIFT(int8_t direction)
{
	CALIBRATE.CICFIR_GAINER_384K_val += direction;
	if (CALIBRATE.CICFIR_GAINER_384K_val < 32)
		CALIBRATE.CICFIR_GAINER_384K_val = 32;
	if (CALIBRATE.CICFIR_GAINER_384K_val > 61)
		CALIBRATE.CICFIR_GAINER_384K_val = 61;
}

static void SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT(int8_t direction)
{
	CALIBRATE.TXCICFIR_GAINER_val += direction;
	if (CALIBRATE.TXCICFIR_GAINER_val < 16)
		CALIBRATE.TXCICFIR_GAINER_val = 16;
	if (CALIBRATE.TXCICFIR_GAINER_val > 48)
		CALIBRATE.TXCICFIR_GAINER_val = 48;
}

static void SYSMENU_HANDL_CALIB_DAC_SHIFT(int8_t direction)
{
	CALIBRATE.DAC_GAINER_val += direction;
	if (CALIBRATE.DAC_GAINER_val < 14)
		CALIBRATE.DAC_GAINER_val = 14;
	if (CALIBRATE.DAC_GAINER_val > 28)
		CALIBRATE.DAC_GAINER_val = 28;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_2200M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_2200m > 0)
		CALIBRATE.rf_out_power_2200m += direction;
	if (CALIBRATE.rf_out_power_2200m == 0 && direction > 0)
		CALIBRATE.rf_out_power_2200m += direction;
	if (CALIBRATE.rf_out_power_2200m > 100)
		CALIBRATE.rf_out_power_2200m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_160M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_160m > 0)
		CALIBRATE.rf_out_power_160m += direction;
	if (CALIBRATE.rf_out_power_160m == 0 && direction > 0)
		CALIBRATE.rf_out_power_160m += direction;
	if (CALIBRATE.rf_out_power_160m > 100)
		CALIBRATE.rf_out_power_160m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_80M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_80m > 0)
		CALIBRATE.rf_out_power_80m += direction;
	if (CALIBRATE.rf_out_power_80m == 0 && direction > 0)
		CALIBRATE.rf_out_power_80m += direction;
	if (CALIBRATE.rf_out_power_80m > 100)
		CALIBRATE.rf_out_power_80m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_40M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_40m > 0)
		CALIBRATE.rf_out_power_40m += direction;
	if (CALIBRATE.rf_out_power_40m == 0 && direction > 0)
		CALIBRATE.rf_out_power_40m += direction;
	if (CALIBRATE.rf_out_power_40m > 100)
		CALIBRATE.rf_out_power_40m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_30M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_30m > 0)
		CALIBRATE.rf_out_power_30m += direction;
	if (CALIBRATE.rf_out_power_30m == 0 && direction > 0)
		CALIBRATE.rf_out_power_30m += direction;
	if (CALIBRATE.rf_out_power_30m > 100)
		CALIBRATE.rf_out_power_30m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_20M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_20m > 0)
		CALIBRATE.rf_out_power_20m += direction;
	if (CALIBRATE.rf_out_power_20m == 0 && direction > 0)
		CALIBRATE.rf_out_power_20m += direction;
	if (CALIBRATE.rf_out_power_20m > 100)
		CALIBRATE.rf_out_power_20m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_17M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_17m > 0)
		CALIBRATE.rf_out_power_17m += direction;
	if (CALIBRATE.rf_out_power_17m == 0 && direction > 0)
		CALIBRATE.rf_out_power_17m += direction;
	if (CALIBRATE.rf_out_power_17m > 100)
		CALIBRATE.rf_out_power_17m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_15M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_15m > 0)
		CALIBRATE.rf_out_power_15m += direction;
	if (CALIBRATE.rf_out_power_15m == 0 && direction > 0)
		CALIBRATE.rf_out_power_15m += direction;
	if (CALIBRATE.rf_out_power_15m > 100)
		CALIBRATE.rf_out_power_15m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_12M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_12m > 0)
		CALIBRATE.rf_out_power_12m += direction;
	if (CALIBRATE.rf_out_power_12m == 0 && direction > 0)
		CALIBRATE.rf_out_power_12m += direction;
	if (CALIBRATE.rf_out_power_12m > 100)
		CALIBRATE.rf_out_power_12m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_10M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_10m > 0)
		CALIBRATE.rf_out_power_10m += direction;
	if (CALIBRATE.rf_out_power_10m == 0 && direction > 0)
		CALIBRATE.rf_out_power_10m += direction;
	if (CALIBRATE.rf_out_power_10m > 100)
		CALIBRATE.rf_out_power_10m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_6M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_6m > 0)
		CALIBRATE.rf_out_power_6m += direction;
	if (CALIBRATE.rf_out_power_6m == 0 && direction > 0)
		CALIBRATE.rf_out_power_6m += direction;
	if (CALIBRATE.rf_out_power_6m > 100)
		CALIBRATE.rf_out_power_6m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_2M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_2m > 0)
		CALIBRATE.rf_out_power_2m += direction;
	if (CALIBRATE.rf_out_power_2m == 0 && direction > 0)
		CALIBRATE.rf_out_power_2m += direction;
	if (CALIBRATE.rf_out_power_2m > 100)
		CALIBRATE.rf_out_power_2m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO()->Freq);
}

static void SYSMENU_HANDL_CALIB_S_METER(int8_t direction)
{
	CALIBRATE.smeter_calibration += direction;
	if (CALIBRATE.smeter_calibration < -50)
		CALIBRATE.smeter_calibration = -50;
	if (CALIBRATE.smeter_calibration > 50)
		CALIBRATE.smeter_calibration = 50;
}

static void SYSMENU_HANDL_CALIB_ADC_OFFSET(int8_t direction)
{
	CALIBRATE.adc_offset += direction;
	if (CALIBRATE.adc_offset < -500)
		CALIBRATE.adc_offset = -500;
	if (CALIBRATE.adc_offset > 500)
		CALIBRATE.adc_offset = 500;
}

static void SYSMENU_HANDL_CALIB_LPF_END(int8_t direction)
{
	CALIBRATE.LPF_END += direction * 100000;
	if (CALIBRATE.LPF_END < 1)
		CALIBRATE.LPF_END = 1;
	if (CALIBRATE.LPF_END > 999999999)
		CALIBRATE.LPF_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_0_START(int8_t direction)
{
	CALIBRATE.BPF_0_START += direction * 100000;
	if (CALIBRATE.BPF_0_START < 1)
		CALIBRATE.BPF_0_START = 1;
	if (CALIBRATE.BPF_0_START > 999999999)
		CALIBRATE.BPF_0_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_0_END(int8_t direction)
{
	CALIBRATE.BPF_0_END += direction * 100000;
	if (CALIBRATE.BPF_0_END < 1)
		CALIBRATE.BPF_0_END = 1;
	if (CALIBRATE.BPF_0_END > 999999999)
		CALIBRATE.BPF_0_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_1_START(int8_t direction)
{
	CALIBRATE.BPF_1_START += direction * 100000;
	if (CALIBRATE.BPF_1_START < 1)
		CALIBRATE.BPF_1_START = 1;
	if (CALIBRATE.BPF_1_START > 999999999)
		CALIBRATE.BPF_1_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_1_END(int8_t direction)
{
	CALIBRATE.BPF_1_END += direction * 100000;
	if (CALIBRATE.BPF_1_END < 1)
		CALIBRATE.BPF_1_END = 1;
	if (CALIBRATE.BPF_1_END > 999999999)
		CALIBRATE.BPF_1_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_2_START(int8_t direction)
{
	CALIBRATE.BPF_2_START += direction * 100000;
	if (CALIBRATE.BPF_2_START < 1)
		CALIBRATE.BPF_2_START = 1;
	if (CALIBRATE.BPF_2_START > 999999999)
		CALIBRATE.BPF_2_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_2_END(int8_t direction)
{
	CALIBRATE.BPF_2_END += direction * 100000;
	if (CALIBRATE.BPF_2_END < 1)
		CALIBRATE.BPF_2_END = 1;
	if (CALIBRATE.BPF_2_END > 999999999)
		CALIBRATE.BPF_2_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_3_START(int8_t direction)
{
	CALIBRATE.BPF_3_START += direction * 100000;
	if (CALIBRATE.BPF_3_START < 1)
		CALIBRATE.BPF_3_START = 1;
	if (CALIBRATE.BPF_3_START > 999999999)
		CALIBRATE.BPF_3_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_3_END(int8_t direction)
{
	CALIBRATE.BPF_3_END += direction * 100000;
	if (CALIBRATE.BPF_3_END < 1)
		CALIBRATE.BPF_3_END = 1;
	if (CALIBRATE.BPF_3_END > 999999999)
		CALIBRATE.BPF_3_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_4_START(int8_t direction)
{
	CALIBRATE.BPF_4_START += direction * 100000;
	if (CALIBRATE.BPF_4_START < 1)
		CALIBRATE.BPF_4_START = 1;
	if (CALIBRATE.BPF_4_START > 999999999)
		CALIBRATE.BPF_4_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_4_END(int8_t direction)
{
	CALIBRATE.BPF_4_END += direction * 100000;
	if (CALIBRATE.BPF_4_END < 1)
		CALIBRATE.BPF_4_END = 1;
	if (CALIBRATE.BPF_4_END > 999999999)
		CALIBRATE.BPF_4_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_5_START(int8_t direction)
{
	CALIBRATE.BPF_5_START += direction * 100000;
	if (CALIBRATE.BPF_5_START < 1)
		CALIBRATE.BPF_5_START = 1;
	if (CALIBRATE.BPF_5_START > 999999999)
		CALIBRATE.BPF_5_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_5_END(int8_t direction)
{
	CALIBRATE.BPF_5_END += direction * 100000;
	if (CALIBRATE.BPF_5_END < 1)
		CALIBRATE.BPF_5_END = 1;
	if (CALIBRATE.BPF_5_END > 999999999)
		CALIBRATE.BPF_5_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_6_START(int8_t direction)
{
	CALIBRATE.BPF_6_START += direction * 100000;
	if (CALIBRATE.BPF_6_START < 1)
		CALIBRATE.BPF_6_START = 1;
	if (CALIBRATE.BPF_6_START > 999999999)
		CALIBRATE.BPF_6_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_6_END(int8_t direction)
{
	CALIBRATE.BPF_6_END += direction * 100000;
	if (CALIBRATE.BPF_6_END < 1)
		CALIBRATE.BPF_6_END = 1;
	if (CALIBRATE.BPF_6_END > 999999999)
		CALIBRATE.BPF_6_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_HPF_START(int8_t direction)
{
	CALIBRATE.BPF_HPF += direction * 100000;
	if (CALIBRATE.BPF_HPF < 1)
		CALIBRATE.BPF_HPF = 1;
	if (CALIBRATE.BPF_HPF > 999999999)
		CALIBRATE.BPF_HPF = 999999999;
}

static void SYSMENU_HANDL_CALIB_SWR_FWD_RATE(int8_t direction)
{
	CALIBRATE.SWR_FWD_Calibration += (float32_t)direction * 0.1f;
	if (CALIBRATE.SWR_FWD_Calibration < 1.0f)
		CALIBRATE.SWR_FWD_Calibration = 1.0f;
	if (CALIBRATE.SWR_FWD_Calibration > 50.0f)
		CALIBRATE.SWR_FWD_Calibration = 50.0f;
}

static void SYSMENU_HANDL_CALIB_SWR_REF_RATE(int8_t direction)
{
	CALIBRATE.SWR_REF_Calibration += (float32_t)direction * 0.1f;
	if (CALIBRATE.SWR_REF_Calibration < 1.0f)
		CALIBRATE.SWR_REF_Calibration = 1.0f;
	if (CALIBRATE.SWR_REF_Calibration > 50.0f)
		CALIBRATE.SWR_REF_Calibration = 50.0f;
}

static void SYSMENU_HANDL_CALIB_VCXO(int8_t direction)
{
	CALIBRATE.VCXO_correction += direction;
	if (CALIBRATE.VCXO_correction < -100)
		CALIBRATE.VCXO_correction = -100;
	if (CALIBRATE.VCXO_correction > 100)
		CALIBRATE.VCXO_correction = 100;
}
//Tisho
static void SYSMENU_HANDL_CALIB_FW_AD8307_SLP(int8_t direction)
{
	CALIBRATE.FW_AD8307_SLP += (float32_t)direction * 0.1f;
	if (CALIBRATE.FW_AD8307_SLP < 20.0f)
		CALIBRATE.FW_AD8307_SLP = 20.0f;
	if (CALIBRATE.FW_AD8307_SLP > 30.0f)
		CALIBRATE.FW_AD8307_SLP = 30.0f;
}

static void SYSMENU_HANDL_CALIB_FW_AD8307_OFFS(int8_t direction)
{
	CALIBRATE.FW_AD8307_OFFS += (float32_t)direction;
	if (CALIBRATE.FW_AD8307_OFFS < 0.1f)
		CALIBRATE.FW_AD8307_OFFS = 0.1f;
	if (CALIBRATE.FW_AD8307_OFFS > 4000.0f)
		CALIBRATE.FW_AD8307_OFFS = 4000.0f;
}

static void SYSMENU_HANDL_CALIB_BW_AD8307_SLP(int8_t direction)
{
	CALIBRATE.BW_AD8307_SLP += (float32_t)direction * 0.1f;
	if (CALIBRATE.BW_AD8307_SLP < 20.0f)
		CALIBRATE.BW_AD8307_SLP = 20.0f;
	if (CALIBRATE.BW_AD8307_SLP > 30.0f)
		CALIBRATE.BW_AD8307_SLP = 30.0f;
}

static void SYSMENU_HANDL_CALIB_BW_AD8307_OFFS(int8_t direction)
{
	CALIBRATE.BW_AD8307_OFFS += (float32_t)direction;
	if (CALIBRATE.BW_AD8307_OFFS < 0.1f)
		CALIBRATE.BW_AD8307_OFFS = 0.1f;
	if (CALIBRATE.BW_AD8307_OFFS > 4000.0f)
		CALIBRATE.BW_AD8307_OFFS = 4000.0f;
}
//end Tisho
static void SYSMENU_HANDL_CALIB_FAN_MEDIUM_START(int8_t direction)
{
	CALIBRATE.FAN_MEDIUM_START += direction;
	if (CALIBRATE.FAN_MEDIUM_START < 10)
		CALIBRATE.FAN_MEDIUM_START = 10;
	if (CALIBRATE.FAN_MEDIUM_START > 100)
		CALIBRATE.FAN_MEDIUM_START = 100;
}

static void SYSMENU_HANDL_CALIB_FAN_MEDIUM_STOP(int8_t direction)
{
	CALIBRATE.FAN_MEDIUM_STOP += direction;
	if (CALIBRATE.FAN_MEDIUM_STOP < 10)
		CALIBRATE.FAN_MEDIUM_STOP = 10;
	if (CALIBRATE.FAN_MEDIUM_STOP > (CALIBRATE.FAN_MEDIUM_START - 1))
		CALIBRATE.FAN_MEDIUM_STOP = CALIBRATE.FAN_MEDIUM_START - 1;
}

static void SYSMENU_HANDL_CALIB_FAN_FULL_START(int8_t direction)
{
	CALIBRATE.FAN_FULL_START += direction;
	if (CALIBRATE.FAN_FULL_START < 10)
		CALIBRATE.FAN_FULL_START = 10;
	if (CALIBRATE.FAN_FULL_START > 100)
		CALIBRATE.FAN_FULL_START = 100;
}

static void SYSMENU_HANDL_CALIB_TRX_MAX_RF_TEMP(int8_t direction)
{
	CALIBRATE.TRX_MAX_RF_TEMP += direction;
	if (CALIBRATE.TRX_MAX_RF_TEMP < 30)
		CALIBRATE.TRX_MAX_RF_TEMP = 30;
	if (CALIBRATE.TRX_MAX_RF_TEMP > 120)
		CALIBRATE.TRX_MAX_RF_TEMP = 120;
}

static void SYSMENU_HANDL_CALIB_TRX_MAX_SWR(int8_t direction)
{
	CALIBRATE.TRX_MAX_SWR += direction;
	if (CALIBRATE.TRX_MAX_SWR < 2)
		CALIBRATE.TRX_MAX_SWR = 2;
	if (CALIBRATE.TRX_MAX_SWR > 50)
		CALIBRATE.TRX_MAX_SWR = 50;
}

static void SYSMENU_HANDL_CALIB_FM_DEVIATION_SCALE(int8_t direction)
{
	CALIBRATE.FM_DEVIATION_SCALE += direction;
	if (CALIBRATE.FM_DEVIATION_SCALE < 1)
		CALIBRATE.FM_DEVIATION_SCALE = 1;
	if (CALIBRATE.FM_DEVIATION_SCALE > 20)
		CALIBRATE.FM_DEVIATION_SCALE = 20;
}

static void SYSMENU_HANDL_CALIB_TUNE_MAX_POWER(int8_t direction)
{
	CALIBRATE.TUNE_MAX_POWER += direction;
	if (CALIBRATE.TUNE_MAX_POWER < 1)
		CALIBRATE.TUNE_MAX_POWER = 1;
	if (CALIBRATE.TUNE_MAX_POWER > 20)
		CALIBRATE.TUNE_MAX_POWER = 20;
}

//SERVICES
void SYSMENU_HANDL_SERVICESMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_services_opened = true;
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_services_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_services_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	//drawSystemMenu(true);
	LCD_redraw(false);
}

//SPECTRUM ANALIZER
static void SYSMENU_HANDL_SPECTRUMMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_spectrum_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_spectrum_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_SPECTRUM_Start(int8_t direction)
{
	if (SYSMENU_spectrum_opened)
	{
		SPEC_EncRotate(direction);
	}
	else
	{
		SYSMENU_spectrum_opened = true;
		SPEC_Start();
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
}

static void SYSMENU_HANDL_SPECTRUM_Begin(int8_t direction)
{
	TRX.SPEC_Begin += direction * 100;
	if (TRX.SPEC_Begin < 100)
		TRX.SPEC_Begin = 100;
}

static void SYSMENU_HANDL_SPECTRUM_End(int8_t direction)
{
	TRX.SPEC_End += direction * 100;
	if (TRX.SPEC_End < 100)
		TRX.SPEC_End = 100;
}

static void SYSMENU_HANDL_SPECTRUM_TopDBM(int8_t direction)
{
	TRX.SPEC_TopDBM += direction;
	if (TRX.SPEC_TopDBM < -140)
		TRX.SPEC_TopDBM = -140;
	if (TRX.SPEC_TopDBM > 40)
		TRX.SPEC_TopDBM = 40;
	if (TRX.SPEC_TopDBM <= TRX.SPEC_BottomDBM)
		TRX.SPEC_TopDBM = TRX.SPEC_BottomDBM + 1;
}

static void SYSMENU_HANDL_SPECTRUM_BottomDBM(int8_t direction)
{
	TRX.SPEC_BottomDBM += direction;
	if (TRX.SPEC_BottomDBM < -140)
		TRX.SPEC_BottomDBM = -140;
	if (TRX.SPEC_BottomDBM > 40)
		TRX.SPEC_BottomDBM = 40;
	if (TRX.SPEC_BottomDBM >= TRX.SPEC_TopDBM)
		TRX.SPEC_BottomDBM = TRX.SPEC_TopDBM - 1;
}

//WSPR Beacon
static void SYSMENU_HANDL_WSPRMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_wspr_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_wspr_item_count;
	sysmenu_onroot = false;
	systemMenuIndex = 0;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_WSPR_Start(int8_t direction)
{
	if (SYSMENU_wspr_opened)
	{
		WSPR_EncRotate(direction);
	}
	else
	{
		SYSMENU_wspr_opened = true;
		WSPR_Start();
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
}

static void SYSMENU_HANDL_WSPR_FREQ_OFFSET(int8_t direction)
{
	TRX.WSPR_FREQ_OFFSET += direction;
	if (TRX.WSPR_FREQ_OFFSET < -2000)
		TRX.WSPR_FREQ_OFFSET = -2000;
	if (TRX.WSPR_FREQ_OFFSET > 2000)
		TRX.WSPR_FREQ_OFFSET = 2000;
}

static void SYSMENU_HANDL_WSPR_BAND160(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_160 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_160 = false;
}

static void SYSMENU_HANDL_WSPR_BAND80(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_80 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_80 = false;
}

static void SYSMENU_HANDL_WSPR_BAND40(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_40 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_40 = false;
}

static void SYSMENU_HANDL_WSPR_BAND30(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_30 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_30 = false;
}

static void SYSMENU_HANDL_WSPR_BAND20(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_20 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_20 = false;
}

static void SYSMENU_HANDL_WSPR_BAND17(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_17 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_17 = false;
}

static void SYSMENU_HANDL_WSPR_BAND15(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_15 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_15 = false;
}

static void SYSMENU_HANDL_WSPR_BAND12(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_12 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_12 = false;
}

static void SYSMENU_HANDL_WSPR_BAND10(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_10 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_10 = false;
}

static void SYSMENU_HANDL_WSPR_BAND6(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_6 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_6 = false;
}

static void SYSMENU_HANDL_WSPR_BAND2(int8_t direction)
{
	if (direction > 0)
		TRX.WSPR_BANDS_2 = true;
	if (direction < 0)
		TRX.WSPR_BANDS_2 = false;
}

//SWR BAND ANALYZER
static void SYSMENU_HANDL_SWR_BAND_START(int8_t direction)
{
	if (SYSMENU_swr_opened)
	{
		SWR_EncRotate(direction);
	}
	else
	{
		SYSMENU_swr_opened = true;
		int8_t band = getBandFromFreq(CurrentVFO()->Freq, true);
		SWR_Start(BANDS[band].startFreq - 100000, BANDS[band].endFreq + 100000);
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
}

//SWR HF ANALYZER
static void SYSMENU_HANDL_SWR_HF_START(int8_t direction)
{
	if (SYSMENU_swr_opened)
	{
		SWR_EncRotate(direction);
	}
	else
	{
		SYSMENU_swr_opened = true;
		SWR_Start(1000000, 60000000);
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
}

//RDA STATS
static void SYSMENU_HANDL_RDA_STATS(int8_t direction)
{
#pragma unused(direction)
	sysmenu_infowindow_opened = true;
	SYSMENU_drawSystemMenu(true);
	WIFI_getRDA();
}

//PROPAGINATION
static void SYSMENU_HANDL_PROPAGINATION(int8_t direction)
{
#pragma unused(direction)
	sysmenu_infowindow_opened = true;
	SYSMENU_drawSystemMenu(true);
	WIFI_getPropagination();
}

//SD FILE MANAGER
static void SYSMENU_HANDL_FILEMANAGER(int8_t direction)
{
#pragma unused(direction)
	sysmenu_filemanager_opened = true;
	SYSMENU_drawSystemMenu(true);
}

//COMMON MENU FUNCTIONS
void SYSMENU_drawSystemMenu(bool draw_background)
{
	if (LCD_busy)
	{
		if (draw_background)
			LCD_UpdateQuery.SystemMenuRedraw = true;
		else
			LCD_UpdateQuery.SystemMenu = true;
		return;
	}
	if (!LCD_systemMenuOpened)
		return;
	if (sysmenu_timeMenuOpened)
	{
		SYSMENU_HANDL_SETTIME(0);
		return;
	}
	else if (sysmenu_wifi_selectap1_menu_opened)
	{
		SYSMENU_WIFI_DrawSelectAP1Menu(draw_background);
	}
	else if (sysmenu_wifi_selectap2_menu_opened)
	{
		SYSMENU_WIFI_DrawSelectAP2Menu(draw_background);
	}
	else if (sysmenu_wifi_selectap3_menu_opened)
	{
		SYSMENU_WIFI_DrawSelectAP3Menu(draw_background);
	}
	else if (sysmenu_wifi_setAP1password_menu_opened)
	{
		SYSMENU_WIFI_DrawAP1passwordMenu(draw_background);
	}
	else if (sysmenu_wifi_setAP2password_menu_opened)
	{
		SYSMENU_WIFI_DrawAP2passwordMenu(draw_background);
	}
	else if (sysmenu_wifi_setAP3password_menu_opened)
	{
		SYSMENU_WIFI_DrawAP3passwordMenu(draw_background);
	}
	else if (sysmenu_trx_setCallsign_menu_opened)
	{
		SYSMENU_TRX_DrawCallsignMenu(draw_background);
	}
	else if (sysmenu_trx_setLocator_menu_opened)
	{
		SYSMENU_TRX_DrawLocatorMenu(draw_background);
	}
	else if (SYSMENU_spectrum_opened)
	{
		SPEC_Draw();
	}
	else if (SYSMENU_wspr_opened)
	{
		WSPR_DoEvents();
		return;
	}
	else if (SYSMENU_TDM_CTRL_opened) //Tisho
	{
		TDM_Voltages();
	}
	else if (SYSMENU_swr_opened)
	{
		SWR_Draw();
	}
	else if (sysmenu_sysinfo_opened)
	{
		SYSMENU_HANDL_SYSINFO(0);
		return;
	}
	else if (sysmenu_filemanager_opened)
	{
		FILEMANAGER_Draw(draw_background);
		return;
	}
	else if (sysmenu_infowindow_opened)
	{
	}
	else
	{
		LCD_busy = true;

		sysmenu_i = 0;
		sysmenu_y = 5;

		if (draw_background)
			LCDDriver_Fill(BG_COLOR);

		uint8_t current_selected_page = systemMenuIndex / LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE;
		if (current_selected_page * LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE > *sysmenu_item_count_selected)
			current_selected_page = 0;

		for (uint8_t m = 0; m < *sysmenu_item_count_selected; m++)
		{
			uint8_t current_page = m / LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE;
			if (current_selected_page == current_page)
				drawSystemMenuElement(sysmenu_handlers_selected[m].title, sysmenu_handlers_selected[m].type, sysmenu_handlers_selected[m].value, sysmenu_handlers_selected[m].enumerate, false);
		}

		LCD_busy = false;
	}

	LCD_UpdateQuery.SystemMenu = false;
	if (draw_background)
		LCD_UpdateQuery.SystemMenuRedraw = false;
}

void SYSMENU_eventRotateSystemMenu(int8_t direction)
{
	if (sysmenu_wifi_selectap1_menu_opened)
	{
		SYSMENU_WIFI_SelectAP1MenuMove(0);
		return;
	}
	if (sysmenu_wifi_selectap2_menu_opened)
	{
		SYSMENU_WIFI_SelectAP2MenuMove(0);
		return;
	}
	if (sysmenu_wifi_selectap3_menu_opened)
	{
		SYSMENU_WIFI_SelectAP3MenuMove(0);
		return;
	}
	if (sysmenu_wifi_setAP1password_menu_opened)
	{
		SYSMENU_WIFI_RotatePasswordChar1(direction);
		return;
	}
	if (sysmenu_wifi_setAP2password_menu_opened)
	{
		SYSMENU_WIFI_RotatePasswordChar2(direction);
		return;
	}
	if (sysmenu_wifi_setAP3password_menu_opened)
	{
		SYSMENU_WIFI_RotatePasswordChar3(direction);
		return;
	}
	if (sysmenu_trx_setCallsign_menu_opened)
	{
		SYSMENU_TRX_RotateCallsignChar(direction);
		return;
	}
	if (sysmenu_trx_setLocator_menu_opened)
	{
		SYSMENU_TRX_RotateLocatorChar(direction);
		return;
	}
	if (sysmenu_timeMenuOpened)
	{
		SYSMENU_HANDL_SETTIME(direction);
		LCD_UpdateQuery.SystemMenu = true;
		return;
	}
	if (sysmenu_filemanager_opened)
	{
		FILEMANAGER_EventRotate(direction);
		return;
	}
	if (sysmenu_handlers_selected[systemMenuIndex].type != SYSMENU_INFOLINE)
		sysmenu_handlers_selected[systemMenuIndex].menuHandler(direction);
	if (sysmenu_handlers_selected[systemMenuIndex].type != SYSMENU_RUN)
		LCD_UpdateQuery.SystemMenuCurrent = true;
}

void SYSMENU_eventCloseSystemMenu(void)
{
	if (sysmenu_wifi_selectap1_menu_opened)
	{
		sysmenu_wifi_selectap1_menu_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
	}
	if (sysmenu_wifi_selectap2_menu_opened)
	{
		sysmenu_wifi_selectap2_menu_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
	}
	if (sysmenu_wifi_selectap3_menu_opened)
	{
		sysmenu_wifi_selectap3_menu_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
	}
	else if (sysmenu_wifi_setAP1password_menu_opened)
	{
		sysmenu_wifi_setAP1password_menu_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
	}
	else if (sysmenu_wifi_setAP2password_menu_opened)
	{
		sysmenu_wifi_setAP2password_menu_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
	}
	else if (sysmenu_wifi_setAP3password_menu_opened)
	{
		sysmenu_wifi_setAP3password_menu_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
	}
	else if (sysmenu_trx_setCallsign_menu_opened)
	{
		sysmenu_trx_setCallsign_menu_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_trx_setLocator_menu_opened)
	{
		sysmenu_trx_setLocator_menu_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_spectrum_opened)
	{
		SYSMENU_spectrum_opened = false;
		SPEC_Stop();
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_wspr_opened)
	{
		SYSMENU_wspr_opened = false;
		WSPR_Stop();
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_timeMenuOpened)
	{
		sysmenu_timeMenuOpened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_swr_opened)
	{
		SYSMENU_swr_opened = false;
		SWR_Stop();
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_TDM_CTRL_opened) //Tisho
	{
		SYSMENU_TDM_CTRL_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_infowindow_opened)
	{
		sysmenu_infowindow_opened = false;
		sysmenu_sysinfo_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_filemanager_opened)
	{
		FILEMANAGER_Closing();
		sysmenu_filemanager_opened = false;
		systemMenuIndex = 0;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_services_opened)
	{
		sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_handlers[0];
		sysmenu_item_count_selected = (uint8_t*)&sysmenu_item_count;
		sysmenu_onroot = true;
		systemMenuIndex = systemMenuRootIndex;
		sysmenu_services_opened = false;
		LCD_systemMenuOpened = false;
		LCD_UpdateQuery.Background = true;
		LCD_redraw(false);
	}
	else
	{
		if (sysmenu_onroot)
		{
			LCD_systemMenuOpened = false;
			LCD_UpdateQuery.Background = true;
			LCD_redraw(false);
		}
		else
		{
			if(sysmenu_handlers_selected == (struct sysmenu_item_handler*)&sysmenu_calibration_handlers[0]) //exit from calibration
				NeedSaveCalibration = true;
			
			sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_handlers[0];
			sysmenu_item_count_selected = (uint8_t*)&sysmenu_item_count;
			sysmenu_onroot = true;
			systemMenuIndex = systemMenuRootIndex;
			SYSMENU_drawSystemMenu(true);
		}
	}
	sysmenu_item_selected_by_enc2 = false;
	NeedSaveSettings = true;
	
	if(SD_USBCardReader)
	{
		SD_USBCardReader = false;
		USBD_Restart();
		TRX_Mute = false;
	}
}

void SYSMENU_eventCloseAllSystemMenu(void)
{
	sysmenu_handlers_selected = (struct sysmenu_item_handler*)&sysmenu_handlers[0];
	sysmenu_item_count_selected = (uint8_t*)&sysmenu_item_count;
	sysmenu_onroot = true;
	systemMenuIndex = systemMenuRootIndex;
	sysmenu_item_selected_by_enc2 = false;
	LCD_systemMenuOpened = false;
	LCD_UpdateQuery.Background = true;
	LCD_redraw(false);
}

//secondary encoder click
void SYSMENU_eventSecEncoderClickSystemMenu(void)
{
	if (sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_MENU || sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_HIDDEN_MENU || sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_RUN || sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_INFOLINE)
	{
		sysmenu_item_selected_by_enc2 = false;
		SYSMENU_eventRotateSystemMenu(1);
	}
	else
	{
		sysmenu_item_selected_by_enc2 = !sysmenu_item_selected_by_enc2;
		LCD_UpdateQuery.SystemMenuCurrent = true;
	}
}

//secondary encoder rotate
void SYSMENU_eventSecRotateSystemMenu(int8_t direction)
{
	//wifi select AP menu
	if (sysmenu_wifi_selectap1_menu_opened)
	{
		if (direction < 0)
			SYSMENU_WIFI_SelectAP1MenuMove(-1);
		else
			SYSMENU_WIFI_SelectAP1MenuMove(1);
		return;
	}
	if (sysmenu_wifi_selectap2_menu_opened)
	{
		if (direction < 0)
			SYSMENU_WIFI_SelectAP2MenuMove(-1);
		else
			SYSMENU_WIFI_SelectAP2MenuMove(1);
		return;
	}
	if (sysmenu_wifi_selectap3_menu_opened)
	{
		if (direction < 0)
			SYSMENU_WIFI_SelectAP3MenuMove(-1);
		else
			SYSMENU_WIFI_SelectAP3MenuMove(1);
		return;
	}
	//wifi set password menu
	if (sysmenu_wifi_setAP1password_menu_opened)
	{
		if (direction < 0 && sysmenu_wifi_selected_ap_password_char_index > 0)
		{
			sysmenu_wifi_selected_ap_password_char_index--;
			SYSMENU_WIFI_DrawAP1passwordMenu(true);
		}
		else if (sysmenu_wifi_selected_ap_password_char_index < (MAX_WIFIPASS_LENGTH - 1))
		{
			sysmenu_wifi_selected_ap_password_char_index++;
			SYSMENU_WIFI_DrawAP1passwordMenu(true);
		}
		return;
	}
	if (sysmenu_wifi_setAP2password_menu_opened)
	{
		if (direction < 0 && sysmenu_wifi_selected_ap_password_char_index > 0)
		{
			sysmenu_wifi_selected_ap_password_char_index--;
			SYSMENU_WIFI_DrawAP2passwordMenu(true);
		}
		else if (sysmenu_wifi_selected_ap_password_char_index < (MAX_WIFIPASS_LENGTH - 1))
		{
			sysmenu_wifi_selected_ap_password_char_index++;
			SYSMENU_WIFI_DrawAP2passwordMenu(true);
		}
		return;
	}
	if (sysmenu_wifi_setAP3password_menu_opened)
	{
		if (direction < 0 && sysmenu_wifi_selected_ap_password_char_index > 0)
		{
			sysmenu_wifi_selected_ap_password_char_index--;
			SYSMENU_WIFI_DrawAP3passwordMenu(true);
		}
		else if (sysmenu_wifi_selected_ap_password_char_index < (MAX_WIFIPASS_LENGTH - 1))
		{
			sysmenu_wifi_selected_ap_password_char_index++;
			SYSMENU_WIFI_DrawAP3passwordMenu(true);
		}
		return;
	}
	//Callsign menu
	if (sysmenu_trx_setCallsign_menu_opened)
	{
		if (direction < 0 && sysmenu_trx_selected_callsign_char_index > 0)
		{
			sysmenu_trx_selected_callsign_char_index--;
			SYSMENU_TRX_DrawCallsignMenu(true);
		}
		else if (sysmenu_trx_selected_callsign_char_index < (MAX_CALLSIGN_LENGTH - 1))
		{
			sysmenu_trx_selected_callsign_char_index++;
			SYSMENU_TRX_DrawCallsignMenu(true);
		}
		return;
	}
	//Locator menu
	if (sysmenu_trx_setLocator_menu_opened)
	{
		if (direction < 0 && sysmenu_trx_selected_locator_char_index > 0)
		{
			sysmenu_trx_selected_locator_char_index--;
			SYSMENU_TRX_DrawLocatorMenu(true);
		}
		else if (sysmenu_trx_selected_locator_char_index < (MAX_CALLSIGN_LENGTH - 1))
		{
			sysmenu_trx_selected_locator_char_index++;
			SYSMENU_TRX_DrawLocatorMenu(true);
		}
		return;
	}
	if (SYSMENU_spectrum_opened)
	{
		SPEC_Stop();
		SYSMENU_spectrum_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	if (SYSMENU_wspr_opened)
	{
		WSPR_Stop();
		SYSMENU_wspr_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	if (SYSMENU_swr_opened)
		return;
	if (sysmenu_infowindow_opened)
		return;
	//time menu
	if (sysmenu_timeMenuOpened)
	{
		LCDDriver_Fill(BG_COLOR);
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
	//File manager
	if (sysmenu_filemanager_opened)
	{
		FILEMANAGER_EventSecondaryRotate(direction);
		return;
	}
	//other
	if (sysmenu_item_selected_by_enc2) //selected by secondary encoder
	{
		SYSMENU_eventRotateSystemMenu(direction);
		return;
	}

	uint8_t current_page = systemMenuIndex / LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE;
	LCD_busy = true;
	LCDDriver_drawFastHLine(0, (5 + (systemMenuIndex - current_page * LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE) * LAYOUT->SYSMENU_ITEM_HEIGHT) + 17, LAYOUT->SYSMENU_W, BG_COLOR);
	LCD_busy = false;
	if (direction < 0)
	{
		if (systemMenuIndex > 0)
			systemMenuIndex--;
		else if (!SYSMENU_hiddenmenu_enabled && sysmenu_onroot)
			systemMenuIndex = *sysmenu_item_count_selected - 2;
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
	if (!SYSMENU_hiddenmenu_enabled && sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_HIDDEN_MENU)
		systemMenuIndex = 0;
	while (systemMenuIndex > 0 && sysmenu_handlers_selected[systemMenuIndex].type == SYSMENU_INFOLINE)
		systemMenuIndex = 0;

	LCD_UpdateQuery.SystemMenuCurrent = true;
	if (sysmenu_onroot)
		systemMenuRootIndex = systemMenuIndex;
	uint8_t new_page = systemMenuIndex / LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE;

	if (current_page != new_page)
		LCD_UpdateQuery.SystemMenuRedraw = true;
}

void SYSMENU_redrawCurrentItem(void)
{
	uint8_t current_page = systemMenuIndex / LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE;
	sysmenu_i = (uint8_t)(systemMenuIndex - current_page * LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE);
	sysmenu_y = 5 + (systemMenuIndex - current_page * LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE) * LAYOUT->SYSMENU_ITEM_HEIGHT;
	drawSystemMenuElement(sysmenu_handlers_selected[systemMenuIndex].title, sysmenu_handlers_selected[systemMenuIndex].type, sysmenu_handlers_selected[systemMenuIndex].value, sysmenu_handlers_selected[systemMenuIndex].enumerate, true);
}

static void drawSystemMenuElement(char *title, SystemMenuType type, uint32_t *value, char enumerate[ENUM_MAX_COUNT][ENUM_MAX_LENGTH], bool onlyVal)
{
	if (!SYSMENU_hiddenmenu_enabled && type == SYSMENU_HIDDEN_MENU)
		return;

	char ctmp[32] = {0};
	if (!onlyVal)
	{
		LCDDriver_Fill_RectXY(0, sysmenu_y, LAYOUT->SYSMENU_W, sysmenu_y + 17, BG_COLOR);
		LCDDriver_printText(title, LAYOUT->SYSMENU_X1, sysmenu_y, FG_COLOR, BG_COLOR, 2);
	}

	uint16_t x_pos = LAYOUT->SYSMENU_X2 - 5 * 12;
	float32_t tmp_float = 0;
	switch (type)
	{
	case SYSMENU_UINT8:
		sprintf(ctmp, "%d", (uint8_t)*value);
		break;
	case SYSMENU_ENUM:
		sprintf(ctmp, "%s", enumerate[(uint8_t)*value]);	
		break;
	case SYSMENU_ENUMR:
		sprintf(ctmp, "%s", enumerate[(uint8_t)*value]);
		break;
	case SYSMENU_UINT16:
		sprintf(ctmp, "%d", (uint16_t)*value);
		break;
	case SYSMENU_UINT32:
		sprintf(ctmp, "%u", (uint32_t)*value);
		break;
	case SYSMENU_UINT32R:
		sprintf(ctmp, "%u", (uint32_t)*value);
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
	case SYSMENU_FLOAT32:
		dma_memcpy(&tmp_float, value, sizeof(float32_t));
		sprintf(ctmp, "%.2f", (double)tmp_float);
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
	case SYSMENU_INFOLINE:
		break;
	case SYSMENU_FUNCBUTTON:
		sprintf(ctmp, "%s", (char *)PERIPH_FrontPanel_FuncButtonsList[TRX.FuncButtons[(uint8_t)*value]].name);
		break;
	}
	
	//if (onlyVal)
		//LCDDriver_Fill_RectWH(x_pos, sysmenu_y, ENUM_MAX_LENGTH * 12, 13, BG_COLOR);
	if (type != SYSMENU_INFOLINE)
	{
		addSymbols(ctmp, ctmp, ENUM_MAX_LENGTH, " ", false);
		LCDDriver_printText(ctmp, x_pos, sysmenu_y, FG_COLOR, BG_COLOR, 2);
	}

	uint8_t current_selected_page = systemMenuIndex / LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE;
	if (systemMenuIndex == sysmenu_i + current_selected_page * LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE)
	{
		if (sysmenu_item_selected_by_enc2)
			LCDDriver_drawFastHLine(0, sysmenu_y + 17, LAYOUT->SYSMENU_W, COLOR->BUTTON_TEXT);
		else
			LCDDriver_drawFastHLine(0, sysmenu_y + 17, LAYOUT->SYSMENU_W, FG_COLOR);
	}
	sysmenu_i++;
	sysmenu_y += LAYOUT->SYSMENU_ITEM_HEIGHT;
}
