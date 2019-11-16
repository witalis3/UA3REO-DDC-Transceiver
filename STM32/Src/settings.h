#ifndef SETTINGS_h
#define SETTINGS_h

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdbool.h>
#include "arm_math.h"
#include "bands.h"

#define ADCDAC_CLOCK 122880000 //Частота генератора АЦП/ЦАП
#define MAX_FREQ_HZ 750000000 //Максимальная частота приёма (из даташита АЦП)
#define MAX_TX_FREQ_HZ 60000000 //Максимальная частота передачи (половина от тактового сигнала ЦАП)
#define ADC_BITS 16 //разрядность АЦП
#define FPGA_BUS_BITS 16 //разрядность данных из FPGA
#define TRX_SAMPLERATE 48000 //частота дискретизации аудио-потока
#define MAX_TX_AMPLITUDE 32700.0f //Максимальный размах при передаче в ЦАП (32767.0f - лимит)
#define AGC_CLIP_THRESHOLD 10000 //Максимальный уровень усиления в AGC, выше него происходит клиппинг
#define AGC_OPTIMAL_THRESHOLD 7000 //Рабочий уровень усиления в AGC
#define TUNE_POWER 100 // % от выбранной в настройках мощности при запуске TUNE (100 - полная)
#define TX_AGC_STEPSIZE 50.0f //Время срабатывания компрессора голосового сигнала на передачу (меньше-быстрее)
#define TX_AGC_MAXGAIN 500.0f //Максимальное усиление микрофона при компрессировании
#define TX_AGC_NOISEGATE 15.0f //Минимальный уровень сигнала для усиления (ниже - шум, отрезаем)
#define TOUCHPAD_DELAY 200 //Время защиты от анти-дребезга нажания на тачпад
#define ADC_VREF 2.25f //опорное напряжение АЦП, при подаче на вход которого АЦП отдаёт максимальное значение, вольт
#define ADC_RF_TRANS_RATIO 4 //коэффициент трансформации трансформатора :) на входе АЦП 
#define ATT_DB 12 //подавление в аттенюаторе
#define PREAMP_GAIN_DB 20 //усиление в предусилителе
#define AUTOGAIN_MAX_AMPLITUDE 1100 //максимальная амлитуда, по достижению которой автокорректировщик входных цепей завершает работу, а при переполнении - снижает усиление
#define ENCODER_INVERT 1 //инвертировать вращение влево-вправо у основного энкодера
#define ENCODER2_INVERT 0 //инвертировать вращение влево-вправо у дополнительного энкодера
#define KEY_HOLD_TIME 1000 //время длительного нажатия на кнопку клавиатуры для срабатывания, мс
#define SHIFT_INTERVAL 400.0f //диапазон расстройки ручкой SHIFT (400.0f = -200hz / +200hz)
#define AUTOGAIN_CORRECTOR_WAITSTEP 7 //ожидание усреднения результатов при работе автокорректора входных цепей
#define EEPROM_WRITE_INTERVAL 10 //Запись в EEPROM не чаще, чем раз в 10 секунд (против износа)
#define MAX_RF_POWER 7.0f //Максимум мощности (для шкалы измерителя)

#define ILI9341 true //выбираем используемый дисплей
//#define ILI9325 true //другие комментируем
#define FSMC_REGISTER_SELECT 18 //из FSMC настроек в STM32Cube (A18, A6, и т.д.)
#define SCREEN_ROTATE 0 //перевернуть экран вверх ногами (при смене - перекалибровать тачпад)

//Данные по пропускной частоте с BPF фильтров (снимаются с помощью ГКЧ или выставляются по чувствительности)
//Далее выставляются средние пограничные частоты срабатывания
#define LPF_END 32000000
#define BPF_0_START 132000000
#define BPF_0_END 165000000
#define BPF_1_START 1600000
#define BPF_1_END 2650000
#define BPF_2_START 2650000
#define BPF_2_END 4850000
#define BPF_3_START 4850000
#define BPF_3_END 7500000
#define BPF_4_START 7500000
#define BPF_4_END 12800000
#define BPF_5_START 12800000
#define BPF_5_END 17000000
#define BPF_6_START 17000000
#define BPF_6_END 34000000
#define BPF_7_HPF 34000000

#define W25Q16_COMMAND_Write_Enable 0x06
#define W25Q16_COMMAND_Erase_Chip 0xC7
#define W25Q16_COMMAND_Sector_Erase 0x20
#define W25Q16_COMMAND_Page_Program 0x02
#define W25Q16_COMMAND_Read_Data 0x03
#define W25Q16_SECTOR_SIZE 4096

#define MAX_WIFIPASS_LENGTH 32
#define EEPROM_OP_DELAY 30

extern struct t_CALIBRATE {
	uint8_t rf_out_power[32]; 
	float32_t adc_calibration; 
	float32_t swr_meter_Rtop; 
	float32_t swr_meter_Rbottom; 
	float32_t swr_meter_fwd_diff; 
	float32_t swr_meter_ref_diff; 
	float32_t swr_meter_diode_drop; 
	float32_t swr_meter_trans_rate; 
	float32_t swr_meter_ref_sub; 
} CALIBRATE;

typedef struct {
	uint32_t Freq;
	uint8_t Mode;
	uint16_t Filter_Width;
} VFO;

extern struct TRX_SETTINGS {
	uint8_t clean_flash;
	bool current_vfo; // false - A; true - B
	VFO VFO_A;
	VFO VFO_B;
	bool AGC;
	bool Preamp;
	bool ATT;
	bool LPF;
	bool BPF;
	bool DNR;
	uint8_t AGC_speed;
	bool BandMapEnabled;
	uint8_t Volume;
	bool InputType_MIC;
	bool InputType_LINE;
	bool InputType_USB;
	bool Fast;
	uint16_t CW_Filter;
	uint16_t SSB_Filter;
	uint16_t FM_Filter;
	uint8_t RF_Power;
	uint8_t	FM_SQL_threshold;
	uint32_t saved_freq[BANDS_COUNT];
	uint8_t FFT_Zoom;
	bool AutoGain;
	bool NotchFilter;
	uint16_t NotchFC;
	bool CWDecoder;
	//system settings
	bool FFT_Enabled;
	uint16_t CW_GENERATOR_SHIFT_HZ;
	uint8_t	ENCODER_SLOW_RATE;
	uint8_t LCD_Brightness;
	uint8_t Standby_Time;
	bool Beeping;
	uint16_t Key_timeout;
	uint8_t FFT_Averaging;
	uint16_t SSB_HPF_pass;
	bool WIFI_Enabled;
	char WIFI_AP[MAX_WIFIPASS_LENGTH];
	char WIFI_PASSWORD[MAX_WIFIPASS_LENGTH];
	int8_t WIFI_TIMEZONE;
	uint16_t SPEC_Begin;
	uint16_t SPEC_End;
	uint16_t CW_SelfHear;
	bool ADC_PGA;
	bool ADC_RAND;
	bool ADC_SHDN;
	bool ADC_DITH;
	uint8_t FFT_Window;
	bool Locked;
	bool CLAR;
	bool TWO_SIGNAL_TUNE;
	uint8_t CIC_GAINER_val;
	uint8_t CICFIR_GAINER_val;
	uint8_t ENDBit;
} TRX;

extern volatile bool NeedSaveSettings;
extern volatile bool EEPROM_Busy;
extern SPI_HandleTypeDef hspi1;

extern void LoadSettings(bool clear);
extern void SaveSettings(void);
extern VFO *CurrentVFO(void);

#endif
