#ifndef SETTINGS_h
#define SETTINGS_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdbool.h>
#include "functions.h"
#include "bands.h"

#define TRX_VERSION 192					//Версия прошивки
#define ADCDAC_CLOCK 122880000			//Частота генератора АЦП/ЦАП
#define MAX_FREQ_HZ 750000000			//Максимальная частота приёма (из даташита АЦП)
#define MAX_TX_FREQ_HZ 60000000			//Максимальная частота передачи (половина от тактового сигнала ЦАП)
#define TRX_SAMPLERATE 48000			//частота дискретизации аудио-потока во время обработки
#define IQ_SAMPLERATE 96000			//частота дискретизации аудио-потока с FPGA
#define MAX_TX_AMPLITUDE 1.0f			//Максимальный размах при передаче в ЦАП
#define AGC_MAX_GAIN 50.0f				//Максимальное усиление в AGC, dB
#define AGC_NOISE_GATE -90.0f			//ниже этого уровня - не усиливаем
#define TUNE_POWER 100					//% от выбранной в настройках мощности при запуске TUNE (100 - полная)
#define TX_AGC_MAXGAIN 500.0f			//Максимальное усиление микрофона при компрессировании
#define TX_AGC_NOISEGATE 0.00001f		//Минимальный уровень сигнала для усиления (ниже - шум, отрезаем)
#define TOUCHPAD_DELAY 200				//Время защиты от анти-дребезга нажания на тачпад
#define AUTOGAIN_MAX_AMPLITUDE 16383.0f //максимальная амлитуда, по достижению которой автокорректировщик входных цепей завершает работу, а при переполнении - снижает усиление
#define AUTOGAIN_CORRECTOR_WAITSTEP 7	//ожидание усреднения результатов при работе автокорректора входных цепей
#define KEY_HOLD_TIME 500				//время длительного нажатия на кнопку клавиатуры для срабатывания, мс
#define MAX_RF_POWER 7.0f				//Максимум мощности (для шкалы измерителя)
#define SHOW_LOGO true					//Отображать логотип при загрузке (из images.h)
#define POWERDOWN_TIMEOUT 1000			//время нажатия на кнопку выключения, для срабатывания, мс
#define USB_RESTART_TIMEOUT 5000		//время, через которое происходит рестарт USB если нет пакетов
#define FPGA_FLASH_IN_HEX false					//включить прошивку FPGA в прошивку STM32
#define SNTP_SYNC_INTERVAL (60 * 60)					//Интервал синхронизации времени через NTP, сек
	
#define ILI9481 true					//он же HX8357B //другие комментируем
#define FMC_REMAP true					//ремап памяти FMC
#define FSMC_REGISTER_SELECT 18			//из FSMC настроек в STM32Cube (A18, A6, и т.д.)
#define SCREEN_ROTATE 0					//перевернуть экран вверх ногами

#define ADC_BITS 16																						//разрядность АЦП
#define FPGA_BUS_BITS 32																				//разрядность данных из FPGA
#define CODEC_BITS 32																					//разрядность данных в аудио-кодеке
#define FPGA_BUS_FULL_SCALE 65536																		//максимальная аплитуда сигнала в шине // powf(2,FPGA_BUS_BITS)
#define FPGA_BUS_FULL_SCALE_POW ((float64_t)FPGA_BUS_FULL_SCALE * (float64_t)FPGA_BUS_FULL_SCALE)		//магнитуда максимального сигнала в шине // (FPGA_BUS_FULL_SCALE*FPGA_BUS_FULL_SCALE)
#define CODEC_BITS_FULL_SCALE 4294967296																//максимальная аплитуда сигнала в шине // powf(2,FPGA_BUS_BITS)
#define CODEC_BITS_FULL_SCALE_POW ((float64_t)CODEC_BITS_FULL_SCALE * (float64_t)CODEC_BITS_FULL_SCALE) //магнитуда максимального сигнала в шине // (FPGA_BUS_FULL_SCALE*FPGA_BUS_FULL_SCALE)
#define ADC_FULL_SCALE 65536																			//максимальная аплитуда сигнала в АЦП // powf(2,ADC_BITS)
#define FLOAT_FULL_SCALE_POW 4
#define USB_DEBUG_ENABLED true	 //разрешить использовать USB как консоль
#define UART_DEBUG_ENABLED false //разрешить использовать UART как консоль
#define SWD_DEBUG_ENABLED false	 //разрешить использовать SWD как консоль
#define AUDIO_DECIM_RATE (IQ_SAMPLERATE / TRX_SAMPLERATE)

// задержки при работе с EEPROM
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

//режим работы двойного приёмника
typedef enum
{
	VFO_SEPARATE,
	VFO_A_AND_B,
	VFO_A_PLUS_B,
} DUAL_RX_TYPE;

//Сохранение настроек по бендам
typedef struct
{
	uint32_t Freq;
	uint8_t Mode;
	bool LNA;
	bool ATT;
	bool ADC_Driver;
	bool ADC_PGA;
	uint8_t FM_SQL_threshold;
	bool DNR;
	bool AGC;
} BAND_SAVED_SETTINGS_TYPE;

extern struct TRX_SETTINGS
{
	uint8_t flash_id;
	bool current_vfo; // false - A; true - B
	VFO VFO_A;
	VFO VFO_B;
	bool Fast;
	BAND_SAVED_SETTINGS_TYPE BANDS_SAVED_SETTINGS[BANDS_COUNT];
	bool LNA;
	bool ATT;
	bool LPF;
	bool BPF;
	bool ADC_Driver;
	uint8_t RX_AGC_speed;
	bool BandMapEnabled;
	bool InputType_MIC;
	bool InputType_LINE;
	bool InputType_USB;
	uint16_t CW_LPF_Filter;
	uint16_t CW_HPF_Filter;
	uint16_t SSB_LPF_Filter;
	uint16_t SSB_HPF_Filter;
	uint16_t AM_LPF_Filter;
	uint16_t FM_LPF_Filter;
	uint8_t RF_Power;
	uint8_t FM_SQL_threshold;
	uint8_t FFT_Zoom;
	bool AutoGain;
	bool CWDecoder;
	bool FFT_Enabled;
	uint16_t CW_GENERATOR_SHIFT_HZ;
	uint16_t Key_timeout;
	uint8_t FFT_Averaging;
	bool WIFI_Enabled;
	char WIFI_AP[MAX_WIFIPASS_LENGTH];
	char WIFI_PASSWORD[MAX_WIFIPASS_LENGTH];
	int8_t WIFI_TIMEZONE;
	uint32_t SPEC_Begin;
	uint32_t SPEC_End;
	int16_t SPEC_TopDBM;
	int16_t SPEC_BottomDBM;
	uint16_t CW_SelfHear;
	bool ADC_PGA;
	bool ADC_RAND;
	bool ADC_SHDN;
	bool ADC_DITH;
	uint8_t FFT_Window;
	bool Locked;
	bool CLAR;
	bool TWO_SIGNAL_TUNE;
	uint8_t IF_Gain;
	bool CW_KEYER;
	uint16_t CW_KEYER_WPM;
	bool S_METER_Style;
	uint8_t TX_AGC_speed;
	bool Debug_Console;
	DUAL_RX_TYPE Dual_RX_Type;
	uint8_t FFT_Style;
	bool ShiftEnabled;
	uint16_t SHIFT_INTERVAL;
	uint8_t DNR_SNR_THRESHOLD;
	uint8_t DNR_AVERAGE;
	uint8_t DNR_MINIMAL;
	bool NOISE_BLANKER;
	uint16_t FRQ_STEP;
	uint16_t FRQ_FAST_STEP;
	uint16_t FRQ_ENC_STEP;
	uint32_t FRQ_ENC_FAST_STEP;
	int8_t AGC_GAIN_TARGET;
	bool WIFI_CAT_SERVER;
	
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
	uint8_t rf_out_power[32];
	int16_t smeter_calibration;
	int16_t adc_offset;
	int16_t att_db;
	int16_t lna_gain_db;
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
	uint32_t BPF_7_HPF;
	float32_t swr_trans_rate;
	int32_t swr_trans_rate_shadow;
	
	uint8_t flash_id; //eeprom check
} CALIBRATE;

extern volatile bool NeedSaveSettings;
extern volatile bool NeedSaveCalibration;
extern volatile bool EEPROM_Busy;

extern void LoadSettings(bool clear);
extern void LoadCalibration(void);
extern void SaveSettings(void);
extern void SaveCalibration(void);
extern void BKPSRAM_Enable(void);
extern void BKPSRAM_Disable(void);
extern VFO *CurrentVFO(void);
extern VFO *SecondaryVFO(void);

#endif
