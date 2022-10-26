#include "system_menu.h"
#include "lcd.h"
#include "settings.h"
#include "audio_filters.h"
#include "bootloader.h"
#include "functions.h"
#include "wifi.h"
#include "front_unit.h"
#include "spec_analyzer.h"
#include "swr_analyzer.h"
#include "fonts.h"
#include "agc.h"
#include "screen_layout.h"
#include "noise_blanker.h"
#include "bands.h"
#include "sd.h"
#include "wspr.h"
#include "hardware.h"
#include "usbd_ua3reo.h"
#include "filemanager.h"
#include "rf_unit.h"
#include "locator.h"
#include "callsign.h"
#include "rtty_decoder.h"
#include "self_test.h"
#include "cw.h"
#include "FT8/FT8_main.h"
#include "INA226_PWR_monitor.h"
#include "auto_calibration.h"

static void SYSMENU_HANDL_TRX_RFPower(int8_t direction);
static void SYSMENU_HANDL_TRX_BandMap(int8_t direction);
static void SYSMENU_HANDL_TRX_ChannelMode(int8_t direction);
static void SYSMENU_HANDL_TRX_AutoGain(int8_t direction);
static void SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE(int8_t direction);
static void SYSMENU_HANDL_TRX_RFFilters(int8_t direction);
static void SYSMENU_HANDL_TRX_INPUT_TYPE_MAIN(int8_t direction);
static void SYSMENU_HANDL_TRX_INPUT_TYPE_DIGI(int8_t direction);
static void SYSMENU_HANDL_TRX_Auto_Input_Switch(int8_t direction);
static void SYSMENU_HANDL_TRX_Auto_Snap(int8_t direction);
static void SYSMENU_HANDL_TRX_RIT_INTERVAL(int8_t direction);
static void SYSMENU_HANDL_TRX_XIT_INTERVAL(int8_t direction);
static void SYSMENU_HANDL_TRX_SAMPLERATE_MAIN(int8_t direction);
static void SYSMENU_HANDL_TRX_SAMPLERATE_FM(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_FAST_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_ENC_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_ENC_FAST_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_ENC_WFM_STEP_KHZ(int8_t direction);
static void SYSMENU_HANDL_TRX_FRQ_CW_STEP_DIVIDER(int8_t direction);
static void SYSMENU_HANDL_TRX_ENC_ACCELERATE(int8_t direction);
static void SYSMENU_HANDL_TRX_ATT_STEP(int8_t direction);
static void SYSMENU_HANDL_TRX_ATT_DB(int8_t direction);
static void SYSMENU_HANDL_TRX_DEBUG_TYPE(int8_t direction);
static void SYSMENU_HANDL_TRX_SetCallsign(int8_t direction);
static void SYSMENU_HANDL_TRX_SetLocator(int8_t direction);
static void SYSMENU_HANDL_TRX_SetURSICode(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_ENABLE(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_OFFSET(int8_t direction);
static void SYSMENU_HANDL_TRX_ATU_I(int8_t direction);
static void SYSMENU_HANDL_TRX_ATU_C(int8_t direction);
static void SYSMENU_HANDL_TRX_ATU_T(int8_t direction);
static void SYSMENU_HANDL_TRX_ATU_Enabled(int8_t direction);
static void SYSMENU_HANDL_TRX_TUNER_Enabled(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_70CM(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_23CM(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_13CM(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_6CM(int8_t direction);
static void SYSMENU_HANDL_TRX_TRANSV_3CM(int8_t direction);
static void SYSMENU_HANDL_TRX_FineRITTune(int8_t direction);
static void SYSMENU_HANDL_TRX_VGA_GAIN(int8_t direction);

static void SYSMENU_HANDL_AUDIO_Volume(int8_t direction);
static void SYSMENU_HANDL_AUDIO_Volume_Step(int8_t direction);
static void SYSMENU_HANDL_AUDIO_IFGain(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR(int8_t direction);
static void SYSMENU_HANDL_AUDIO_NOISE_BLANKER(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AGC(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AGC_GAIN_TARGET(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_Gain(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_Boost(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_NOISE_GATE(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR1_THRES(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR2_THRES(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_AVERAGE(int8_t direction);
static void SYSMENU_HANDL_AUDIO_DNR_MINMAL(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_LOW_SSB(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_MID_SSB(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_HIG_SSB(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_LOW_AMFM(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_MID_AMFM(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_EQ_HIG_AMFM(int8_t direction);
static void SYSMENU_HANDL_AUDIO_MIC_REVERBER(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_EQ_LOW(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_EQ_MID(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_EQ_HIG(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_AGC_SSB_Speed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_AGC_CW_Speed(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_AGC_Max_gain(int8_t direction);
static void SYSMENU_HANDL_AUDIO_RX_AGC_Hold(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_CompressorSpeed_SSB(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_CompressorSpeed_AMFM(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_CompressorMaxGain_SSB(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_CompressorMaxGain_AMFM(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FMSquelch(int8_t direction);
static void SYSMENU_HANDL_AUDIO_Squelch(int8_t direction);
static void SYSMENU_HANDL_AUDIO_Beeper(int8_t direction);
static void SYSMENU_HANDL_AUDIO_CTCSS_Freq(int8_t direction);
static void SYSMENU_HANDL_AUDIO_SELFHEAR_Volume(int8_t direction);
static void SYSMENU_HANDL_AUDIO_FM_Stereo(int8_t direction);
static void SYSMENU_HANDL_AUDIO_AGC_Spectral(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_CESSB(int8_t direction);
static void SYSMENU_HANDL_AUDIO_TX_CESSB_COMPRESS_DB(int8_t direction);
static void SYSMENU_HANDL_AUDIO_VAD_THRESHOLD(int8_t direction);
static void SYSMENU_HANDL_AUDIO_VOX(int8_t direction);
static void SYSMENU_HANDL_AUDIO_VOX_TIMEOUT(int8_t direction);
static void SYSMENU_HANDL_AUDIO_VOX_THRESHOLD(int8_t direction);

static void SYSMENU_HANDL_CW_Pitch(int8_t direction);
static void SYSMENU_HANDL_CW_SelfHear(int8_t direction);
static void SYSMENU_HANDL_CW_Keyer(int8_t direction);
static void SYSMENU_HANDL_CW_Keyer_WPM(int8_t direction);
static void SYSMENU_HANDL_CW_Key_timeout(int8_t direction);
static void SYSMENU_HANDL_CW_GaussFilter(int8_t direction);
static void SYSMENU_HANDL_CW_DotToDashRate(int8_t direction);
static void SYSMENU_HANDL_CW_Iambic(int8_t direction);
static void SYSMENU_HANDL_CW_Invert(int8_t direction);
static void SYSMENU_HANDL_CW_PTT_Type(int8_t direction);

static void SYSMENU_HANDL_SCREEN_FFT_Enabled(int8_t direction);
static void SYSMENU_HANDL_SCREEN_COLOR_THEME(int8_t direction);
static void SYSMENU_HANDL_SCREEN_LAYOUT_THEME(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Averaging(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Window(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Zoom(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_ZoomCW(int8_t direction);
static void SYSMENU_HANDL_SCREEN_LCD_Brightness(int8_t direction);
static void SYSMENU_HANDL_SCREEN_LCD_SleepTimeout(int8_t direction);
static void SYSMENU_HANDL_SCREEN_WTF_Moving(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Height(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Style(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_BW_Style(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Color(int8_t direction);
static void SYSMENU_HANDL_SCREEN_WTF_Color(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_FreqGrid(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_dBmGrid(int8_t direction);
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
#if HRDW_HAS_WIFI
static void SYSMENU_HANDL_SCREEN_FFT_DXCluster(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_DXCluster_Azimuth(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_DXCluster_Timeout(int8_t direction);
#endif
static void SYSMENU_HANDL_SCREEN_Show_Sec_VFO(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FFT_Scale_Type(int8_t direction);
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
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON25(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON26(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON27(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON28(int8_t direction);
#if FUNCBUTTONS_COUNT > 28
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON29(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON30(int8_t direction);
#if FUNCBUTTONS_COUNT > 30
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON31(int8_t direction);
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON32(int8_t direction);
#endif
#endif

static void SYSMENU_HANDL_DECODERS_CW_Decoder(int8_t direction);
static void SYSMENU_HANDL_DECODERS_CW_Decoder_Threshold(int8_t direction);
static void SYSMENU_HANDL_DECODERS_RDS_Decoder(int8_t direction);
static void SYSMENU_HANDL_DECODERS_RTTY_Speed(int8_t direction);
static void SYSMENU_HANDL_DECODERS_RTTY_Shift(int8_t direction);
static void SYSMENU_HANDL_DECODERS_RTTY_Freq(int8_t direction);
static void SYSMENU_HANDL_DECODERS_RTTY_StopBits(int8_t direction);
static void SYSMENU_HANDL_DECODERS_RTTY_InvertBits(int8_t direction);

static void SYSMENU_HANDL_ADC_PGA(int8_t direction);
static void SYSMENU_HANDL_ADC_RAND(int8_t direction);
static void SYSMENU_HANDL_ADC_SHDN(int8_t direction);
static void SYSMENU_HANDL_ADC_DITH(int8_t direction);
static void SYSMENU_HANDL_ADC_DRIVER(int8_t direction);

#if HRDW_HAS_WIFI
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
#endif

#if HRDW_HAS_SD
static void SYSMENU_HANDL_SD_Filemanager(int8_t direction);
static void SYSMENU_HANDL_SD_FormatDialog(int8_t direction);
static void SYSMENU_HANDL_SD_Format(int8_t direction);
static void SYSMENU_HANDL_SD_ExportSettingsDialog(int8_t direction);
static void SYSMENU_HANDL_SD_ExportSettings(int8_t direction);
static void SYSMENU_HANDL_SD_ImportSettingsDialog(int8_t direction);
static void SYSMENU_HANDL_SD_ImportSettings(int8_t direction);
static void SYSMENU_HANDL_SD_USB(int8_t direction);
#endif

static void SYSMENU_HANDL_SETTIME(int8_t direction);
static void SYSMENU_HANDL_Bootloader(int8_t direction);
#if HRDW_HAS_SD
static void SYSMENU_HANDL_OTA_Update(int8_t direction);
#endif
static void SYSMENU_HANDL_SYSINFO(int8_t direction);
static void SYSMENU_HANDL_SUPPORT(int8_t direction);
static void SYSMENU_HANDL_Back(int8_t direction);

static void SYSMENU_HANDL_CALIB_ENCODER_SLOW_RATE(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_INVERT(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER2_INVERT(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_DEBOUNCE(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER2_DEBOUNCE(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_ON_FALLING(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENCODER_ACCELERATION(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_unit_type(int8_t direction);
static void SYSMENU_HANDL_CALIB_TangentType(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_48K_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_96K_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_192K_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_CICCOMP_384K_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_DAC_SHIFT(int8_t direction);
static void SYSMENU_HANDL_CALIB_DAC_driver_mode(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_2200M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_160M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_80M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_40M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_30M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_20M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_17M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_15M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_12M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_CB(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_10M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_6M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_4M(int8_t direction);
static void SYSMENU_HANDL_CALIB_RF_GAIN_2M(int8_t direction);
static void SYSMENU_HANDL_CALIB_S_METER_HF(int8_t direction);
static void SYSMENU_HANDL_CALIB_S_METER_VHF(int8_t direction);
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
static void SYSMENU_HANDL_CALIB_BPF_7_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_7_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_8_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_8_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_9_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_BPF_9_END(int8_t direction);
static void SYSMENU_HANDL_CALIB_HPF_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_MAX_RF_POWER_ON_METER(int8_t direction);
static void SYSMENU_HANDL_CALIB_SWR_FWD_RATE_HF(int8_t direction);
static void SYSMENU_HANDL_CALIB_SWR_REF_RATE_HF(int8_t direction);
static void SYSMENU_HANDL_CALIB_SWR_FWD_RATE_6M(int8_t direction);
static void SYSMENU_HANDL_CALIB_SWR_REF_RATE_6M(int8_t direction);
static void SYSMENU_HANDL_CALIB_SWR_FWD_RATE_VHF(int8_t direction);
static void SYSMENU_HANDL_CALIB_SWR_REF_RATE_VHF(int8_t direction);
static void SYSMENU_HANDL_CALIB_VCXO(int8_t direction);
static void SYSMENU_HANDL_CALIB_FW_AD8307_SLP(int8_t direction);  // Tisho
static void SYSMENU_HANDL_CALIB_FW_AD8307_OFFS(int8_t direction); // Tisho
static void SYSMENU_HANDL_CALIB_BW_AD8307_SLP(int8_t direction);  // Tisho
static void SYSMENU_HANDL_CALIB_BW_AD8307_OFFS(int8_t direction); // Tisho
static void SYSMENU_HANDL_CALIB_FAN_MEDIUM_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_FAN_MEDIUM_STOP(int8_t direction);
static void SYSMENU_HANDL_CALIB_FAN_FULL_START(int8_t direction);
static void SYSMENU_HANDL_CALIB_TRX_MAX_RF_TEMP(int8_t direction);
static void SYSMENU_HANDL_CALIB_TRX_MAX_SWR(int8_t direction);
static void SYSMENU_HANDL_CALIB_FM_DEVIATION_SCALE(int8_t direction);
static void SYSMENU_HANDL_CALIB_SSB_POWER_ADDITION(int8_t direction);
static void SYSMENU_HANDL_CALIB_AM_MODULATION_INDEX(int8_t direction);
static void SYSMENU_HANDL_CALIB_TUNE_MAX_POWER(int8_t direction);
static void SYSMENU_HANDL_CALIB_RTC_COARSE_CALIBRATION(int8_t direction);
static void SYSMENU_HANDL_CALIB_RTC_CALIBRATION(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_2200m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_160m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_80m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_60m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_40m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_30m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_20m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_17m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_15m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_12m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_CB(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_10m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_6m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_4m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_FM(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_2m(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_70cm(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_TRANSV_70cm(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_TRANSV_23cm(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_TRANSV_13cm(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_TRANSV_6cm(int8_t direction);
static void SYSMENU_HANDL_CALIB_EXT_TRANSV_3cm(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_NOTHAM(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_2200m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_160m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_80m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_60m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_40m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_30m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_20m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_17m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_15m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_12m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_CB(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_10m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_6m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_4m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_FM(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_2m(int8_t direction);
static void SYSMENU_HANDL_CALIB_NOTX_70cm(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENABLE_60m_band(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENABLE_4m_band(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENABLE_AIR_band(int8_t direction);
static void SYSMENU_HANDL_CALIB_ENABLE_marine_band(int8_t direction);
static void SYSMENU_HANDL_CALIB_OTA_update(int8_t direction);
static void SYSMENU_HANDL_CALIB_TX_StartDelay(int8_t direction);
static void SYSMENU_HANDL_CALIB_PWR_VLT_Calibration(int8_t direction);
static void SYSMENU_HANDL_CALIB_PWR_CUR_Calibration(int8_t direction);
static void SYSMENU_HANDL_CALIB_LCD_Rotate(int8_t direction);
static void SYSMENU_HANDL_INA226_PWR_MON(int8_t direction); // Tisho
static void SYSMENU_HANDL_INA226_CUR_CALL(int8_t direction);
static void SYSMENU_HANDL_CALIB_ATU_AVERAGING(int8_t direction);
static void SYSMENU_HANDL_CALIB_CAT_Type(int8_t direction);
static void SYSMENU_HANDL_CALIB_LNA_compensation(int8_t direction);
static void SYSMENU_HANDL_CALIB_TwoSignalTune_Balance(int8_t direction);
static void SYSMENU_HANDL_CALIB_LinearPowerControl(int8_t direction);
static void SYSMENU_HANDL_CALIB_FlashGT911(int8_t direction);
static void SYSMENU_HANDL_CALIB_IF_GAIN_MIN(int8_t direction);
static void SYSMENU_HANDL_CALIB_IF_GAIN_MAX(int8_t direction);
static void SYSMENU_HANDL_CALIB_SETTINGS_RESET(int8_t direction);
static void SYSMENU_HANDL_CALIB_CALIBRATION_RESET(int8_t direction);
static void SYSMENU_HANDL_CALIB_WIFI_RESET(int8_t direction);

static void SYSMENU_HANDL_TRXMENU(int8_t direction);
static void SYSMENU_HANDL_AUDIOMENU(int8_t direction);
static void SYSMENU_HANDL_CWMENU(int8_t direction);
static void SYSMENU_HANDL_LCDMENU(int8_t direction);
static void SYSMENU_HANDL_DECODERSMENU(int8_t direction);
static void SYSMENU_HANDL_ADCMENU(int8_t direction);
static void SYSMENU_HANDL_WIFIMENU(int8_t direction);
static void SYSMENU_HANDL_SDMENU(int8_t direction);
static void SYSMENU_HANDL_CALIBRATIONMENU(int8_t direction);

#if HRDW_HAS_WIFI
static void SYSMENU_HANDL_DX_CLUSTER(int8_t direction);
static void SYSMENU_HANDL_RDA_STATS(int8_t direction);
static void SYSMENU_HANDL_PROPAGINATION(int8_t direction);
static void SYSMENU_HANDL_DAYNIGHT_MAP(int8_t direction);
static void SYSMENU_HANDL_IONOGRAM(int8_t direction);
#endif
static void SYSMENU_HANDL_SPECTRUMMENU(int8_t direction);
static void SYSMENU_HANDL_SWR_ANALYSER_MENU(int8_t direction);
static void SYSMENU_HANDL_WSPRMENU(int8_t direction);
#if HRDW_HAS_SD
static void SYSMENU_HANDL_RECORD_CQ_WAV(int8_t direction);
#endif
static void SYSMENU_HANDL_FT8_Decoder(int8_t direction);	 // Tisho
static void SYSMENU_HANDL_SWR_Tandem_Ctrl(int8_t direction); // Tisho
static void SYSMENU_HANDL_LOCATOR_INFO(int8_t direction);
static void SYSMENU_HANDL_CALLSIGN_INFO(int8_t direction);
static void SYSMENU_HANDL_SELF_TEST(int8_t direction);
static void SYSMENU_HANDL_AUTO_CALIBRATION(int8_t direction);

static void SYSMENU_HANDL_SPECTRUM_Begin(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_Start(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_End(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_TopDBM(int8_t direction);
static void SYSMENU_HANDL_SPECTRUM_BottomDBM(int8_t direction);

static void SYSMENU_HANDL_SWR_BAND_START(int8_t direction);
static void SYSMENU_HANDL_SWR_HF_START(int8_t direction);
static void SYSMENU_HANDL_SWR_CUSTOM_Begin(int8_t direction);
static void SYSMENU_HANDL_SWR_CUSTOM_Start(int8_t direction);
static void SYSMENU_HANDL_SWR_CUSTOM_End(int8_t direction);

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

static void SYSMENU_HANDL_AUTO_CALIBRATION_SWR(int8_t direction);
static void SYSMENU_HANDL_AUTO_CALIBRATION_POWER(int8_t direction);

static bool SYSMENU_HANDL_CHECK_HAS_LPF(void);
static bool SYSMENU_HANDL_CHECK_HAS_HPF(void);
static bool SYSMENU_HANDL_CHECK_HAS_BPF_8(void);
static bool SYSMENU_HANDL_CHECK_HAS_BPF_9(void);
static bool SYSMENU_HANDL_CHECK_HAS_RFFILTERS_BYPASS(void);
static bool SYSMENU_HANDL_CHECK_HIDDEN_ENABLED(void);

#ifdef LCD_SMALL_INTERFACE
	#define interactive_menu_top 46
#else
	#define interactive_menu_top 54
#endif

const static struct sysmenu_item_handler sysmenu_handlers[] =
	{
		{"TRX Settings", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_TRXMENU},
		{"AUDIO Settings", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_AUDIOMENU},
		{"CW Settings", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_CWMENU},
		{"SCREEN Settings", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_LCDMENU},
		{"Decoders", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_DECODERSMENU},
		{"ADC/DAC Settings", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_ADCMENU},
		#if HRDW_HAS_WIFI
		{"WIFI Settings", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_WIFIMENU},
		#endif
		#if HRDW_HAS_SD
		{"SD Card", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_SDMENU},
		#endif
		{"Set Clock Time", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SETTIME},
		{"DFU Mode", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_Bootloader},
		#if HRDW_HAS_SD
		{"OTA Update", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_OTA_Update},
		#endif
		{"Services", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SERVICESMENU},
		{"System info", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SYSINFO},
		{"Support project", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SUPPORT},
		{"Calibration", SYSMENU_MENU, SYSMENU_HANDL_CHECK_HIDDEN_ENABLED, 0, SYSMENU_HANDL_CALIBRATIONMENU},
};

const static struct sysmenu_item_handler sysmenu_trx_handlers[] =
	{
		{"RF Power", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.RF_Power, SYSMENU_HANDL_TRX_RFPower},
		{"Channel Mode", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.ChannelMode, SYSMENU_HANDL_TRX_ChannelMode},
		{"Band Map", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.BandMapEnabled, SYSMENU_HANDL_TRX_BandMap},
		{"AutoGainer", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.AutoGain, SYSMENU_HANDL_TRX_AutoGain},
#if !defined(FRONTPANEL_LITE)
		{"RF_Filters", SYSMENU_BOOLEAN, SYSMENU_HANDL_CHECK_HAS_RFFILTERS_BYPASS, (uint32_t *)&TRX.RF_Filters, SYSMENU_HANDL_TRX_RFFilters},
#endif
		{"Two Signal TUNE", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.TWO_SIGNAL_TUNE, SYSMENU_HANDL_TRX_TWO_SIGNAL_TUNE},
		{"RIT Interval", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.RIT_INTERVAL, SYSMENU_HANDL_TRX_RIT_INTERVAL},
		{"XIT Interval", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.XIT_INTERVAL, SYSMENU_HANDL_TRX_XIT_INTERVAL},
#if !defined(FRONTPANEL_LITE) && !defined(FRONTPANEL_X1)
		{"Fine RIT Tune", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FineRITTune, SYSMENU_HANDL_TRX_FineRITTune},
#endif
		{"TRX Samplerate", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.SAMPLERATE_MAIN, SYSMENU_HANDL_TRX_SAMPLERATE_MAIN, {"48khz", "96khz", "192khz", "384khz"}},
		{"FM Samplerate", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.SAMPLERATE_FM, SYSMENU_HANDL_TRX_SAMPLERATE_FM, {"48khz", "96khz", "192khz", "384khz"}},
		{"Freq Step", SYSMENU_UINT32R, NULL, (uint32_t *)&TRX.FRQ_STEP, SYSMENU_HANDL_TRX_FRQ_STEP},
		{"Freq Step FAST", SYSMENU_UINT32R, NULL, (uint32_t *)&TRX.FRQ_FAST_STEP, SYSMENU_HANDL_TRX_FRQ_FAST_STEP},
		{"Freq Step ENC2", SYSMENU_UINT32R, NULL, (uint32_t *)&TRX.FRQ_ENC_STEP, SYSMENU_HANDL_TRX_FRQ_ENC_STEP},
		{"Freq Step ENC2 FAST", SYSMENU_UINT32R, NULL, (uint32_t *)&TRX.FRQ_ENC_FAST_STEP, SYSMENU_HANDL_TRX_FRQ_ENC_FAST_STEP},
		{"Freq Step WFM, kHz", SYSMENU_UINT32R, NULL, (uint32_t *)&TRX.FRQ_ENC_WFM_STEP_KHZ, SYSMENU_HANDL_TRX_FRQ_ENC_WFM_STEP_KHZ},
		{"CW Freq Step divider", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.FRQ_CW_STEP_DIVIDER, SYSMENU_HANDL_TRX_FRQ_CW_STEP_DIVIDER},
		{"Encoder Accelerate", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Encoder_Accelerate, SYSMENU_HANDL_TRX_ENC_ACCELERATE},
		{"Att step, dB", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.ATT_STEP, SYSMENU_HANDL_TRX_ATT_STEP},
		{"Attenuation, dB", SYSMENU_FLOAT32, NULL, (uint32_t *)&TRX.ATT_DB, SYSMENU_HANDL_TRX_ATT_DB},
#if HRDW_HAS_VGA
		{"VGA Gain, dB", SYSMENU_FLOAT32, NULL, (uint32_t *)&TRX.VGA_GAIN, SYSMENU_HANDL_TRX_VGA_GAIN},
#endif
#if HRDW_HAS_USB_DEBUG
		{"DEBUG Console", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.Debug_Type, SYSMENU_HANDL_TRX_DEBUG_TYPE, {"OFF", "SYSTEM", "WIFI", "BUTTONS", "TOUCH", "CAT"}},
#endif
		{"Auto Input Switch", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Auto_Input_Switch, SYSMENU_HANDL_TRX_Auto_Input_Switch},
		{"Auto Snap", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Auto_Snap, SYSMENU_HANDL_TRX_Auto_Snap},
		{"Input Type MAIN", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.InputType_MAIN, SYSMENU_HANDL_TRX_INPUT_TYPE_MAIN, {"MIC", "LINE", "USB"}},
		{"Input Type DIGI", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.InputType_DIGI, SYSMENU_HANDL_TRX_INPUT_TYPE_DIGI, {"MIC", "LINE", "USB"}},
		{"Callsign", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_TRX_SetCallsign},
		{"Locator", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_TRX_SetLocator},
		{"URSI Code", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_TRX_SetURSICode},
		{"Transverter 70cm", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Transverter_70cm, SYSMENU_HANDL_TRX_TRANSV_70CM},
		{"Transverter 23cm", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Transverter_23cm, SYSMENU_HANDL_TRX_TRANSV_23CM},
		{"Transverter 13cm", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Transverter_13cm, SYSMENU_HANDL_TRX_TRANSV_13CM},
		{"Transverter 6cm", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Transverter_6cm, SYSMENU_HANDL_TRX_TRANSV_6CM},
		{"Transverter 3cm", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Transverter_3cm, SYSMENU_HANDL_TRX_TRANSV_3CM},
		{"Custom Transverter", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Custom_Transverter_Enabled, SYSMENU_HANDL_TRX_TRANSV_ENABLE},
		{"Transverter Offset, mHz", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.Transverter_Offset_Mhz, SYSMENU_HANDL_TRX_TRANSV_OFFSET},
		{"TUNER Enabled", SYSMENU_BOOLEAN, SYSMENU_HANDL_CHECK_HAS_ATU, (uint32_t *)&TRX.TUNER_Enabled, SYSMENU_HANDL_TRX_TUNER_Enabled},
		{"ATU Enabled", SYSMENU_BOOLEAN, SYSMENU_HANDL_CHECK_HAS_ATU, (uint32_t *)&TRX.ATU_Enabled, SYSMENU_HANDL_TRX_ATU_Enabled},
		{"ATU Ind", SYSMENU_ATU_I, SYSMENU_HANDL_CHECK_HAS_ATU, (uint32_t *)&TRX.ATU_I, SYSMENU_HANDL_TRX_ATU_I},
		{"ATU Cap", SYSMENU_ATU_C, SYSMENU_HANDL_CHECK_HAS_ATU, (uint32_t *)&TRX.ATU_C, SYSMENU_HANDL_TRX_ATU_C},
		{"ATU T", SYSMENU_BOOLEAN, SYSMENU_HANDL_CHECK_HAS_ATU, (uint32_t *)&TRX.ATU_T, SYSMENU_HANDL_TRX_ATU_T},
		{"", SYSMENU_INFOLINE, SYSMENU_HANDL_CHECK_HAS_ATU, 0},
		{"SWR:", SYSMENU_INFOLINE, SYSMENU_HANDL_CHECK_HAS_ATU, 0},
		{TRX_SWR_SMOOTHED_STR, SYSMENU_INFOLINE, SYSMENU_HANDL_CHECK_HAS_ATU, 0},
};

const static struct sysmenu_item_handler sysmenu_audio_handlers[] =
	{
#if defined(FRONTPANEL_X1) || defined(FRONTPANEL_LITE)
		{"Volume", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.Volume, SYSMENU_HANDL_AUDIO_Volume},
		{"Volume Step", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.Volume_Step, SYSMENU_HANDL_AUDIO_Volume_Step},
#endif
		{"IF Gain, dB", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.IF_Gain, SYSMENU_HANDL_AUDIO_IFGain},
		{"DNR", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.DNR_shadow, SYSMENU_HANDL_AUDIO_DNR, {"OFF", "DNR1", "DNR2"}},
		{"Noise blanker", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.NOISE_BLANKER, SYSMENU_HANDL_AUDIO_NOISE_BLANKER},
		{"AGC", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.AGC_shadow, SYSMENU_HANDL_AUDIO_AGC},
		{"AGC Gain target, LKFS", SYSMENU_INT8, NULL, (uint32_t *)&TRX.AGC_GAIN_TARGET, SYSMENU_HANDL_AUDIO_AGC_GAIN_TARGET},
		{"Mic Gain, dB", SYSMENU_FLOAT32, NULL, (uint32_t *)&TRX.MIC_GAIN_DB, SYSMENU_HANDL_AUDIO_MIC_Gain},
		{"Mic Boost", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.MIC_Boost, SYSMENU_HANDL_AUDIO_MIC_Boost},
		{"MIC Noise Gate", SYSMENU_INT8, NULL, (uint32_t *)&TRX.MIC_NOISE_GATE, SYSMENU_HANDL_AUDIO_MIC_NOISE_GATE},
		{"DNR1 Threshold", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.DNR1_SNR_THRESHOLD, SYSMENU_HANDL_AUDIO_DNR1_THRES},
#if !defined(FRONTPANEL_LITE)
		{"DNR2 Threshold", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.DNR2_SNR_THRESHOLD, SYSMENU_HANDL_AUDIO_DNR2_THRES},
#endif
		{"DNR Average", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.DNR_AVERAGE, SYSMENU_HANDL_AUDIO_DNR_AVERAGE},
		{"DNR Minimal", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.DNR_MINIMAL, SYSMENU_HANDL_AUDIO_DNR_MINMAL},
		{"SSB HPF RX Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.SSB_HPF_RX_Filter, SYSMENU_HANDL_AUDIO_SSB_HPF_RX_pass},
		{"SSB HPF TX Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.SSB_HPF_TX_Filter, SYSMENU_HANDL_AUDIO_SSB_HPF_TX_pass},
		{"SSB LPF RX Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.SSB_LPF_RX_Filter, SYSMENU_HANDL_AUDIO_SSB_LPF_RX_pass},
		{"SSB LPF TX Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.SSB_LPF_TX_Filter, SYSMENU_HANDL_AUDIO_SSB_LPF_TX_pass},
		{"CW LPF Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.CW_LPF_Filter, SYSMENU_HANDL_AUDIO_CW_LPF_pass},
		{"DIGI LPF Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.DIGI_LPF_Filter, SYSMENU_HANDL_AUDIO_DIGI_LPF_pass},
		{"AM LPF RX Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.AM_LPF_RX_Filter, SYSMENU_HANDL_AUDIO_AM_LPF_RX_pass},
		{"AM LPF TX Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.AM_LPF_TX_Filter, SYSMENU_HANDL_AUDIO_AM_LPF_TX_pass},
		{"FM LPF RX Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.FM_LPF_RX_Filter, SYSMENU_HANDL_AUDIO_FM_LPF_RX_pass},
		{"FM LPF TX Pass", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.FM_LPF_TX_Filter, SYSMENU_HANDL_AUDIO_FM_LPF_TX_pass},
		{"Squelch", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.SQL_shadow, SYSMENU_HANDL_AUDIO_Squelch},
		{"FM Squelch level, dbm", SYSMENU_INT8, NULL, (uint32_t *)&TRX.FM_SQL_threshold_dbm_shadow, SYSMENU_HANDL_AUDIO_FMSquelch},
		{"MIC EQ Low SSB", SYSMENU_INT8, NULL, (uint32_t *)&TRX.MIC_EQ_LOW_SSB, SYSMENU_HANDL_AUDIO_MIC_EQ_LOW_SSB},
		{"MIC EQ Mid SSB", SYSMENU_INT8, NULL, (uint32_t *)&TRX.MIC_EQ_MID_SSB, SYSMENU_HANDL_AUDIO_MIC_EQ_MID_SSB},
		{"MIC EQ High SSB", SYSMENU_INT8, NULL, (uint32_t *)&TRX.MIC_EQ_HIG_SSB, SYSMENU_HANDL_AUDIO_MIC_EQ_HIG_SSB},
		{"MIC EQ Low AMFM", SYSMENU_INT8, NULL, (uint32_t *)&TRX.MIC_EQ_LOW_AMFM, SYSMENU_HANDL_AUDIO_MIC_EQ_LOW_AMFM},
		{"MIC EQ Mid AMFM", SYSMENU_INT8, NULL, (uint32_t *)&TRX.MIC_EQ_MID_AMFM, SYSMENU_HANDL_AUDIO_MIC_EQ_MID_AMFM},
		{"MIC EQ High AMFM", SYSMENU_INT8, NULL, (uint32_t *)&TRX.MIC_EQ_HIG_AMFM, SYSMENU_HANDL_AUDIO_MIC_EQ_HIG_AMFM},
#if !defined(STM32F407xx)
		{"MIC Reverber", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.MIC_REVERBER, SYSMENU_HANDL_AUDIO_MIC_REVERBER},
#endif
		{"RX EQ Low", SYSMENU_INT8, NULL, (uint32_t *)&TRX.RX_EQ_LOW, SYSMENU_HANDL_AUDIO_RX_EQ_LOW},
		{"RX EQ Mid", SYSMENU_INT8, NULL, (uint32_t *)&TRX.RX_EQ_MID, SYSMENU_HANDL_AUDIO_RX_EQ_MID},
		{"RX EQ High", SYSMENU_INT8, NULL, (uint32_t *)&TRX.RX_EQ_HIG, SYSMENU_HANDL_AUDIO_RX_EQ_HIG},
		{"RX AGC SSB Speed", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.RX_AGC_SSB_speed, SYSMENU_HANDL_AUDIO_RX_AGC_SSB_Speed},
		{"RX AGC CW Speed", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.RX_AGC_CW_speed, SYSMENU_HANDL_AUDIO_RX_AGC_CW_Speed},
		{"RX AGC Max gain", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.RX_AGC_Max_gain, SYSMENU_HANDL_AUDIO_RX_AGC_Max_gain},
		{"RX AGC Hold time", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.RX_AGC_Hold, SYSMENU_HANDL_AUDIO_RX_AGC_Hold},
		{"TX Compr Speed SSB", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.TX_Compressor_speed_SSB, SYSMENU_HANDL_AUDIO_TX_CompressorSpeed_SSB},
		{"TX Compr MaxGain SSB", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.TX_Compressor_maxgain_SSB, SYSMENU_HANDL_AUDIO_TX_CompressorMaxGain_SSB},
		{"TX Compr Speed AMFM", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.TX_Compressor_speed_AMFM, SYSMENU_HANDL_AUDIO_TX_CompressorSpeed_AMFM},
		{"TX Compr MaxGain AMFM", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.TX_Compressor_maxgain_AMFM, SYSMENU_HANDL_AUDIO_TX_CompressorMaxGain_AMFM},
		{"Beeper", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Beeper, SYSMENU_HANDL_AUDIO_Beeper},
		{"CTCSS Frequency", SYSMENU_FLOAT32, NULL, (uint32_t *)&TRX.CTCSS_Freq, SYSMENU_HANDL_AUDIO_CTCSS_Freq},
		{"SelfHear Volume", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.SELFHEAR_Volume, SYSMENU_HANDL_AUDIO_SELFHEAR_Volume},
#if !defined(STM32F407xx)
		{"WFM Stereo", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FM_Stereo, SYSMENU_HANDL_AUDIO_FM_Stereo},
#endif
		{"AGC Spectral", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.AGC_Spectral, SYSMENU_HANDL_AUDIO_AGC_Spectral},
		{"TX CESSB", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.TX_CESSB, SYSMENU_HANDL_AUDIO_TX_CESSB},
		{"TX CESSB Compress, dB", SYSMENU_FLOAT32, NULL, (uint32_t *)&TRX.TX_CESSB_COMPRESS_DB, SYSMENU_HANDL_AUDIO_TX_CESSB_COMPRESS_DB},
		{"VAD Threshold", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.VAD_THRESHOLD, SYSMENU_HANDL_AUDIO_VAD_THRESHOLD},
		{"VOX", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.VOX, SYSMENU_HANDL_AUDIO_VOX},
		{"VOX Timeout, ms", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.VOX_TIMEOUT, SYSMENU_HANDL_AUDIO_VOX_TIMEOUT},
		{"VOX Threshold, dbFS", SYSMENU_INT8, NULL, (uint32_t *)&TRX.VOX_THRESHOLD, SYSMENU_HANDL_AUDIO_VOX_THRESHOLD},
};

const static struct sysmenu_item_handler sysmenu_cw_handlers[] =
	{
		{"CW Key timeout", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.CW_Key_timeout, SYSMENU_HANDL_CW_Key_timeout},
		{"CW Pitch", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.CW_Pitch, SYSMENU_HANDL_CW_Pitch},
		{"CW Self Hear", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.CW_SelfHear, SYSMENU_HANDL_CW_SelfHear},
		{"CW Keyer", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.CW_KEYER, SYSMENU_HANDL_CW_Keyer},
		{"CW Keyer WPM", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.CW_KEYER_WPM, SYSMENU_HANDL_CW_Keyer_WPM},
		{"CW Gauss filter", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.CW_GaussFilter, SYSMENU_HANDL_CW_GaussFilter},
		{"CW DotToDash Rate", SYSMENU_FLOAT32, NULL, (uint32_t *)&TRX.CW_DotToDashRate, SYSMENU_HANDL_CW_DotToDashRate},
		{"CW Iambic Keyer", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.CW_Iambic, SYSMENU_HANDL_CW_Iambic},
		{"CW Key Invert", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.CW_Invert, SYSMENU_HANDL_CW_Invert},
		{"CW PTT Type", SYSMENU_ENUMR, NULL, (uint32_t *)&TRX.CW_PTT_Type, SYSMENU_HANDL_CW_PTT_Type, {"Key", "PTT", "KEY+PTT"}},
};

const static struct sysmenu_item_handler sysmenu_screen_handlers[] =
	{
		{"FFT Zoom", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.FFT_Zoom, SYSMENU_HANDL_SCREEN_FFT_Zoom},
		{"FFT Zoom CW", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.FFT_ZoomCW, SYSMENU_HANDL_SCREEN_FFT_ZoomCW},
#ifdef HAS_BRIGHTNESS_CONTROL
		{"LCD Brightness", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.LCD_Brightness, SYSMENU_HANDL_SCREEN_LCD_Brightness},
		{"LCD Sleep Timeout", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.LCD_SleepTimeout, SYSMENU_HANDL_SCREEN_LCD_SleepTimeout},
#endif
		{"Color Theme", SYSMENU_ENUMR, NULL, (uint32_t *)&TRX.ColorThemeId, SYSMENU_HANDL_SCREEN_COLOR_THEME, {"Black", "White", "Colored"}},
#ifdef LAY_480x320
		{"Layout Theme", SYSMENU_ENUMR, NULL, (uint32_t *)&TRX.LayoutThemeId, SYSMENU_HANDL_SCREEN_LAYOUT_THEME, {"Default", "7 Segm"}},
#endif
#ifdef LAY_800x480
		{"Layout Theme", SYSMENU_ENUMR, NULL, (uint32_t *)&TRX.LayoutThemeId, SYSMENU_HANDL_SCREEN_LAYOUT_THEME, {"Default", "Analog", "7 Segm", "Classic", "Default+", "Analog+"}},
#endif
		{"FFT Speed", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.FFT_Speed, SYSMENU_HANDL_SCREEN_FFT_Speed},
		{"FFT Automatic", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FFT_Automatic, SYSMENU_HANDL_SCREEN_FFT_Automatic},
		{"FFT Sensitivity", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.FFT_Sensitivity, SYSMENU_HANDL_SCREEN_FFT_Sensitivity},
		{"FFT Manual Bottom, dBm", SYSMENU_INT16, NULL, (uint32_t *)&TRX.FFT_ManualBottom, SYSMENU_HANDL_SCREEN_FFT_ManualBottom},
		{"FFT Manual Top, dBm", SYSMENU_INT16, NULL, (uint32_t *)&TRX.FFT_ManualTop, SYSMENU_HANDL_SCREEN_FFT_ManualTop},
		{"FFT Height", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.FFT_Height, SYSMENU_HANDL_SCREEN_FFT_Height},
		{"FFT Style", SYSMENU_ENUMR, NULL, (uint32_t *)&TRX.FFT_Style, SYSMENU_HANDL_SCREEN_FFT_Style, {"", "Gradien", "Fill", "Dots", "Contour"}},
#if !defined(FRONTPANEL_LITE)
		{"FFT BW Style", SYSMENU_ENUMR, NULL, (uint32_t *)&TRX.FFT_BW_Style, SYSMENU_HANDL_SCREEN_FFT_BW_Style, {"", "Fill", "LowOp", "Line"}},
#endif
		{"FFT Color", SYSMENU_ENUMR, NULL, (uint32_t *)&TRX.FFT_Color, SYSMENU_HANDL_SCREEN_FFT_Color, {"", "Blu>Y>R", "BlB>Y>R", "BlR>Y>R", "Bla>Y>R", "Bla>Y>G", "Bla>R", "Bla>G", "Bla>Blu", "Bla>W"}},
		{"WTF Color", SYSMENU_ENUMR, NULL, (uint32_t *)&TRX.WTF_Color, SYSMENU_HANDL_SCREEN_WTF_Color, {"", "Blu>Y>R", "BlB>Y>R", "BlR>Y>R", "Bla>Y>R", "Bla>Y>G", "Bla>R", "Bla>G", "Bla>Blu", "Bla>W"}},
		{"FFT Freq Grid", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.FFT_FreqGrid, SYSMENU_HANDL_SCREEN_FFT_FreqGrid, {"NO", "Top", "All", "Bott"}},
#if !defined(FRONTPANEL_LITE)
		{"FFT dBm Grid", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FFT_dBmGrid, SYSMENU_HANDL_SCREEN_FFT_dBmGrid},
#endif
		{"FFT Background", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FFT_Background, SYSMENU_HANDL_SCREEN_FFT_Background},
#if !defined(FRONTPANEL_LITE)
		{"FFT Lens", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FFT_Lens, SYSMENU_HANDL_SCREEN_FFT_Lens},
		{"FFT Hold Peaks", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FFT_HoldPeaks, SYSMENU_HANDL_SCREEN_FFT_HoldPeaks},
		{"FFT 3D Mode", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.FFT_3D, SYSMENU_HANDL_SCREEN_FFT_3D, {"NO", "Lines", "Dots"}},
#endif
		{"FFT Enabled", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FFT_Enabled, SYSMENU_HANDL_SCREEN_FFT_Enabled},
		{"WTF Moving", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WTF_Moving, SYSMENU_HANDL_SCREEN_WTF_Moving},
		{"FFT Compressor", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FFT_Compressor, SYSMENU_HANDL_SCREEN_FFT_Compressor},
		{"FFT Averaging", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.FFT_Averaging, SYSMENU_HANDL_SCREEN_FFT_Averaging},
		{"FFT Window", SYSMENU_ENUMR, NULL, (uint32_t *)&TRX.FFT_Window, SYSMENU_HANDL_SCREEN_FFT_Window, {"", "Dolph", "Blckman", "Nutall", "BlNutll", "Hann", "Hamming", "No"}},
		{"FFT Scale Type", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.FFT_Scale_Type, SYSMENU_HANDL_SCREEN_FFT_Scale_Type, {"Ampl", "Squared", "dBm"}},
#if HRDW_HAS_WIFI
		{"FFT DXCluster", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FFT_DXCluster, SYSMENU_HANDL_SCREEN_FFT_DXCluster},
		{"FFT DXCluster Azimuth", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.FFT_DXCluster_Azimuth, SYSMENU_HANDL_SCREEN_FFT_DXCluster_Azimuth},
		{"FFT DXCluster Timeout", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.FFT_DXCluster_Timeout, SYSMENU_HANDL_SCREEN_FFT_DXCluster_Timeout},
#endif
#if !defined(FRONTPANEL_LITE)
		{"Show Sec VFO", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.Show_Sec_VFO, SYSMENU_HANDL_SCREEN_Show_Sec_VFO},
#endif
#ifdef HRDW_HAS_FUNCBUTTONS
		{"Func button 1", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[0], SYSMENU_HANDL_SCREEN_FUNC_BUTTON1},
		{"Func button 2", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[1], SYSMENU_HANDL_SCREEN_FUNC_BUTTON2},
		{"Func button 3", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[2], SYSMENU_HANDL_SCREEN_FUNC_BUTTON3},
		{"Func button 4", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[3], SYSMENU_HANDL_SCREEN_FUNC_BUTTON4},
		{"Func button 5", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[4], SYSMENU_HANDL_SCREEN_FUNC_BUTTON5},
		{"Func button 6", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[5], SYSMENU_HANDL_SCREEN_FUNC_BUTTON6},
		{"Func button 7", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[6], SYSMENU_HANDL_SCREEN_FUNC_BUTTON7},
		{"Func button 8", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[7], SYSMENU_HANDL_SCREEN_FUNC_BUTTON8},
		{"Func button 9", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[8], SYSMENU_HANDL_SCREEN_FUNC_BUTTON9},
		{"Func button 10", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[9], SYSMENU_HANDL_SCREEN_FUNC_BUTTON10},
		{"Func button 11", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[10], SYSMENU_HANDL_SCREEN_FUNC_BUTTON11},
		{"Func button 12", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[11], SYSMENU_HANDL_SCREEN_FUNC_BUTTON12},
		{"Func button 13", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[12], SYSMENU_HANDL_SCREEN_FUNC_BUTTON13},
		{"Func button 14", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[13], SYSMENU_HANDL_SCREEN_FUNC_BUTTON14},
		{"Func button 15", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[14], SYSMENU_HANDL_SCREEN_FUNC_BUTTON15},
		{"Func button 16", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[15], SYSMENU_HANDL_SCREEN_FUNC_BUTTON16},
		{"Func button 17", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[16], SYSMENU_HANDL_SCREEN_FUNC_BUTTON17},
		{"Func button 18", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[17], SYSMENU_HANDL_SCREEN_FUNC_BUTTON18},
		{"Func button 19", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[18], SYSMENU_HANDL_SCREEN_FUNC_BUTTON19},
		{"Func button 20", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[19], SYSMENU_HANDL_SCREEN_FUNC_BUTTON20},
		{"Func button 21", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[20], SYSMENU_HANDL_SCREEN_FUNC_BUTTON21},
		{"Func button 22", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[21], SYSMENU_HANDL_SCREEN_FUNC_BUTTON22},
		{"Func button 23", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[22], SYSMENU_HANDL_SCREEN_FUNC_BUTTON23},
		{"Func button 24", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[23], SYSMENU_HANDL_SCREEN_FUNC_BUTTON24},
		{"Func button 25", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[24], SYSMENU_HANDL_SCREEN_FUNC_BUTTON25},
#if (FUNCBUTTONS_ON_PAGE * FUNCBUTTONS_PAGES) > 25
		{"Func button 26", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[25], SYSMENU_HANDL_SCREEN_FUNC_BUTTON26},
		{"Func button 27", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[26], SYSMENU_HANDL_SCREEN_FUNC_BUTTON27},
#if (FUNCBUTTONS_ON_PAGE * FUNCBUTTONS_PAGES) > 27
		{"Func button 28", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[27], SYSMENU_HANDL_SCREEN_FUNC_BUTTON28},
		{"Func button 29", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[28], SYSMENU_HANDL_SCREEN_FUNC_BUTTON29},
		{"Func button 30", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[29], SYSMENU_HANDL_SCREEN_FUNC_BUTTON30},
#if (FUNCBUTTONS_ON_PAGE * FUNCBUTTONS_PAGES) > 30
		{"Func button 31", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[30], SYSMENU_HANDL_SCREEN_FUNC_BUTTON31},
		{"Func button 32", SYSMENU_FUNCBUTTON, NULL, (uint32_t *)&TRX.FuncButtons[31], SYSMENU_HANDL_SCREEN_FUNC_BUTTON32},
#endif
#endif
#endif
#endif
};

const static struct sysmenu_item_handler sysmenu_decoders_handlers[] =
	{
		#if !defined(FRONTPANEL_LITE)
		{"CW Decoder", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.CW_Decoder, SYSMENU_HANDL_DECODERS_CW_Decoder},
		{"CW Decoder Threshold", SYSMENU_UINT8, NULL, (uint32_t *)&TRX.CW_Decoder_Threshold, SYSMENU_HANDL_DECODERS_CW_Decoder_Threshold},
		{"RDS Decoder", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.RDS_Decoder, SYSMENU_HANDL_DECODERS_RDS_Decoder},
		#endif
		{"RTTY Speed", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.RTTY_Speed, SYSMENU_HANDL_DECODERS_RTTY_Speed},
		{"RTTY Shift", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.RTTY_Shift, SYSMENU_HANDL_DECODERS_RTTY_Shift},
		{"RTTY Freq", SYSMENU_UINT16, NULL, (uint32_t *)&TRX.RTTY_Freq, SYSMENU_HANDL_DECODERS_RTTY_Freq},
		{"RTTY StopBits", SYSMENU_ENUM, NULL, (uint32_t *)&TRX.RTTY_StopBits, SYSMENU_HANDL_DECODERS_RTTY_StopBits, {"1", "1.5", "2"}},
		{"RTTY InvertBits", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.RTTY_InvertBits, SYSMENU_HANDL_DECODERS_RTTY_InvertBits},
};

const static struct sysmenu_item_handler sysmenu_adc_handlers[] =
	{
		{"ADC Driver", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.ADC_Driver, SYSMENU_HANDL_ADC_DRIVER},
		#if !defined(FRONTPANEL_LITE)
		{"ADC Preamp", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.ADC_PGA, SYSMENU_HANDL_ADC_PGA},
		{"ADC Dither", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.ADC_DITH, SYSMENU_HANDL_ADC_DITH},
		{"ADC Randomizer", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.ADC_RAND, SYSMENU_HANDL_ADC_RAND},
		#endif
		{"ADC Shutdown", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.ADC_SHDN, SYSMENU_HANDL_ADC_SHDN},
};

#if HRDW_HAS_WIFI
const static struct sysmenu_item_handler sysmenu_wifi_handlers[] =
	{
		{"WIFI Enabled", SYSMENU_BOOLEAN, NULL, (uint32_t *)&WIFI.Enabled, SYSMENU_HANDL_WIFI_Enabled},
		{"WIFI Network 1", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_WIFI_SelectAP1},
		{"WIFI Network 1 Pass", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_WIFI_SetAP1password},
		{"WIFI Network 2", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_WIFI_SelectAP2},
		{"WIFI Network 2 Pass", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_WIFI_SetAP2password},
		{"WIFI Network 3", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_WIFI_SelectAP3},
		{"WIFI Network 3 Pass", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_WIFI_SetAP3password},
		{"WIFI Timezone", SYSMENU_INT8, NULL, (uint32_t *)&WIFI.Timezone, SYSMENU_HANDL_WIFI_Timezone},
		{"WIFI CAT Server", SYSMENU_BOOLEAN, NULL, (uint32_t *)&WIFI.CAT_Server, SYSMENU_HANDL_WIFI_CAT_Server},
		{"WIFI Update ESP firmware", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_WIFI_UpdateFW},
		{"", SYSMENU_INFOLINE, 0, 0},
		{"NET:", SYSMENU_INFOLINE, 0, 0},
		{WIFI_AP, SYSMENU_INFOLINE, 0, 0},
		{"", SYSMENU_INFOLINE, 0, 0},
		{"IP:", SYSMENU_INFOLINE, 0, 0},
		{WIFI_IP, SYSMENU_INFOLINE, 0, 0},
};
#endif

#if HRDW_HAS_SD
const static struct sysmenu_item_handler sysmenu_sd_handlers[] =
	{
		{"File Manager", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SD_Filemanager},
		{"USB SD Card Reader", SYSMENU_BOOLEAN, NULL, (uint32_t *)&SD_USBCardReader, SYSMENU_HANDL_SD_USB},
		{"Export Settings to SD", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SD_ExportSettingsDialog},
		{"Import Settings from SD", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SD_ImportSettingsDialog},
		{"Format SD card", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SD_FormatDialog},
};

const static struct sysmenu_item_handler sysmenu_sd_export_handlers[] =
	{
		{"Back", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_Back},
		{"Yes, Export Settings", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SD_ExportSettings},
};

const static struct sysmenu_item_handler sysmenu_sd_import_handlers[] =
	{
		{"Back", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_Back},
		{"Yes, Import Settings", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SD_ImportSettings},
};
const static struct sysmenu_item_handler sysmenu_sd_format_handlers[] =
	{
		{"Back", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_Back},
		{"Yes, Format SD Card", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SD_Format},
};
#endif

const static struct sysmenu_item_handler sysmenu_calibration_handlers[] =
	{
		{"Encoder invert", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.ENCODER_INVERT, SYSMENU_HANDL_CALIB_ENCODER_INVERT},
		{"Encoder2 invert", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.ENCODER2_INVERT, SYSMENU_HANDL_CALIB_ENCODER2_INVERT},
		{"Encoder debounce", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.ENCODER_DEBOUNCE, SYSMENU_HANDL_CALIB_ENCODER_DEBOUNCE},
		{"Encoder2 debounce", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.ENCODER2_DEBOUNCE, SYSMENU_HANDL_CALIB_ENCODER2_DEBOUNCE},
		{"Encoder slow rate", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.ENCODER_SLOW_RATE, SYSMENU_HANDL_CALIB_ENCODER_SLOW_RATE},
		{"Encoder on falling", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.ENCODER_ON_FALLING, SYSMENU_HANDL_CALIB_ENCODER_ON_FALLING},
		{"Encoder acceleration", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.ENCODER_ACCELERATION, SYSMENU_HANDL_CALIB_ENCODER_ACCELERATION},
#if !defined(FRONTPANEL_LITE)
		{"RF-Unit Type", SYSMENU_ENUM, NULL, (uint32_t *)&CALIBRATE.RF_unit_type, SYSMENU_HANDL_CALIB_RF_unit_type, {"QRP", "BIG", "SPLIT", "RU4PN", "WF-100D"}},
#endif
#if defined(FRONTPANEL_BIG_V1) || defined(FRONTPANEL_WF_100D)
		{"Tangent Type", SYSMENU_ENUM, NULL, (uint32_t *)&CALIBRATE.TangentType, SYSMENU_HANDL_CALIB_TangentType, {"MH-36", "MH-48"}},
#endif
		{"CICCOMP 48K Shift", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.CICFIR_GAINER_48K_val, SYSMENU_HANDL_CALIB_CICCOMP_48K_SHIFT},
		{"CICCOMP 96K Shift", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.CICFIR_GAINER_96K_val, SYSMENU_HANDL_CALIB_CICCOMP_96K_SHIFT},
		{"CICCOMP 192K Shift", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.CICFIR_GAINER_192K_val, SYSMENU_HANDL_CALIB_CICCOMP_192K_SHIFT},
		{"CICCOMP 384K Shift", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.CICFIR_GAINER_384K_val, SYSMENU_HANDL_CALIB_CICCOMP_384K_SHIFT},
		{"TX CICCOMP Shift", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.TXCICFIR_GAINER_val, SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT},
		{"DAC Shift", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.DAC_GAINER_val, SYSMENU_HANDL_CALIB_DAC_SHIFT},
#if !defined(FRONTPANEL_LITE)
		{"DAC Driver Mode", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.DAC_driver_mode, SYSMENU_HANDL_CALIB_DAC_driver_mode},
#endif
		{"RF GAIN 2200m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_2200m, SYSMENU_HANDL_CALIB_RF_GAIN_2200M},
		{"RF GAIN 160m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_160m, SYSMENU_HANDL_CALIB_RF_GAIN_160M},
		{"RF GAIN 80m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_80m, SYSMENU_HANDL_CALIB_RF_GAIN_80M},
		{"RF GAIN 40m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_40m, SYSMENU_HANDL_CALIB_RF_GAIN_40M},
		{"RF GAIN 30m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_30m, SYSMENU_HANDL_CALIB_RF_GAIN_30M},
		{"RF GAIN 20m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_20m, SYSMENU_HANDL_CALIB_RF_GAIN_20M},
		{"RF GAIN 17m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_17m, SYSMENU_HANDL_CALIB_RF_GAIN_17M},
		{"RF GAIN 15m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_15m, SYSMENU_HANDL_CALIB_RF_GAIN_15M},
		{"RF GAIN 12m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_12m, SYSMENU_HANDL_CALIB_RF_GAIN_12M},
		{"RF GAIN CB", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_cb, SYSMENU_HANDL_CALIB_RF_GAIN_CB},
		{"RF GAIN 10m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_10m, SYSMENU_HANDL_CALIB_RF_GAIN_10M},
#if !defined(FRONTPANEL_LITE)
		{"RF GAIN 6m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_6m, SYSMENU_HANDL_CALIB_RF_GAIN_6M},
		{"RF GAIN 4m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_4m, SYSMENU_HANDL_CALIB_RF_GAIN_4M},
		{"RF GAIN 2m", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.rf_out_power_2m, SYSMENU_HANDL_CALIB_RF_GAIN_2M},
#endif
		{"S METER HF", SYSMENU_INT16, NULL, (uint32_t *)&CALIBRATE.smeter_calibration_hf, SYSMENU_HANDL_CALIB_S_METER_HF},
		{"S METER VHF", SYSMENU_INT16, NULL, (uint32_t *)&CALIBRATE.smeter_calibration_vhf, SYSMENU_HANDL_CALIB_S_METER_VHF},
		{"ADC OFFSET", SYSMENU_INT16, NULL, (uint32_t *)&CALIBRATE.adc_offset, SYSMENU_HANDL_CALIB_ADC_OFFSET},
#if !defined(FRONTPANEL_LITE)
		{"LPF END", SYSMENU_UINT32, SYSMENU_HANDL_CHECK_HAS_LPF, (uint32_t *)&CALIBRATE.RFU_LPF_END, SYSMENU_HANDL_CALIB_LPF_END},
		{"HPF START", SYSMENU_UINT32, SYSMENU_HANDL_CHECK_HAS_LPF, (uint32_t *)&CALIBRATE.RFU_HPF_START, SYSMENU_HANDL_CALIB_HPF_START},
		{"BPF 0 START", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_0_START, SYSMENU_HANDL_CALIB_BPF_0_START},
		{"BPF 0 END", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_0_END, SYSMENU_HANDL_CALIB_BPF_0_END},
		{"BPF 1 START", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_1_START, SYSMENU_HANDL_CALIB_BPF_1_START},
		{"BPF 1 END", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_1_END, SYSMENU_HANDL_CALIB_BPF_1_END},
		{"BPF 2 START", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_2_START, SYSMENU_HANDL_CALIB_BPF_2_START},
		{"BPF 2 END", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_2_END, SYSMENU_HANDL_CALIB_BPF_2_END},
		{"BPF 3 START", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_3_START, SYSMENU_HANDL_CALIB_BPF_3_START},
		{"BPF 3 END", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_3_END, SYSMENU_HANDL_CALIB_BPF_3_END},
		{"BPF 4 START", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_4_START, SYSMENU_HANDL_CALIB_BPF_4_START},
		{"BPF 4 END", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_4_END, SYSMENU_HANDL_CALIB_BPF_4_END},
		{"BPF 5 START", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_5_START, SYSMENU_HANDL_CALIB_BPF_5_START},
		{"BPF 5 END", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_5_END, SYSMENU_HANDL_CALIB_BPF_5_END},
		{"BPF 6 START", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_6_START, SYSMENU_HANDL_CALIB_BPF_6_START},
		{"BPF 6 END", SYSMENU_UINT32, NULL, (uint32_t *)&CALIBRATE.RFU_BPF_6_END, SYSMENU_HANDL_CALIB_BPF_6_END},
		{"BPF 7 START", SYSMENU_UINT32, SYSMENU_HANDL_CHECK_HAS_BPF_8, (uint32_t *)&CALIBRATE.RFU_BPF_7_START, SYSMENU_HANDL_CALIB_BPF_7_START},
		{"BPF 7 END", SYSMENU_UINT32, SYSMENU_HANDL_CHECK_HAS_BPF_8, (uint32_t *)&CALIBRATE.RFU_BPF_7_END, SYSMENU_HANDL_CALIB_BPF_7_END},
		{"BPF 8 START", SYSMENU_UINT32, SYSMENU_HANDL_CHECK_HAS_BPF_9, (uint32_t *)&CALIBRATE.RFU_BPF_8_START, SYSMENU_HANDL_CALIB_BPF_8_START},
		{"BPF 8 END", SYSMENU_UINT32, SYSMENU_HANDL_CHECK_HAS_BPF_9, (uint32_t *)&CALIBRATE.RFU_BPF_8_END, SYSMENU_HANDL_CALIB_BPF_8_END},
#endif
		{"MAX PWR on Meter", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.MAX_RF_POWER_ON_METER, SYSMENU_HANDL_CALIB_MAX_RF_POWER_ON_METER},
		{"MAX Power in TUNE", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.TUNE_MAX_POWER, SYSMENU_HANDL_CALIB_TUNE_MAX_POWER},
		{"SSB Power addition", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.SSB_POWER_ADDITION, SYSMENU_HANDL_CALIB_SSB_POWER_ADDITION},
		{"SWR FWD RATE HF", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.SWR_FWD_Calibration_HF, SYSMENU_HANDL_CALIB_SWR_FWD_RATE_HF},
		{"SWR BWD RATE HF", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.SWR_BWD_Calibration_HF, SYSMENU_HANDL_CALIB_SWR_REF_RATE_HF},
#if !defined(FRONTPANEL_LITE)
		{"SWR FWD RATE 6M", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.SWR_FWD_Calibration_6M, SYSMENU_HANDL_CALIB_SWR_FWD_RATE_6M},
		{"SWR BWD RATE 6M", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.SWR_BWD_Calibration_6M, SYSMENU_HANDL_CALIB_SWR_REF_RATE_6M},
		{"SWR FWD RATE VHF", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.SWR_FWD_Calibration_VHF, SYSMENU_HANDL_CALIB_SWR_FWD_RATE_VHF},
		{"SWR BWD RATE VHF", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.SWR_BWD_Calibration_VHF, SYSMENU_HANDL_CALIB_SWR_REF_RATE_VHF},
#endif
		{"VCXO Correction", SYSMENU_INT16, NULL, (uint32_t *)&CALIBRATE.VCXO_correction, SYSMENU_HANDL_CALIB_VCXO},
#ifdef SWR_AD8307_LOG
		{"FW_AD8307_Slope (mv/dB)", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.FW_AD8307_SLP, SYSMENU_HANDL_CALIB_FW_AD8307_SLP},
		{"FW_AD8307_Offset (mV)", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.FW_AD8307_OFFS, SYSMENU_HANDL_CALIB_FW_AD8307_OFFS},
		{"BW_AD8307_Slope (mv/dB)", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.BW_AD8307_SLP, SYSMENU_HANDL_CALIB_BW_AD8307_SLP},
		{"BW_AD8307_Offset (mV)", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.BW_AD8307_OFFS, SYSMENU_HANDL_CALIB_BW_AD8307_OFFS},
#endif
#if !defined(FRONTPANEL_LITE)
		{"FAN Medium start", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.FAN_MEDIUM_START, SYSMENU_HANDL_CALIB_FAN_MEDIUM_START},
		{"FAN Medium stop", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.FAN_MEDIUM_STOP, SYSMENU_HANDL_CALIB_FAN_MEDIUM_STOP},
		{"FAN Full start", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.FAN_FULL_START, SYSMENU_HANDL_CALIB_FAN_FULL_START},
		{"MAX RF Temp", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.TRX_MAX_RF_TEMP, SYSMENU_HANDL_CALIB_TRX_MAX_RF_TEMP},
#endif
		{"MAX SWR", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.TRX_MAX_SWR, SYSMENU_HANDL_CALIB_TRX_MAX_SWR},
		{"FM Deviation Scale", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.FM_DEVIATION_SCALE, SYSMENU_HANDL_CALIB_FM_DEVIATION_SCALE},
		{"AM Modulation Index", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.AM_MODULATION_INDEX, SYSMENU_HANDL_CALIB_AM_MODULATION_INDEX},
		{"RTC COARSE CALIBR", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.RTC_Coarse_Calibration, SYSMENU_HANDL_CALIB_RTC_COARSE_CALIBRATION},
		{"RTC FINE CALIBR", SYSMENU_INT16, NULL, (uint32_t *)&CALIBRATE.RTC_Calibration, SYSMENU_HANDL_CALIB_RTC_CALIBRATION},
#if !defined(FRONTPANEL_LITE)
		{"EXT 2200m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_2200m, SYSMENU_HANDL_CALIB_EXT_2200m},
		{"EXT 160m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_160m, SYSMENU_HANDL_CALIB_EXT_160m},
		{"EXT 80m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_80m, SYSMENU_HANDL_CALIB_EXT_80m},
		{"EXT 60m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_60m, SYSMENU_HANDL_CALIB_EXT_60m},
		{"EXT 40m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_40m, SYSMENU_HANDL_CALIB_EXT_40m},
		{"EXT 30m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_30m, SYSMENU_HANDL_CALIB_EXT_30m},
		{"EXT 20m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_20m, SYSMENU_HANDL_CALIB_EXT_20m},
		{"EXT 17m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_17m, SYSMENU_HANDL_CALIB_EXT_17m},
		{"EXT 15m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_15m, SYSMENU_HANDL_CALIB_EXT_15m},
		{"EXT 12m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_12m, SYSMENU_HANDL_CALIB_EXT_12m},
		{"EXT CB", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_CB, SYSMENU_HANDL_CALIB_EXT_CB},
		{"EXT 10m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_10m, SYSMENU_HANDL_CALIB_EXT_10m},
		{"EXT 6m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_6m, SYSMENU_HANDL_CALIB_EXT_6m},
		{"EXT 4m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_4m, SYSMENU_HANDL_CALIB_EXT_4m},
		{"EXT FM", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_FM, SYSMENU_HANDL_CALIB_EXT_FM},
		{"EXT 2m", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_2m, SYSMENU_HANDL_CALIB_EXT_2m},
		{"EXT 70cm", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_70cm, SYSMENU_HANDL_CALIB_EXT_70cm},
		{"EXT Transv 70cm", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_TRANSV_70cm, SYSMENU_HANDL_CALIB_EXT_TRANSV_70cm},
		{"EXT Transv 23cm", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_TRANSV_23cm, SYSMENU_HANDL_CALIB_EXT_TRANSV_23cm},
		{"EXT Transv 13cm", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_TRANSV_13cm, SYSMENU_HANDL_CALIB_EXT_TRANSV_13cm},
		{"EXT Transv 6cm", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_TRANSV_6cm, SYSMENU_HANDL_CALIB_EXT_TRANSV_6cm},
		{"EXT Transv 3cm", SYSMENU_B4, NULL, (uint32_t *)&CALIBRATE.EXT_TRANSV_3cm, SYSMENU_HANDL_CALIB_EXT_TRANSV_3cm},
#endif
		{"NOTX NOT HAM", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_NOTHAM, SYSMENU_HANDL_CALIB_NOTX_NOTHAM},
		{"NOTX 2200m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_2200m, SYSMENU_HANDL_CALIB_NOTX_2200m},
		{"NOTX 160m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_160m, SYSMENU_HANDL_CALIB_NOTX_160m},
		{"NOTX 80m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_80m, SYSMENU_HANDL_CALIB_NOTX_80m},
		{"NOTX 60m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_60m, SYSMENU_HANDL_CALIB_NOTX_60m},
		{"NOTX 40m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_40m, SYSMENU_HANDL_CALIB_NOTX_40m},
		{"NOTX 30m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_30m, SYSMENU_HANDL_CALIB_NOTX_30m},
		{"NOTX 20m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_20m, SYSMENU_HANDL_CALIB_NOTX_20m},
		{"NOTX 17m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_17m, SYSMENU_HANDL_CALIB_NOTX_17m},
		{"NOTX 15m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_15m, SYSMENU_HANDL_CALIB_NOTX_15m},
		{"NOTX 12m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_12m, SYSMENU_HANDL_CALIB_NOTX_12m},
		{"NOTX CB", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_CB, SYSMENU_HANDL_CALIB_NOTX_CB},
		{"NOTX 10m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_10m, SYSMENU_HANDL_CALIB_NOTX_10m},
#if !defined(FRONTPANEL_LITE)
		{"NOTX 6m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_6m, SYSMENU_HANDL_CALIB_NOTX_6m},
		{"NOTX 4m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_4m, SYSMENU_HANDL_CALIB_NOTX_4m},
		{"NOTX 2m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_2m, SYSMENU_HANDL_CALIB_NOTX_2m},
		{"NOTX 70cm", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.NOTX_70cm, SYSMENU_HANDL_CALIB_NOTX_70cm},
#endif
		{"ENABLE 60M Band", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.ENABLE_60m_band, SYSMENU_HANDL_CALIB_ENABLE_60m_band},
#if !defined(FRONTPANEL_LITE)
		{"ENABLE 4M Band", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.ENABLE_4m_band, SYSMENU_HANDL_CALIB_ENABLE_4m_band},
		{"ENABLE Air Band", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.ENABLE_AIR_band, SYSMENU_HANDL_CALIB_ENABLE_AIR_band},
		{"ENABLE Marine Band", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.ENABLE_marine_band, SYSMENU_HANDL_CALIB_ENABLE_marine_band},
#endif
#if HRDW_HAS_WIFI
		{"OTA Update", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.OTA_update, SYSMENU_HANDL_CALIB_OTA_update},
#endif
		{"TX Start Delay", SYSMENU_UINT16, NULL, (uint32_t *)&CALIBRATE.TX_StartDelay, SYSMENU_HANDL_CALIB_TX_StartDelay},
		{"LCD Rotate", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.LCD_Rotate, SYSMENU_HANDL_CALIB_LCD_Rotate},
#if defined(FRONTPANEL_BIG_V1) || defined(FRONTPANEL_WF_100D)
		{"INA226_PWR_MON", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.INA226_EN, SYSMENU_HANDL_INA226_PWR_MON},				  // Tisho
		{"INA226_Cur_Calc(mA/Bit)", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.INA226_CurCalc, SYSMENU_HANDL_INA226_CUR_CALL}, // Tisho
#endif
#if defined(FRONTPANEL_X1) || defined(FRONTPANEL_LITE)
		{"PWR VLT Calibr", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.PWR_VLT_Calibration, SYSMENU_HANDL_CALIB_PWR_VLT_Calibration},
#endif
		#if defined(FRONTPANEL_X1)
		{"PWR CUR Calibr", SYSMENU_FLOAT32, NULL, (uint32_t *)&CALIBRATE.PWR_CUR_Calibration, SYSMENU_HANDL_CALIB_PWR_CUR_Calibration},
#endif
		{"ATU Averaging", SYSMENU_UINT8, SYSMENU_HANDL_CHECK_HAS_ATU, (uint32_t *)&CALIBRATE.ATU_AVERAGING, SYSMENU_HANDL_CALIB_ATU_AVERAGING},
		{"CAT Type", SYSMENU_ENUM, NULL, (uint32_t *)&CALIBRATE.CAT_Type, SYSMENU_HANDL_CALIB_CAT_Type, {"FT-450", "TS2000"}},
#if !defined(FRONTPANEL_LITE)
		{"LNA Compensation", SYSMENU_INT8, NULL, (uint32_t *)&CALIBRATE.LNA_compensation, SYSMENU_HANDL_CALIB_LNA_compensation},
#endif
		{"TSignal Balance", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.TwoSignalTune_Balance, SYSMENU_HANDL_CALIB_TwoSignalTune_Balance},
		{"Linear Pwr Control", SYSMENU_BOOLEAN, NULL, (uint32_t *)&CALIBRATE.LinearPowerControl, SYSMENU_HANDL_CALIB_LinearPowerControl},
#ifdef TOUCHPAD_GT911
		{"Flash GT911", SYSMENU_RUN, NULL, NULL, SYSMENU_HANDL_CALIB_FlashGT911},
#endif
		{"IF Gain MIN", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.IF_GAIN_MIN, SYSMENU_HANDL_CALIB_IF_GAIN_MIN},
		{"IF Gain MAX", SYSMENU_UINT8, NULL, (uint32_t *)&CALIBRATE.IF_GAIN_MAX, SYSMENU_HANDL_CALIB_IF_GAIN_MAX},
		{"Settings reset", SYSMENU_RUN, NULL, NULL, SYSMENU_HANDL_CALIB_SETTINGS_RESET},
		{"Calibrate reset", SYSMENU_RUN, NULL, NULL, SYSMENU_HANDL_CALIB_CALIBRATION_RESET},
		{"WiFi settings reset", SYSMENU_RUN, NULL, NULL, SYSMENU_HANDL_CALIB_WIFI_RESET},
};

const static struct sysmenu_item_handler sysmenu_swr_analyser_handlers[] =
	{
		{"Band SWR", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SWR_BAND_START},
		{"HF SWR", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SWR_HF_START},
		{"Custom SWR", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SWR_CUSTOM_Start},
		{"Custom Begin, kHz", SYSMENU_UINT32, NULL, (uint32_t *)&TRX.SWR_CUSTOM_Begin, SYSMENU_HANDL_SWR_CUSTOM_Begin},
		{"Custom End, kHz", SYSMENU_UINT32, NULL, (uint32_t *)&TRX.SWR_CUSTOM_End, SYSMENU_HANDL_SWR_CUSTOM_End},
};

const static struct sysmenu_item_handler sysmenu_spectrum_handlers[] =
	{
		{"Spectrum START", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SPECTRUM_Start},
		{"Begin, mHz", SYSMENU_UINT32, NULL, (uint32_t *)&TRX.SPEC_Begin, SYSMENU_HANDL_SPECTRUM_Begin},
		{"End, mHz", SYSMENU_UINT32, NULL, (uint32_t *)&TRX.SPEC_End, SYSMENU_HANDL_SPECTRUM_End},
		{"Top, dBm", SYSMENU_INT16, NULL, (uint32_t *)&TRX.SPEC_TopDBM, SYSMENU_HANDL_SPECTRUM_TopDBM},
		{"Bottom, dBm", SYSMENU_INT16, NULL, (uint32_t *)&TRX.SPEC_BottomDBM, SYSMENU_HANDL_SPECTRUM_BottomDBM},
};

const static struct sysmenu_item_handler sysmenu_wspr_handlers[] =
	{
		{"WSPR Beacon START", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_WSPR_Start},
		{"Freq offset", SYSMENU_INT16, NULL, (uint32_t *)&TRX.WSPR_FREQ_OFFSET, SYSMENU_HANDL_WSPR_FREQ_OFFSET},
		{"BAND 160m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_160, SYSMENU_HANDL_WSPR_BAND160},
		{"BAND 80m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_80, SYSMENU_HANDL_WSPR_BAND80},
		{"BAND 40m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_40, SYSMENU_HANDL_WSPR_BAND40},
		{"BAND 30m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_30, SYSMENU_HANDL_WSPR_BAND30},
		{"BAND 20m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_20, SYSMENU_HANDL_WSPR_BAND20},
		{"BAND 17m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_17, SYSMENU_HANDL_WSPR_BAND17},
		{"BAND 15m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_15, SYSMENU_HANDL_WSPR_BAND15},
		{"BAND 12m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_12, SYSMENU_HANDL_WSPR_BAND12},
		{"BAND 10m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_10, SYSMENU_HANDL_WSPR_BAND10},
		{"BAND 6m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_6, SYSMENU_HANDL_WSPR_BAND6},
		{"BAND 2m", SYSMENU_BOOLEAN, NULL, (uint32_t *)&TRX.WSPR_BANDS_2, SYSMENU_HANDL_WSPR_BAND2},
		{"", SYSMENU_INFOLINE, 0, 0},
};

const static struct sysmenu_item_handler sysmenu_auto_calibration_handlers[] =
	{
		{"Calibrate SWR", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_AUTO_CALIBRATION_SWR},
		{"Calibrate Power", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_AUTO_CALIBRATION_POWER},
};

const static struct sysmenu_item_handler sysmenu_services_handlers[] =
	{
#if HRDW_HAS_WIFI && !defined(FRONTPANEL_X1)
		{"DX Cluster", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_DX_CLUSTER},
		{"Propagination", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_PROPAGINATION},
		#if LCD_WIDTH >= 800
		{"DayNight Map", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_DAYNIGHT_MAP},
		{"Ionogram", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_IONOGRAM},
		#endif
		{"RDA Statistics", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_RDA_STATS},
#endif
		{"SWR Analyzer", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_SWR_ANALYSER_MENU},
		{"Spectrum Analyzer", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_SPECTRUMMENU},
#ifndef STM32F407xx
		{"WSPR Beacon", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_WSPRMENU},
#endif
#if HRDW_HAS_SD
		{"Record CQ message", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_RECORD_CQ_WAV},
#endif
#if FT8_SUPPORT
		{"FT-8", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_FT8_Decoder}, // Tisho
#endif
#ifdef SWR_AD8307_LOG
		{"SWR Tandem Match Contr.", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SWR_Tandem_Ctrl}, // Tisho
#endif
#ifdef LAY_800x480
		{"Locator info", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_LOCATOR_INFO},
		{"Callsign info", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_CALLSIGN_INFO},
#endif
		{"Self Test", SYSMENU_RUN, NULL, 0, SYSMENU_HANDL_SELF_TEST},
		{"Auto Calibration", SYSMENU_MENU, NULL, 0, SYSMENU_HANDL_AUTO_CALIBRATION},
};

static struct sysmenu_menu_wrapper sysmenu_wrappers[] = {
	{.menu_handler = sysmenu_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_trx_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_audio_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_cw_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_screen_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_decoders_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_adc_handlers, .currentIndex = 0},
	#if HRDW_HAS_WIFI
	{.menu_handler = sysmenu_wifi_handlers, .currentIndex = 0},
	#endif
	#if HRDW_HAS_SD
	{.menu_handler = sysmenu_sd_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_sd_export_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_sd_import_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_sd_format_handlers, .currentIndex = 0},
	#endif
	{.menu_handler = sysmenu_calibration_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_swr_analyser_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_spectrum_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_wspr_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_services_handlers, .currentIndex = 0},
	{.menu_handler = sysmenu_auto_calibration_handlers, .currentIndex = 0},
};

// COMMON MENU
static void drawSystemMenuElement(const struct sysmenu_item_handler *menuElement, bool onlyVal);
#if HRDW_HAS_WIFI
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
#endif
static void SYSMENU_TRX_DrawCallsignMenu(bool full_redraw);
static void SYSMENU_TRX_RotateCallsignChar(int8_t dir);
static void SYSMENU_TRX_DrawLocatorMenu(bool full_redraw);
static void SYSMENU_TRX_RotateLocatorChar(int8_t dir);
static void SYSMENU_TRX_DrawURSICodeMenu(bool full_redraw);
static void SYSMENU_TRX_RotateURSICodeChar(int8_t dir);
static uint8_t SYSTMENU_getVisibleIdFromReal(uint8_t realIndex);
static uint8_t SYSTMENU_getPageFromRealIndex(uint8_t realIndex);
static void setCurrentMenuIndex(uint8_t index);
static uint8_t getCurrentMenuIndex(void);
static uint16_t getIndexByName(const struct sysmenu_item_handler *menu, uint16_t menu_length, char* name);
	
static const struct sysmenu_item_handler *sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_handlers[0];
static uint8_t sysmenu_item_count = sizeof(sysmenu_handlers) / sizeof(sysmenu_handlers[0]);
static uint16_t sysmenu_y = 5;
static uint8_t sysmenu_i = 0;
static bool sysmenu_onroot = true;
bool SYSMENU_hiddenmenu_enabled = false;
static bool sysmenu_services_opened = false;
static bool sysmenu_infowindow_opened = false;
static bool sysmenu_sysinfo_opened = false;
static bool sysmenu_filemanager_opened = false;
static bool sysmenu_item_selected_by_enc2 = false;
bool sysmenu_ota_opened = false;
uint8_t sysmenu_ota_opened_state = 0;
bool SYSMENU_FT8_DECODER_opened = false;

// WIFI
static bool sysmenu_wifi_needupdate_ap = true;
bool sysmenu_wifi_selectap1_menu_opened = false;
bool sysmenu_wifi_selectap2_menu_opened = false;
bool sysmenu_wifi_selectap3_menu_opened = false;
static bool sysmenu_wifi_setAP1password_menu_opened = false;
static bool sysmenu_wifi_setAP2password_menu_opened = false;
static bool sysmenu_wifi_setAP3password_menu_opened = false;
static bool sysmenu_trx_setCallsign_menu_opened = false;
static bool sysmenu_trx_setLocator_menu_opened = false;
static bool sysmenu_trx_setURSICode_menu_opened = false;
static uint8_t sysmenu_wifi_selected_ap_index = 0;
static uint8_t sysmenu_wifi_selected_ap_password_char_index = 0;
static uint8_t sysmenu_trx_selected_callsign_char_index = 0;
static uint8_t sysmenu_trx_selected_locator_char_index = 0;
static uint8_t sysmenu_trx_selected_ursi_code_char_index = 0;

// Time menu
static bool sysmenu_timeMenuOpened = false;
static uint8_t TimeMenuSelection = 0;

// TRX MENU

static void SYSMENU_HANDL_TRXMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_trx_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_trx_handlers) / sizeof(sysmenu_trx_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void SYSMENU_TRX_RFPOWER_HOTKEY(void)
{
	SYSMENU_HANDL_TRXMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "RF Power");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_TRX_STEP_HOTKEY(void)
{
	SYSMENU_HANDL_TRXMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "Freq Step");
	if (TRX.Fast)
		index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "Freq Step FAST");
	
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

static void SYSMENU_HANDL_TRX_BandMap(int8_t direction)
{
	if (direction > 0)
		TRX.BandMapEnabled = true;
	if (direction < 0)
		TRX.BandMapEnabled = false;
}

static void SYSMENU_HANDL_TRX_ChannelMode(int8_t direction)
{
	if (direction > 0)
		TRX.ChannelMode = true;
	if (direction < 0)
		TRX.ChannelMode = false;
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

static void SYSMENU_HANDL_TRX_FineRITTune(int8_t direction)
{
	if (direction > 0)
		TRX.FineRITTune = true;
	if (direction < 0)
		TRX.FineRITTune = false;
}

static void SYSMENU_HANDL_TRX_RFPower(int8_t direction)
{
	if (direction > 0 || TRX.RF_Power > 0)
		TRX.RF_Power += direction;
	if (TRX.RF_Power > 100)
		TRX.RF_Power = 100;

	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_TRX_Auto_Input_Switch(int8_t direction)
{
	if (direction > 0)
		TRX.Auto_Input_Switch = true;
	if (direction < 0)
		TRX.Auto_Input_Switch = false;
}

static void SYSMENU_HANDL_TRX_Auto_Snap(int8_t direction)
{
	if (direction > 0)
		TRX.Auto_Snap = true;
	if (direction < 0)
		TRX.Auto_Snap = false;
}

static void SYSMENU_HANDL_TRX_INPUT_TYPE_MAIN(int8_t direction)
{
	if (direction > 0 || TRX.InputType_MAIN > 0)
		TRX.InputType_MAIN += direction;
	if (TRX.InputType_MAIN > 2)
		TRX.InputType_MAIN = 2;
	CODEC_TXRX_mode();
}

static void SYSMENU_HANDL_TRX_INPUT_TYPE_DIGI(int8_t direction)
{
	if (direction > 0 || TRX.InputType_DIGI > 0)
		TRX.InputType_DIGI += direction;
	if (TRX.InputType_DIGI > 2)
		TRX.InputType_DIGI = 2;
	CODEC_TXRX_mode();
}

static void SYSMENU_HANDL_TRX_DEBUG_TYPE(int8_t direction)
{
	if (direction > 0 || TRX.Debug_Type > 0)
		TRX.Debug_Type += direction;
	if (TRX.Debug_Type > 5)
		TRX.Debug_Type = 5;
}

static void SYSMENU_HANDL_TRX_SAMPLERATE_MAIN(int8_t direction)
{
	if (direction > 0 || TRX.SAMPLERATE_MAIN > 0)
		TRX.SAMPLERATE_MAIN += direction;
	if (TRX.SAMPLERATE_MAIN > 3)
		TRX.SAMPLERATE_MAIN = 3;
	
	#ifdef STM32F407xx
	if (TRX.SAMPLERATE_MAIN > 1)
		TRX.SAMPLERATE_MAIN = 1;
	#endif
	
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	TRX.BANDS_SAVED_SETTINGS[band].SAMPLERATE = TRX.SAMPLERATE_MAIN;

	FFT_Init();
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_TRX_SAMPLERATE_FM(int8_t direction)
{
	if (direction > 0 || TRX.SAMPLERATE_FM > 0)
		TRX.SAMPLERATE_FM += direction;
	if (TRX.SAMPLERATE_FM > 3)
		TRX.SAMPLERATE_FM = 3;

	#ifdef STM32F407xx
	if (TRX.SAMPLERATE_FM > 1)
		TRX.SAMPLERATE_FM = 1;
	#endif
	
	FFT_Init();
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_TRX_RIT_INTERVAL(int8_t direction)
{
	TRX.RIT_INTERVAL += direction * 100;
	if (TRX.RIT_INTERVAL < 100)
		TRX.RIT_INTERVAL = 100;
	if (TRX.RIT_INTERVAL > 10000)
		TRX.RIT_INTERVAL = 10000;
}

static void SYSMENU_HANDL_TRX_XIT_INTERVAL(int8_t direction)
{
	TRX.XIT_INTERVAL += direction * 100;
	if (TRX.XIT_INTERVAL < 100)
		TRX.XIT_INTERVAL = 100;
	if (TRX.XIT_INTERVAL > 10000)
		TRX.XIT_INTERVAL = 10000;
}

static const uint32_t freq_steps[] = {1, 10, 25, 50, 100, 500, 1000, 2500, 5000, 25000, 50000, 100000, 250000, 500000};
static void SYSMENU_HANDL_TRX_FRQ_STEP(int8_t direction)
{
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

static void SYSMENU_HANDL_TRX_FRQ_ENC_WFM_STEP_KHZ(int8_t direction)
{
	const uint32_t wfm_freq_steps[] = {1, 2, 5, 10, 20, 25, 50, 100, 200, 500, 1000};
	
	for (uint8_t i = 0; i < ARRLENTH(wfm_freq_steps); i++)
		if (TRX.FRQ_ENC_WFM_STEP_KHZ == wfm_freq_steps[i])
		{
			if (direction < 0)
			{
				if (i > 0)
					TRX.FRQ_ENC_WFM_STEP_KHZ = wfm_freq_steps[i - 1];
				else
					TRX.FRQ_ENC_WFM_STEP_KHZ = wfm_freq_steps[0];
				return;
			}
			else
			{
				if (i < (ARRLENTH(wfm_freq_steps) - 1))
					TRX.FRQ_ENC_WFM_STEP_KHZ = wfm_freq_steps[i + 1];
				else
					TRX.FRQ_ENC_WFM_STEP_KHZ = wfm_freq_steps[ARRLENTH(wfm_freq_steps) - 1];
				return;
			}
		}
	TRX.FRQ_ENC_WFM_STEP_KHZ = wfm_freq_steps[0];
}

static void SYSMENU_HANDL_TRX_FRQ_CW_STEP_DIVIDER(int8_t direction)
{
	TRX.FRQ_CW_STEP_DIVIDER += direction;
	if (TRX.FRQ_CW_STEP_DIVIDER < 1)
		TRX.FRQ_CW_STEP_DIVIDER = 1;
	if (TRX.FRQ_CW_STEP_DIVIDER > 100)
		TRX.FRQ_CW_STEP_DIVIDER = 100;
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

static void SYSMENU_HANDL_TRX_ATT_DB(int8_t direction)
{
	TRX.ATT_DB += (float32_t)direction * 0.5f;
	if (TRX.ATT_DB < 0.5f)
		TRX.ATT_DB = 0.5f;
	if (TRX.ATT_DB > 31.5f)
		TRX.ATT_DB = 31.5f;
}

#if HRDW_HAS_VGA
static void SYSMENU_HANDL_TRX_VGA_GAIN(int8_t direction)
{
	TRX.VGA_GAIN += (float32_t)direction * 1.5f;
	if (TRX.VGA_GAIN < 10.5f)
		TRX.VGA_GAIN = 10.5f;
	if (TRX.VGA_GAIN > 33.0f)
		TRX.VGA_GAIN = 33.0f;
}
#endif

static void SYSMENU_TRX_Callsign_keyboardHandler(uint32_t parameter)
{
	if(parameter == '<') //backspace
	{
		TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] = 0;
		
		if (sysmenu_trx_selected_callsign_char_index > 0)
			sysmenu_trx_selected_callsign_char_index--;
	} else {
		TRX.CALLSIGN[sysmenu_trx_selected_callsign_char_index] = parameter;
		
		if (sysmenu_trx_selected_callsign_char_index < (MAX_CALLSIGN_LENGTH - 1))
			sysmenu_trx_selected_callsign_char_index++;
	}
	
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_TRX_DrawCallsignMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("CALLSIGN:", 5, 5, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	}

	LCDDriver_printText(TRX.CALLSIGN, 10, 37, COLOR_GREEN, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	LCDDriver_drawFastHLine(8 + sysmenu_trx_selected_callsign_char_index * RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, interactive_menu_top, RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, COLOR_RED);

	#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	LCD_printKeyboard(SYSMENU_TRX_Callsign_keyboardHandler, false);
	#endif
}

static void SYSMENU_TRX_Locator_keyboardHandler(uint32_t parameter)
{
	if(parameter == '<') //backspace
	{
		TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] = 0;
		
		if (sysmenu_trx_selected_locator_char_index > 0)
			sysmenu_trx_selected_locator_char_index--;
	} else {
		TRX.LOCATOR[sysmenu_trx_selected_locator_char_index] = parameter;
		
		if (sysmenu_trx_selected_locator_char_index < (MAX_CALLSIGN_LENGTH - 1))
			sysmenu_trx_selected_locator_char_index++;
	}
	
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_TRX_DrawLocatorMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("LOCATOR:", 5, 5, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	}

	LCDDriver_printText(TRX.LOCATOR, 10, 37, COLOR_GREEN, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	LCDDriver_drawFastHLine(8 + sysmenu_trx_selected_locator_char_index * RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, interactive_menu_top, RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, COLOR_RED);

	#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	LCD_printKeyboard(SYSMENU_TRX_Locator_keyboardHandler, false);
	#endif
}

static void SYSMENU_TRX_URSICode_keyboardHandler(uint32_t parameter)
{
	if(parameter == '<') //backspace
	{
		TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] = 0;
		
		if (sysmenu_trx_selected_ursi_code_char_index > 0)
			sysmenu_trx_selected_ursi_code_char_index--;
	} else {
		TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] = parameter;
		
		if (sysmenu_trx_selected_ursi_code_char_index < (MAX_CALLSIGN_LENGTH - 1))
			sysmenu_trx_selected_ursi_code_char_index++;
	}
	
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_TRX_DrawURSICodeMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("URSI Code:", 5, 5, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	}

	LCDDriver_printText(TRX.URSI_CODE, 10, 37, COLOR_GREEN, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	LCDDriver_drawFastHLine(8 + sysmenu_trx_selected_ursi_code_char_index * RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, interactive_menu_top, RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, COLOR_RED);

	#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	LCD_printKeyboard(SYSMENU_TRX_URSICode_keyboardHandler, false);
	#endif
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

static void SYSMENU_TRX_RotateURSICodeChar(int8_t dir)
{
	bool full_redraw = false;
	if (TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] == 0)
		full_redraw = true;
	TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] += dir;

	// do not show special characters
	if (TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] >= 1 && TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] <= 32 && dir > 0)
		TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] = 33;
	if (TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] >= 1 && TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] <= 32 && dir < 0)
		TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] = 0;
	if (TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] >= 127)
		TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] = 0;
	if (TRX.URSI_CODE[sysmenu_trx_selected_ursi_code_char_index] == 0)
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

static void SYSMENU_HANDL_TRX_SetURSICode(int8_t direction)
{
#pragma unused(direction)
	sysmenu_trx_setURSICode_menu_opened = true;
	SYSMENU_TRX_DrawURSICodeMenu(true);
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_TRX_TRANSV_ENABLE(int8_t direction)
{
	if (direction > 0)
		TRX.Custom_Transverter_Enabled = true;
	if (direction < 0)
		TRX.Custom_Transverter_Enabled = false;
}

static void SYSMENU_HANDL_TRX_TRANSV_OFFSET(int8_t direction)
{
	TRX.Transverter_Offset_Mhz += direction;
	if (TRX.Transverter_Offset_Mhz < 1)
		TRX.Transverter_Offset_Mhz = 1;
	if (TRX.Transverter_Offset_Mhz > 15000)
		TRX.Transverter_Offset_Mhz = 15000;
}

static void SYSMENU_HANDL_TRX_TRANSV_70CM(int8_t direction)
{
	if (direction > 0)
		TRX.Transverter_70cm = true;
	if (direction < 0)
		TRX.Transverter_70cm = false;
}

static void SYSMENU_HANDL_TRX_TRANSV_23CM(int8_t direction)
{
	if (direction > 0)
		TRX.Transverter_23cm = true;
	if (direction < 0)
		TRX.Transverter_23cm = false;

	BANDS[BANDID_23cm].selectable = TRX.Transverter_23cm;
}

static void SYSMENU_HANDL_TRX_TRANSV_13CM(int8_t direction)
{
	if (direction > 0)
		TRX.Transverter_13cm = true;
	if (direction < 0)
		TRX.Transverter_13cm = false;

	BANDS[BANDID_13cm].selectable = TRX.Transverter_13cm;
}

static void SYSMENU_HANDL_TRX_TRANSV_6CM(int8_t direction)
{
	if (direction > 0)
		TRX.Transverter_6cm = true;
	if (direction < 0)
		TRX.Transverter_6cm = false;

	BANDS[BANDID_6cm].selectable = TRX.Transverter_6cm;
}

static void SYSMENU_HANDL_TRX_TRANSV_3CM(int8_t direction)
{
	if (direction > 0)
		TRX.Transverter_3cm = true;
	if (direction < 0)
		TRX.Transverter_3cm = false;

	BANDS[BANDID_3cm].selectable = TRX.Transverter_3cm;
}

static void SYSMENU_HANDL_TRX_ATU_I(int8_t direction)
{
	if (TRX.ATU_I > 0 || direction > 0)
		TRX.ATU_I += direction;
	if (TRX.ATU_I > ATU_MAXPOS)
		TRX.ATU_I = ATU_MAXPOS;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_I = TRX.ATU_I;
		TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_C = TRX.ATU_C;
		TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_T = TRX.ATU_T;
	}
}

static void SYSMENU_HANDL_TRX_ATU_C(int8_t direction)
{
	if (TRX.ATU_C > 0 || direction > 0)
		TRX.ATU_C += direction;
	if (TRX.ATU_C > ATU_MAXPOS)
		TRX.ATU_C = ATU_MAXPOS;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_I = TRX.ATU_I;
		TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_C = TRX.ATU_C;
		TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_T = TRX.ATU_T;
	}
}

static void SYSMENU_HANDL_TRX_ATU_T(int8_t direction)
{
	if (direction > 0)
		TRX.ATU_T = true;
	if (direction < 0)
		TRX.ATU_T = false;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
	{
		TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_I = TRX.ATU_I;
		TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_C = TRX.ATU_C;
		TRX.BANDS_SAVED_SETTINGS[band].BEST_ATU_T = TRX.ATU_T;
	}
}

static void SYSMENU_HANDL_TRX_ATU_Enabled(int8_t direction)
{
	if (direction > 0)
		TRX.ATU_Enabled = true;
	if (direction < 0)
		TRX.ATU_Enabled = false;
}

static void SYSMENU_HANDL_TRX_TUNER_Enabled(int8_t direction)
{
	if (direction > 0)
		TRX.TUNER_Enabled = true;
	if (direction < 0)
		TRX.TUNER_Enabled = false;
}

// AUDIO MENU

static void SYSMENU_HANDL_AUDIOMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_audio_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_audio_handlers) / sizeof(sysmenu_audio_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void SYSMENU_AUDIO_BW_SSB_HOTKEY(void)
{
	SYSMENU_HANDL_AUDIOMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "SSB LPF RX Pass");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_AUDIO_IF_HOTKEY(void)
{
	SYSMENU_HANDL_AUDIOMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "IF Gain, dB");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_CW_HOTKEY(void)
{
	SYSMENU_HANDL_AUDIOMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "CW LPF Pass");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_AM_HOTKEY(void)
{
	SYSMENU_HANDL_AUDIOMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "AM LPF RX Pass");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_AUDIO_BW_FM_HOTKEY(void)
{
	SYSMENU_HANDL_AUDIOMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "FM LPF RX Pass");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_AUDIO_HPF_SSB_HOTKEY(void)
{
	SYSMENU_HANDL_AUDIOMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "SSB HPF RX Pass");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_AUDIO_SQUELCH_HOTKEY(void)
{
	SYSMENU_HANDL_AUDIOMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "FM Squelch level, dbm");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_AUDIO_DNR_HOTKEY(void)
{
	SYSMENU_HANDL_AUDIOMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "DNR1 Threshold");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_AUDIO_AGC_HOTKEY(void)
{
	SYSMENU_HANDL_AUDIOMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "AGC");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

static void SYSMENU_HANDL_AUDIO_Volume(int8_t direction)
{
	if (direction > 0 || TRX.Volume > 0)
		TRX.Volume += direction;
	if (TRX.Volume > 100)
		TRX.Volume = 100;
}

static void SYSMENU_HANDL_AUDIO_Volume_Step(int8_t direction)
{
	if (direction > 0 || TRX.Volume_Step > 1)
		TRX.Volume_Step += direction;
	if (TRX.Volume_Step > 25)
		TRX.Volume_Step = 25;
}

static void SYSMENU_HANDL_AUDIO_DNR(int8_t direction)
{
	TRX_TemporaryMute();

	if (direction > 0 || CurrentVFO->DNR_Type > 0)
		CurrentVFO->DNR_Type += direction;
	if (CurrentVFO->DNR_Type > 2)
		CurrentVFO->DNR_Type = 2;

	TRX.DNR_shadow = CurrentVFO->DNR_Type;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].DNR_Type = CurrentVFO->DNR_Type;
}

static void SYSMENU_HANDL_AUDIO_AGC(int8_t direction)
{
	if (direction > 0)
		CurrentVFO->AGC = true;
	if (direction < 0)
		CurrentVFO->AGC = false;

	TRX.AGC_shadow = CurrentVFO->AGC;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].AGC = CurrentVFO->AGC;
}

static void SYSMENU_HANDL_AUDIO_NOISE_BLANKER(int8_t direction)
{
	if (direction > 0)
		TRX.NOISE_BLANKER = true;
	if (direction < 0)
		TRX.NOISE_BLANKER = false;
}

static void SYSMENU_HANDL_AUDIO_IFGain(int8_t direction)
{
	TRX.IF_Gain += direction;
	if (TRX.IF_Gain < CALIBRATE.IF_GAIN_MIN)
		TRX.IF_Gain = CALIBRATE.IF_GAIN_MIN;
	if (TRX.IF_Gain > CALIBRATE.IF_GAIN_MAX)
		TRX.IF_Gain = CALIBRATE.IF_GAIN_MAX;
	
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band > 0)
		TRX.BANDS_SAVED_SETTINGS[band].IF_Gain = TRX.IF_Gain;
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
	TRX.MIC_GAIN_DB += direction * 0.1f;
	if (TRX.MIC_GAIN_DB < 1.0f)
		TRX.MIC_GAIN_DB = 1.0f;
	if (TRX.MIC_GAIN_DB > 20.0f)
		TRX.MIC_GAIN_DB = 20.0f;
}

static void SYSMENU_HANDL_AUDIO_MIC_Boost(int8_t direction)
{
	if (direction < 0)
		TRX.MIC_Boost = false;
	if (direction > 0)
		TRX.MIC_Boost = true;

	// reinit codec
	CODEC_TXRX_mode();
}

static void SYSMENU_HANDL_AUDIO_DNR1_THRES(int8_t direction)
{
	TRX.DNR1_SNR_THRESHOLD += direction;
	if (TRX.DNR1_SNR_THRESHOLD < 1)
		TRX.DNR1_SNR_THRESHOLD = 1;
	if (TRX.DNR1_SNR_THRESHOLD > 100)
		TRX.DNR1_SNR_THRESHOLD = 100;
}

static void SYSMENU_HANDL_AUDIO_DNR2_THRES(int8_t direction)
{
	TRX.DNR2_SNR_THRESHOLD += direction;
	if (TRX.DNR2_SNR_THRESHOLD < 1)
		TRX.DNR2_SNR_THRESHOLD = 1;
	if (TRX.DNR2_SNR_THRESHOLD > 100)
		TRX.DNR2_SNR_THRESHOLD = 100;
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

static void SYSMENU_HANDL_AUDIO_MIC_EQ_LOW_SSB(int8_t direction)
{
	TRX.MIC_EQ_LOW_SSB += direction;
	if (TRX.MIC_EQ_LOW_SSB < -50)
		TRX.MIC_EQ_LOW_SSB = -50;
	if (TRX.MIC_EQ_LOW_SSB > 50)
		TRX.MIC_EQ_LOW_SSB = 50;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_LOW_AMFM(int8_t direction)
{
	TRX.MIC_EQ_LOW_AMFM += direction;
	if (TRX.MIC_EQ_LOW_AMFM < -50)
		TRX.MIC_EQ_LOW_AMFM = -50;
	if (TRX.MIC_EQ_LOW_AMFM > 50)
		TRX.MIC_EQ_LOW_AMFM = 50;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_MID_SSB(int8_t direction)
{
	TRX.MIC_EQ_MID_SSB += direction;
	if (TRX.MIC_EQ_MID_SSB < -50)
		TRX.MIC_EQ_MID_SSB = -50;
	if (TRX.MIC_EQ_MID_SSB > 50)
		TRX.MIC_EQ_MID_SSB = 50;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_MID_AMFM(int8_t direction)
{
	TRX.MIC_EQ_MID_AMFM += direction;
	if (TRX.MIC_EQ_MID_AMFM < -50)
		TRX.MIC_EQ_MID_AMFM = -50;
	if (TRX.MIC_EQ_MID_AMFM > 50)
		TRX.MIC_EQ_MID_AMFM = 50;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_HIG_SSB(int8_t direction)
{
	TRX.MIC_EQ_HIG_SSB += direction;
	if (TRX.MIC_EQ_HIG_SSB < -50)
		TRX.MIC_EQ_HIG_SSB = -50;
	if (TRX.MIC_EQ_HIG_SSB > 50)
		TRX.MIC_EQ_HIG_SSB = 50;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_MIC_EQ_HIG_AMFM(int8_t direction)
{
	TRX.MIC_EQ_HIG_AMFM += direction;
	if (TRX.MIC_EQ_HIG_AMFM < -50)
		TRX.MIC_EQ_HIG_AMFM = -50;
	if (TRX.MIC_EQ_HIG_AMFM > 50)
		TRX.MIC_EQ_HIG_AMFM = 50;
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

static void SYSMENU_HANDL_AUDIO_MIC_NOISE_GATE(int8_t direction)
{
	TRX.MIC_NOISE_GATE += direction;
	if (TRX.MIC_NOISE_GATE < -120)
		TRX.MIC_NOISE_GATE = -120;
	if (TRX.MIC_NOISE_GATE > 0)
		TRX.MIC_NOISE_GATE = 0;
}

static void SYSMENU_HANDL_AUDIO_RX_EQ_LOW(int8_t direction)
{
	TRX.RX_EQ_LOW += direction;
	if (TRX.RX_EQ_LOW < -50)
		TRX.RX_EQ_LOW = -50;
	if (TRX.RX_EQ_LOW > 50)
		TRX.RX_EQ_LOW = 50;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_RX_EQ_MID(int8_t direction)
{
	TRX.RX_EQ_MID += direction;
	if (TRX.RX_EQ_MID < -50)
		TRX.RX_EQ_MID = -50;
	if (TRX.RX_EQ_MID > 50)
		TRX.RX_EQ_MID = 50;
	NeedReinitAudioFilters = true;
}

static void SYSMENU_HANDL_AUDIO_RX_EQ_HIG(int8_t direction)
{
	TRX.RX_EQ_HIG += direction;
	if (TRX.RX_EQ_HIG < -50)
		TRX.RX_EQ_HIG = -50;
	if (TRX.RX_EQ_HIG > 50)
		TRX.RX_EQ_HIG = 50;
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

static void SYSMENU_HANDL_AUDIO_RX_AGC_Max_gain(int8_t direction)
{
	TRX.RX_AGC_Max_gain += direction;
	if (TRX.RX_AGC_Max_gain < 1)
		TRX.RX_AGC_Max_gain = 1;
	if (TRX.RX_AGC_Max_gain > 50)
		TRX.RX_AGC_Max_gain = 50;
}

static void SYSMENU_HANDL_AUDIO_RX_AGC_Hold(int8_t direction)
{
	if (TRX.RX_AGC_Hold > 0 || direction > 0)
		TRX.RX_AGC_Hold += direction * 100;
	if (TRX.RX_AGC_Hold > 5000)
		TRX.RX_AGC_Hold = 5000;
}

static void SYSMENU_HANDL_AUDIO_TX_CompressorSpeed_SSB(int8_t direction)
{
	TRX.TX_Compressor_speed_SSB += direction;
	if (TRX.TX_Compressor_speed_SSB < 1)
		TRX.TX_Compressor_speed_SSB = 1;
	if (TRX.TX_Compressor_speed_SSB > 200)
		TRX.TX_Compressor_speed_SSB = 200;
}

static void SYSMENU_HANDL_AUDIO_TX_CompressorSpeed_AMFM(int8_t direction)
{
	TRX.TX_Compressor_speed_AMFM += direction;
	if (TRX.TX_Compressor_speed_AMFM < 1)
		TRX.TX_Compressor_speed_AMFM = 1;
	if (TRX.TX_Compressor_speed_AMFM > 200)
		TRX.TX_Compressor_speed_AMFM = 200;
}

static void SYSMENU_HANDL_AUDIO_TX_CompressorMaxGain_SSB(int8_t direction)
{
	TRX.TX_Compressor_maxgain_SSB += direction;
	if (TRX.TX_Compressor_maxgain_SSB < 1)
		TRX.TX_Compressor_maxgain_SSB = 1;
	if (TRX.TX_Compressor_maxgain_SSB > 30)
		TRX.TX_Compressor_maxgain_SSB = 30;
}

static void SYSMENU_HANDL_AUDIO_TX_CompressorMaxGain_AMFM(int8_t direction)
{
	TRX.TX_Compressor_maxgain_AMFM += direction;
	if (TRX.TX_Compressor_maxgain_AMFM < 1)
		TRX.TX_Compressor_maxgain_AMFM = 1;
	if (TRX.TX_Compressor_maxgain_AMFM > 30)
		TRX.TX_Compressor_maxgain_AMFM = 30;
}

static void SYSMENU_HANDL_AUDIO_FMSquelch(int8_t direction)
{
	CurrentVFO->FM_SQL_threshold_dbm += direction;
	TRX.FM_SQL_threshold_dbm_shadow = CurrentVFO->FM_SQL_threshold_dbm;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].FM_SQL_threshold_dbm = CurrentVFO->FM_SQL_threshold_dbm;
}

static void SYSMENU_HANDL_AUDIO_Squelch(int8_t direction)
{
	if (direction > 0)
		CurrentVFO->SQL = true;
	if (direction < 0)
		CurrentVFO->SQL = false;
	TRX.SQL_shadow = CurrentVFO->SQL;

	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].SQL = CurrentVFO->SQL;
}

void SYSMENU_HANDL_AUDIO_SSB_HPF_RX_pass(int8_t direction)
{
	if (TRX.SSB_HPF_RX_Filter > 0 || direction > 0)
		TRX.SSB_HPF_RX_Filter += direction * 50;
	if (TRX.SSB_HPF_RX_Filter > MAX_HPF_WIDTH)
		TRX.SSB_HPF_RX_Filter = MAX_HPF_WIDTH;
	
	if (TRX.SSB_HPF_RX_Filter > TRX.SSB_LPF_RX_Filter)
		TRX.SSB_HPF_RX_Filter = TRX.SSB_LPF_RX_Filter;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

void SYSMENU_HANDL_AUDIO_SSB_HPF_TX_pass(int8_t direction)
{
	if (TRX.SSB_HPF_TX_Filter > 0 || direction > 0)
		TRX.SSB_HPF_TX_Filter += direction * 50;
	if (TRX.SSB_HPF_TX_Filter > MAX_HPF_WIDTH)
		TRX.SSB_HPF_TX_Filter = MAX_HPF_WIDTH;
	
	if (TRX.SSB_HPF_TX_Filter > TRX.SSB_LPF_TX_Filter)
		TRX.SSB_HPF_TX_Filter = TRX.SSB_LPF_TX_Filter;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

void SYSMENU_HANDL_AUDIO_CW_LPF_pass(int8_t direction)
{
	if (TRX.CW_LPF_Filter > 50 || direction > 0)
		TRX.CW_LPF_Filter += direction * 50;
	if (TRX.CW_LPF_Filter > MAX_LPF_WIDTH_CW)
		TRX.CW_LPF_Filter = MAX_LPF_WIDTH_CW;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

void SYSMENU_HANDL_AUDIO_DIGI_LPF_pass(int8_t direction)
{
	if (TRX.DIGI_LPF_Filter > 50 || direction > 0)
		TRX.DIGI_LPF_Filter += direction * 50;
	if (TRX.DIGI_LPF_Filter > MAX_LPF_WIDTH_SSB)
		TRX.DIGI_LPF_Filter = MAX_LPF_WIDTH_SSB;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

void SYSMENU_HANDL_AUDIO_SSB_LPF_RX_pass(int8_t direction)
{
	if (TRX.SSB_LPF_RX_Filter > 100 || direction > 0)
		TRX.SSB_LPF_RX_Filter += direction * 100;
	if (TRX.SSB_LPF_RX_Filter > MAX_LPF_WIDTH_SSB)
		TRX.SSB_LPF_RX_Filter = MAX_LPF_WIDTH_SSB;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

void SYSMENU_HANDL_AUDIO_SSB_LPF_TX_pass(int8_t direction)
{
	if (TRX.SSB_LPF_TX_Filter > 100 || direction > 0)
		TRX.SSB_LPF_TX_Filter += direction * 100;
	if (TRX.SSB_LPF_TX_Filter > MAX_LPF_WIDTH_SSB)
		TRX.SSB_LPF_TX_Filter = MAX_LPF_WIDTH_SSB;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

void SYSMENU_HANDL_AUDIO_AM_LPF_RX_pass(int8_t direction)
{
	if (TRX.AM_LPF_RX_Filter > 100 || direction > 0)
		TRX.AM_LPF_RX_Filter += direction * 100;
	if (TRX.AM_LPF_RX_Filter > MAX_LPF_WIDTH_AM)
		TRX.AM_LPF_RX_Filter = MAX_LPF_WIDTH_AM;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

void SYSMENU_HANDL_AUDIO_AM_LPF_TX_pass(int8_t direction)
{
	if (TRX.AM_LPF_TX_Filter > 100 || direction > 0)
		TRX.AM_LPF_TX_Filter += direction * 100;
	if (TRX.AM_LPF_TX_Filter > MAX_LPF_WIDTH_AM)
		TRX.AM_LPF_TX_Filter = MAX_LPF_WIDTH_AM;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

void SYSMENU_HANDL_AUDIO_FM_LPF_RX_pass(int8_t direction)
{
	if (TRX.FM_LPF_RX_Filter > 1000 || direction > 0)
		TRX.FM_LPF_RX_Filter += direction * 1000;
	if (TRX.FM_LPF_RX_Filter > MAX_LPF_WIDTH_NFM)
		TRX.FM_LPF_RX_Filter = MAX_LPF_WIDTH_NFM;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

void SYSMENU_HANDL_AUDIO_FM_LPF_TX_pass(int8_t direction)
{
	if (TRX.FM_LPF_TX_Filter > 1000 || direction > 0)
		TRX.FM_LPF_TX_Filter += direction * 1000;
	if (TRX.FM_LPF_TX_Filter > MAX_LPF_WIDTH_NFM)
		TRX.FM_LPF_TX_Filter = MAX_LPF_WIDTH_NFM;

	TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	TRX_setMode(CurrentVFO->Mode, CurrentVFO);
}

static void SYSMENU_HANDL_AUDIO_Beeper(int8_t direction)
{
	if (direction > 0)
		TRX.Beeper = true;
	if (direction < 0)
		TRX.Beeper = false;
}

static void SYSMENU_HANDL_AUDIO_CTCSS_Freq(int8_t direction)
{
	uint16_t current_pos = 0;
	for (uint16_t i = 0; i < CTCSS_FREQS_COUNT; i++)
		if (CTCSS_Freqs[i] == TRX.CTCSS_Freq)
			current_pos = i;
	if (direction > 0)
		current_pos++;
	if (direction < 0 && current_pos > 0)
		current_pos--;
	if (current_pos >= CTCSS_FREQS_COUNT)
		current_pos = CTCSS_FREQS_COUNT - 1;
	TRX.CTCSS_Freq = CTCSS_Freqs[current_pos];
}

static void SYSMENU_HANDL_AUDIO_SELFHEAR_Volume(int8_t direction)
{
	TRX.SELFHEAR_Volume += direction;
	if (TRX.SELFHEAR_Volume > 100)
		TRX.SELFHEAR_Volume = 100;
	if (TRX.SELFHEAR_Volume < 1)
		TRX.SELFHEAR_Volume = 1;
}

static void SYSMENU_HANDL_AUDIO_FM_Stereo(int8_t direction)
{
	if (direction > 0)
		TRX.FM_Stereo = true;
	if (direction < 0)
		TRX.FM_Stereo = false;
}

static void SYSMENU_HANDL_AUDIO_AGC_Spectral(int8_t direction)
{
	if (direction > 0)
		TRX.AGC_Spectral = true;
	if (direction < 0)
		TRX.AGC_Spectral = false;
}

static void SYSMENU_HANDL_AUDIO_TX_CESSB(int8_t direction)
{
	if (direction > 0)
		TRX.TX_CESSB = true;
	if (direction < 0)
		TRX.TX_CESSB = false;
}

static void SYSMENU_HANDL_AUDIO_TX_CESSB_COMPRESS_DB(int8_t direction)
{
	TRX.TX_CESSB_COMPRESS_DB += direction * 0.1f;
	if (TRX.TX_CESSB_COMPRESS_DB < 1.0f)
		TRX.TX_CESSB_COMPRESS_DB = 1.0f;
	if (TRX.TX_CESSB_COMPRESS_DB > 20.0f)
		TRX.TX_CESSB_COMPRESS_DB = 20.0f;
}

static void SYSMENU_HANDL_AUDIO_VAD_THRESHOLD(int8_t direction)
{
	TRX.VAD_THRESHOLD += direction;
	if (TRX.VAD_THRESHOLD > 200)
		TRX.VAD_THRESHOLD = 200;
	if (TRX.VAD_THRESHOLD < 1)
		TRX.VAD_THRESHOLD = 1;
}

static void SYSMENU_HANDL_AUDIO_VOX(int8_t direction)
{
	if (direction > 0)
		TRX.VOX = true;
	if (direction < 0)
		TRX.VOX = false;
}
static void SYSMENU_HANDL_AUDIO_VOX_TIMEOUT(int8_t direction)
{
	TRX.VOX_TIMEOUT += direction * 50;
	if (TRX.VOX_TIMEOUT > 2000)
		TRX.VOX_TIMEOUT = 2000;
	if (TRX.VOX_TIMEOUT < 50)
		TRX.VOX_TIMEOUT = 50;
}
static void SYSMENU_HANDL_AUDIO_VOX_THRESHOLD(int8_t direction)
{
	TRX.VOX_THRESHOLD += direction;
	if (TRX.VOX_THRESHOLD > 0)
		TRX.VOX_THRESHOLD = 0;
	if (TRX.VOX_THRESHOLD < -120)
		TRX.VOX_THRESHOLD = -120;
}

// CW MENU

static void SYSMENU_HANDL_CWMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_cw_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_cw_handlers) / sizeof(sysmenu_cw_handlers[0]);
	sysmenu_onroot = false;
	LCD_redraw(false);
}

void SYSMENU_CW_WPM_HOTKEY(void)
{
	SYSMENU_HANDL_CWMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "CW Keyer WPM");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

void SYSMENU_CW_KEYER_HOTKEY(void)
{
	SYSMENU_HANDL_CWMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "CW Keyer");
	setCurrentMenuIndex(index);
	LCD_redraw(false);
}

static void SYSMENU_HANDL_CW_Pitch(int8_t direction)
{
	TRX.CW_Pitch += direction * 10;
	if (TRX.CW_Pitch < 50)
		TRX.CW_Pitch = 50;
	if (TRX.CW_Pitch > 10000)
		TRX.CW_Pitch = 10000;
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

static void SYSMENU_HANDL_CW_DotToDashRate(int8_t direction)
{
	TRX.CW_DotToDashRate += 0.01f * direction;
	if (TRX.CW_DotToDashRate < 3.0f)
		TRX.CW_DotToDashRate = 3.0f;
	if (TRX.CW_DotToDashRate > 5.0f)
		TRX.CW_DotToDashRate = 5.0f;
}

static void SYSMENU_HANDL_CW_Iambic(int8_t direction)
{
	if (direction > 0)
		TRX.CW_Iambic = true;
	if (direction < 0)
		TRX.CW_Iambic = false;
}

static void SYSMENU_HANDL_CW_Invert(int8_t direction)
{
	if (direction > 0)
		TRX.CW_Invert = true;
	if (direction < 0)
		TRX.CW_Invert = false;
}

static void SYSMENU_HANDL_CW_PTT_Type(int8_t direction)
{
	if (direction > 0 || TRX.CW_PTT_Type > 0)
		TRX.CW_PTT_Type += direction;
	if (TRX.CW_PTT_Type > 2)
		TRX.CW_PTT_Type = 2;

	KEYER_symbol_status = 0;
}

// SCREEN MENU

static void SYSMENU_HANDL_LCDMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_screen_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_screen_handlers) / sizeof(sysmenu_screen_handlers[0]);
	sysmenu_onroot = false;
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
	if (TRX.FFT_Averaging > (FFT_MAX_MEANS + FFT_MAX_AVER))
		TRX.FFT_Averaging = (FFT_MAX_MEANS + FFT_MAX_AVER);
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
	if (TRX.FFT_Height > 5)
		TRX.FFT_Height = 5;

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

static void SYSMENU_HANDL_SCREEN_FFT_BW_Style(int8_t direction)
{
	TRX.FFT_BW_Style += direction;
	if (TRX.FFT_BW_Style < 1)
		TRX.FFT_BW_Style = 1;
	if (TRX.FFT_BW_Style > 3)
		TRX.FFT_BW_Style = 3;

	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_Color(int8_t direction)
{
	TRX.FFT_Color += direction;
	if (TRX.FFT_Color < 1)
		TRX.FFT_Color = 1;
	if (TRX.FFT_Color > 9)
		TRX.FFT_Color = 9;

	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_WTF_Color(int8_t direction)
{
	TRX.WTF_Color += direction;
	if (TRX.WTF_Color < 1)
		TRX.WTF_Color = 1;
	if (TRX.WTF_Color > 9)
		TRX.WTF_Color = 9;

	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_FreqGrid(int8_t direction)
{
	TRX.FFT_FreqGrid += direction;
	if (TRX.FFT_FreqGrid < 0)
		TRX.FFT_FreqGrid = 0;
	if (TRX.FFT_FreqGrid > 3)
		TRX.FFT_FreqGrid = 3;

	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_FFT_dBmGrid(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_dBmGrid = true;
	if (direction < 0)
		TRX.FFT_dBmGrid = false;

	FFT_Init();
}

static void SYSMENU_HANDL_SCREEN_WTF_Moving(int8_t direction)
{
	if (direction > 0)
		TRX.WTF_Moving = true;
	if (direction < 0)
		TRX.WTF_Moving = false;
}

#if HRDW_HAS_WIFI
static void SYSMENU_HANDL_SCREEN_FFT_DXCluster(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_DXCluster = true;
	if (direction < 0)
		TRX.FFT_DXCluster = false;
}

static void SYSMENU_HANDL_SCREEN_FFT_DXCluster_Azimuth(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_DXCluster_Azimuth = true;
	if (direction < 0)
		TRX.FFT_DXCluster_Azimuth = false;
}

static void SYSMENU_HANDL_SCREEN_FFT_DXCluster_Timeout(int8_t direction)
{
	if (TRX.FFT_DXCluster_Timeout > 0 || direction > 0)
		TRX.FFT_DXCluster_Timeout += direction;
	if (TRX.FFT_DXCluster_Timeout > 30)
		TRX.FFT_DXCluster_Timeout = 30;

	WIFI_DXCLUSTER_list_count = 0;
	TRX_DXCluster_UpdateTime = 0;
}
#endif

static void SYSMENU_HANDL_SCREEN_FFT_Background(int8_t direction)
{
	if (direction > 0)
		TRX.FFT_Background = true;
	if (direction < 0)
		TRX.FFT_Background = false;
	
	FFT_Init();
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

static void SYSMENU_HANDL_SCREEN_LCD_SleepTimeout(int8_t direction)
{
	if (TRX.LCD_SleepTimeout > 0 || direction > 0)
		TRX.LCD_SleepTimeout += direction;
	if (TRX.LCD_SleepTimeout > 1000)
		TRX.LCD_SleepTimeout = 1000;
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
	if (TRX.FFT_Sensitivity < 1)
		TRX.FFT_Sensitivity = 1;
	if (TRX.FFT_Sensitivity > FFT_MAX_TOP_SCALE)
		TRX.FFT_Sensitivity = FFT_MAX_TOP_SCALE;
}

static void SYSMENU_HANDL_SCREEN_Show_Sec_VFO(int8_t direction)
{
	if (direction > 0)
		TRX.Show_Sec_VFO = true;
	if (direction < 0)
		TRX.Show_Sec_VFO = false;
}

static void SYSMENU_HANDL_SCREEN_FFT_Scale_Type(int8_t direction)
{
	if (TRX.FFT_Scale_Type > 0 || direction > 0)
		TRX.FFT_Scale_Type += direction;
	if (TRX.FFT_Scale_Type > 2)
		TRX.FFT_Scale_Type = 2;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON1(int8_t direction)
{
	if (TRX.FuncButtons[0] > 0 || direction > 0)
		TRX.FuncButtons[0] += direction;
	if (TRX.FuncButtons[0] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[0] = FUNCBUTTONS_COUNT - 1;
}

#if FUNCBUTTONS_COUNT > 1
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

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON25(int8_t direction)
{
	if (TRX.FuncButtons[24] > 0 || direction > 0)
		TRX.FuncButtons[24] += direction;
	if (TRX.FuncButtons[24] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[24] = FUNCBUTTONS_COUNT - 1;
}

#if (FUNCBUTTONS_ON_PAGE * FUNCBUTTONS_PAGES) > 25

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON26(int8_t direction)
{
	if (TRX.FuncButtons[25] > 0 || direction > 0)
		TRX.FuncButtons[25] += direction;
	if (TRX.FuncButtons[25] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[25] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON27(int8_t direction)
{
	if (TRX.FuncButtons[26] > 0 || direction > 0)
		TRX.FuncButtons[26] += direction;
	if (TRX.FuncButtons[26] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[26] = FUNCBUTTONS_COUNT - 1;
}

#if (FUNCBUTTONS_ON_PAGE * FUNCBUTTONS_PAGES) > 27

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON28(int8_t direction)
{
	if (TRX.FuncButtons[27] > 0 || direction > 0)
		TRX.FuncButtons[27] += direction;
	if (TRX.FuncButtons[27] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[27] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON29(int8_t direction)
{
	if (TRX.FuncButtons[28] > 0 || direction > 0)
		TRX.FuncButtons[28] += direction;
	if (TRX.FuncButtons[28] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[28] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON30(int8_t direction)
{
	if (TRX.FuncButtons[29] > 0 || direction > 0)
		TRX.FuncButtons[29] += direction;
	if (TRX.FuncButtons[29] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[29] = FUNCBUTTONS_COUNT - 1;
}

#if (FUNCBUTTONS_ON_PAGE * FUNCBUTTONS_PAGES) > 30
static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON31(int8_t direction)
{
	if (TRX.FuncButtons[30] > 0 || direction > 0)
		TRX.FuncButtons[30] += direction;
	if (TRX.FuncButtons[30] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[30] = FUNCBUTTONS_COUNT - 1;
}

static void SYSMENU_HANDL_SCREEN_FUNC_BUTTON32(int8_t direction)
{
	if (TRX.FuncButtons[31] > 0 || direction > 0)
		TRX.FuncButtons[31] += direction;
	if (TRX.FuncButtons[31] >= FUNCBUTTONS_COUNT)
		TRX.FuncButtons[31] = FUNCBUTTONS_COUNT - 1;
}
#endif
#endif
#endif
#endif

// DECODERS MENU

static void SYSMENU_HANDL_DECODERSMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_decoders_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_decoders_handlers) / sizeof(sysmenu_decoders_handlers[0]);
	sysmenu_onroot = false;
	LCD_redraw(false);
}

static void SYSMENU_HANDL_DECODERS_CW_Decoder(int8_t direction)
{
	if (direction > 0)
		TRX.CW_Decoder = true;
	if (direction < 0)
		TRX.CW_Decoder = false;
}

static void SYSMENU_HANDL_DECODERS_CW_Decoder_Threshold(int8_t direction)
{
	if (TRX.CW_Decoder_Threshold > 1 || direction > 0)
		TRX.CW_Decoder_Threshold += direction;
	if (TRX.CW_Decoder_Threshold > 50)
		TRX.CW_Decoder_Threshold = 50;
}

static void SYSMENU_HANDL_DECODERS_RDS_Decoder(int8_t direction)
{
	if (direction > 0)
		TRX.RDS_Decoder = true;
	if (direction < 0)
		TRX.RDS_Decoder = false;
}

static void SYSMENU_HANDL_DECODERS_RTTY_InvertBits(int8_t direction)
{
	if (direction > 0)
		TRX.RTTY_InvertBits = true;
	if (direction < 0)
		TRX.RTTY_InvertBits = false;
}

static void SYSMENU_HANDL_DECODERS_RTTY_Speed(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.RTTY_Speed == 45)
			TRX.RTTY_Speed = 50;
		else if (TRX.RTTY_Speed == 50)
			TRX.RTTY_Speed = 75;
		else if (TRX.RTTY_Speed == 75)
			TRX.RTTY_Speed = 100;
		else if (TRX.RTTY_Speed == 100)
			TRX.RTTY_Speed = 150;
		else if (TRX.RTTY_Speed == 150)
			TRX.RTTY_Speed = 300;
	}
	if (direction < 0)
	{
		if (TRX.RTTY_Speed == 300)
			TRX.RTTY_Speed = 150;
		else if (TRX.RTTY_Speed == 150)
			TRX.RTTY_Speed = 100;
		else if (TRX.RTTY_Speed == 100)
			TRX.RTTY_Speed = 75;
		else if (TRX.RTTY_Speed == 75)
			TRX.RTTY_Speed = 50;
		else if (TRX.RTTY_Speed == 50)
			TRX.RTTY_Speed = 45;
	}

	RTTYDecoder_Init();
}

static void SYSMENU_HANDL_DECODERS_RTTY_Shift(int8_t direction)
{
	if (direction > 0)
	{
		if (TRX.RTTY_Shift == 85)
			TRX.RTTY_Shift = 170;
		else if (TRX.RTTY_Shift == 170)
			TRX.RTTY_Shift = 182;
		else if (TRX.RTTY_Shift == 182)
			TRX.RTTY_Shift = 200;
		else if (TRX.RTTY_Shift == 200)
			TRX.RTTY_Shift = 220;
		else if (TRX.RTTY_Shift == 220)
			TRX.RTTY_Shift = 240;
		else if (TRX.RTTY_Shift == 240)
			TRX.RTTY_Shift = 350;
		else if (TRX.RTTY_Shift == 350)
			TRX.RTTY_Shift = 425;
		else if (TRX.RTTY_Shift == 425)
			TRX.RTTY_Shift = 450;
		else if (TRX.RTTY_Shift == 450)
			TRX.RTTY_Shift = 850;
	}
	if (direction < 0)
	{
		if (TRX.RTTY_Shift == 850)
			TRX.RTTY_Shift = 450;
		else if (TRX.RTTY_Shift == 450)
			TRX.RTTY_Shift = 425;
		else if (TRX.RTTY_Shift == 425)
			TRX.RTTY_Shift = 350;
		else if (TRX.RTTY_Shift == 350)
			TRX.RTTY_Shift = 240;
		else if (TRX.RTTY_Shift == 240)
			TRX.RTTY_Shift = 220;
		else if (TRX.RTTY_Shift == 220)
			TRX.RTTY_Shift = 200;
		else if (TRX.RTTY_Shift == 200)
			TRX.RTTY_Shift = 182;
		else if (TRX.RTTY_Shift == 182)
			TRX.RTTY_Shift = 170;
		else if (TRX.RTTY_Shift == 170)
			TRX.RTTY_Shift = 85;
	}

	RTTYDecoder_Init();
}

static void SYSMENU_HANDL_DECODERS_RTTY_Freq(int8_t direction)
{
	if (TRX.RTTY_Freq > 50 || direction > 0)
		TRX.RTTY_Freq += direction * 50;
	if (TRX.RTTY_Freq > 3000)
		TRX.RTTY_Freq = 3000;

	RTTYDecoder_Init();
}

static void SYSMENU_HANDL_DECODERS_RTTY_StopBits(int8_t direction)
{
	if (TRX.RTTY_StopBits > 0 || direction > 0)
		TRX.RTTY_StopBits += direction;
	if (TRX.RTTY_StopBits > 2)
		TRX.RTTY_StopBits = 2;

	RTTYDecoder_Init();
}

// ADC/DAC MENU

static void SYSMENU_HANDL_ADCMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_adc_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_adc_handlers) / sizeof(sysmenu_adc_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_ADC_DRIVER(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_Driver = true;
	if (direction < 0)
		TRX.ADC_Driver = false;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
		TRX.BANDS_SAVED_SETTINGS[band].ADC_Driver = TRX.ADC_Driver;
	FPGA_NeedSendParams = true;
}

static void SYSMENU_HANDL_ADC_PGA(int8_t direction)
{
	if (direction > 0)
		TRX.ADC_PGA = true;
	if (direction < 0)
		TRX.ADC_PGA = false;
	int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
	if (band >= 0)
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

// WIFI MENU
#if HRDW_HAS_WIFI
static void SYSMENU_HANDL_WIFIMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_wifi_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_wifi_handlers) / sizeof(sysmenu_wifi_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_WIFI_RedrawSelectAPMenu(void)
{
	WIFI_State = WIFI_READY;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_WIFI_DrawSelectAP1Menu(bool full_redraw)
{
	#define Y_SPAN (RASTR_FONT_H * LAYOUT->SYSMENU_FONT_SIZE)
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		uint16_t curr_y = 5;
		LCDDriver_printText("NET1 Found:", curr_y, 5, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		curr_y += Y_SPAN + 5;
		LCDDriver_printText(">Refresh", 10, curr_y, COLOR_WHITE, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		curr_y += Y_SPAN;
		for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
			LCDDriver_printText((char *)WIFI_FoundedAP[i], 10, curr_y + i * Y_SPAN, COLOR_GREEN, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		LCDDriver_drawFastHLine(0, 10 + Y_SPAN * 2 + sysmenu_wifi_selected_ap_index * Y_SPAN, LAYOUT->SYSMENU_W, FG_COLOR);
	}
	if (sysmenu_wifi_needupdate_ap)
	{
		if (WIFI_ListAP(SYSMENU_HANDL_WIFI_RedrawSelectAPMenu))
			sysmenu_wifi_needupdate_ap = false;
		else
			LCD_UpdateQuery.SystemMenu = true;
	}
}

static void SYSMENU_WIFI_DrawSelectAP2Menu(bool full_redraw)
{
	#define Y_SPAN (RASTR_FONT_H * LAYOUT->SYSMENU_FONT_SIZE)
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		uint16_t curr_y = 5;
		LCDDriver_printText("NET2 Found:", curr_y, 5, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		curr_y += Y_SPAN + 5;
		LCDDriver_printText(">Refresh", 10, curr_y, COLOR_WHITE, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		curr_y += Y_SPAN;
		for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
			LCDDriver_printText((char *)WIFI_FoundedAP[i], 10, curr_y + i * Y_SPAN, COLOR_GREEN, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		LCDDriver_drawFastHLine(0, 10 + Y_SPAN * 2 + sysmenu_wifi_selected_ap_index * Y_SPAN, LAYOUT->SYSMENU_W, FG_COLOR);
	}
	if (sysmenu_wifi_needupdate_ap)
	{
		if (WIFI_ListAP(SYSMENU_HANDL_WIFI_RedrawSelectAPMenu))
			sysmenu_wifi_needupdate_ap = false;
		else
			LCD_UpdateQuery.SystemMenu = true;
	}
}

static void SYSMENU_WIFI_DrawSelectAP3Menu(bool full_redraw)
{
	#define Y_SPAN (RASTR_FONT_H * LAYOUT->SYSMENU_FONT_SIZE)
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		uint16_t curr_y = 5;
		LCDDriver_printText("NET3 Found:", curr_y, 5, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		curr_y += Y_SPAN + 5;
		LCDDriver_printText(">Refresh", 10, curr_y, COLOR_WHITE, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		curr_y += Y_SPAN;
		for (uint8_t i = 0; i < WIFI_FOUNDED_AP_MAXCOUNT; i++)
			LCDDriver_printText((char *)WIFI_FoundedAP[i], 10, curr_y + i * Y_SPAN, COLOR_GREEN, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		LCDDriver_drawFastHLine(0, 10 + Y_SPAN * 2 + sysmenu_wifi_selected_ap_index * Y_SPAN, LAYOUT->SYSMENU_W, FG_COLOR);
	}
	if (sysmenu_wifi_needupdate_ap)
	{
		if (WIFI_ListAP(SYSMENU_HANDL_WIFI_RedrawSelectAPMenu))
			sysmenu_wifi_needupdate_ap = false;
		else
			LCD_UpdateQuery.SystemMenu = true;
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
		if (sysmenu_wifi_selected_ap_index == 0)
		{
			sysmenu_wifi_needupdate_ap = true;
			dma_memset((void *)WIFI_FoundedAP, 0, sizeof(WIFI_FoundedAP));
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		else
		{
			strcpy(WIFI.AP_1, (char *)&WIFI_FoundedAP[sysmenu_wifi_selected_ap_index - 1]);
			WIFI_State = WIFI_CONFIGURED;
			sysmenu_wifi_selectap1_menu_opened = false;
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
		if (sysmenu_wifi_selected_ap_index == 0)
		{
			sysmenu_wifi_needupdate_ap = true;
			dma_memset((void *)WIFI_FoundedAP, 0, sizeof(WIFI_FoundedAP));
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		else
		{
			strcpy(WIFI.AP_2, (char *)&WIFI_FoundedAP[sysmenu_wifi_selected_ap_index - 1]);
			WIFI_State = WIFI_CONFIGURED;
			sysmenu_wifi_selectap2_menu_opened = false;
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
		if (sysmenu_wifi_selected_ap_index == 0)
		{
			sysmenu_wifi_needupdate_ap = true;
			dma_memset((void *)WIFI_FoundedAP, 0, sizeof(WIFI_FoundedAP));
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		else
		{
			strcpy(WIFI.AP_3, (char *)&WIFI_FoundedAP[sysmenu_wifi_selected_ap_index - 1]);
			WIFI_State = WIFI_CONFIGURED;
			sysmenu_wifi_selectap3_menu_opened = false;
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
	}
}

static void SYSMENU_WIFI_AP1_Password_keyboardHandler(uint32_t parameter)
{
	if(parameter == '<') //backspace
	{
		WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] = 0;
		
		if (sysmenu_wifi_selected_ap_password_char_index > 0)
			sysmenu_wifi_selected_ap_password_char_index--;
	} else {
		WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] = parameter;
		
		if (sysmenu_wifi_selected_ap_password_char_index < (MAX_WIFIPASS_LENGTH - 1))
			sysmenu_wifi_selected_ap_password_char_index++;
	}
	
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_WIFI_DrawAP1passwordMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("NET1 Password:", 5, 5, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	}

	LCDDriver_printText(WIFI.Password_1, 10, 37, COLOR_GREEN, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	LCDDriver_drawFastHLine(8 + sysmenu_wifi_selected_ap_password_char_index * RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, interactive_menu_top, RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, COLOR_RED);

	#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	LCD_printKeyboard(SYSMENU_WIFI_AP1_Password_keyboardHandler, true);
	#endif
}

static void SYSMENU_WIFI_AP2_Password_keyboardHandler(uint32_t parameter)
{
	if(parameter == '<') //backspace
	{
		WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] = 0;
		
		if (sysmenu_wifi_selected_ap_password_char_index > 0)
			sysmenu_wifi_selected_ap_password_char_index--;
	} else {
		WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] = parameter;
		
		if (sysmenu_wifi_selected_ap_password_char_index < (MAX_WIFIPASS_LENGTH - 1))
			sysmenu_wifi_selected_ap_password_char_index++;
	}
	
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_WIFI_DrawAP2passwordMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("NET2 Password:", 5, 5, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	}

	LCDDriver_printText(WIFI.Password_2, 10, 37, COLOR_GREEN, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	LCDDriver_drawFastHLine(8 + sysmenu_wifi_selected_ap_password_char_index * RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, interactive_menu_top, RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, COLOR_RED);

	#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	LCD_printKeyboard(SYSMENU_WIFI_AP2_Password_keyboardHandler, true);
	#endif
}

static void SYSMENU_WIFI_AP3_Password_keyboardHandler(uint32_t parameter)
{
	if(parameter == '<') //backspace
	{
		WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] = 0;
		
		if (sysmenu_wifi_selected_ap_password_char_index > 0)
			sysmenu_wifi_selected_ap_password_char_index--;
	} else {
		WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] = parameter;
		
		if (sysmenu_wifi_selected_ap_password_char_index < (MAX_WIFIPASS_LENGTH - 1))
			sysmenu_wifi_selected_ap_password_char_index++;
	}
	
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_WIFI_DrawAP3passwordMenu(bool full_redraw)
{
	if (full_redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("NET3 Password:", 5, 5, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	}

	LCDDriver_printText(WIFI.Password_3, 10, 37, COLOR_GREEN, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	LCDDriver_drawFastHLine(8 + sysmenu_wifi_selected_ap_password_char_index * RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, interactive_menu_top, RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE, COLOR_RED);

	#if (defined(HAS_TOUCHPAD) && defined(LAY_800x480))
	LCD_printKeyboard(SYSMENU_WIFI_AP3_Password_keyboardHandler, true);
	#endif
}

static void SYSMENU_WIFI_RotatePasswordChar1(int8_t dir)
{
	bool full_redraw = false;
	if (WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;
	WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] += dir;

	// do not show special characters
	if (WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] >= 1 && WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir > 0)
		WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] = 33;
	if (WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] >= 1 && WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir < 0)
		WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] >= 127)
		WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (WIFI.Password_1[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		LCD_UpdateQuery.SystemMenuRedraw = true;
	else
		LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_WIFI_RotatePasswordChar2(int8_t dir)
{
	bool full_redraw = false;
	if (WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;
	WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] += dir;

	// do not show special characters
	if (WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] >= 1 && WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir > 0)
		WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] = 33;
	if (WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] >= 1 && WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir < 0)
		WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] >= 127)
		WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (WIFI.Password_2[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		LCD_UpdateQuery.SystemMenuRedraw = true;
	else
		LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_WIFI_RotatePasswordChar3(int8_t dir)
{
	bool full_redraw = false;
	if (WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;
	WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] += dir;

	// do not show special characters
	if (WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] >= 1 && WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir > 0)
		WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] = 33;
	if (WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] >= 1 && WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] <= 32 && dir < 0)
		WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] >= 127)
		WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] = 0;
	if (WIFI.Password_3[sysmenu_wifi_selected_ap_password_char_index] == 0)
		full_redraw = true;

	if (full_redraw)
		LCD_UpdateQuery.SystemMenuRedraw = true;
	else
		LCD_UpdateQuery.SystemMenu = true;
}

static void SYSMENU_HANDL_WIFI_Enabled(int8_t direction)
{
	if (direction > 0)
		WIFI.Enabled = true;
	if (direction < 0)
		WIFI.Enabled = false;
	
	NeedSaveWiFi = true;
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
	WIFI.Timezone += direction;
	if (WIFI.Timezone < -12)
		WIFI.Timezone = -12;
	if (WIFI.Timezone > 12)
		WIFI.Timezone = 12;
	WIFI_State = WIFI_INITED;
	
	NeedSaveWiFi = true;
}

static void SYSMENU_HANDL_WIFI_CAT_Server(int8_t direction)
{
	if (direction > 0)
		WIFI.CAT_Server = true;
	if (direction < 0)
		WIFI.CAT_Server = false;
	
	NeedSaveWiFi = true;
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
#endif

// SD MENU
#if HRDW_HAS_SD
static void SYSMENU_HANDL_SDMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_sd_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_sd_handlers) / sizeof(sysmenu_sd_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

void SYSMENU_SD_FILEMANAGER_HOTKEY(void)
{
	SYSMENU_HANDL_SDMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "File Manager");
	setCurrentMenuIndex(index);
	LCD_UpdateQuery.SystemMenuRedraw = true;
	SYSMENU_HANDL_SD_Filemanager(0);
}

static void SYSMENU_HANDL_SD_Filemanager(int8_t direction)
{
	sysmenu_filemanager_opened = true;
	SYSMENU_drawSystemMenu(true, false);
}

static void SYSMENU_HANDL_SD_ExportSettingsDialog(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_sd_export_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_sd_export_handlers) / sizeof(sysmenu_sd_export_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_SD_ImportSettingsDialog(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_sd_import_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_sd_import_handlers) / sizeof(sysmenu_sd_import_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}
static void SYSMENU_HANDL_SD_FormatDialog(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_sd_format_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_sd_format_handlers) / sizeof(sysmenu_sd_format_handlers[0]);
	sysmenu_onroot = false;
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
#endif
// SET TIME MENU

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
	if (direction == 0)
	{
		if(LCD_busy)
			return;
		LCD_busy = true;
		#ifdef LCD_SMALL_INTERFACE
			#define x_pos_clk 50
			#define y_pos_clk 50
		#else
			#define x_pos_clk 76
			#define y_pos_clk 100
		#endif
		sprintf(ctmp, "%d", Hours);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printText(ctmp, x_pos_clk, y_pos_clk, COLOR->BUTTON_TEXT, TimeMenuSelection == 0 ? FG_COLOR : BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		LCDDriver_printText(":", LCDDriver_GetCurrentXOffset(), y_pos_clk, COLOR->BUTTON_TEXT, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		sprintf(ctmp, "%d", Minutes);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printText(ctmp, LCDDriver_GetCurrentXOffset(), y_pos_clk, COLOR->BUTTON_TEXT, TimeMenuSelection == 1 ? FG_COLOR : BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		LCDDriver_printText(":", LCDDriver_GetCurrentXOffset(), y_pos_clk, COLOR->BUTTON_TEXT, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		sprintf(ctmp, "%d", Seconds);
		addSymbols(ctmp, ctmp, 2, "0", false);
		LCDDriver_printText(ctmp, LCDDriver_GetCurrentXOffset(), y_pos_clk, COLOR->BUTTON_TEXT, TimeMenuSelection == 2 ? FG_COLOR : BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
		LCD_busy = false;
	}
}

// FLASH MENU

static void SYSMENU_HANDL_Bootloader(int8_t direction)
{
#pragma unused(direction)
	CODEC_CleanBuffer();
	TRX.NeedGoToBootloader = true;
	SaveSettings();
	HAL_Delay(500);
	SCB->AIRCR = 0x05FA0004;

	TRX_Inited = false;
	LCD_busy = true;
}

// OTA update

#if HRDW_HAS_SD
static void SYSMENU_HANDL_OTA_Update(int8_t direction)
{
	FILEMANAGER_OTAUpdate_reset();
	FILEMANAGER_OTAUpdate_handler();
}
#endif

// SYSTEM INFO

static void SYSMENU_HANDL_SYSINFO(int8_t direction)
{
	sysmenu_infowindow_opened = true;
	sysmenu_sysinfo_opened = true;
	if (direction != 0)
		LCDDriver_Fill(BG_COLOR);
#define y_offs (LAYOUT->SYSMENU_FONT_SIZE * RASTR_FONT_H + LAYOUT->SYSMENU_FONT_SIZE * 2)
	uint16_t y = 10;
	char out[80];
	sprintf(out, "STM32 FW ver: %s", version_string);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	y += y_offs;
	sprintf(out, "FPGA FW ver: %d.%d.%d", FPGA_FW_Version[2], FPGA_FW_Version[1], FPGA_FW_Version[0]);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	y += y_offs;
	#if HRDW_HAS_WIFI
	sprintf(out, "WIFI IP: %s", WIFI_IP);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	y += y_offs;
	#endif
	sprintf(out, "FPGA SAMPLES: %d     ", dbg_FPGA_samples);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	y += y_offs;
	sprintf(out, "STM32 VOLTAGE: %f     ", TRX_STM32_VREF);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	y += y_offs;
	sprintf(out, "IQ PHASE: %f     ", TRX_IQ_phase_error);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	y += y_offs;
	sprintf(out, "ADC MIN/MAX: %d/%d     ", TRX_ADC_MINAMPLITUDE, TRX_ADC_MAXAMPLITUDE);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	y += y_offs;
	sprintf(out, "VCXO ERROR: %d     ", TRX_VCXO_ERROR);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	y += y_offs;
	sprintf(out, "VBAT VOLT: %.2f     ", TRX_VBAT_Voltage);
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	y += y_offs;
	sprintf(out, "ALC: %.2fv (%d%%)    ", TRX_ALC_IN, getPowerFromALC(TRX_ALC_IN));
	LCDDriver_printText(out, 5, y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);

	LCD_UpdateQuery.SystemMenu = true;
}

// SYSTEM INFO

static void SYSMENU_HANDL_SUPPORT(int8_t direction)
{
	sysmenu_infowindow_opened = true;
	LCDDriver_Fill(BG_COLOR);
#ifndef LCD_SMALL_INTERFACE
	LCDDriver_printText("MasterCard: 5599 0020 2091 0792", 10, 10, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
#else
	LCDDriver_printText("MasterCard:", 10, 10, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	LCDDriver_printText("5599 0020 2091 0792", 10, 20, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
#endif
}

//Back to prev menu

static void SYSMENU_HANDL_Back(int8_t direction)
{
	SYSMENU_eventCloseSystemMenu();
	LCD_redraw(false);
}

// CALIBRATION MENU

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

static void SYSMENU_HANDL_CALIB_RF_unit_type(int8_t direction)
{
	if (CALIBRATE.RF_unit_type > 0 || direction > 0)
		CALIBRATE.RF_unit_type += direction;
	if (CALIBRATE.RF_unit_type > 3)
		CALIBRATE.RF_unit_type = 3;

	if (CALIBRATE.RF_unit_type == RF_UNIT_QRP)
	{
		CALIBRATE.rf_out_power_2200m = 29;			   // 2200m
		CALIBRATE.rf_out_power_160m = 29;			   // 160m
		CALIBRATE.rf_out_power_80m = 27;			   // 80m
		CALIBRATE.rf_out_power_40m = 26;			   // 40m
		CALIBRATE.rf_out_power_30m = 26;			   // 30m
		CALIBRATE.rf_out_power_20m = 24;			   // 20m
		CALIBRATE.rf_out_power_17m = 24;			   // 17m
		CALIBRATE.rf_out_power_15m = 24;			   // 15m
		CALIBRATE.rf_out_power_12m = 24;			   // 12m
		CALIBRATE.rf_out_power_cb = 24;				   // 27mhz
		CALIBRATE.rf_out_power_10m = 24;			   // 10m
		CALIBRATE.rf_out_power_6m = 13;				   // 6m
		CALIBRATE.rf_out_power_4m = 13;				   // 4m
		CALIBRATE.rf_out_power_2m = 100;			   // 2m
		CALIBRATE.RFU_LPF_END = 60000 * 1000;		   // LPF
		CALIBRATE.RFU_HPF_START = 60000 * 1000;		   // HPF U14-RF1
		CALIBRATE.RFU_BPF_0_START = 135 * 1000 * 1000; // 2m U14-RF3
		CALIBRATE.RFU_BPF_0_END = 150 * 1000 * 1000;   // 2m
		CALIBRATE.RFU_BPF_1_START = 1500 * 1000;	   // 160m U16-RF2
		CALIBRATE.RFU_BPF_1_END = 2400 * 1000;		   // 160m
		CALIBRATE.RFU_BPF_2_START = 2400 * 1000;	   // 80m U16-RF4
		CALIBRATE.RFU_BPF_2_END = 4700 * 1000;		   // 80m
		CALIBRATE.RFU_BPF_3_START = 4700 * 1000;	   // 40m U16-RF1
		CALIBRATE.RFU_BPF_3_END = 7200 * 1000;		   // 40m
		CALIBRATE.RFU_BPF_4_START = 7200 * 1000;	   // 30m U16-RF3
		CALIBRATE.RFU_BPF_4_END = 11500 * 1000;		   // 30m
		CALIBRATE.RFU_BPF_5_START = 11500 * 1000;	   // 20,17m U14-RF2
		CALIBRATE.RFU_BPF_5_END = 21000 * 1000;		   // 20,17m
		CALIBRATE.RFU_BPF_6_START = 21000 * 1000;	   // 15,12,10,6m U14-RF4
		CALIBRATE.RFU_BPF_6_END = 64000 * 1000;		   // 15,12,10,6m
		CALIBRATE.RFU_BPF_7_START = 0;				   // disabled on qrp version
		CALIBRATE.RFU_BPF_7_END = 0;				   // disabled on qrp version
		CALIBRATE.RFU_BPF_8_START = 0;				   // disabled on qrp version
		CALIBRATE.RFU_BPF_8_END = 0;				   // disabled on qrp version
		CALIBRATE.SWR_FWD_Calibration_HF = 11.0f;	   // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 11.0f;	   // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_6M = 10.0f;	   // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_6M = 10.0f;	   // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_VHF = 3.6f;	   // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_VHF = 3.6f;	   // SWR Transormator rate return
		CALIBRATE.TUNE_MAX_POWER = 2;				   // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 7;					   // Max TRX Power for indication
	}
	if (CALIBRATE.RF_unit_type == RF_UNIT_BIG || CALIBRATE.RF_unit_type == RF_UNIT_SPLIT || CALIBRATE.RF_unit_type == RF_UNIT_RU4PN)
	{
		CALIBRATE.rf_out_power_2200m = 40;		   // 2200m
		CALIBRATE.rf_out_power_160m = 40;		   // 160m
		CALIBRATE.rf_out_power_80m = 40;		   // 80m
		CALIBRATE.rf_out_power_40m = 40;		   // 40m
		CALIBRATE.rf_out_power_30m = 40;		   // 30m
		CALIBRATE.rf_out_power_20m = 40;		   // 20m
		CALIBRATE.rf_out_power_17m = 40;		   // 17m
		CALIBRATE.rf_out_power_15m = 40;		   // 15m
		CALIBRATE.rf_out_power_12m = 40;		   // 12m
		CALIBRATE.rf_out_power_cb = 40;			   // 27mhz
		CALIBRATE.rf_out_power_10m = 40;		   // 10m
		CALIBRATE.rf_out_power_6m = 40;			   // 6m
		CALIBRATE.rf_out_power_4m = 40;			   // 4m
		CALIBRATE.rf_out_power_2m = 50;			   // 2m
		CALIBRATE.RFU_LPF_END = 0;				   // disabled in BIG version
		CALIBRATE.RFU_HPF_START = 0;			   // disabled in BIG version
		CALIBRATE.RFU_BPF_0_START = 0 * 1000;	   // 2200m
		CALIBRATE.RFU_BPF_0_END = 1000 * 1000;	   // 2200m
		CALIBRATE.RFU_BPF_1_START = 1000 * 1000;   // 160m
		CALIBRATE.RFU_BPF_1_END = 2500 * 1000;	   // 160m
		CALIBRATE.RFU_BPF_2_START = 2500 * 1000;   // 80m
		CALIBRATE.RFU_BPF_2_END = 5000 * 1000;	   // 80m
		CALIBRATE.RFU_BPF_3_START = 5000 * 1000;   // 40m
		CALIBRATE.RFU_BPF_3_END = 8500 * 1000;	   // 40m
		CALIBRATE.RFU_BPF_4_START = 8500 * 1000;   // 30m
		CALIBRATE.RFU_BPF_4_END = 12000 * 1000;	   // 30m
		CALIBRATE.RFU_BPF_5_START = 12000 * 1000;  // 20m
		CALIBRATE.RFU_BPF_5_END = 17000 * 1000;	   // 20m
		CALIBRATE.RFU_BPF_6_START = 17000 * 1000;  // 17,15,12m
		CALIBRATE.RFU_BPF_6_END = 25500 * 1000;	   // 17,15,12m
		CALIBRATE.RFU_BPF_7_START = 25500 * 1000;  // CB,10m
		CALIBRATE.RFU_BPF_7_END = 35000 * 1000;	   // CB,10m
		CALIBRATE.RFU_BPF_8_START = 35000 * 1000;  // 6m
		CALIBRATE.RFU_BPF_8_END = 70000 * 1000;	   // 6m
		CALIBRATE.SWR_FWD_Calibration_HF = 22.0f;  // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 22.0f;  // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_6M = 22.0f;  // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_6M = 22.0f;  // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_VHF = 22.0f; // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_VHF = 22.0f; // SWR Transormator rate return
		CALIBRATE.TUNE_MAX_POWER = 10;			   // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 100;			   // Max TRX Power for indication
	}
	if (CALIBRATE.RF_unit_type == RF_UNIT_RU4PN)
	{
		CALIBRATE.RFU_LPF_END = 60000 * 1000;		   // LPF
		CALIBRATE.RFU_HPF_START = 60000 * 1000;		   // HPF
		CALIBRATE.RFU_BPF_0_START = 135 * 1000 * 1000; // 2m U14-RF3
		CALIBRATE.RFU_BPF_0_END = 150 * 1000 * 1000;   // 2m
		CALIBRATE.RFU_BPF_1_START = 1500 * 1000;	   // 160m U16-RF2
		CALIBRATE.RFU_BPF_1_END = 2400 * 1000;		   // 160m
		CALIBRATE.RFU_BPF_2_START = 2400 * 1000;	   // 80m U16-RF4
		CALIBRATE.RFU_BPF_2_END = 4700 * 1000;		   // 80m
		CALIBRATE.RFU_BPF_3_START = 4700 * 1000;	   // 40m U16-RF1
		CALIBRATE.RFU_BPF_3_END = 7200 * 1000;		   // 40m
		CALIBRATE.RFU_BPF_4_START = 7200 * 1000;	   // 30m U16-RF3
		CALIBRATE.RFU_BPF_4_END = 11500 * 1000;		   // 30m
		CALIBRATE.RFU_BPF_5_START = 11500 * 1000;	   // 20,17m U14-RF2
		CALIBRATE.RFU_BPF_5_END = 21000 * 1000;		   // 20,17m
		CALIBRATE.RFU_BPF_6_START = 21000 * 1000;	   // 15,12,10,6m U14-RF4
		CALIBRATE.RFU_BPF_6_END = 64000 * 1000;		   // 15,12,10,6m
		CALIBRATE.RFU_BPF_7_START = 0;				   // disabled on qrp version
		CALIBRATE.RFU_BPF_7_END = 0;				   // disabled on qrp version
		CALIBRATE.RFU_BPF_8_START = 0;				   // disabled on qrp version
		CALIBRATE.RFU_BPF_8_END = 0;				   // disabled on qrp version
	}
	if (CALIBRATE.RF_unit_type == RF_UNIT_WF_100D)
	{
		CALIBRATE.rf_out_power_2200m = 17;			   // 2200m
		CALIBRATE.rf_out_power_160m = 17;			   // 160m
		CALIBRATE.rf_out_power_80m = 20;			   // 80m
		CALIBRATE.rf_out_power_40m = 22;			   // 40m
		CALIBRATE.rf_out_power_30m = 24;			   // 30m
		CALIBRATE.rf_out_power_20m = 25;			   // 20m
		CALIBRATE.rf_out_power_17m = 30;			   // 17m
		CALIBRATE.rf_out_power_15m = 35;			   // 15m
		CALIBRATE.rf_out_power_12m = 38;			   // 12m
		CALIBRATE.rf_out_power_cb = 40;				   // 27mhz
		CALIBRATE.rf_out_power_10m = 40;			   // 10m
		CALIBRATE.rf_out_power_6m = 40;				   // 6m
		CALIBRATE.rf_out_power_4m = 40;				   // 4m
		CALIBRATE.rf_out_power_2m = 70;				   // 2m
		CALIBRATE.RFU_LPF_END = 53 * 1000 * 1000;	   // LPF
		CALIBRATE.RFU_HPF_START = 60 * 1000 * 1000;	   // HPF
		CALIBRATE.RFU_BPF_0_START = 1600 * 1000;	   // 1.6-2.5mH
		CALIBRATE.RFU_BPF_0_END = 2500 * 1000;		   //
		CALIBRATE.RFU_BPF_1_START = 2500 * 1000;	   // 2.5-4mHz
		CALIBRATE.RFU_BPF_1_END = 4000 * 1000;		   //
		CALIBRATE.RFU_BPF_2_START = 6000 * 1000;	   // 6-7.3mHz
		CALIBRATE.RFU_BPF_2_END = 7300 * 1000;		   //
		CALIBRATE.RFU_BPF_3_START = 7300 * 1000;	   // 7-12mHz
		CALIBRATE.RFU_BPF_3_END = 12000 * 1000;		   //
		CALIBRATE.RFU_BPF_4_START = 12000 * 1000;	   // 12-14.5mHz
		CALIBRATE.RFU_BPF_4_END = 14500 * 1000;		   //
		CALIBRATE.RFU_BPF_5_START = 14500 * 1000;	   // 14.5-21.5mHz
		CALIBRATE.RFU_BPF_5_END = 21500 * 1000;		   //
		CALIBRATE.RFU_BPF_6_START = 21500 * 1000;	   // 21.5-30 mHz
		CALIBRATE.RFU_BPF_6_END = 30000 * 1000;		   //
		CALIBRATE.RFU_BPF_7_START = 135 * 1000 * 1000; // 135-150mHz
		CALIBRATE.RFU_BPF_7_END = 150 * 1000 * 1000;   //
		CALIBRATE.RFU_BPF_8_START = 0;				   // disabled
		CALIBRATE.RFU_BPF_8_END = 0;				   // disabled
		CALIBRATE.SWR_FWD_Calibration_HF = 17.5f;	   // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_HF = 17.5f;	   // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_6M = 19.0f;	   // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_6M = 19.0f;	   // SWR Transormator rate return
		CALIBRATE.SWR_FWD_Calibration_VHF = 21.0f;	   // SWR Transormator rate forward
		CALIBRATE.SWR_BWD_Calibration_VHF = 9.5f;	   // SWR Transormator rate return
		CALIBRATE.TUNE_MAX_POWER = 15;				   // Maximum RF power in Tune mode
		CALIBRATE.MAX_RF_POWER_ON_METER = 100;				   // Max TRX Power for indication
	}
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_CALIB_TangentType(int8_t direction)
{
	if (CALIBRATE.TangentType > 0 || direction > 0)
		CALIBRATE.TangentType += direction;
	if (CALIBRATE.TangentType > 1)
		CALIBRATE.TangentType = 1;
}

static void SYSMENU_HANDL_CALIBRATIONMENU(int8_t direction)
{
#pragma unused(direction)
	if (!SYSMENU_hiddenmenu_enabled)
		return;
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_calibration_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_calibration_handlers) / sizeof(sysmenu_calibration_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

#if FT8_SUPPORT
static void SYSMENU_HANDL_FT8_Decoder(int8_t direction) // Tisho
{
	if (SYSMENU_FT8_DECODER_opened)
	{
		FT8_EncRotate(direction);
	}
	else
	{
		SYSMENU_FT8_DECODER_opened = true;
		InitFT8_Decoder();
	}
}
#endif

static void SYSMENU_HANDL_SWR_Tandem_Ctrl(int8_t direction) // Tisho
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
	if (CALIBRATE.CICFIR_GAINER_48K_val > 0 || direction > 0)
		CALIBRATE.CICFIR_GAINER_48K_val += direction;
	if (CALIBRATE.CICFIR_GAINER_48K_val > 62)
		CALIBRATE.CICFIR_GAINER_48K_val = 62;
}

static void SYSMENU_HANDL_CALIB_CICCOMP_96K_SHIFT(int8_t direction)
{
	if (CALIBRATE.CICFIR_GAINER_96K_val > 0 || direction > 0)
		CALIBRATE.CICFIR_GAINER_96K_val += direction;
	if (CALIBRATE.CICFIR_GAINER_96K_val > 62)
		CALIBRATE.CICFIR_GAINER_96K_val = 62;
}

static void SYSMENU_HANDL_CALIB_CICCOMP_192K_SHIFT(int8_t direction)
{
	if (CALIBRATE.CICFIR_GAINER_192K_val > 0 || direction > 0)
		CALIBRATE.CICFIR_GAINER_192K_val += direction;
	if (CALIBRATE.CICFIR_GAINER_192K_val > 62)
		CALIBRATE.CICFIR_GAINER_192K_val = 62;
}

static void SYSMENU_HANDL_CALIB_CICCOMP_384K_SHIFT(int8_t direction)
{
	if (CALIBRATE.CICFIR_GAINER_384K_val > 0 || direction > 0)
		CALIBRATE.CICFIR_GAINER_384K_val += direction;
	if (CALIBRATE.CICFIR_GAINER_384K_val > 62)
		CALIBRATE.CICFIR_GAINER_384K_val = 62;
}

static void SYSMENU_HANDL_CALIB_TXCICCOMP_SHIFT(int8_t direction)
{
	if (CALIBRATE.TXCICFIR_GAINER_val > 0 || direction > 0)
		CALIBRATE.TXCICFIR_GAINER_val += direction;
	if (CALIBRATE.TXCICFIR_GAINER_val > 48)
		CALIBRATE.TXCICFIR_GAINER_val = 48;
}

static void SYSMENU_HANDL_CALIB_DAC_SHIFT(int8_t direction)
{
	if (CALIBRATE.DAC_GAINER_val > 0 || direction > 0)
		CALIBRATE.DAC_GAINER_val += direction;
	if (CALIBRATE.DAC_GAINER_val > 27)
		CALIBRATE.DAC_GAINER_val = 27;
}

static void SYSMENU_HANDL_CALIB_DAC_driver_mode(int8_t direction)
{
	if (CALIBRATE.DAC_driver_mode > 0)
		CALIBRATE.DAC_driver_mode += direction;
	if (CALIBRATE.DAC_driver_mode == 0 && direction > 0)
		CALIBRATE.DAC_driver_mode += direction;
	if (CALIBRATE.DAC_driver_mode > 2)
		CALIBRATE.DAC_driver_mode = 2;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_2200M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_2200m > 0)
		CALIBRATE.rf_out_power_2200m += direction;
	if (CALIBRATE.rf_out_power_2200m == 0 && direction > 0)
		CALIBRATE.rf_out_power_2200m += direction;
	if (CALIBRATE.rf_out_power_2200m > 100)
		CALIBRATE.rf_out_power_2200m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_160M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_160m > 0)
		CALIBRATE.rf_out_power_160m += direction;
	if (CALIBRATE.rf_out_power_160m == 0 && direction > 0)
		CALIBRATE.rf_out_power_160m += direction;
	if (CALIBRATE.rf_out_power_160m > 100)
		CALIBRATE.rf_out_power_160m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_80M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_80m > 0)
		CALIBRATE.rf_out_power_80m += direction;
	if (CALIBRATE.rf_out_power_80m == 0 && direction > 0)
		CALIBRATE.rf_out_power_80m += direction;
	if (CALIBRATE.rf_out_power_80m > 100)
		CALIBRATE.rf_out_power_80m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_40M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_40m > 0)
		CALIBRATE.rf_out_power_40m += direction;
	if (CALIBRATE.rf_out_power_40m == 0 && direction > 0)
		CALIBRATE.rf_out_power_40m += direction;
	if (CALIBRATE.rf_out_power_40m > 100)
		CALIBRATE.rf_out_power_40m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_30M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_30m > 0)
		CALIBRATE.rf_out_power_30m += direction;
	if (CALIBRATE.rf_out_power_30m == 0 && direction > 0)
		CALIBRATE.rf_out_power_30m += direction;
	if (CALIBRATE.rf_out_power_30m > 100)
		CALIBRATE.rf_out_power_30m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_20M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_20m > 0)
		CALIBRATE.rf_out_power_20m += direction;
	if (CALIBRATE.rf_out_power_20m == 0 && direction > 0)
		CALIBRATE.rf_out_power_20m += direction;
	if (CALIBRATE.rf_out_power_20m > 100)
		CALIBRATE.rf_out_power_20m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_17M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_17m > 0)
		CALIBRATE.rf_out_power_17m += direction;
	if (CALIBRATE.rf_out_power_17m == 0 && direction > 0)
		CALIBRATE.rf_out_power_17m += direction;
	if (CALIBRATE.rf_out_power_17m > 100)
		CALIBRATE.rf_out_power_17m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_15M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_15m > 0)
		CALIBRATE.rf_out_power_15m += direction;
	if (CALIBRATE.rf_out_power_15m == 0 && direction > 0)
		CALIBRATE.rf_out_power_15m += direction;
	if (CALIBRATE.rf_out_power_15m > 100)
		CALIBRATE.rf_out_power_15m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_12M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_12m > 0)
		CALIBRATE.rf_out_power_12m += direction;
	if (CALIBRATE.rf_out_power_12m == 0 && direction > 0)
		CALIBRATE.rf_out_power_12m += direction;
	if (CALIBRATE.rf_out_power_12m > 100)
		CALIBRATE.rf_out_power_12m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_CB(int8_t direction)
{
	if (CALIBRATE.rf_out_power_cb > 0)
		CALIBRATE.rf_out_power_cb += direction;
	if (CALIBRATE.rf_out_power_cb == 0 && direction > 0)
		CALIBRATE.rf_out_power_cb += direction;
	if (CALIBRATE.rf_out_power_cb > 100)
		CALIBRATE.rf_out_power_cb = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_10M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_10m > 0)
		CALIBRATE.rf_out_power_10m += direction;
	if (CALIBRATE.rf_out_power_10m == 0 && direction > 0)
		CALIBRATE.rf_out_power_10m += direction;
	if (CALIBRATE.rf_out_power_10m > 100)
		CALIBRATE.rf_out_power_10m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_6M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_6m > 0)
		CALIBRATE.rf_out_power_6m += direction;
	if (CALIBRATE.rf_out_power_6m == 0 && direction > 0)
		CALIBRATE.rf_out_power_6m += direction;
	if (CALIBRATE.rf_out_power_6m > 100)
		CALIBRATE.rf_out_power_6m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_4M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_4m > 0)
		CALIBRATE.rf_out_power_4m += direction;
	if (CALIBRATE.rf_out_power_4m == 0 && direction > 0)
		CALIBRATE.rf_out_power_4m += direction;
	if (CALIBRATE.rf_out_power_4m > 100)
		CALIBRATE.rf_out_power_4m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RF_GAIN_2M(int8_t direction)
{
	if (CALIBRATE.rf_out_power_2m > 0)
		CALIBRATE.rf_out_power_2m += direction;
	if (CALIBRATE.rf_out_power_2m == 0 && direction > 0)
		CALIBRATE.rf_out_power_2m += direction;
	if (CALIBRATE.rf_out_power_2m > 100)
		CALIBRATE.rf_out_power_2m = 100;

	TRX_MAX_TX_Amplitude = getMaxTXAmplitudeOnFreq(CurrentVFO->Freq);
	APROC_TX_clip_gain = 1.0f;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_S_METER_HF(int8_t direction)
{
	CALIBRATE.smeter_calibration_hf += direction;
	if (CALIBRATE.smeter_calibration_hf < -50)
		CALIBRATE.smeter_calibration_hf = -50;
	if (CALIBRATE.smeter_calibration_hf > 50)
		CALIBRATE.smeter_calibration_hf = 50;
}

static void SYSMENU_HANDL_CALIB_S_METER_VHF(int8_t direction)
{
	CALIBRATE.smeter_calibration_vhf += direction;
	if (CALIBRATE.smeter_calibration_vhf < -50)
		CALIBRATE.smeter_calibration_vhf = -50;
	if (CALIBRATE.smeter_calibration_vhf > 50)
		CALIBRATE.smeter_calibration_vhf = 50;
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
	CALIBRATE.RFU_LPF_END += direction * 100000;
	if (CALIBRATE.RFU_LPF_END < 1)
		CALIBRATE.RFU_LPF_END = 1;
	if (CALIBRATE.RFU_LPF_END > 999999999)
		CALIBRATE.RFU_LPF_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_0_START(int8_t direction)
{
	CALIBRATE.RFU_BPF_0_START += direction * 100000;
	if (CALIBRATE.RFU_BPF_0_START < 1)
		CALIBRATE.RFU_BPF_0_START = 1;
	if (CALIBRATE.RFU_BPF_0_START > 999999999)
		CALIBRATE.RFU_BPF_0_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_0_END(int8_t direction)
{
	CALIBRATE.RFU_BPF_0_END += direction * 100000;
	if (CALIBRATE.RFU_BPF_0_END < 1)
		CALIBRATE.RFU_BPF_0_END = 1;
	if (CALIBRATE.RFU_BPF_0_END > 999999999)
		CALIBRATE.RFU_BPF_0_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_1_START(int8_t direction)
{
	CALIBRATE.RFU_BPF_1_START += direction * 100000;
	if (CALIBRATE.RFU_BPF_1_START < 1)
		CALIBRATE.RFU_BPF_1_START = 1;
	if (CALIBRATE.RFU_BPF_1_START > 999999999)
		CALIBRATE.RFU_BPF_1_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_1_END(int8_t direction)
{
	CALIBRATE.RFU_BPF_1_END += direction * 100000;
	if (CALIBRATE.RFU_BPF_1_END < 1)
		CALIBRATE.RFU_BPF_1_END = 1;
	if (CALIBRATE.RFU_BPF_1_END > 999999999)
		CALIBRATE.RFU_BPF_1_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_2_START(int8_t direction)
{
	CALIBRATE.RFU_BPF_2_START += direction * 100000;
	if (CALIBRATE.RFU_BPF_2_START < 1)
		CALIBRATE.RFU_BPF_2_START = 1;
	if (CALIBRATE.RFU_BPF_2_START > 999999999)
		CALIBRATE.RFU_BPF_2_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_2_END(int8_t direction)
{
	CALIBRATE.RFU_BPF_2_END += direction * 100000;
	if (CALIBRATE.RFU_BPF_2_END < 1)
		CALIBRATE.RFU_BPF_2_END = 1;
	if (CALIBRATE.RFU_BPF_2_END > 999999999)
		CALIBRATE.RFU_BPF_2_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_3_START(int8_t direction)
{
	CALIBRATE.RFU_BPF_3_START += direction * 100000;
	if (CALIBRATE.RFU_BPF_3_START < 1)
		CALIBRATE.RFU_BPF_3_START = 1;
	if (CALIBRATE.RFU_BPF_3_START > 999999999)
		CALIBRATE.RFU_BPF_3_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_3_END(int8_t direction)
{
	CALIBRATE.RFU_BPF_3_END += direction * 100000;
	if (CALIBRATE.RFU_BPF_3_END < 1)
		CALIBRATE.RFU_BPF_3_END = 1;
	if (CALIBRATE.RFU_BPF_3_END > 999999999)
		CALIBRATE.RFU_BPF_3_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_4_START(int8_t direction)
{
	CALIBRATE.RFU_BPF_4_START += direction * 100000;
	if (CALIBRATE.RFU_BPF_4_START < 1)
		CALIBRATE.RFU_BPF_4_START = 1;
	if (CALIBRATE.RFU_BPF_4_START > 999999999)
		CALIBRATE.RFU_BPF_4_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_4_END(int8_t direction)
{
	CALIBRATE.RFU_BPF_4_END += direction * 100000;
	if (CALIBRATE.RFU_BPF_4_END < 1)
		CALIBRATE.RFU_BPF_4_END = 1;
	if (CALIBRATE.RFU_BPF_4_END > 999999999)
		CALIBRATE.RFU_BPF_4_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_5_START(int8_t direction)
{
	CALIBRATE.RFU_BPF_5_START += direction * 100000;
	if (CALIBRATE.RFU_BPF_5_START < 1)
		CALIBRATE.RFU_BPF_5_START = 1;
	if (CALIBRATE.RFU_BPF_5_START > 999999999)
		CALIBRATE.RFU_BPF_5_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_5_END(int8_t direction)
{
	CALIBRATE.RFU_BPF_5_END += direction * 100000;
	if (CALIBRATE.RFU_BPF_5_END < 1)
		CALIBRATE.RFU_BPF_5_END = 1;
	if (CALIBRATE.RFU_BPF_5_END > 999999999)
		CALIBRATE.RFU_BPF_5_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_6_START(int8_t direction)
{
	CALIBRATE.RFU_BPF_6_START += direction * 100000;
	if (CALIBRATE.RFU_BPF_6_START < 1)
		CALIBRATE.RFU_BPF_6_START = 1;
	if (CALIBRATE.RFU_BPF_6_START > 999999999)
		CALIBRATE.RFU_BPF_6_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_6_END(int8_t direction)
{
	CALIBRATE.RFU_BPF_6_END += direction * 100000;
	if (CALIBRATE.RFU_BPF_6_END < 1)
		CALIBRATE.RFU_BPF_6_END = 1;
	if (CALIBRATE.RFU_BPF_6_END > 999999999)
		CALIBRATE.RFU_BPF_6_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_7_START(int8_t direction)
{
	CALIBRATE.RFU_BPF_7_START += direction * 100000;
	if (CALIBRATE.RFU_BPF_7_START < 1)
		CALIBRATE.RFU_BPF_7_START = 1;
	if (CALIBRATE.RFU_BPF_7_START > 999999999)
		CALIBRATE.RFU_BPF_7_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_7_END(int8_t direction)
{
	CALIBRATE.RFU_BPF_7_END += direction * 100000;
	if (CALIBRATE.RFU_BPF_7_END < 1)
		CALIBRATE.RFU_BPF_7_END = 1;
	if (CALIBRATE.RFU_BPF_7_END > 999999999)
		CALIBRATE.RFU_BPF_7_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_8_START(int8_t direction)
{
	CALIBRATE.RFU_BPF_8_START += direction * 100000;
	if (CALIBRATE.RFU_BPF_8_START < 1)
		CALIBRATE.RFU_BPF_8_START = 1;
	if (CALIBRATE.RFU_BPF_8_START > 999999999)
		CALIBRATE.RFU_BPF_8_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_BPF_8_END(int8_t direction)
{
	CALIBRATE.RFU_BPF_8_END += direction * 100000;
	if (CALIBRATE.RFU_BPF_8_END < 1)
		CALIBRATE.RFU_BPF_8_END = 1;
	if (CALIBRATE.RFU_BPF_8_END > 999999999)
		CALIBRATE.RFU_BPF_8_END = 999999999;
}

static void SYSMENU_HANDL_CALIB_HPF_START(int8_t direction)
{
	CALIBRATE.RFU_HPF_START += direction * 100000;
	if (CALIBRATE.RFU_HPF_START < 1)
		CALIBRATE.RFU_HPF_START = 1;
	if (CALIBRATE.RFU_HPF_START > 999999999)
		CALIBRATE.RFU_HPF_START = 999999999;
}

static void SYSMENU_HANDL_CALIB_SWR_FWD_RATE_HF(int8_t direction)
{
	CALIBRATE.SWR_FWD_Calibration_HF += (float32_t)direction * 0.1f;
	if (CALIBRATE.SWR_FWD_Calibration_HF < 1.0f)
		CALIBRATE.SWR_FWD_Calibration_HF = 1.0f;
	if (CALIBRATE.SWR_FWD_Calibration_HF > 200.0f)
		CALIBRATE.SWR_FWD_Calibration_HF = 200.0f;
}

static void SYSMENU_HANDL_CALIB_SWR_REF_RATE_HF(int8_t direction)
{
	CALIBRATE.SWR_BWD_Calibration_HF += (float32_t)direction * 0.1f;
	if (CALIBRATE.SWR_BWD_Calibration_HF < 1.0f)
		CALIBRATE.SWR_BWD_Calibration_HF = 1.0f;
	if (CALIBRATE.SWR_BWD_Calibration_HF > 200.0f)
		CALIBRATE.SWR_BWD_Calibration_HF = 200.0f;
}

static void SYSMENU_HANDL_CALIB_SWR_FWD_RATE_6M(int8_t direction)
{
	CALIBRATE.SWR_FWD_Calibration_6M += (float32_t)direction * 0.1f;
	if (CALIBRATE.SWR_FWD_Calibration_6M < 1.0f)
		CALIBRATE.SWR_FWD_Calibration_6M = 1.0f;
	if (CALIBRATE.SWR_FWD_Calibration_6M > 200.0f)
		CALIBRATE.SWR_FWD_Calibration_6M = 200.0f;
}

static void SYSMENU_HANDL_CALIB_SWR_REF_RATE_6M(int8_t direction)
{
	CALIBRATE.SWR_BWD_Calibration_6M += (float32_t)direction * 0.1f;
	if (CALIBRATE.SWR_BWD_Calibration_6M < 1.0f)
		CALIBRATE.SWR_BWD_Calibration_6M = 1.0f;
	if (CALIBRATE.SWR_BWD_Calibration_6M > 200.0f)
		CALIBRATE.SWR_BWD_Calibration_6M = 200.0f;
}

static void SYSMENU_HANDL_CALIB_SWR_FWD_RATE_VHF(int8_t direction)
{
	CALIBRATE.SWR_FWD_Calibration_VHF += (float32_t)direction * 0.1f;
	if (CALIBRATE.SWR_FWD_Calibration_VHF < 1.0f)
		CALIBRATE.SWR_FWD_Calibration_VHF = 1.0f;
	if (CALIBRATE.SWR_FWD_Calibration_VHF > 200.0f)
		CALIBRATE.SWR_FWD_Calibration_VHF = 200.0f;
}

static void SYSMENU_HANDL_CALIB_SWR_REF_RATE_VHF(int8_t direction)
{
	CALIBRATE.SWR_BWD_Calibration_VHF += (float32_t)direction * 0.1f;
	if (CALIBRATE.SWR_BWD_Calibration_VHF < 1.0f)
		CALIBRATE.SWR_BWD_Calibration_VHF = 1.0f;
	if (CALIBRATE.SWR_BWD_Calibration_VHF > 200.0f)
		CALIBRATE.SWR_BWD_Calibration_VHF = 200.0f;
}

static void SYSMENU_HANDL_CALIB_MAX_RF_POWER_ON_METER(int8_t direction)
{
	CALIBRATE.MAX_RF_POWER_ON_METER += direction;
	if (CALIBRATE.MAX_RF_POWER_ON_METER < 5)
		CALIBRATE.MAX_RF_POWER_ON_METER = 5;
	if (CALIBRATE.MAX_RF_POWER_ON_METER > 200)
		CALIBRATE.MAX_RF_POWER_ON_METER = 200;
}

static void SYSMENU_HANDL_CALIB_VCXO(int8_t direction)
{
	CALIBRATE.VCXO_correction += direction;
	
	if (CALIBRATE.VCXO_correction < -32750)
		CALIBRATE.VCXO_correction = -32750;
	if (CALIBRATE.VCXO_correction > 32750)
		CALIBRATE.VCXO_correction = 32750;
}
// Tisho
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
// end Tisho
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

static void SYSMENU_HANDL_CALIB_SSB_POWER_ADDITION(int8_t direction)
{
	if (CALIBRATE.SSB_POWER_ADDITION > 0 || direction > 0)
		CALIBRATE.SSB_POWER_ADDITION += direction;
	if (CALIBRATE.SSB_POWER_ADDITION > 90)
		CALIBRATE.SSB_POWER_ADDITION = 90;
}

static void SYSMENU_HANDL_CALIB_AM_MODULATION_INDEX(int8_t direction)
{
	CALIBRATE.AM_MODULATION_INDEX += direction;
	if (CALIBRATE.AM_MODULATION_INDEX < 1)
		CALIBRATE.AM_MODULATION_INDEX = 1;
	if (CALIBRATE.AM_MODULATION_INDEX > 150)
		CALIBRATE.AM_MODULATION_INDEX = 150;
}

static void SYSMENU_HANDL_CALIB_TUNE_MAX_POWER(int8_t direction)
{
	CALIBRATE.TUNE_MAX_POWER += direction;
	if (CALIBRATE.TUNE_MAX_POWER < 1)
		CALIBRATE.TUNE_MAX_POWER = 1;
	if (CALIBRATE.TUNE_MAX_POWER > 120)
		CALIBRATE.TUNE_MAX_POWER = 120;
	ATU_TunePowerStabilized = false;
}

static void SYSMENU_HANDL_CALIB_RTC_CALIBRATION(int8_t direction)
{
	CALIBRATE.RTC_Calibration += direction;
	if (CALIBRATE.RTC_Calibration < -511)
		CALIBRATE.RTC_Calibration = -511;
	if (CALIBRATE.RTC_Calibration > 511)
		CALIBRATE.RTC_Calibration = 511;

	RTC_Calibration();
}

static void SYSMENU_HANDL_CALIB_RTC_COARSE_CALIBRATION(int8_t direction)
{
	CALIBRATE.RTC_Coarse_Calibration += direction;
	if (CALIBRATE.RTC_Coarse_Calibration < 1)
		CALIBRATE.RTC_Coarse_Calibration = 1;
	if (CALIBRATE.RTC_Coarse_Calibration > 250)
		CALIBRATE.RTC_Coarse_Calibration = 250;

	RTC_Calibration();
}

static void SYSMENU_HANDL_CALIB_EXT_2200m(int8_t direction)
{
	if (CALIBRATE.EXT_2200m > 0 || direction > 0)
		CALIBRATE.EXT_2200m += direction;
	if (CALIBRATE.EXT_2200m > 15)
		CALIBRATE.EXT_2200m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_160m(int8_t direction)
{
	if (CALIBRATE.EXT_160m > 0 || direction > 0)
		CALIBRATE.EXT_160m += direction;
	if (CALIBRATE.EXT_160m > 15)
		CALIBRATE.EXT_160m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_80m(int8_t direction)
{
	if (CALIBRATE.EXT_80m > 0 || direction > 0)
		CALIBRATE.EXT_80m += direction;
	if (CALIBRATE.EXT_80m > 15)
		CALIBRATE.EXT_80m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_60m(int8_t direction)
{
	if (CALIBRATE.EXT_60m > 0 || direction > 0)
		CALIBRATE.EXT_60m += direction;
	if (CALIBRATE.EXT_60m > 15)
		CALIBRATE.EXT_60m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_40m(int8_t direction)
{
	if (CALIBRATE.EXT_40m > 0 || direction > 0)
		CALIBRATE.EXT_40m += direction;
	if (CALIBRATE.EXT_40m > 15)
		CALIBRATE.EXT_40m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_30m(int8_t direction)
{
	if (CALIBRATE.EXT_30m > 0 || direction > 0)
		CALIBRATE.EXT_30m += direction;
	if (CALIBRATE.EXT_30m > 15)
		CALIBRATE.EXT_30m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_20m(int8_t direction)
{
	if (CALIBRATE.EXT_20m > 0 || direction > 0)
		CALIBRATE.EXT_20m += direction;
	if (CALIBRATE.EXT_20m > 15)
		CALIBRATE.EXT_20m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_17m(int8_t direction)
{
	if (CALIBRATE.EXT_17m > 0 || direction > 0)
		CALIBRATE.EXT_17m += direction;
	if (CALIBRATE.EXT_17m > 15)
		CALIBRATE.EXT_17m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_15m(int8_t direction)
{
	if (CALIBRATE.EXT_15m > 0 || direction > 0)
		CALIBRATE.EXT_15m += direction;
	if (CALIBRATE.EXT_15m > 15)
		CALIBRATE.EXT_15m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_12m(int8_t direction)
{
	if (CALIBRATE.EXT_12m > 0 || direction > 0)
		CALIBRATE.EXT_12m += direction;
	if (CALIBRATE.EXT_12m > 15)
		CALIBRATE.EXT_12m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_CB(int8_t direction)
{
	if (CALIBRATE.EXT_CB > 0 || direction > 0)
		CALIBRATE.EXT_CB += direction;
	if (CALIBRATE.EXT_CB > 15)
		CALIBRATE.EXT_CB = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_10m(int8_t direction)
{
	if (CALIBRATE.EXT_10m > 0 || direction > 0)
		CALIBRATE.EXT_10m += direction;
	if (CALIBRATE.EXT_10m > 15)
		CALIBRATE.EXT_10m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_6m(int8_t direction)
{
	if (CALIBRATE.EXT_6m > 0 || direction > 0)
		CALIBRATE.EXT_6m += direction;
	if (CALIBRATE.EXT_6m > 15)
		CALIBRATE.EXT_6m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_4m(int8_t direction)
{
	if (CALIBRATE.EXT_4m > 0 || direction > 0)
		CALIBRATE.EXT_4m += direction;
	if (CALIBRATE.EXT_4m > 15)
		CALIBRATE.EXT_4m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_FM(int8_t direction)
{
	if (CALIBRATE.EXT_FM > 0 || direction > 0)
		CALIBRATE.EXT_FM += direction;
	if (CALIBRATE.EXT_FM > 15)
		CALIBRATE.EXT_FM = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_2m(int8_t direction)
{
	if (CALIBRATE.EXT_2m > 0 || direction > 0)
		CALIBRATE.EXT_2m += direction;
	if (CALIBRATE.EXT_2m > 15)
		CALIBRATE.EXT_2m = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_70cm(int8_t direction)
{
	if (CALIBRATE.EXT_70cm > 0 || direction > 0)
		CALIBRATE.EXT_70cm += direction;
	if (CALIBRATE.EXT_70cm > 15)
		CALIBRATE.EXT_70cm = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_TRANSV_70cm(int8_t direction)
{
	if (CALIBRATE.EXT_TRANSV_70cm > 0 || direction > 0)
		CALIBRATE.EXT_TRANSV_70cm += direction;
	if (CALIBRATE.EXT_TRANSV_70cm > 15)
		CALIBRATE.EXT_TRANSV_70cm = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_TRANSV_23cm(int8_t direction)
{
	if (CALIBRATE.EXT_TRANSV_23cm > 0 || direction > 0)
		CALIBRATE.EXT_TRANSV_23cm += direction;
	if (CALIBRATE.EXT_TRANSV_23cm > 15)
		CALIBRATE.EXT_TRANSV_23cm = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_TRANSV_13cm(int8_t direction)
{
	if (CALIBRATE.EXT_TRANSV_13cm > 0 || direction > 0)
		CALIBRATE.EXT_TRANSV_13cm += direction;
	if (CALIBRATE.EXT_TRANSV_13cm > 15)
		CALIBRATE.EXT_TRANSV_13cm = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_TRANSV_6cm(int8_t direction)
{
	if (CALIBRATE.EXT_TRANSV_6cm > 0 || direction > 0)
		CALIBRATE.EXT_TRANSV_6cm += direction;
	if (CALIBRATE.EXT_TRANSV_6cm > 15)
		CALIBRATE.EXT_TRANSV_6cm = 15;
}

static void SYSMENU_HANDL_CALIB_EXT_TRANSV_3cm(int8_t direction)
{
	if (CALIBRATE.EXT_TRANSV_3cm > 0 || direction > 0)
		CALIBRATE.EXT_TRANSV_3cm += direction;
	if (CALIBRATE.EXT_TRANSV_3cm > 15)
		CALIBRATE.EXT_TRANSV_3cm = 15;
}

static void SYSMENU_HANDL_CALIB_NOTX_NOTHAM(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_NOTHAM = true;
	if (direction < 0)
		CALIBRATE.NOTX_NOTHAM = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_2200m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_2200m = true;
	if (direction < 0)
		CALIBRATE.NOTX_2200m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_160m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_160m = true;
	if (direction < 0)
		CALIBRATE.NOTX_160m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_80m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_80m = true;
	if (direction < 0)
		CALIBRATE.NOTX_80m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_60m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_60m = true;
	if (direction < 0)
		CALIBRATE.NOTX_60m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_40m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_40m = true;
	if (direction < 0)
		CALIBRATE.NOTX_40m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_30m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_30m = true;
	if (direction < 0)
		CALIBRATE.NOTX_30m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_20m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_20m = true;
	if (direction < 0)
		CALIBRATE.NOTX_20m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_17m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_17m = true;
	if (direction < 0)
		CALIBRATE.NOTX_17m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_15m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_15m = true;
	if (direction < 0)
		CALIBRATE.NOTX_15m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_12m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_12m = true;
	if (direction < 0)
		CALIBRATE.NOTX_12m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_CB(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_CB = true;
	if (direction < 0)
		CALIBRATE.NOTX_CB = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_10m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_10m = true;
	if (direction < 0)
		CALIBRATE.NOTX_10m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_6m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_6m = true;
	if (direction < 0)
		CALIBRATE.NOTX_6m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_4m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_4m = true;
	if (direction < 0)
		CALIBRATE.NOTX_4m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_2m(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_2m = true;
	if (direction < 0)
		CALIBRATE.NOTX_2m = false;
}

static void SYSMENU_HANDL_CALIB_NOTX_70cm(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.NOTX_70cm = true;
	if (direction < 0)
		CALIBRATE.NOTX_70cm = false;
}

static void SYSMENU_HANDL_CALIB_ENABLE_60m_band(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENABLE_60m_band = true;
	if (direction < 0)
		CALIBRATE.ENABLE_60m_band = false;

	BANDS[BANDID_60m].selectable = CALIBRATE.ENABLE_60m_band;
}

static void SYSMENU_HANDL_CALIB_ENABLE_4m_band(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENABLE_4m_band = true;
	if (direction < 0)
		CALIBRATE.ENABLE_4m_band = false;

	BANDS[BANDID_4m].selectable = CALIBRATE.ENABLE_4m_band;
}

static void SYSMENU_HANDL_CALIB_ENABLE_AIR_band(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENABLE_AIR_band = true;
	if (direction < 0)
		CALIBRATE.ENABLE_AIR_band = false;

	BANDS[BANDID_AIR].selectable = CALIBRATE.ENABLE_AIR_band;
}

static void SYSMENU_HANDL_CALIB_ENABLE_marine_band(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.ENABLE_marine_band = true;
	if (direction < 0)
		CALIBRATE.ENABLE_marine_band = false;

	BANDS[BANDID_Marine].selectable = CALIBRATE.ENABLE_marine_band;
}

static void SYSMENU_HANDL_CALIB_OTA_update(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.OTA_update = true;
	if (direction < 0)
		CALIBRATE.OTA_update = false;
}

static void SYSMENU_HANDL_CALIB_TX_StartDelay(int8_t direction)
{
	if (CALIBRATE.TX_StartDelay > 0 || direction > 0)
		CALIBRATE.TX_StartDelay += direction;
	if (CALIBRATE.TX_StartDelay > 2000)
		CALIBRATE.TX_StartDelay = 2000;
}

static void SYSMENU_HANDL_CALIB_LCD_Rotate(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.LCD_Rotate = true;
	if (direction < 0)
		CALIBRATE.LCD_Rotate = false;

	LCD_Init();
	LCD_redraw(false);
}

static void SYSMENU_HANDL_CALIB_LinearPowerControl(int8_t direction)
{
	if (direction > 0)
		CALIBRATE.LinearPowerControl = true;
	if (direction < 0)
		CALIBRATE.LinearPowerControl = false;
}

static void SYSMENU_HANDL_INA226_PWR_MON(int8_t direction)
{
	if (direction > 0)
	{
		CALIBRATE.INA226_EN = true;
		INA226_Init();
	}
	if (direction < 0)
		CALIBRATE.INA226_EN = false;
}

static void SYSMENU_HANDL_INA226_CUR_CALL(int8_t direction)
{
	CALIBRATE.INA226_CurCalc += (float32_t)direction * 0.01f;
	if (CALIBRATE.INA226_CurCalc < 0.01f)
		CALIBRATE.INA226_CurCalc = 0.01f;
	if (CALIBRATE.INA226_CurCalc > 10.0f)
		CALIBRATE.INA226_CurCalc = 10.0f;
}

static void SYSMENU_HANDL_CALIB_PWR_VLT_Calibration(int8_t direction)
{
	CALIBRATE.PWR_VLT_Calibration += (float32_t)direction * 1.0f;
	if (CALIBRATE.PWR_VLT_Calibration < 1.0f)
		CALIBRATE.PWR_VLT_Calibration = 1.0f;
	if (CALIBRATE.PWR_VLT_Calibration > 1500.0f)
		CALIBRATE.PWR_VLT_Calibration = 1500.0f;
}

static void SYSMENU_HANDL_CALIB_PWR_CUR_Calibration(int8_t direction)
{
	CALIBRATE.PWR_CUR_Calibration += (float32_t)direction * 0.01f;
	if (CALIBRATE.PWR_CUR_Calibration < 0.01f)
		CALIBRATE.PWR_CUR_Calibration = 0.01f;
	if (CALIBRATE.PWR_CUR_Calibration > 5.0f)
		CALIBRATE.PWR_CUR_Calibration = 5.0f;
}

static void SYSMENU_HANDL_CALIB_ATU_AVERAGING(int8_t direction)
{
	if (CALIBRATE.ATU_AVERAGING > 0 || direction > 0)
		CALIBRATE.ATU_AVERAGING += direction;
	if (CALIBRATE.ATU_AVERAGING > 15)
		CALIBRATE.ATU_AVERAGING = 15;
}

static void SYSMENU_HANDL_CALIB_CAT_Type(int8_t direction)
{
	if (CALIBRATE.CAT_Type > 0 || direction > 0)
		CALIBRATE.CAT_Type += direction;
	if (CALIBRATE.CAT_Type > 1)
		CALIBRATE.CAT_Type = 1;
}

static void SYSMENU_HANDL_CALIB_LNA_compensation(int8_t direction)
{
	CALIBRATE.LNA_compensation += direction;
	if (CALIBRATE.LNA_compensation > 0)
		CALIBRATE.LNA_compensation = 0;
	if (CALIBRATE.LNA_compensation < -50)
		CALIBRATE.LNA_compensation = -50;
}

static void SYSMENU_HANDL_CALIB_TwoSignalTune_Balance(int8_t direction)
{
	if (CALIBRATE.TwoSignalTune_Balance > 0 || direction > 0)
		CALIBRATE.TwoSignalTune_Balance += direction;
	if (CALIBRATE.TwoSignalTune_Balance > 100)
		CALIBRATE.TwoSignalTune_Balance = 100;
}

static void SYSMENU_HANDL_CALIB_FlashGT911(int8_t direction)
{
	#ifdef TOUCHPAD_GT911
	LCD_showError("Flashing GT911...", true);
	LCD_busy = true;
	bool result = GT911_Flash();
	LCD_busy = false;
	if(result)
		LCD_showError("Success!", true);
	else
		LCD_showError("Error!", true);
	#endif
}

static void SYSMENU_HANDL_CALIB_IF_GAIN_MIN(int8_t direction)
{
	if (CALIBRATE.IF_GAIN_MIN > 0 || direction > 0)
		CALIBRATE.IF_GAIN_MIN += direction;
	if (CALIBRATE.IF_GAIN_MIN > 200)
		CALIBRATE.IF_GAIN_MIN = 200;
}

static void SYSMENU_HANDL_CALIB_IF_GAIN_MAX(int8_t direction)
{
	if (CALIBRATE.IF_GAIN_MAX > 0 || direction > 0)
		CALIBRATE.IF_GAIN_MAX += direction;
	if (CALIBRATE.IF_GAIN_MAX > 200)
		CALIBRATE.IF_GAIN_MAX = 200;
}

static void SYSMENU_HANDL_CALIB_SETTINGS_RESET(int8_t direction)
{
	LoadSettings(true);
}

static void SYSMENU_HANDL_CALIB_CALIBRATION_RESET(int8_t direction)
{
	LoadCalibration(true);
}

static void SYSMENU_HANDL_CALIB_WIFI_RESET(int8_t direction)
{
	LoadWiFiSettings(true);
}

// SERVICES
void SYSMENU_HANDL_SERVICESMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_services_opened = true;
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_services_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_services_handlers) / sizeof(sysmenu_services_handlers[0]);
	sysmenu_onroot = false;
	// drawSystemMenu(true);
	LCD_redraw(false);
}

void SYSMENU_SERVICE_FT8_HOTKEY(void)
{
	SYSMENU_HANDL_SERVICESMENU(0);
	uint16_t index = getIndexByName(sysmenu_handlers_selected, sysmenu_item_count, "FT-8 Decoder");
	setCurrentMenuIndex(index);
	#if FT8_SUPPORT
	SYSMENU_HANDL_FT8_Decoder(1);
	#endif
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// SWR ANALYZER
static void SYSMENU_HANDL_SWR_ANALYSER_MENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_swr_analyser_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_swr_analyser_handlers) / sizeof(sysmenu_swr_analyser_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_SWR_CUSTOM_Start(int8_t direction)
{
	if (SYSMENU_swr_opened)
	{
		SWR_EncRotate(direction);
	}
	else
	{
		SYSMENU_swr_opened = true;
		SWR_Start(TRX.SWR_CUSTOM_Begin * 1000, TRX.SWR_CUSTOM_End * 1000);
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
}

static void SYSMENU_HANDL_SWR_CUSTOM_Begin(int8_t direction)
{
	TRX.SWR_CUSTOM_Begin += direction * 100;
	if (TRX.SWR_CUSTOM_Begin < 100)
		TRX.SWR_CUSTOM_Begin = 100;
}

static void SYSMENU_HANDL_SWR_CUSTOM_End(int8_t direction)
{
	TRX.SWR_CUSTOM_End += direction * 100;
	if (TRX.SWR_CUSTOM_End < 100)
		TRX.SWR_CUSTOM_End = 100;
}

// SWR BAND ANALYZER
static void SYSMENU_HANDL_SWR_BAND_START(int8_t direction)
{
	if (SYSMENU_swr_opened)
	{
		SWR_EncRotate(direction);
	}
	else
	{
		SYSMENU_swr_opened = true;
		int8_t band = getBandFromFreq(CurrentVFO->Freq, true);
		SWR_Start(BANDS[band].startFreq - 100000, BANDS[band].endFreq + 100000);
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
}

// SWR HF ANALYZER
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

// SPECTRUM ANALYZER
static void SYSMENU_HANDL_SPECTRUMMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_spectrum_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_spectrum_handlers) / sizeof(sysmenu_spectrum_handlers[0]);
	sysmenu_onroot = false;
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
	if (TRX.SPEC_Begin > 0 || direction > 0)
		TRX.SPEC_Begin += direction;
	if (TRX.SPEC_Begin > MAX_RX_FREQ_HZ)
		TRX.SPEC_Begin = 1;
}

static void SYSMENU_HANDL_SPECTRUM_End(int8_t direction)
{
	if (TRX.SPEC_End > 0 || direction > 0)
		TRX.SPEC_End += direction;
	if (TRX.SPEC_End > MAX_RX_FREQ_HZ)
		TRX.SPEC_End = 1;
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

// Auto calibration
static void SYSMENU_HANDL_AUTO_CALIBRATION(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_auto_calibration_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_auto_calibration_handlers) / sizeof(sysmenu_auto_calibration_handlers[0]);
	sysmenu_onroot = false;
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_AUTO_CALIBRATION_SWR(int8_t direction)
{
	SYSMENU_auto_calibration_opened = true;
	AUTO_CALIBRATION_Start_SWR();
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SYSMENU_HANDL_AUTO_CALIBRATION_POWER(int8_t direction)
{
	SYSMENU_auto_calibration_opened = true;
	AUTO_CALIBRATION_Start_POWER();
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// WSPR Beacon
static void SYSMENU_HANDL_WSPRMENU(int8_t direction)
{
#pragma unused(direction)
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_wspr_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_wspr_handlers) / sizeof(sysmenu_wspr_handlers[0]);
	sysmenu_onroot = false;
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

#if HRDW_HAS_WIFI
// RDA STATS
static void SYSMENU_HANDL_RDA_STATS(int8_t direction)
{
	sysmenu_infowindow_opened = true;
	SYSMENU_drawSystemMenu(true, false);
	WIFI_getRDA();
}

// DX_CLUSTER
static void SYSMENU_HANDL_DX_CLUSTER(int8_t direction)
{
	sysmenu_infowindow_opened = true;
	SYSMENU_drawSystemMenu(true, false);
	WIFI_getDXCluster();
}

// PROPAGINATION
static void SYSMENU_HANDL_PROPAGINATION(int8_t direction)
{
	sysmenu_infowindow_opened = true;
	SYSMENU_drawSystemMenu(true, false);
	WIFI_getPropagination();
}

// DAY/NIGHT MAP
static void SYSMENU_HANDL_DAYNIGHT_MAP(int8_t direction)
{
	sysmenu_infowindow_opened = true;
	SYSMENU_drawSystemMenu(true, false);
	WIFI_getDayNightMap();
}

// IONOGRAM
static void SYSMENU_HANDL_IONOGRAM(int8_t direction)
{
	sysmenu_infowindow_opened = true;
	SYSMENU_drawSystemMenu(true, false);
	WIFI_getIonogram();
}

// RECORD CQ MESSAGE
static void SYSMENU_HANDL_RECORD_CQ_WAV(int8_t direction)
{
	sysmenu_filemanager_opened = true;
	FILEMANAGER_StartRecCQWav();
}
#endif

// LOCATOR INFO
static void SYSMENU_HANDL_LOCATOR_INFO(int8_t direction)
{
	SYSMENU_locator_info_opened = true;
	LOCINFO_Start();
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// CALLSIGN INFO
static void SYSMENU_HANDL_CALLSIGN_INFO(int8_t direction)
{
	SYSMENU_callsign_info_opened = true;
	CALLSIGN_INFO_Start();
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// SELF TEST
static void SYSMENU_HANDL_SELF_TEST(int8_t direction)
{
	SYSMENU_selftest_opened = true;
	SELF_TEST_Start();
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// COMMON MENU FUNCTIONS
void SYSMENU_drawSystemMenu(bool draw_background, bool only_infolines)
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
		if(only_infolines) return;
		SYSMENU_HANDL_SETTIME(0);
		return;
	}
	else 
	#if HRDW_HAS_WIFI
	if (sysmenu_wifi_selectap1_menu_opened)
	{
		if(only_infolines) return;
		SYSMENU_WIFI_DrawSelectAP1Menu(draw_background);
	}
	else if (sysmenu_wifi_selectap2_menu_opened)
	{
		if(only_infolines) return;
		SYSMENU_WIFI_DrawSelectAP2Menu(draw_background);
	}
	else if (sysmenu_wifi_selectap3_menu_opened)
	{
		if(only_infolines) return;
		SYSMENU_WIFI_DrawSelectAP3Menu(draw_background);
	}
	else if (sysmenu_wifi_setAP1password_menu_opened)
	{
		if(only_infolines) return;
		SYSMENU_WIFI_DrawAP1passwordMenu(draw_background);
	}
	else if (sysmenu_wifi_setAP2password_menu_opened)
	{
		if(only_infolines) return;
		SYSMENU_WIFI_DrawAP2passwordMenu(draw_background);
	}
	else if (sysmenu_wifi_setAP3password_menu_opened)
	{
		if(only_infolines) return;
		SYSMENU_WIFI_DrawAP3passwordMenu(draw_background);
	}
	else 
	#endif
	if (sysmenu_trx_setCallsign_menu_opened)
	{
		if(only_infolines) return;
		SYSMENU_TRX_DrawCallsignMenu(draw_background);
	}
	else if (sysmenu_trx_setLocator_menu_opened)
	{
		if(only_infolines) return;
		SYSMENU_TRX_DrawLocatorMenu(draw_background);
	}
	else if (sysmenu_trx_setURSICode_menu_opened)
	{
		if(only_infolines) return;
		SYSMENU_TRX_DrawURSICodeMenu(draw_background);
	}
	else if (SYSMENU_spectrum_opened)
	{
		if(only_infolines) return;
		SPEC_Draw();
	}
	else if (SYSMENU_wspr_opened)
	{
		if(only_infolines) return;
		WSPR_DoEvents();
		return;
	}
#if FT8_SUPPORT
	else if (SYSMENU_FT8_DECODER_opened)
	{
		if (only_infolines) return;
		if (draw_background) {
			InitFT8_Decoder();
			LCD_UpdateQuery.SystemMenuRedraw = false;
		}
		MenagerFT8();
		return;
	}
#endif
	else if (SYSMENU_TDM_CTRL_opened)
	{
		if(only_infolines) return;
		TDM_Voltages();
	}
	else if (SYSMENU_swr_opened)
	{
		if(only_infolines) return;
		SWR_Draw();
	}
	else if (SYSMENU_locator_info_opened)
	{
		if(only_infolines) return;
		LOCINFO_Draw();
	}
	else if (SYSMENU_callsign_info_opened)
	{
		if(only_infolines) return;
		CALLSIGN_INFO_Draw();
	}
	else if (SYSMENU_selftest_opened)
	{
		if(only_infolines) return;
		SELF_TEST_Draw();
		return;
	}
	else if (SYSMENU_auto_calibration_opened)
	{
		if(only_infolines) return;
		AUTO_CALIBRATION_Draw();
		return;
	}
	else if (sysmenu_sysinfo_opened)
	{
		if(only_infolines) return;
		SYSMENU_HANDL_SYSINFO(0);
		return;
	}
	else if (sysmenu_filemanager_opened)
	{
		if(only_infolines) return;
		FILEMANAGER_Draw(draw_background);
		return;
	}
	#if HRDW_HAS_SD
	else if (sysmenu_ota_opened)
	{
		if(only_infolines) return;
		FILEMANAGER_OTAUpdate_handler();
		return;
	}
	#endif
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

		uint8_t current_selected_page = SYSTMENU_getPageFromRealIndex(getCurrentMenuIndex());
		if (current_selected_page * LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE > sysmenu_item_count)
			current_selected_page = 0;

		uint8_t visible = 0;
		uint8_t current_page = 0;
		for (uint8_t m = 0; m < sysmenu_item_count; m++)
		{
			if (sysmenu_handlers_selected[m].checkVisibleHandler == NULL || sysmenu_handlers_selected[m].checkVisibleHandler())
			{
				if(!only_infolines) {
					if (current_selected_page == current_page)
						drawSystemMenuElement(&sysmenu_handlers_selected[m], false);
				} 
				else 
				{
					if (current_selected_page == current_page) {
						if(sysmenu_handlers_selected[m].type == SYSMENU_INFOLINE) {
							drawSystemMenuElement(&sysmenu_handlers_selected[m], false);
						} else { // dummy pass
							sysmenu_i++;
							sysmenu_y += LAYOUT->SYSMENU_ITEM_HEIGHT;
						}
					}
				}

				visible++;
				if (visible >= LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE)
				{
					visible = 0;
					current_page++;
				}
			}
		}

		LCD_UpdateQuery.SystemMenuInfolines = false;
		LCD_busy = false;
	}

	LCD_UpdateQuery.SystemMenu = false;
	if (draw_background)
		LCD_UpdateQuery.SystemMenuRedraw = false;
}

void SYSMENU_eventRotateSystemMenu(int8_t direction)
{
	if (direction < -1)
		direction = -1;
	if (direction > 1)
		direction = 1;
	
	#if HRDW_HAS_WIFI
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
	#endif
	
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
	if (sysmenu_trx_setURSICode_menu_opened)
	{
		SYSMENU_TRX_RotateURSICodeChar(direction);
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
	if (SYSMENU_auto_calibration_opened)
	{
		AUTO_CALIBRATION_EncRotate(direction);
		return;
	}
	if(sysmenu_infowindow_opened)
	{
		return;
	}
	
	if (sysmenu_handlers_selected[getCurrentMenuIndex()].type != SYSMENU_INFOLINE)
		sysmenu_handlers_selected[getCurrentMenuIndex()].menuHandler(direction);
	if (sysmenu_handlers_selected[getCurrentMenuIndex()].type != SYSMENU_RUN)
		LCD_UpdateQuery.SystemMenuCurrent = true;
}

void SYSMENU_eventCloseSystemMenu(void)
{
	#if HRDW_HAS_WIFI
	if (sysmenu_wifi_selectap1_menu_opened)
	{
		sysmenu_wifi_selectap1_menu_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
		NeedSaveWiFi = true;
	}
	if (sysmenu_wifi_selectap2_menu_opened)
	{
		sysmenu_wifi_selectap2_menu_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
		NeedSaveWiFi = true;
	}
	if (sysmenu_wifi_selectap3_menu_opened)
	{
		sysmenu_wifi_selectap3_menu_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
		NeedSaveWiFi = true;
	}
	else if (sysmenu_wifi_setAP1password_menu_opened)
	{
		sysmenu_wifi_setAP1password_menu_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
		NeedSaveWiFi = true;
	}
	else if (sysmenu_wifi_setAP2password_menu_opened)
	{
		sysmenu_wifi_setAP2password_menu_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
		NeedSaveWiFi = true;
	}
	else if (sysmenu_wifi_setAP3password_menu_opened)
	{
		sysmenu_wifi_setAP3password_menu_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		WIFI_State = WIFI_CONFIGURED;
		NeedSaveWiFi = true;
	}
	else 
	#endif
	if (sysmenu_trx_setCallsign_menu_opened)
	{
		sysmenu_trx_setCallsign_menu_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_trx_setLocator_menu_opened)
	{
		sysmenu_trx_setLocator_menu_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_trx_setURSICode_menu_opened)
	{
		sysmenu_trx_setURSICode_menu_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_spectrum_opened)
	{
		SYSMENU_spectrum_opened = false;
		SPEC_Stop();
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_wspr_opened)
	{
		SYSMENU_wspr_opened = false;
		WSPR_Stop();
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_timeMenuOpened)
	{
		sysmenu_timeMenuOpened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_swr_opened)
	{
		SYSMENU_swr_opened = false;
		SWR_Stop();
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_locator_info_opened)
	{
		SYSMENU_locator_info_opened = false;
		LOCINFO_Stop();
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_callsign_info_opened)
	{
		SYSMENU_callsign_info_opened = false;
		CALLSIGN_INFO_Stop();
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_selftest_opened)
	{
		SYSMENU_selftest_opened = false;
		SELF_TEST_Stop();
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (SYSMENU_auto_calibration_opened)
	{
		SYSMENU_auto_calibration_opened = false;
		AUTO_CALIBRATION_Stop();
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
#if FT8_SUPPORT
	else if (SYSMENU_FT8_DECODER_opened)
	{
		FT8_DecodeActiveFlg = false;
		SYSMENU_FT8_DECODER_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
#endif
	else if (SYSMENU_TDM_CTRL_opened)
	{
		SYSMENU_TDM_CTRL_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_infowindow_opened)
	{
		sysmenu_infowindow_opened = false;
		sysmenu_sysinfo_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_filemanager_opened)
	{
		FILEMANAGER_Closing();
		sysmenu_filemanager_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	else if (sysmenu_services_opened)
	{
		sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_handlers[0];
		sysmenu_item_count = sizeof(sysmenu_handlers) / sizeof(sysmenu_handlers[0]);
		sysmenu_onroot = true;
		sysmenu_services_opened = false;
		LCD_systemMenuOpened = false;
		LCD_UpdateQuery.Background = true;
		LCD_redraw(false);
	}
	else if (sysmenu_ota_opened)
	{
		sysmenu_ota_opened = false;
		LCD_showInfo("OTA cancelled", true);
		LCD_UpdateQuery.SystemMenuRedraw = true;
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
			if (sysmenu_handlers_selected == (const struct sysmenu_item_handler *)&sysmenu_calibration_handlers[0]) // exit from calibration
				NeedSaveCalibration = true;

			sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_handlers[0];
			sysmenu_item_count = sizeof(sysmenu_handlers) / sizeof(sysmenu_handlers[0]);
			sysmenu_onroot = true;
			SYSMENU_drawSystemMenu(true, false);
		}
	}
	LCD_hideKeyboard();
	#if HRDW_HAS_WIFI
	WIFI_AbortCallback();
	#endif
	sysmenu_item_selected_by_enc2 = false;
	NeedSaveSettings = true;

	#if HRDW_HAS_SD
	if (SD_USBCardReader)
	{
		SD_USBCardReader = false;
		USBD_Restart();
		TRX_Mute = false;
	}
	#endif
}

void SYSMENU_eventCloseAllSystemMenu(void)
{
	if (sysmenu_ota_opened)
	{
		sysmenu_ota_opened = false;
		LCD_showInfo("OTA cancelled", true);
	}
	sysmenu_handlers_selected = (const struct sysmenu_item_handler *)&sysmenu_handlers[0];
	sysmenu_item_count = sizeof(sysmenu_handlers) / sizeof(sysmenu_handlers[0]);
	sysmenu_onroot = true;
	sysmenu_item_selected_by_enc2 = false;
	LCD_systemMenuOpened = false;
	LCD_hideKeyboard();
	#if HRDW_HAS_WIFI
	WIFI_AbortCallback();
	#endif
	LCD_UpdateQuery.Background = true;

#if FT8_SUPPORT
	if (SYSMENU_FT8_DECODER_opened) // Tisho
	{
		FT8_DecodeActiveFlg = false;
		SYSMENU_FT8_DECODER_opened = false;
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
#endif

	LCD_redraw(false);
}

// secondary encoder click
void SYSMENU_eventSecEncoderClickSystemMenu(void)
{
	#if FT8_SUPPORT
	if (SYSMENU_FT8_DECODER_opened)
	{
		FT8_Enc2Click();
		return;
	}
	#endif
	if (SYSMENU_auto_calibration_opened)
	{
		AUTO_CALIBRATION_Enc2Click();
		return;
	}
	
	if (sysmenu_handlers_selected[getCurrentMenuIndex()].type == SYSMENU_MENU || sysmenu_handlers_selected[getCurrentMenuIndex()].type == SYSMENU_RUN || sysmenu_handlers_selected[getCurrentMenuIndex()].type == SYSMENU_INFOLINE)
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

// secondary encoder rotate
void SYSMENU_eventSecRotateSystemMenu(int8_t direction)
{
	#if HRDW_HAS_WIFI
	// wifi select AP menu
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
	
	// wifi set password menu
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
	#endif
	
	// Callsign menu
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
	// Locator menu
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
	// URSI Code menu
	if (sysmenu_trx_setURSICode_menu_opened)
	{
		if (direction < 0 && sysmenu_trx_selected_ursi_code_char_index > 0)
		{
			sysmenu_trx_selected_ursi_code_char_index--;
			SYSMENU_TRX_DrawURSICodeMenu(true);
		}
		else if (sysmenu_trx_selected_ursi_code_char_index < (MAX_CALLSIGN_LENGTH - 1))
		{
			sysmenu_trx_selected_ursi_code_char_index++;
			SYSMENU_TRX_DrawURSICodeMenu(true);
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
	if (SYSMENU_locator_info_opened)
		return;
	if (SYSMENU_callsign_info_opened)
		return;
	if (SYSMENU_selftest_opened)
	{
		SELF_TEST_EncRotate(direction);
		return;
	}
	if (SYSMENU_auto_calibration_opened)
	{
		AUTO_CALIBRATION_Enc2Rotate(direction);
		return;
	}
#if FT8_SUPPORT
	if (SYSMENU_FT8_DECODER_opened) // Tisho
	{
		FT8_Enc2Rotate(direction);
		return;
	}
#endif
	if (sysmenu_infowindow_opened)
		return;
	// time menu
	if (sysmenu_timeMenuOpened)
	{
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
	// File manager
	if (sysmenu_filemanager_opened)
	{
		FILEMANAGER_EventSecondaryRotate(direction);
		return;
	}
	// other
	if (sysmenu_item_selected_by_enc2) // selected by secondary encoder
	{
		SYSMENU_eventRotateSystemMenu(direction);
		return;
	}

	// clear selection line
	LCD_busy = true;
	sysmenu_y = 5 + SYSTMENU_getVisibleIdFromReal(getCurrentMenuIndex()) * LAYOUT->SYSMENU_ITEM_HEIGHT;
	LCDDriver_drawFastHLine(0, sysmenu_y + LAYOUT->SYSMENU_ITEM_HEIGHT - 1, LAYOUT->SYSMENU_W, BG_COLOR);
	LCD_busy = false;
	// current page
	uint8_t current_selected_page = SYSTMENU_getPageFromRealIndex(getCurrentMenuIndex());
	// do moving
	if (direction < 0)
	{
		if (getCurrentMenuIndex() > 0)
			setCurrentMenuIndex(getCurrentMenuIndex() - 1);
		else
			setCurrentMenuIndex(sysmenu_item_count - 1);

		while (sysmenu_handlers_selected[getCurrentMenuIndex()].type == SYSMENU_INFOLINE || (sysmenu_handlers_selected[getCurrentMenuIndex()].checkVisibleHandler != NULL && !sysmenu_handlers_selected[getCurrentMenuIndex()].checkVisibleHandler()))
		{
			if (getCurrentMenuIndex() == 0)
				setCurrentMenuIndex(sysmenu_item_count - 1);
			else
				setCurrentMenuIndex(getCurrentMenuIndex() - 1);
		}
	}
	else
	{
		setCurrentMenuIndex(getCurrentMenuIndex() + 1);
		if (getCurrentMenuIndex() >= sysmenu_item_count)
			setCurrentMenuIndex(0);

		while (sysmenu_handlers_selected[getCurrentMenuIndex()].type == SYSMENU_INFOLINE || (sysmenu_handlers_selected[getCurrentMenuIndex()].checkVisibleHandler != NULL && !sysmenu_handlers_selected[getCurrentMenuIndex()].checkVisibleHandler()))
		{
			if (getCurrentMenuIndex() >= sysmenu_item_count - 1)
				setCurrentMenuIndex(0);
			else
				setCurrentMenuIndex(getCurrentMenuIndex() + 1);
		}
	}
	if (getCurrentMenuIndex() >= sysmenu_item_count)
		setCurrentMenuIndex(0);

	LCD_UpdateQuery.SystemMenuCurrent = true;

	// pager
	uint8_t new_page = SYSTMENU_getPageFromRealIndex(getCurrentMenuIndex());
	if (current_selected_page != new_page)
		LCD_UpdateQuery.SystemMenuRedraw = true;
}

void SYSMENU_redrawCurrentItem(void)
{
	sysmenu_i = SYSTMENU_getVisibleIdFromReal(getCurrentMenuIndex());
	sysmenu_y = 5 + SYSTMENU_getVisibleIdFromReal(getCurrentMenuIndex()) * LAYOUT->SYSMENU_ITEM_HEIGHT;
	drawSystemMenuElement(&sysmenu_handlers_selected[getCurrentMenuIndex()], true);
}

static void drawSystemMenuElement(const struct sysmenu_item_handler *menuElement, bool onlyVal)
{
	if (menuElement->checkVisibleHandler != NULL && !menuElement->checkVisibleHandler())
		return;
	char ctmp[32] = {0};
	char enum_value[ENUM_MAX_LENGTH + 1] = {0};
	float32_t *atu_i = ATU_I_VALS;
	float32_t *atu_c = ATU_C_VALS;
	float32_t float_tmp_val = 0;

	if (!onlyVal)
	{
		LCDDriver_Fill_RectXY(0, sysmenu_y, LAYOUT->SYSMENU_W, sysmenu_y + 17, BG_COLOR);
		LCDDriver_printText(menuElement->title, LAYOUT->SYSMENU_X1, sysmenu_y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	}

	uint16_t x_pos = LAYOUT->SYSMENU_X2 - RASTR_FONT_W * LAYOUT->SYSMENU_FONT_SIZE * ENUM_MAX_LENGTH;
	float32_t tmp_float = 0;
	uint32_t tmp_uint32 = 0;
	switch (menuElement->type)
	{
	case SYSMENU_UINT8:
		sprintf(ctmp, "%d", (uint8_t)*menuElement->value);
		break;
	case SYSMENU_B4:
		sprintf(ctmp, "%d%d%d%d", bitRead((uint8_t)*menuElement->value, 3), bitRead((uint8_t)*menuElement->value, 2), bitRead((uint8_t)*menuElement->value, 1), bitRead((uint8_t)*menuElement->value, 0));
		break;
	case SYSMENU_ENUM:
	case SYSMENU_ENUMR:
		strncpy(enum_value, menuElement->enumerate[(uint8_t)*menuElement->value], ENUM_MAX_LENGTH);
		sprintf(ctmp, "%s", enum_value);
		break;
	case SYSMENU_UINT16:
		sprintf(ctmp, "%d", (uint16_t)*menuElement->value);
		break;
	case SYSMENU_UINT32:
  case SYSMENU_UINT32R:
		tmp_uint32 = (uint32_t)*menuElement->value;
		while((uint32_t)(pow(10, ENUM_MAX_LENGTH) - 1) <= tmp_uint32) {
			tmp_uint32 /= 10;
		}
		sprintf(ctmp, "%u", tmp_uint32);
		break;
	case SYSMENU_UINT64:
		sprintf(ctmp, "%llu", (uint64_t)*menuElement->value);
		break;
	case SYSMENU_INT8:
		sprintf(ctmp, "%d", (int8_t)*menuElement->value);
		break;
	case SYSMENU_INT16:
		sprintf(ctmp, "%d", (int16_t)*menuElement->value);
		break;
	case SYSMENU_INT32:
		sprintf(ctmp, "%d", (int32_t)*menuElement->value);
		break;
	case SYSMENU_FLOAT32:
		dma_memcpy(&tmp_float, menuElement->value, sizeof(float32_t));
		sprintf(ctmp, "%.2f", (double)tmp_float);
		break;
	case SYSMENU_BOOLEAN:
		sprintf(ctmp, "%d", (int8_t)*menuElement->value);
		if ((uint8_t)*menuElement->value == 1)
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
	case SYSMENU_INFOLINE:
		break;
	case SYSMENU_FUNCBUTTON:
#if HRDW_HAS_FUNCBUTTONS
		sprintf(ctmp, "%s", (const char *)PERIPH_FrontPanel_FuncButtonsList[(uint8_t)*menuElement->value].name);
#endif
		break;
	case SYSMENU_ATU_I:
		float_tmp_val = 0;
		for (uint8_t i = 0; i < ATU_MAXPOS; i++)
		{
			if (bitRead((uint8_t)*menuElement->value, i))
				float_tmp_val += atu_i[i + 1];
		}
		sprintf(ctmp, "%.2fuH", (double)float_tmp_val);
		break;
	case SYSMENU_ATU_C:
		float_tmp_val = 0;
		for (uint8_t i = 0; i < ATU_MAXPOS; i++)
		{
			if (bitRead((uint8_t)*menuElement->value, i))
				float_tmp_val += atu_c[i + 1];
		}
		sprintf(ctmp, "%dpF", (uint32_t)float_tmp_val);
		break;
	}

	if (menuElement->type != SYSMENU_INFOLINE)
	{
		addSymbols(ctmp, ctmp, ENUM_MAX_LENGTH, " ", false);
		LCDDriver_printText(ctmp, x_pos, sysmenu_y, FG_COLOR, BG_COLOR, LAYOUT->SYSMENU_FONT_SIZE);
	}

	if (SYSTMENU_getVisibleIdFromReal(getCurrentMenuIndex()) == sysmenu_i)
	{
		if (sysmenu_item_selected_by_enc2)
			LCDDriver_drawFastHLine(0, sysmenu_y + LAYOUT->SYSMENU_ITEM_HEIGHT - 1, LAYOUT->SYSMENU_W, COLOR->BUTTON_TEXT);
		else
			LCDDriver_drawFastHLine(0, sysmenu_y + LAYOUT->SYSMENU_ITEM_HEIGHT - 1, LAYOUT->SYSMENU_W, FG_COLOR);
	}
	sysmenu_i++;
	sysmenu_y += LAYOUT->SYSMENU_ITEM_HEIGHT;
}

static uint8_t SYSTMENU_getVisibleIdFromReal(uint8_t realIndex)
{
	uint8_t visible = 0;
	for (uint8_t i = 0; i < getCurrentMenuIndex(); i++)
	{
		if (sysmenu_handlers_selected[i].checkVisibleHandler == NULL || sysmenu_handlers_selected[i].checkVisibleHandler())
		{
			visible++;
			if (visible >= LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE)
			{
				visible = 0;
			}
		}
	}
	return visible;
}

static uint8_t SYSTMENU_getPageFromRealIndex(uint8_t realIndex)
{
	uint8_t visible = 0;
	uint8_t page = 0;
	for (uint8_t i = 0; i < getCurrentMenuIndex(); i++)
	{
		if (sysmenu_handlers_selected[i].checkVisibleHandler == NULL || sysmenu_handlers_selected[i].checkVisibleHandler())
		{
			visible++;
			if (visible >= LAYOUT->SYSMENU_MAX_ITEMS_ON_PAGE)
			{
				visible = 0;
				page++;
			}
		}
	}
	return page;
}

static bool SYSMENU_HANDL_CHECK_HAS_LPF(void)
{
	switch (CALIBRATE.RF_unit_type)
	{
	case RF_UNIT_QRP:
		return true;
	case RF_UNIT_RU4PN:
		return true;
	case RF_UNIT_WF_100D:
		return true;
	case RF_UNIT_BIG:
		return false;
	case RF_UNIT_SPLIT:
		return false;
	}

	return false;
}

static bool SYSMENU_HANDL_CHECK_HAS_HPF(void)
{
	switch (CALIBRATE.RF_unit_type)
	{
	case RF_UNIT_QRP:
		return true;
	case RF_UNIT_RU4PN:
		return true;
	case RF_UNIT_WF_100D:
		return true;
	case RF_UNIT_BIG:
		return false;
	case RF_UNIT_SPLIT:
		return false;
	}

	return false;
}

static bool SYSMENU_HANDL_CHECK_HAS_BPF_8(void)
{
	switch (CALIBRATE.RF_unit_type)
	{
	case RF_UNIT_QRP:
		return false;
	case RF_UNIT_RU4PN:
		return false;
	case RF_UNIT_WF_100D:
		return true;
	case RF_UNIT_BIG:
		return true;
	case RF_UNIT_SPLIT:
		return true;
	}

	return false;
}

static bool SYSMENU_HANDL_CHECK_HAS_BPF_9(void)
{
	switch (CALIBRATE.RF_unit_type)
	{
	case RF_UNIT_QRP:
		return false;
	case RF_UNIT_RU4PN:
		return false;
	case RF_UNIT_WF_100D:
		return false;
	case RF_UNIT_BIG:
		return true;
	case RF_UNIT_SPLIT:
		return true;
	}

	return false;
}

bool SYSMENU_HANDL_CHECK_HAS_ATU(void)
{
	switch (CALIBRATE.RF_unit_type)
	{
	case RF_UNIT_QRP:
		return false;
	case RF_UNIT_RU4PN:
		return true;
	case RF_UNIT_WF_100D:
		return true;
	case RF_UNIT_BIG:
		return true;
	case RF_UNIT_SPLIT:
		return true;
	}

	return false;
}

static bool SYSMENU_HANDL_CHECK_HAS_RFFILTERS_BYPASS(void)
{
	switch (CALIBRATE.RF_unit_type)
	{
	case RF_UNIT_QRP:
		return true;
	case RF_UNIT_RU4PN:
		return true;
	case RF_UNIT_WF_100D:
		return true;
	case RF_UNIT_BIG:
		return false;
	case RF_UNIT_SPLIT:
		return false;
	}

	return false;
}

static bool SYSMENU_HANDL_CHECK_HIDDEN_ENABLED(void)
{
	return SYSMENU_hiddenmenu_enabled;
}

static void setCurrentMenuIndex(uint8_t index)
{
	uint8_t count = sizeof(sysmenu_wrappers) / sizeof(sysmenu_wrappers[0]);
	for (uint8_t i = 0; i < count; i++)
		if (sysmenu_wrappers[i].menu_handler == sysmenu_handlers_selected)
			sysmenu_wrappers[i].currentIndex = index;
}

static uint8_t getCurrentMenuIndex(void)
{
	uint8_t count = sizeof(sysmenu_wrappers) / sizeof(sysmenu_wrappers[0]);
	for (uint8_t i = 0; i < count; i++)
		if (sysmenu_wrappers[i].menu_handler == sysmenu_handlers_selected)
			return sysmenu_wrappers[i].currentIndex;

	return 0;
}

static uint16_t getIndexByName(const struct sysmenu_item_handler *menu, uint16_t menu_length, char* name) 
{
	for(uint16_t i = 0; i < menu_length; i++)
		if(strcmp(menu[i].title, name) == 0)
			return i;
	return 0;
}
