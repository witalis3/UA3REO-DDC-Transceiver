#ifndef SETTINGS_h
#define SETTINGS_h

#include "bands.h"
#include "functions.h"
#include "hardware.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STM32_VERSION_STR "8.2.0" // current STM32 version

#if defined(FRONTPANEL_LITE_V2_MINI) || defined(FRONTPANEL_MINI)
#define FPGA_VERSION_STR "6.8.0" // needed FPGA version Wolf-Mini
#elif defined(FRONTPANEL_LITE)
#define FPGA_VERSION_STR "8.0.0" // needed FPGA version Wolf-Lite
#else
#define FPGA_VERSION_STR "8.2.0" // needed FPGA version Wolf/Wolf-2/Wolf-X1
#endif

#define SETT_VERSION 103        // Settings config version
#define CALIB_VERSION 68        // Calibration config version
#define WIFI_SETTINGS_VERSION 5 // WiFi config version

#define TRX_SAMPLERATE 48000                  // audio stream sampling rate during processing and TX (NOT RX!)
#define MAX_TX_AMPLITUDE_MULT 0.85f           // Maximum amplitude when transmitting to FPGA
#define AGC_CLIPPING 6.0f                     // Limit over target in AGC, dB
#define TOUCHPAD_DELAY 200                    // Anti-bounce time for pressing the touchpad
#define AUTOGAIN_TARGET_AMPLITUDE 20000.0f    // target autocorrector amplitude
#define AUTOGAIN_MAX_AMPLITUDE 30000.0f       // maximum autocorrector amplitude
#define AUTOGAIN_CORRECTOR_WAITSTEP 5         // waiting for the averaging of the results when the auto-corrector of the input circuits is running
#define KEY_HOLD_TIME 500                     // time of long pressing of the keyboard button for triggering, ms
#define SHOW_LOGO true                        // Show logo on boot (from images.h)
#define POWERDOWN_TIMEOUT 1000                // time of pressing the shutdown button, for operation, ms
#define POWERDOWN_FORCE_TIMEOUT 2000          // force time
#define USB_RESTART_TIMEOUT 5000              // time after which USB restart occurs if there are no packets
#define SNTP_SYNC_INTERVAL (60 * 2)           // Time synchronization interval via NTP, sec
#define SCANNER_NOSIGNAL_TIME 50              // time to continue sweeping if signal too low
#define SCANNER_SIGNAL_TIME_FM 5000           // time to continue sweeping if signal founded for FM
#define SCANNER_SIGNAL_TIME_OTHER 1000        // time to continue sweeping if signal founded for SSB
#define SCANNER_FREQ_STEP_WFM 100000          // step for freq scanner for WFM
#define SCANNER_FREQ_STEP_NFM 25000           // step for freq scanner for NFM
#define SCANNER_FREQ_STEP_OTHER 500           // step for freq scanner for SSB
#define ENCODER_MIN_RATE_ACCELERATION 2.0f    // encoder enable rounding if lower than value
#define DXCLUSTER_UPDATE_TIME (1000 * 60 * 1) // interval to get cluster info, 1min
#define NORMAL_SWR_SAVED 1.5f                 // ATU SWR target for saved settings
#define NORMAL_SWR_TUNE 1.2f                  // ATU SWR target for new tune
#define IDLE_LCD_BRIGHTNESS 5                 // Low brightness for IDLE mode (dimmer)
#define CW_ADD_GAIN_IF 20.0f                  // additional IF gain in CW
#define CW_ADD_GAIN_AF 10.0f                  // additional AF gain in CW
#define TX_LPF_TIMEOUT (180 * 1000)           // TX LPF On Timeout, millisec (3 min)
#define SWR_PROTECTOR_MAX_POWER 20.0f         // drop down to PWR %, if SWR high

#define FULL_DUPLEX TRX.Full_Duplex             // Enable duplex RX and TX
#define SHOW_RX_FFT_ON_TX FULL_DUPLEX           // Show RX FFT channel on TX
#define LISTEN_RX_AUDIO_ON_TX SHOW_RX_FFT_ON_TX // Process audio RX channel for TX mode

#define CODEC_BITS_FULL_SCALE 4294967296 // maximum signal amplitude in the bus // powf (2, FPGA_BUS_BITS)
#define USB_DEBUG_ENABLED true           // allow using USB as a console
#define SWD_DEBUG_ENABLED false          // enable SWD as a console
#define LCD_DEBUG_ENABLED false          // enable LCD as a console

#define AUTOGAINER_TAGET (ADC_FULL_SCALE / 3)
#define AUTOGAINER_HYSTERESIS (ADC_FULL_SCALE / 13)

#define MAX_WIFIPASS_LENGTH 32
#define MAX_CALLSIGN_LENGTH 16
#define MAX_CW_MACROS_LENGTH 61
#define ALLQSO_TOKEN_SIZE 16

#define W25Q16_COMMAND_Write_Disable 0x04
#define W25Q16_COMMAND_Write_Enable 0x06
#define W25Q16_COMMAND_Erase_Chip 0xC7
#define W25Q16_COMMAND_Sector_Erase 0x20
#define W25Q16_COMMAND_32KBlock_Erase 0x52
#define W25Q16_COMMAND_Page_Program 0x02
#define W25Q16_COMMAND_Read_Data 0x03
#define W25Q16_COMMAND_FastRead_Data 0x0B
#define W25Q16_COMMAND_Power_Down 0xB9
#define W25Q16_COMMAND_Power_Up 0xAB
#define W25Q16_COMMAND_GetStatus 0x05
#define W25Q16_COMMAND_WriteStatus 0x01
#define W25Q16_PAGE_SIZE 256
#define W25Q16_SECTOR_SIZE 4096

#define EEPROM_SECTOR_CALIBRATION 0
#define EEPROM_SECTOR_SETTINGS 4
#define EEPROM_SECTOR_WIFI 8
#define EEPROM_SECTOR_DPD 12
#define EEPROM_REPEAT_TRYES 10 // command tryes

#define MEMORY_CHANNELS_COUNT 35
#define BANDS_MEMORIES_COUNT 3

#define ATU_MAXLENGTH 7
#define ATU_0x0_MAXPOS B8(00000000)
#define ATU_5x5_MAXPOS B8(00011111)
#define ATU_6x6_MAXPOS B8(00111111)
#define ATU_7x7_MAXPOS B8(01111111)

#ifdef FRONTPANEL_MINI
static float32_t ATU_I_VALS[ATU_MAXLENGTH + 1] = {0.0, 0.1, 0.22, 0.45, 1.0, 2.2, 4.5, 4.5};
static float32_t ATU_C_VALS[ATU_MAXLENGTH + 1] = {0.0, 22.0, 47.0, 100.0, 220.0, 470.0, 1000.0, 1000.0};
#else
static float32_t ATU_I_VALS[ATU_MAXLENGTH + 1] = {0.0, 0.05, 0.1, 0.22, 0.45, 1.0, 2.2, 4.4};
static float32_t ATU_C_VALS[ATU_MAXLENGTH + 1] = {0.0, 10.0, 22.0, 47.0, 100.0, 220.0, 470.0, 1000.0};
#endif

// FRONT PANELS
#ifdef FRONTPANEL_NONE
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_COUNT 1
#define FUNCBUTTONS_ON_PAGE 1
#define FUNCBUTTONS_PAGES 1
static char ota_config_frontpanel[] = "NONE";
#define ATU_MAXPOS ATU_0x0_MAXPOS
#endif

#ifdef FRONTPANEL_SMALL_V1
#define HRDW_MCP3008_1 true
#define HRDW_MCP3008_2 true
#define HRDW_MCP3008_3 true
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_COUNT 1
#define FUNCBUTTONS_ON_PAGE 1
#define FUNCBUTTONS_PAGES 1
static char ota_config_frontpanel[] = "SMALL";
#define ATU_MAXPOS                                                                                                                 \
	((CALIBRATE.RF_unit_type == RF_UNIT_BIG || CALIBRATE.RF_unit_type == RF_UNIT_RU4PN || CALIBRATE.RF_unit_type == RF_UNIT_KT_100S) \
	     ? ATU_5x5_MAXPOS                                                                                                            \
	     : ((CALIBRATE.RF_unit_type == RF_UNIT_SPLIT) ? ATU_7x7_MAXPOS : ATU_0x0_MAXPOS))
#endif

#ifdef FRONTPANEL_LITE
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 100.0f
#define FUNCBUTTONS_ON_PAGE 5
#define FUNCBUTTONS_PAGES 7
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 1)
static char ota_config_frontpanel[] = "LITE";
#define ATU_MAXPOS ATU_0x0_MAXPOS
#endif

#ifdef FRONTPANEL_BIG_V1
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_ON_PAGE 8
#define FUNCBUTTONS_PAGES 5
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 5)
static char ota_config_frontpanel[] = "BIG";
#define ATU_MAXPOS                                                                                                                 \
	((CALIBRATE.RF_unit_type == RF_UNIT_BIG || CALIBRATE.RF_unit_type == RF_UNIT_RU4PN || CALIBRATE.RF_unit_type == RF_UNIT_KT_100S) \
	     ? ATU_5x5_MAXPOS                                                                                                            \
	     : ((CALIBRATE.RF_unit_type == RF_UNIT_SPLIT) ? ATU_7x7_MAXPOS : ATU_0x0_MAXPOS))
#endif

#ifdef FRONTPANEL_WF_100D
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_ON_PAGE 9
#define FUNCBUTTONS_PAGES 4
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 6)
static char ota_config_frontpanel[] = "WF_100D";
#define ATU_MAXPOS ATU_7x7_MAXPOS
#endif

#ifdef FRONTPANEL_WOLF_2
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 1024.0f
#define FUNCBUTTONS_ON_PAGE 9
#define FUNCBUTTONS_PAGES 4
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 6)
static char ota_config_frontpanel[] = "WOLF-2";
#define ATU_MAXPOS ATU_7x7_MAXPOS
#endif

#ifdef FRONTPANEL_X1
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 100.0f
#define FUNCBUTTONS_ON_PAGE 4
#define FUNCBUTTONS_PAGES 9
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 2)
static char ota_config_frontpanel[] = "X1";
#define ATU_MAXPOS ATU_0x0_MAXPOS
#endif

#ifdef FRONTPANEL_MINI
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 100.0f
#define FUNCBUTTONS_ON_PAGE 4
#define FUNCBUTTONS_PAGES 9
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 2)
static char ota_config_frontpanel[] = "Mini";
#define ATU_MAXPOS ATU_6x6_MAXPOS
#endif

#ifdef FRONTPANEL_LITE_V2_MINI
#define HRDW_HAS_FUNCBUTTONS true
#define MAX_VOLUME_VALUE 100.0f
#define FUNCBUTTONS_ON_PAGE 4
#define FUNCBUTTONS_PAGES 8
#define FUNCBUTTONS_COUNT (FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE + 2)
static char ota_config_frontpanel[] = "LiteV2-Mini";
#define ATU_MAXPOS ATU_0x0_MAXPOS
#endif

// LCDs
#if defined(LCD_ILI9481)
static char ota_config_lcd[] = "ILI9481";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif
#if defined(LCD_ILI9481_IPS)
static char ota_config_lcd[] = "ILI9481_IPS";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif
#if defined(LCD_HX8357B)
static char ota_config_lcd[] = "HX8357B";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif
#if defined(LCD_HX8357C) && !defined(LCD_SLOW)
static char ota_config_lcd[] = "HX8357C";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif
#if defined(LCD_HX8357C) && defined(LCD_SLOW)
static char ota_config_lcd[] = "HX8357C-SLOW";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif
#if defined(LCD_ILI9486)
static char ota_config_lcd[] = "ILI9486";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif
#if defined(LCD_ILI9341)
static char ota_config_lcd[] = "ILI9341";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif
#if defined(LCD_ST7789)
static char ota_config_lcd[] = "LCD_ST7789";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif
#if defined(LCD_ST7735S)
static char ota_config_lcd[] = "ST7735S";
#ifdef STM32H743xx
#define FT8_SUPPORT false
#endif
#endif

#if defined(LCD_RA8875)
#ifdef FRONTPANEL_WF_100D
static char ota_config_lcd[] = "WF-100D";
#else
#ifdef FRONTPANEL_WOLF_2
static char ota_config_lcd[] = "WOLF-2";
#else
static char ota_config_lcd[] = "RA8875";
#endif
#endif

#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

#if defined(LCD_NONE)
static char ota_config_lcd[] = "NONE";
#ifdef STM32H743xx
#define FT8_SUPPORT true
#endif
#endif

// TOUCHPADs
#if defined(TOUCHPAD_GT911)
static char ota_config_touchpad[] = "GT911";
#else
static char ota_config_touchpad[] = "NONE";
#endif

typedef enum {
	TRX_MODE_LSB,
	TRX_MODE_USB,
	TRX_MODE_CW,
	TRX_MODE_NFM,
	TRX_MODE_WFM,
	TRX_MODE_AM,
	TRX_MODE_SAM,
	TRX_MODE_DIGI_L,
	TRX_MODE_DIGI_U,
	TRX_MODE_IQ,
	TRX_MODE_LOOPBACK,
	TRX_MODE_RTTY,
} TRX_MODE;
#define TRX_MODE_COUNT 12

typedef struct {
	uint64_t Freq;
	uint64_t RealRXFreq;
	uint_fast16_t HPF_RX_Filter_Width;
	uint_fast16_t HPF_TX_Filter_Width;
	uint_fast16_t LPF_RX_Filter_Width;
	uint_fast16_t LPF_TX_Filter_Width;
	uint_fast8_t Mode;
	uint_fast16_t NotchFC;
	uint8_t DNR_Type; // 0-disabled 1-dnr 2-dnr2
	int8_t FM_SQL_threshold_dbm;
	bool ManualNotchFilter;
	bool AutoNotchFilter;
	bool AGC;
	bool SQL;
	bool RepeaterMode;
} VFO;

#if HRDW_HAS_DUAL_RX
// dual receiver operating mode
typedef enum {
	VFO_A_AND_B,
	VFO_A_PLUS_B,
} DUAL_RX_TYPE;
#endif

// CAT type
typedef enum {
	CAT_FT450,
	CAT_TS2000,
} CAT_TYPE;

// trx input
typedef enum {
	TRX_INPUT_MIC,
	TRX_INPUT_LINE,
	TRX_INPUT_USB,
} TRX_INPUT_TYPE;

// debug level
typedef enum {
	TRX_DEBUG_OFF,
	TRX_DEBUG_SYSTEM,
	TRX_DEBUG_WIFI,
	TRX_DEBUG_BUTTONS,
	TRX_DEBUG_TOUCH,
	TRX_DEBUG_CAT,
	TRX_DEBUG_I2C,
} TRX_DEBUG_TYPE;

// IQ SAMPLERATE
typedef enum {
	TRX_SAMPLERATE_K48,
	TRX_SAMPLERATE_K96,
	TRX_SAMPLERATE_K192,
	TRX_SAMPLERATE_K384,
} TRX_IQ_SAMPLERATE_VALUE;

// RF UNIT TYPE
typedef enum {
	RF_UNIT_QRP,
	RF_UNIT_BIG,
	RF_UNIT_SPLIT,
	RF_UNIT_RU4PN,
	RF_UNIT_KT_100S,
	RF_UNIT_WF_100D,
} TRX_RF_UNIT_TYPE;

// TANGENT TYPE
typedef enum {
	TANGENT_MH36,
	TANGENT_MH48,
} TRX_TANGENT_TYPE;

// CW PTT TYPE
typedef enum {
	KEY_PTT,
	EXT_PTT,
	KEY_AND_EXT_PTT,
} CW_PTT_TYPE;

// DX Cluster Type
typedef enum {
	DX_CLUSTER_RBN,
	DX_CLUSTER_DX_SUMMIT,
} DX_CLUSTER_TYPE;

// ENC2 FUNC MODE
typedef enum {
	ENC_FUNC_PAGER,
	ENC_FUNC_FAST_STEP,
	ENC_FUNC_SET_WPM,
	ENC_FUNC_SET_RIT,
	ENC_FUNC_SET_NOTCH,
	ENC_FUNC_SET_LPF,
	ENC_FUNC_SET_HPF,
	ENC_FUNC_SET_SQL,
	ENC_FUNC_SET_VOLUME,
	ENC_FUNC_SET_IF,
	ENC_FUNC_SET_MEM,
} ENC2_FUNC_MODE;

// Save settings by band
typedef struct {
	uint64_t Freq;
	float32_t ATT_DB;
	int8_t FM_SQL_threshold_dbm;
	uint8_t Mode;
	uint8_t DNR_Type;
	uint8_t ANT1_ATU_I;
	uint8_t ANT1_ATU_C;
	uint8_t ANT2_ATU_I;
	uint8_t ANT2_ATU_C;
	uint8_t IF_Gain;
	uint8_t RF_Gain;
	uint8_t RF_Gain_By_Mode_CW;
	uint8_t RF_Gain_By_Mode_SSB;
	uint8_t RF_Gain_By_Mode_FM;
	uint8_t RF_Gain_By_Mode_AM;
	uint8_t RF_Gain_By_Mode_DIGI;
	bool LNA;
	bool ATT;
	bool ANT_selected;
	bool ANT_mode;
	bool ADC_Driver;
	bool ADC_PGA;
	bool AGC;
	bool RepeaterMode;
	bool SQL;
	bool Fast;
	bool ANT1_ATU_T;
	bool ANT2_ATU_T;
	TRX_IQ_SAMPLERATE_VALUE SAMPLERATE;
} BAND_SAVED_SETTINGS_TYPE;

// Save memory channels
typedef struct {
	uint64_t Freq;
	uint8_t Mode;
} CHANNEL_SAVED_SETTINGS_TYPE;

extern struct TRX_SETTINGS {
	uint8_t flash_id; // version check
	bool NeedGoToBootloader;
	// TRX
	float32_t ATT_DB;
	uint32_t FRQ_STEP;
	uint32_t FRQ_FAST_STEP;
	uint32_t FRQ_ENC_STEP;
	uint32_t FRQ_ENC_FAST_STEP;
	uint32_t FRQ_ENC_WFM_STEP_KHZ;
	float32_t FRQ_ENC_FM_STEP_KHZ;
	float32_t FRQ_ENC_AM_STEP_KHZ;
	VFO VFO_A;
	VFO VFO_B;
	uint16_t RIT_INTERVAL;
	uint16_t XIT_INTERVAL;
	int16_t REPEATER_Offset;
	uint8_t ATT_STEP;
	uint8_t RF_Gain;
	uint8_t RF_Gain_By_Mode_CW;
	uint8_t RF_Gain_By_Mode_SSB;
	uint8_t RF_Gain_By_Mode_FM;
	uint8_t RF_Gain_By_Mode_AM;
	uint8_t RF_Gain_By_Mode_DIGI;
	uint8_t FRQ_CW_STEP_DIVIDER;
	uint8_t ATU_I;
	uint8_t ATU_C;
	DX_CLUSTER_TYPE DXCluster_Type;
	TRX_DEBUG_TYPE Debug_Type;
	TRX_IQ_SAMPLERATE_VALUE SAMPLERATE_MAIN;
	TRX_IQ_SAMPLERATE_VALUE SAMPLERATE_FM;
	TRX_INPUT_TYPE InputType_MAIN;
	TRX_INPUT_TYPE InputType_DIGI;
#if HRDW_HAS_DUAL_RX
	DUAL_RX_TYPE Dual_RX_Type;
	bool Dual_RX;
#endif
	bool selected_vfo; // false - A; true - B
	bool Fast;
	bool LNA;
	bool ATT;
	bool RF_Filters;
	bool RF_Gain_For_Each_Band;
	bool RF_Gain_For_Each_Mode;
	bool ANT_selected; // false - 1, true - 2
	bool ANT_mode;     // false - RX=TX, true - 1RX 2TX
	bool ChannelMode;
	bool RIT_Enabled;
	bool XIT_Enabled;
	bool SPLIT_Enabled;
	bool FineRITTune;
	bool TWO_SIGNAL_TUNE;
	bool BandMapEnabled;
	bool AutoGain;
	bool Locked;
	bool Encoder_Accelerate;
	bool TUNER_Enabled;
	bool ATU_Enabled;
	bool ATU_T;
	bool Custom_Transverter_Enabled;
	bool Transverter_70cm;
	bool Transverter_23cm;
	bool Transverter_13cm;
	bool Transverter_6cm;
	bool Transverter_3cm;
	bool Transverter_QO100;
	bool Auto_Input_Switch;
	bool Auto_Snap;
	bool Full_Duplex;
	bool Digital_Pre_Distortion;
	bool Split_Mode_Sync_Freq;
	bool FT8_Auto_CQ;
	char CALLSIGN[MAX_CALLSIGN_LENGTH + 1];
	char LOCATOR[MAX_CALLSIGN_LENGTH + 1];
	char URSI_CODE[MAX_CALLSIGN_LENGTH + 1];
	// AUDIO
	float32_t CTCSS_Freq;
	float32_t MIC_GAIN_DB;
	float32_t TX_CESSB_COMPRESS_DB;
	uint16_t Volume;
	uint16_t RX_AGC_Hold_Time;
	uint16_t RX_AGC_Hold_Limiter;
	uint16_t RX_AGC_Hold_Step_Up;
	uint16_t RX_AGC_Hold_Step_Down;
	uint16_t CW_LPF_Filter;
	uint16_t DIGI_LPF_Filter;
	uint16_t SSB_LPF_RX_Filter;
	uint16_t SSB_LPF_TX_Filter;
	uint16_t SSB_HPF_RX_Filter;
	uint16_t SSB_HPF_TX_Filter;
	uint16_t AM_LPF_RX_Filter;
	uint16_t AM_LPF_TX_Filter;
	uint16_t FM_LPF_RX_Filter;
	uint16_t FM_LPF_TX_Filter;
	uint16_t FM_HPF_RX_Filter;
	uint16_t VOX_TIMEOUT;
	uint8_t CW_LPF_Stages;
	uint8_t SSB_LPF_Stages;
	uint8_t AMFM_LPF_Stages;
	uint8_t Volume_Step;
	uint8_t IF_Gain;
	uint8_t MIC_REVERBER;
	uint8_t DNR1_SNR_THRESHOLD;
	uint8_t DNR2_SNR_THRESHOLD;
	uint8_t DNR_AVERAGE;
	uint8_t DNR_MINIMAL;
	uint8_t VAD_THRESHOLD;
	uint8_t NOISE_BLANKER_THRESHOLD;
	uint8_t RX_AGC_SSB_speed;
	uint8_t RX_AGC_CW_speed;
	uint8_t RX_AGC_Max_gain;
	uint8_t TX_Compressor_speed_SSB;
	uint8_t TX_Compressor_maxgain_SSB;
	uint8_t TX_Compressor_speed_AMFM;
	uint8_t TX_Compressor_maxgain_AMFM;
	uint8_t SELFHEAR_Volume;
	uint8_t LINE_Volume;
	uint8_t CODEC_Out_Volume;
	int8_t MIC_NOISE_GATE;
	int8_t RX_EQ_P1;
	int8_t RX_EQ_P2;
	int8_t RX_EQ_P3;
	int8_t RX_EQ_P4;
	int8_t RX_EQ_P5;
	int8_t RX_EQ_P1_WFM;
	int8_t RX_EQ_P2_WFM;
	int8_t RX_EQ_P3_WFM;
	int8_t RX_EQ_P4_WFM;
	int8_t RX_EQ_P5_WFM;
	int8_t MIC_EQ_P1_SSB;
	int8_t MIC_EQ_P2_SSB;
	int8_t MIC_EQ_P3_SSB;
	int8_t MIC_EQ_P4_SSB;
	int8_t MIC_EQ_P5_SSB;
	int8_t MIC_EQ_P1_AMFM;
	int8_t MIC_EQ_P2_AMFM;
	int8_t MIC_EQ_P3_AMFM;
	int8_t MIC_EQ_P4_AMFM;
	int8_t MIC_EQ_P5_AMFM;
	int8_t AGC_GAIN_TARGET;
	int8_t VOX_THRESHOLD;
	bool Mute;
	bool AFAmp_Mute;
	bool MIC_Boost;
	bool BluetoothAudio_Enabled;
	bool NOISE_BLANKER;
	bool Beeper;
	bool FM_Stereo;
	bool AGC_Spectral;
	bool VOX;
	bool TX_CESSB;
	// CW
	float32_t CW_DotToDashRate;
	uint16_t CW_Pitch;
	uint16_t CW_Key_timeout;
	uint16_t CW_SelfHear;
	uint16_t CW_KEYER_WPM;
	CW_PTT_TYPE CW_PTT_Type;
	bool CW_KEYER;
	bool CW_OneSymbolMemory;
	bool CW_GaussFilter;
	bool CW_Iambic;
	uint8_t CW_Iambic_Type;
	bool CW_Invert;
	char CW_Macros_1[MAX_CW_MACROS_LENGTH + 1];
	char CW_Macros_2[MAX_CW_MACROS_LENGTH + 1];
	char CW_Macros_3[MAX_CW_MACROS_LENGTH + 1];
	char CW_Macros_4[MAX_CW_MACROS_LENGTH + 1];
	char CW_Macros_5[MAX_CW_MACROS_LENGTH + 1];
	// SCREEN
	int16_t FFT_ManualBottom;
	int16_t FFT_ManualTop;
	uint16_t LCD_SleepTimeout;
	int8_t FFT_FreqGrid;
	uint8_t ColorThemeId;
	uint8_t LayoutThemeId;
	uint8_t FFT_Zoom;
	uint8_t FFT_ZoomCW;
	uint8_t LCD_Brightness;
	uint8_t FFT_Sensitivity;
	uint8_t FFT_Speed;
	uint8_t FFT_Averaging;
	uint8_t FFT_Window;
	uint8_t FFT_Height;
	uint8_t FFT_Style;
	uint8_t FFT_BW_Style;
	uint8_t FFT_Color;
	uint8_t WTF_Color;
	uint8_t FFT_3D;
	uint8_t FFT_DXCluster_Timeout;
	uint8_t FFT_Scale_Type;
	uint8_t FuncButtons[(FUNCBUTTONS_PAGES * FUNCBUTTONS_ON_PAGE)];
	bool FFT_Enabled;
	bool WTF_Moving;
	bool FFT_Automatic;
	bool FFT_Compressor;
	bool FFT_dBmGrid;
	bool FFT_Background;
	bool FFT_Lens;
	bool FFT_HoldPeaks;
	bool FFT_DXCluster;
	bool FFT_DXCluster_Azimuth;
	bool Show_Sec_VFO;
	bool AnalogMeterShowPWR;
	bool EnableBottomNavigationButtons;
	// DECODERS
	uint16_t RTTY_Speed;
	uint16_t RTTY_Shift;
	uint16_t RTTY_Freq;
	uint8_t RTTY_StopBits;
	uint8_t CW_Decoder_Threshold;
	bool CW_Decoder;
	bool RDS_Decoder;
	bool RTTY_InvertBits;
	// ADC
	bool ADC_Driver;
	bool ADC_PGA;
	bool ADC_RAND;
	bool ADC_SHDN;
	bool ADC_DITH;
	// SERVICES
	uint32_t SWR_CUSTOM_Begin;
	uint32_t SWR_CUSTOM_End;
	uint32_t SPEC_Begin;
	uint32_t SPEC_End;
	int16_t SPEC_TopDBM;
	int16_t SPEC_BottomDBM;
	int16_t WSPR_FREQ_OFFSET;
	bool WSPR_BANDS_160;
	bool WSPR_BANDS_80;
	bool WSPR_BANDS_40;
	bool WSPR_BANDS_30;
	bool WSPR_BANDS_20;
	bool WSPR_BANDS_17;
	bool WSPR_BANDS_15;
	bool WSPR_BANDS_12;
	bool WSPR_BANDS_10;
	bool WSPR_BANDS_6;
	bool WSPR_BANDS_2;
	// Shadow variables
	uint8_t FRONTPANEL_funcbuttons_page;
	ENC2_FUNC_MODE ENC2_func_mode;
	uint8_t DNR_shadow;
	int8_t FM_SQL_threshold_dbm_shadow;
	bool SQL_shadow;
	bool AGC_shadow;
	bool Notch_on_shadow;
	bool RepeaterMode_shadow;
	// Memory
	BAND_SAVED_SETTINGS_TYPE BANDS_SAVED_SETTINGS[BANDS_COUNT];
	//
	uint8_t csum;   // check sum
	uint8_t ENDBit; // end bit
} TRX;

extern struct TRX_CALIBRATE {
	uint8_t flash_id; // version check

	float32_t SWR_FWD_Calibration_HF;
	float32_t SWR_BWD_Calibration_HF;
	float32_t SWR_FWD_Calibration_6M;
	float32_t SWR_BWD_Calibration_6M;
	float32_t SWR_FWD_Calibration_VHF;
	float32_t SWR_BWD_Calibration_VHF;
	float32_t FW_AD8307_SLP;
	float32_t FW_AD8307_OFFS;
	float32_t BW_AD8307_SLP;
	float32_t BW_AD8307_OFFS;
	float32_t INA226_CurCalc; // X_mA/Bit Coeficient is dependant on the used shunt (tolerances and soldering)
	float32_t PWR_VLT_Calibration;
	float32_t PWR_CUR_Calibration;
	uint32_t RFU_LPF_END;
	uint32_t RFU_HPF_START;
	uint32_t RFU_BPF_0_START; // UHF
	uint32_t RFU_BPF_0_END;   // UHF
	uint32_t RFU_BPF_1_START;
	uint32_t RFU_BPF_1_END;
	uint32_t RFU_BPF_2_START;
	uint32_t RFU_BPF_2_END;
	uint32_t RFU_BPF_3_START;
	uint32_t RFU_BPF_3_END;
	uint32_t RFU_BPF_4_START;
	uint32_t RFU_BPF_4_END;
	uint32_t RFU_BPF_5_START;
	uint32_t RFU_BPF_5_END;
	uint32_t RFU_BPF_6_START;
	uint32_t RFU_BPF_6_END;
	uint32_t RFU_BPF_7_START;
	uint32_t RFU_BPF_7_END;
	uint32_t RFU_BPF_8_START;
	uint32_t RFU_BPF_8_END;
	int16_t RTC_Calibration;
	int16_t VCXO_correction;
	uint16_t TCXO_frequency;
	uint16_t MAX_ChargePump_Freq;
	uint16_t TX_StartDelay;
	int16_t smeter_calibration_hf;
	int16_t smeter_calibration_vhf;
	uint16_t Transverter_Custom_Offset_Mhz;
	uint16_t Transverter_70cm_RF_Mhz;
	uint16_t Transverter_70cm_IF_Mhz;
	uint16_t Transverter_23cm_RF_Mhz;
	uint16_t Transverter_23cm_IF_Mhz;
	uint16_t Transverter_13cm_RF_Mhz;
	uint16_t Transverter_13cm_IF_Mhz;
	uint16_t Transverter_6cm_RF_Mhz;
	uint16_t Transverter_6cm_IF_Mhz;
	uint16_t Transverter_3cm_RF_Mhz;
	uint16_t Transverter_3cm_IF_Mhz;
	uint32_t Transverter_QO100_RF_Khz;
	uint32_t Transverter_QO100_IF_RX_Khz;
	uint16_t Transverter_QO100_IF_TX_Mhz;
	uint16_t KTY81_Calibration;
	uint16_t TOUCHPAD_TIMEOUT;
	uint16_t TOUCHPAD_CLICK_THRESHOLD;
	uint16_t TOUCHPAD_CLICK_TIMEOUT;
	uint16_t TOUCHPAD_HOLD_TIMEOUT;
	uint16_t TOUCHPAD_SWIPE_THRESHOLD_PX;
	uint8_t DAC_driver_mode;
	uint8_t rf_out_power_2200m;
	uint8_t rf_out_power_160m;
	uint8_t rf_out_power_80m;
	uint8_t rf_out_power_60m;
	uint8_t rf_out_power_40m;
	uint8_t rf_out_power_30m;
	uint8_t rf_out_power_20m;
	uint8_t rf_out_power_17m;
	uint8_t rf_out_power_15m;
	uint8_t rf_out_power_12m;
	uint8_t rf_out_power_cb;
	uint8_t rf_out_power_10m;
	uint8_t rf_out_power_6m;
	uint8_t rf_out_power_4m;
	uint8_t rf_out_power_2m;
	uint8_t rf_out_power_70cm;
	uint8_t rf_out_power_23cm;
	uint8_t rf_out_power_13cm;
	uint8_t rf_out_power_6cm;
	uint8_t rf_out_power_3cm;
	uint8_t rf_out_power_QO100;
	uint8_t ENCODER_DEBOUNCE;
	uint8_t ENCODER2_DEBOUNCE;
	uint8_t ENCODER_SLOW_RATE;
	uint8_t CICFIR_GAINER_48K_val;
	uint8_t CICFIR_GAINER_96K_val;
	uint8_t CICFIR_GAINER_192K_val;
	uint8_t CICFIR_GAINER_384K_val;
	uint8_t TXCICFIR_GAINER_val;
	uint8_t DAC_GAINER_val;
	uint8_t MAX_RF_POWER_ON_METER;
	uint8_t ENCODER_ACCELERATION;
	uint8_t FAN_MEDIUM_START;
	uint8_t FAN_MEDIUM_STOP;
	uint8_t FAN_FULL_START;
	uint8_t TRX_MAX_RF_TEMP;
	uint8_t TRX_MAX_SWR;
	uint8_t FM_DEVIATION_SCALE;
	uint8_t SSB_POWER_ADDITION;
	uint8_t AM_MODULATION_INDEX;
	uint8_t TUNE_MAX_POWER;
	uint8_t RTC_Coarse_Calibration;
	uint8_t EXT_2200m;
	uint8_t EXT_160m;
	uint8_t EXT_80m;
	uint8_t EXT_60m;
	uint8_t EXT_40m;
	uint8_t EXT_30m;
	uint8_t EXT_20m;
	uint8_t EXT_17m;
	uint8_t EXT_15m;
	uint8_t EXT_12m;
	uint8_t EXT_CB;
	uint8_t EXT_10m;
	uint8_t EXT_6m;
	uint8_t EXT_4m;
	uint8_t EXT_FM;
	uint8_t EXT_2m;
	uint8_t EXT_70cm;
	uint8_t EXT_TRANSV_70cm;
	uint8_t EXT_TRANSV_23cm;
	uint8_t EXT_TRANSV_13cm;
	uint8_t EXT_TRANSV_6cm;
	uint8_t EXT_TRANSV_3cm;
	uint8_t EXT_TRANSV_QO100;
	uint8_t ATU_AVERAGING;
	uint8_t TwoSignalTune_Balance;
	uint8_t IF_GAIN_MIN;
	uint8_t IF_GAIN_MAX;
	int8_t LNA_compensation;
	TRX_RF_UNIT_TYPE RF_unit_type;
	TRX_TANGENT_TYPE TangentType;
	CAT_TYPE CAT_Type;
	bool ENCODER_INVERT;
	bool ENCODER2_INVERT;
	bool ENCODER_ON_FALLING;
	bool NOTX_NOTHAM;
	bool NOTX_2200m;
	bool NOTX_160m;
	bool NOTX_80m;
	bool NOTX_60m;
	bool NOTX_40m;
	bool NOTX_30m;
	bool NOTX_20m;
	bool NOTX_17m;
	bool NOTX_15m;
	bool NOTX_12m;
	bool NOTX_CB;
	bool NOTX_10m;
	bool NOTX_6m;
	bool NOTX_4m;
	bool NOTX_FM;
	bool NOTX_2m;
	bool NOTX_70cm;
	bool ENABLE_60m_band;
	bool ENABLE_4m_band;
	bool ENABLE_AIR_band;
	bool ENABLE_marine_band;
	bool OTA_update;
	bool LCD_Rotate;
	bool TOUCHPAD_horizontal_flip;
	bool INA226_EN;
	bool LinearPowerControl;
	bool ALC_Port_Enabled;
	bool ALC_Inverted_Logic;
	CHANNEL_SAVED_SETTINGS_TYPE MEMORY_CHANNELS[MEMORY_CHANNELS_COUNT];
	uint32_t BAND_MEMORIES[BANDS_COUNT][BANDS_MEMORIES_COUNT];

	uint8_t csum;   // check sum
	uint8_t ENDBit; // end bit
} CALIBRATE;

extern struct TRX_WIFI {
	uint8_t flash_id; // version check
	// WIFI
	char AP_1[MAX_WIFIPASS_LENGTH + 1];
	char Password_1[MAX_WIFIPASS_LENGTH + 1];
	char AP_2[MAX_WIFIPASS_LENGTH + 1];
	char Password_2[MAX_WIFIPASS_LENGTH + 1];
	char AP_3[MAX_WIFIPASS_LENGTH + 1];
	char Password_3[MAX_WIFIPASS_LENGTH + 1];
	char ALLQSO_TOKEN[ALLQSO_TOKEN_SIZE + 1];
	char ALLQSO_LOGID[ALLQSO_TOKEN_SIZE + 1];
	float32_t Timezone;
	bool Enabled;
	bool CAT_Server;
	uint8_t csum;   // check sum
	uint8_t ENDBit; // end bit
} WIFI;

extern volatile bool NeedSaveSettings;
extern volatile bool NeedSaveCalibration;
extern volatile bool NeedSaveWiFi;
extern volatile bool EEPROM_Busy;
extern VFO *CurrentVFO;
extern VFO *SecondaryVFO;
extern bool EEPROM_Enabled;

extern void LoadSettings(bool clear);
extern void LoadCalibration(bool clear);
extern void LoadWiFiSettings(bool clear);
extern void SaveSettings(void);
extern void SaveCalibration(void);
extern void SaveWiFiSettings(void);
extern void SaveSettingsToEEPROM(void);
extern void BKPSRAM_Enable(void);
extern void BKPSRAM_Disable(void);
extern void RTC_Calibration(void);
extern bool LoadDPDSettings(uint8_t *out, uint32_t size, uint32_t sector_offset);
extern bool SaveDPDSettings(uint8_t *in, uint32_t size, uint32_t sector_offset);

#endif
