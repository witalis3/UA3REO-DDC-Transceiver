#ifndef SETTINGS_h
#define SETTINGS_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "functions.h"
#include "bands.h"
#include "front_unit.h"

#define SETT_VERSION 26					   // Settings config version
#define CALIB_VERSION 26				   // Calibration config version
#define ADC_CLOCK 122880000				   // ADC generator frequency
#define DAC_CLOCK 188160000				   // DAC generator frequency
#define MAX_RX_FREQ_HZ 750000000		   // Maximum receive frequency (from the ADC datasheet)
#define MAX_TX_FREQ_HZ DAC_CLOCK		   // Maximum transmission frequency
#define TRX_SAMPLERATE 48000			   // audio stream sampling rate during processing and TX (NOT RX!)
#define MAX_TX_AMPLITUDE 1.0f			   // Maximum amplitude when transmitting to FPGA
#define AGC_MAX_GAIN 30.0f				   // Maximum gain in AGC, dB
#define AGC_CLIPPING 6.0f				   // Limit over target in AGC, dB
#define TOUCHPAD_DELAY 200				   // Anti-bounce time for pressing the touchpad
#define AUTOGAIN_TARGET_AMPLITUDE 20000.0f // maximum amplitude, upon reaching which the autocorrector of the input circuits terminates, and in case of overflow it reduces the gain
#define AUTOGAIN_MAX_AMPLITUDE 30000.0f	   // maximum amplitude, upon reaching which the autocorrector of the input circuits terminates, and in case of overflow it reduces the gain
#define AUTOGAIN_CORRECTOR_WAITSTEP 5	   // waiting for the averaging of the results when the auto-corrector of the input circuits is running
#define KEY_HOLD_TIME 500				   // time of long pressing of the keyboard button for triggering, ms
#define MAX_RF_POWER 7.0f				   // Maximum power (for meter scale)
#define SHOW_LOGO true					   // Show logo on boot (from images.h)
#define POWERDOWN_TIMEOUT 1000			   // time of pressing the shutdown button, for operation, ms
#define POWERDOWN_FORCE_TIMEOUT 2000	   // force time
#define USB_RESTART_TIMEOUT 5000		   // time after which USB restart occurs if there are no packets
#define FPGA_FLASH_IN_HEX false			   // enable FPGA firmware in STM32 firmware
#define SNTP_SYNC_INTERVAL (60 * 60)	   // Time synchronization interval via NTP, sec
#define SCANNER_NOSIGNAL_TIME 50		   //time to continue sweeping if signal too low
#define SCANNER_SIGNAL_TIME_FM 5000		   //time to continue sweeping if signal founded for FM
#define SCANNER_SIGNAL_TIME_OTHER 1000	   //time to continue sweeping if signal founded for SSB
#define SCANNER_FREQ_STEP_WFM 100000	   //step for freq scanner for WFM
#define SCANNER_FREQ_STEP_NFM 25000		   //step for freq scanner for NFM
#define SCANNER_FREQ_STEP_OTHER 500		   //step for freq scanner for SSB
#define ENCODER_MIN_RATE_ACCELERATION 1.2f //encoder enable rounding if lower than value

//FRONT-PANEL, LCD AND TANGENT types moved to KEIL TARGETS
//#define RF_UNIT_QRP_V1
#define RF_UNIT_BIG_V1

//select how the SWR and the power is measured
//#define SWR_AD8307_LOG true			//Enable if used log amplifier for the power measurement

//SPI Speed
#define SPI_FRONT_UNIT_PRESCALER SPI_BAUDRATEPRESCALER_2
#define SPI_SD_PRESCALER SPI_BAUDRATEPRESCALER_4
#define SPI_EEPROM_PRESCALER SPI_BAUDRATEPRESCALER_2

#define SCREEN_ROTATE false // turn the screen upside down

//#define ADC_BITS 16																						// ADC bit depth
//#define FPGA_BUS_BITS 32																				// bitness of data from FPGA
//#define CODEC_BITS 32																					// bitness of data in the audio codec
//#define FPGA_BUS_FULL_SCALE 65536																		// maximum signal amplitude in the bus // powf (2, FPGA_BUS_BITS)
//#define FPGA_BUS_FULL_SCALE_POW ((float64_t)FPGA_BUS_FULL_SCALE * (float64_t)FPGA_BUS_FULL_SCALE)		// maximum bus signal magnitude // (FPGA_BUS_FULL_SCALE * FPGA_BUS_FULL_SCALE)
#define CODEC_BITS_FULL_SCALE 4294967296 // maximum signal amplitude in the bus // powf (2, FPGA_BUS_BITS)
//#define CODEC_BITS_FULL_SCALE_POW ((float64_t)CODEC_BITS_FULL_SCALE * (float64_t)CODEC_BITS_FULL_SCALE) // maximum bus signal magnitude // (FPGA_BUS_FULL_SCALE * FPGA_BUS )_FULL_SCALE
#define ADC_FULL_SCALE 65536 // maximum signal amplitude in the ADC // powf (2, ADC_BITS)
#define FLOAT_FULL_SCALE_POW 4
#define USB_DEBUG_ENABLED true	// allow using USB as a console
#define SWD_DEBUG_ENABLED false // enable SWD as a console
#define LCD_DEBUG_ENABLED false // enable LCD as a console
#define DCDC_FREQ_0 999024
#define DCDC_FREQ_1 1200782
#define ADC_INPUT_IMPEDANCE 200.0f //50ohm -> 1:4 trans
#define ADC_RANGE 2.25f
#define ADC_RANGE_PGA 1.5f

#define ADC_LNA_GAIN_DB 20.0f
#define ADC_DRIVER_GAIN_DB 25.5f
#define ADC_PGA_GAIN_DB 3.522f
#define AUTOGAINER_TAGET (ADC_FULL_SCALE / 3)
#define AUTOGAINER_HYSTERESIS 5000

#define MAX_WIFIPASS_LENGTH 32
#define MAX_CALLSIGN_LENGTH 16

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
#define W25Q16_SECTOR_SIZE 4096
#define EEPROM_SECTOR_CALIBRATION 0
#define EEPROM_SECTOR_SETTINGS 4
#define EEPROM_REPEAT_TRYES 10 // command tryes

typedef struct
{
	uint32_t Freq;
	uint_fast8_t Mode;
	uint_fast16_t HPF_Filter_Width;
	uint_fast16_t LPF_RX_Filter_Width;
	uint_fast16_t LPF_TX_Filter_Width;
	bool ManualNotchFilter;
	bool AutoNotchFilter;
	uint_fast16_t NotchFC;
	uint8_t DNR_Type; //0-disabled 1-dnr 2-dnr2
	uint8_t FM_SQL_threshold;
	bool AGC;
} VFO;

// dual receiver operating mode
typedef enum
{
	VFO_A_AND_B,
	VFO_A_PLUS_B,
} DUAL_RX_TYPE;

// trx input
typedef enum
{
	TRX_INPUT_MIC,
	TRX_INPUT_LINE,
	TRX_INPUT_USB,
} TRX_INPUT_TYPE;

// debug level
typedef enum
{
	TRX_DEBUG_OFF,
	TRX_DEBUG_SYSTEM,
	TRX_DEBUG_WIFI,
	TRX_DEBUG_BUTTONS,
	TRX_DEBUG_TOUCH,
} TRX_DEBUG_TYPE;

// debug level
typedef enum
{
	TRX_SAMPLERATE_K48,
	TRX_SAMPLERATE_K96,
	TRX_SAMPLERATE_K192,
	TRX_SAMPLERATE_K384,
} TRX_IQ_SAMPLERATE_VALUE;

// Save settings by band
typedef struct
{
	uint32_t Freq;
	uint8_t Mode;
	bool LNA;
	float32_t ATT_DB;
	bool ATT;
	bool ANT;
	bool ADC_Driver;
	bool ADC_PGA;
	uint8_t DNR_Type;
	bool AGC;
	uint8_t FM_SQL_threshold;
} BAND_SAVED_SETTINGS_TYPE;

extern struct TRX_SETTINGS
{
	uint8_t flash_id; //version check
	//TRX
	bool current_vfo; // false - A; true - B
	VFO VFO_A;
	VFO VFO_B;
	bool Fast;
	BAND_SAVED_SETTINGS_TYPE BANDS_SAVED_SETTINGS[BANDS_COUNT];
	bool LNA;
	bool ATT;
	float32_t ATT_DB;
	uint8_t ATT_STEP;
	bool RF_Filters;
	bool ANT;
	uint8_t RF_Power;
	bool ShiftEnabled;
	uint16_t SHIFT_INTERVAL;
	bool TWO_SIGNAL_TUNE;
	TRX_IQ_SAMPLERATE_VALUE SAMPLERATE_MAIN;
	TRX_IQ_SAMPLERATE_VALUE SAMPLERATE_WFM;
	uint16_t FRQ_STEP;
	uint16_t FRQ_FAST_STEP;
	uint16_t FRQ_ENC_STEP;
	uint32_t FRQ_ENC_FAST_STEP;
	TRX_DEBUG_TYPE Debug_Type;
	bool BandMapEnabled;
	TRX_INPUT_TYPE InputType;
	bool AutoGain;
	bool Locked;
	bool CLAR;
	bool Dual_RX;
	DUAL_RX_TYPE Dual_RX_Type;
	bool Encoder_Accelerate;
	char CALLSIGN[MAX_CALLSIGN_LENGTH];
	char LOCATOR[MAX_CALLSIGN_LENGTH];
	bool Transverter_Enabled;
	uint16_t Transverter_Offset_Mhz;
	uint8_t ATU_I;
	uint8_t ATU_C;
	bool ATU_T;
	//AUDIO
	uint8_t IF_Gain;
	uint8_t FM_SQL_threshold;
	int8_t AGC_GAIN_TARGET;
	uint8_t MIC_GAIN;
	int8_t RX_EQ_LOW;
	int8_t RX_EQ_MID;
	int8_t RX_EQ_HIG;
	int8_t MIC_EQ_LOW;
	int8_t MIC_EQ_MID;
	int8_t MIC_EQ_HIG;
	uint8_t MIC_REVERBER;
	uint8_t DNR_SNR_THRESHOLD;
	uint8_t DNR_AVERAGE;
	uint8_t DNR_MINIMAL;
	bool NOISE_BLANKER;
	uint8_t RX_AGC_SSB_speed;
	uint8_t RX_AGC_CW_speed;
	uint8_t TX_Compressor_speed;
	uint8_t TX_Compressor_maxgain;
	uint16_t CW_LPF_Filter;
	uint16_t CW_HPF_Filter;
	uint16_t SSB_LPF_RX_Filter;
	uint16_t SSB_LPF_TX_Filter;
	uint16_t SSB_HPF_Filter;
	uint16_t AM_LPF_RX_Filter;
	uint16_t AM_LPF_TX_Filter;
	uint16_t FM_LPF_RX_Filter;
	uint16_t FM_LPF_TX_Filter;
	bool Beeper;
	bool VAD_Squelch;
	//CW
	bool CWDecoder;
	uint16_t CW_GENERATOR_SHIFT_HZ;
	uint16_t CW_Key_timeout;
	uint16_t CW_SelfHear;
	bool CW_KEYER;
	uint16_t CW_KEYER_WPM;
	bool CW_GaussFilter;
	//SCREEN
	uint8_t ColorThemeId;
	uint8_t LayoutThemeId;
	bool FFT_Enabled;
	uint8_t FFT_Zoom;
	uint8_t FFT_ZoomCW;
	uint8_t LCD_Brightness;
	bool WTF_Moving;
	bool FFT_Automatic;
	uint8_t FFT_Sensitivity;
	uint8_t FFT_Speed;
	uint8_t FFT_Averaging;
	uint8_t FFT_Window;
	uint8_t FFT_Height;
	uint8_t FFT_Style;
	uint8_t FFT_Color;
	bool FFT_Compressor;
	int8_t FFT_Grid;
	bool FFT_Background;
	bool FFT_Lens;
	bool FFT_HoldPeaks;
	uint8_t FFT_3D;
	int16_t FFT_ManualBottom;
	int16_t FFT_ManualTop;
	uint8_t FuncButtons[FUNCBUTTONS_COUNT];
	//ADC
	bool ADC_Driver;
	bool ADC_PGA;
	bool ADC_RAND;
	bool ADC_SHDN;
	bool ADC_DITH;
	//WIFI
	bool WIFI_Enabled;
	char WIFI_AP1[MAX_WIFIPASS_LENGTH];
	char WIFI_PASSWORD1[MAX_WIFIPASS_LENGTH];
	char WIFI_AP2[MAX_WIFIPASS_LENGTH];
	char WIFI_PASSWORD2[MAX_WIFIPASS_LENGTH];
	char WIFI_AP3[MAX_WIFIPASS_LENGTH];
	char WIFI_PASSWORD3[MAX_WIFIPASS_LENGTH];
	int8_t WIFI_TIMEZONE;
	bool WIFI_CAT_SERVER;
	//SERVICES
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
	//
	uint8_t csum;	//check sum
	uint8_t ENDBit; //end bit
} TRX;

extern struct TRX_CALIBRATE
{
	uint8_t flash_id; //version check
	uint8_t rf_unit_id; //rf-unit type
	bool ENCODER_INVERT;
	bool ENCODER2_INVERT;
	uint8_t ENCODER_DEBOUNCE;
	uint8_t ENCODER2_DEBOUNCE;
	uint8_t ENCODER_SLOW_RATE;
	bool ENCODER_ON_FALLING;
	uint8_t CICFIR_GAINER_48K_val;
	uint8_t CICFIR_GAINER_96K_val;
	uint8_t CICFIR_GAINER_192K_val;
	uint8_t CICFIR_GAINER_384K_val;
	uint8_t TXCICFIR_GAINER_val;
	uint8_t DAC_GAINER_val;
	uint8_t rf_out_power_2200m;
	uint8_t rf_out_power_160m;
	uint8_t rf_out_power_80m;
	uint8_t rf_out_power_40m;
	uint8_t rf_out_power_30m;
	uint8_t rf_out_power_20m;
	uint8_t rf_out_power_17m;
	uint8_t rf_out_power_15m;
	uint8_t rf_out_power_12m;
	uint8_t rf_out_power_10m;
	uint8_t rf_out_power_6m;
	uint8_t rf_out_power_2m;
	int16_t smeter_calibration;
	int16_t adc_offset;
	uint32_t LPF_END;
	uint32_t BPF_0_START; //UHF
	uint32_t BPF_0_END;	  //UHF
	uint32_t BPF_1_START;
	uint32_t BPF_1_END;
	uint32_t BPF_2_START;
	uint32_t BPF_2_END;
	uint32_t BPF_3_START;
	uint32_t BPF_3_END;
	uint32_t BPF_4_START;
	uint32_t BPF_4_END;
	uint32_t BPF_5_START;
	uint32_t BPF_5_END;
	uint32_t BPF_6_START;
	uint32_t BPF_6_END;
	uint32_t BPF_7_START;
	uint32_t BPF_7_END;
	uint32_t BPF_8_START;
	uint32_t BPF_8_END;
	uint32_t BPF_9_START;
	uint32_t BPF_9_END;
	uint32_t BPF_HPF;
	float32_t SWR_FWD_Calibration;
	float32_t SWR_REF_Calibration;
	int8_t VCXO_correction;
	uint8_t ENCODER_ACCELERATION;
	float32_t FW_AD8307_SLP;
	float32_t FW_AD8307_OFFS;
	float32_t BW_AD8307_SLP;
	float32_t BW_AD8307_OFFS;
	uint8_t FAN_MEDIUM_START;
	uint8_t FAN_MEDIUM_STOP;
	uint8_t FAN_FULL_START;
	uint8_t TRX_MAX_RF_TEMP;
	uint8_t TRX_MAX_SWR;
	uint8_t FM_DEVIATION_SCALE;
	uint8_t TUNE_MAX_POWER;
	uint8_t RTC_Coarse_Calibration;
	int16_t RTC_Calibration;

	uint8_t csum;	//check sum
	uint8_t ENDBit; //end bit
} CALIBRATE;

extern char version_string[19]; //1.2.3-yymmdd.hhmmss
extern volatile bool NeedSaveSettings;
extern volatile bool NeedSaveCalibration;
extern volatile bool EEPROM_Busy;

extern void InitSettings(void);
extern void LoadSettings(bool clear);
extern void LoadCalibration(bool clear);
extern void SaveSettings(void);
extern void SaveCalibration(void);
extern void SaveSettingsToEEPROM(void);
extern void BKPSRAM_Enable(void);
extern void BKPSRAM_Disable(void);
extern VFO *CurrentVFO(void);
extern VFO *SecondaryVFO(void);
extern void RTC_Calibration(void);

#ifdef FRONTPANEL_SMALL_V1
#define HRDW_MCP3008_1 true
#define HRDW_MCP3008_2 true
#define HRDW_MCP3008_3 true
#endif
#ifdef FRONTPANEL_BIG_V1
#define HRDW_MCP3008_1 true
#define HRDW_HAS_FUNCBUTTONS true
#endif
#ifdef RF_UNIT_BIG_V1
#define HAS_ATU true
#define MAX_ATU_POS B8(00011111)
#endif

#endif
