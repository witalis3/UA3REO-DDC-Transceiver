#ifndef SETTINGS_h
#define SETTINGS_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdbool.h>
#include "functions.h"
#include "bands.h"

#define SETT_VERSION 203				// Settings config version
#define CALIB_VERSION 201				// Calibration config version
#define ADC_CLOCK 122880000				// ADC generator frequency
#define DAC_CLOCK 199680000				// DAC generator frequency
#define MAX_RX_FREQ_HZ 750000000		// Maximum receive frequency (from the ADC datasheet)
#define MAX_TX_FREQ_HZ DAC_CLOCK		// Maximum transmission frequency (half of the DAC clock signal)
#define TRX_SAMPLERATE 48000			// audio stream sampling rate during processing
#define IQ_SAMPLERATE 96000				// sampling rate of the audio stream from FPGA
#define MAX_TX_AMPLITUDE 0.8f			// Maximum amplitude when transmitting to FPGA
#define AGC_MAX_GAIN 30.0f				// Maximum gain in AGC, dB
#define AGC_CLIPPING 6.0f				 // Limit over target in AGC, dB
#define TUNE_POWER 100					// % of the power selected in the settings when starting TUNE (100 - full)
#define TX_AGC_MAXGAIN 5.0f				// Maximum microphone gain during compression
#define TX_AGC_NOISEGATE 0.00001f		// Minimum signal level for amplification (below - noise, cut off)
#define TOUCHPAD_DELAY 200				// Anti-bounce time for pressing the touchpad
#define AUTOGAIN_TARGET_AMPLITUDE 20000.0f // maximum amplitude, upon reaching which the autocorrector of the input circuits terminates, and in case of overflow it reduces the gain
#define AUTOGAIN_LNA_MAX_AMPLITUDE 25000.0f // maximum amplitude, without imd from LNA
#define AUTOGAIN_MAX_AMPLITUDE 30000.0f // maximum amplitude, upon reaching which the autocorrector of the input circuits terminates, and in case of overflow it reduces the gain
#define AUTOGAIN_CORRECTOR_WAITSTEP 5	// waiting for the averaging of the results when the auto-corrector of the input circuits is running
#define KEY_HOLD_TIME 500				// time of long pressing of the keyboard button for triggering, ms
#define MAX_RF_POWER 7.0f				// Maximum power (for meter scale)
#define SHOW_LOGO true					// Show logo on boot (from images.h)
#define POWERDOWN_TIMEOUT 1000			// time of pressing the shutdown button, for operation, ms
#define USB_RESTART_TIMEOUT 5000		// time after which USB restart occurs if there are no packets
#define FPGA_FLASH_IN_HEX false			// enable FPGA firmware in STM32 firmware
#define SNTP_SYNC_INTERVAL (60 * 60)	// Time synchronization interval via NTP, sec
#define FAN_MEDIUM_START 50				// Temperature at which the fan starts at half power
#define FAN_MEDIUM_STOP 40				// Temperature at which the fan stops
#define FAN_FULL_START 70				// Temperature at which the fan starts at full power
#define SCANNER_NOSIGNAL_TIME 50	//time to continue sweeping if signal too low
#define SCANNER_SIGNAL_TIME 1000	//time to continue sweeping if signal founded
#define SCANNER_FREQ_STEP	500		//step for freq scanner
#define ENCODER_ACCELERATION	50		//acceleration rate if rotate
#define ENCODER_MIN_RATE_ACCELERATION	1.2f //encoder enable rounding if lower than value
#define TRX_MAX_RF_TEMP	80			//maximum rf unit themperature to enable protect
#define TRX_MAX_SWR		5				//maximum SWR to enable protect (NOT IN TUNE MODE!)

// select LCD and Touchpad, comment on others
//#define LCD_ILI9481 true
//#define LCD_HX8357B true
#define LCD_RA8875 true
#define TOUCHPAD_GT911 true	

#define FMC_REMAP true			// FMC memory remap
#define FSMC_REGISTER_SELECT 18 // from FSMC settings in STM32Cube (A18, A6, etc.)
#define SCREEN_ROTATE 0			// turn the screen upside down

#define ADC_BITS 16																						// ADC bit depth
#define FPGA_BUS_BITS 32																				// bitness of data from FPGA
#define CODEC_BITS 32																					// bitness of data in the audio codec
#define FPGA_BUS_FULL_SCALE 65536																		// maximum signal amplitude in the bus // powf (2, FPGA_BUS_BITS)
#define FPGA_BUS_FULL_SCALE_POW ((float64_t)FPGA_BUS_FULL_SCALE * (float64_t)FPGA_BUS_FULL_SCALE)		// maximum bus signal magnitude // (FPGA_BUS_FULL_SCALE * FPGA_BUS_FULL_SCALE)
#define CODEC_BITS_FULL_SCALE 4294967296																// maximum signal amplitude in the bus // powf (2, FPGA_BUS_BITS)
#define CODEC_BITS_FULL_SCALE_POW ((float64_t)CODEC_BITS_FULL_SCALE * (float64_t)CODEC_BITS_FULL_SCALE) // maximum bus signal magnitude // (FPGA_BUS_FULL_SCALE * FPGA_BUS )_FULL_SCALE
#define ADC_FULL_SCALE 65536																			// maximum signal amplitude in the ADC // powf (2, ADC_BITS)
#define FLOAT_FULL_SCALE_POW 4
#define USB_DEBUG_ENABLED true	// allow using USB as a console
#define SWD_DEBUG_ENABLED false // enable SWD as a console
#define LCD_DEBUG_ENABLED false // enable LCD as a console
#define AUDIO_DECIM_RATE (IQ_SAMPLERATE / TRX_SAMPLERATE)
#define DCDC_FREQ_0 1000000
#define DCDC_FREQ_1 1200000
#define ADC_LNA_GAIN_DB 11.0f
#define ADC_DRIVER_GAIN_DB 20.0f
#define ADC_PGA_GAIN_DB 3.522f

// delays when working with EEPROM
#define EEPROM_CO_DELAY 0	   // command
#define EEPROM_AD_DELAY 0	   // addr
#define EEPROM_WR_DELAY 5	   // write
#define EEPROM_RD_DELAY 0	   // read
#define EEPROM_ERASE_DELAY 40  // do erase
#define EEPROM_REPEAT_TRYES 40 // command tryes

#define MAX_WIFIPASS_LENGTH 32

#define W25Q16_COMMAND_Write_Disable 0x04
#define W25Q16_COMMAND_Write_Enable 0x06
#define W25Q16_COMMAND_Erase_Chip 0xC7
#define W25Q16_COMMAND_Sector_Erase 0x20
#define W25Q16_COMMAND_Page_Program 0x02
#define W25Q16_COMMAND_Read_Data 0x03
#define W25Q16_COMMAND_Power_Down 0xB9
#define W25Q16_COMMAND_Power_Up 0xAB
#define W25Q16_SECTOR_SIZE 4096

typedef struct
{
	uint32_t Freq;
	uint_fast8_t Mode;
	uint_fast16_t HPF_Filter_Width;
	uint_fast16_t LPF_Filter_Width;
	bool ManualNotchFilter;
	bool AutoNotchFilter;
	uint_fast16_t NotchFC;
	bool DNR;
	bool AGC;
} VFO;

// dual receiver operating mode
typedef enum
{
	VFO_A_AND_B,
	VFO_A_PLUS_B,
} DUAL_RX_TYPE;

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
	uint8_t FM_SQL_threshold;
	bool DNR;
	bool AGC;
} BAND_SAVED_SETTINGS_TYPE;

extern struct TRX_SETTINGS
{
	uint8_t flash_id;
	//TRX
	bool current_vfo; // false - A; true - B
	VFO VFO_A;
	VFO VFO_B;
	bool Fast;
	BAND_SAVED_SETTINGS_TYPE BANDS_SAVED_SETTINGS[BANDS_COUNT];
	bool LNA;
	float32_t ATT_DB;
	uint8_t ATT_STEP;
	bool ATT;
	bool RF_Filters;
	bool ANT;
	uint8_t RF_Power;
	bool ShiftEnabled;
	uint16_t SHIFT_INTERVAL;
	bool TWO_SIGNAL_TUNE;
	uint16_t FRQ_STEP;
	uint16_t FRQ_FAST_STEP;
	uint16_t FRQ_ENC_STEP;
	uint32_t FRQ_ENC_FAST_STEP;
	bool Debug_Console;
	bool BandMapEnabled;
	bool InputType_MIC;
	bool InputType_LINE;
	bool InputType_USB;
	bool AutoGain;
	bool Locked;
	bool CLAR;
	bool Dual_RX;
	DUAL_RX_TYPE Dual_RX_Type;
	bool Encoder_Accelerate;
	//AUDIO
	uint8_t IF_Gain;
	int8_t AGC_GAIN_TARGET;
	uint8_t MIC_GAIN;
	int8_t RX_EQ_LOW;
	int8_t RX_EQ_MID;
	int8_t RX_EQ_HIG;
	int8_t MIC_EQ_LOW;
	int8_t MIC_EQ_MID;
	int8_t MIC_EQ_HIG;
	uint8_t DNR_SNR_THRESHOLD;
	uint8_t DNR_AVERAGE;
	uint8_t DNR_MINIMAL;
	bool NOISE_BLANKER;
	uint8_t RX_AGC_speed;
	uint8_t TX_AGC_speed;
	uint16_t CW_LPF_Filter;
	uint16_t CW_HPF_Filter;
	uint16_t SSB_LPF_Filter;
	uint16_t SSB_HPF_Filter;
	uint16_t AM_LPF_Filter;
	uint16_t FM_LPF_Filter;
	uint8_t FM_SQL_threshold;
	bool Beeper;
	bool VAD_Squelch;
	//CW
	bool CWDecoder;
	uint16_t CW_GENERATOR_SHIFT_HZ;
	uint16_t CW_Key_timeout;
	uint16_t CW_SelfHear;
	bool CW_KEYER;
	uint16_t CW_KEYER_WPM;
	//SCREEN
	bool FFT_Enabled;
	uint8_t FFT_Zoom;
	uint8_t FFT_Speed;
	uint8_t FFT_Averaging;
	uint8_t FFT_Window;
	uint8_t FFT_Height;
	uint8_t FFT_Color;
	bool FFT_Compressor;
	int8_t FFT_Grid;
	bool FFT_Background;
	//ADC
	bool ADC_Driver;
	bool ADC_PGA;
	bool ADC_RAND;
	bool ADC_SHDN;
	bool ADC_DITH;
	//WIFI
	bool WIFI_Enabled;
	char WIFI_AP[MAX_WIFIPASS_LENGTH];
	char WIFI_PASSWORD[MAX_WIFIPASS_LENGTH];
	int8_t WIFI_TIMEZONE;
	bool WIFI_CAT_SERVER;
	//SERVICES
	uint32_t SPEC_Begin;
	uint32_t SPEC_End;
	int16_t SPEC_TopDBM;
	int16_t SPEC_BottomDBM;
	//
	uint8_t ENDBit;
} TRX;

extern struct TRX_CALIBRATE
{
	bool ENCODER_INVERT;
	bool ENCODER2_INVERT;
	uint8_t ENCODER_DEBOUNCE;
	uint8_t ENCODER2_DEBOUNCE;
	uint8_t ENCODER_SLOW_RATE;
	bool ENCODER_ON_FALLING;
	uint8_t CIC_GAINER_val;
	uint8_t CICFIR_GAINER_val;
	uint8_t TXCICFIR_GAINER_val;
	uint8_t DAC_GAINER_val;
	uint8_t rf_out_power_lf;
	uint8_t rf_out_power_hf_low;
	uint8_t rf_out_power_hf;
	uint8_t rf_out_power_hf_high;
	uint8_t rf_out_power_vhf;
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
	uint32_t BPF_HPF;
	float32_t swr_trans_rate;
	int32_t swr_trans_rate_shadow;
	int8_t VCXO_correction;

	uint8_t flash_id; //eeprom check
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
extern void BKPSRAM_Enable(void);
extern void BKPSRAM_Disable(void);
extern VFO *CurrentVFO(void);
extern VFO *SecondaryVFO(void);

#endif
