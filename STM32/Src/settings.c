#include "settings.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include "trx_manager.h"
#include "lcd.h"
#include "fpga.h"
#include "main.h"
#include "bands.h"
#include "peripheral.h"

//W25Q16
static uint8_t Write_Enable = W25Q16_COMMAND_Write_Enable;
static uint8_t Write_Disable = W25Q16_COMMAND_Write_Disable;
//static uint8_t Erase_Chip = W25Q16_COMMAND_Erase_Chip;
static uint8_t Sector_Erase = W25Q16_COMMAND_Sector_Erase;
static uint8_t Page_Program = W25Q16_COMMAND_Page_Program;
static uint8_t Read_Data = W25Q16_COMMAND_Read_Data;
static uint8_t Power_Down = W25Q16_COMMAND_Power_Down;
static uint8_t Power_Up = W25Q16_COMMAND_Power_Up;

static uint8_t Address[3] = {0x00};
struct TRX_SETTINGS TRX;
struct TRX_CALIBRATE CALIBRATE = {0};

static uint8_t write_clone[W25Q16_SECTOR_SIZE] = {0};
static uint8_t read_clone[W25Q16_SECTOR_SIZE] = {0};
static uint8_t verify_clone[W25Q16_SECTOR_SIZE] = {0};

volatile bool NeedSaveSettings = false;
volatile bool NeedSaveCalibration = false;
volatile bool EEPROM_Busy = false;
static bool EEPROM_Enabled = true;

static bool EEPROM_Sector_Erase(uint16_t size, uint32_t start, uint8_t eeprom_bank, bool verify, bool force);
static bool EEPROM_Write_Data(uint8_t *Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verify, bool force);
static bool EEPROM_Read_Data(uint8_t *Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verif, bool force);
static void EEPROM_PowerDown(void);
static void EEPROM_PowerUp(void);

const char *MODE_DESCR[TRX_MODE_COUNT] = {
	"LSB",
	"USB",
	"CW-L",
	"CW-U",
	"NFM",
	"WFM",
	"AM",
	"DIGL",
	"DIGU",
	"IQ",
	"LOOP",
	"NOTX",
};

void LoadSettings(bool clear)
{
	BKPSRAM_Enable();
	memcpy(&TRX, BACKUP_SRAM_ADDR, sizeof(TRX));

	//Проверка, данные в backup sram корректные, иначе используем второй банк
	if (TRX.ENDBit != 100)
	{
		sendToDebug_strln("[ERR] BACKUP SRAM bank 1 incorrect, loading from bank 2");
		memcpy(&TRX, BACKUP_SRAM_ADDR + sizeof(TRX), sizeof(TRX));
		if (TRX.ENDBit != 100)
			sendToDebug_strln("[ERR] BACKUP SRAM bank 2 incorrect");
	}
	else
		sendToDebug_strln("[OK] BACKUP SRAM data succesfully loaded");
	BKPSRAM_Disable();

	if (TRX.flash_id != TRX_VERSION || clear || TRX.ENDBit != 100) //code to trace new clean flash
	{
		sendToDebug_str("[ERR] Flash ID: ");
		sendToDebug_uint8(TRX.flash_id, false);
		TRX.flash_id = TRX_VERSION;			 //ID прошивки в eeprom, если не совпадает - используем дефолтные
		TRX.VFO_A.Freq = 7100000;			 //сохранённая частота VFO-A
		TRX.VFO_A.Mode = TRX_MODE_LSB;		 //сохранённая мода VFO-A
		TRX.VFO_A.LPF_Filter_Width = 2700;	 //сохранённая ширина полосы VFO-A
		TRX.VFO_A.HPF_Filter_Width = 300;	 //сохранённая ширина полосы VFO-A
		TRX.VFO_A.ManualNotchFilter = false; //нотч-фильтр для вырезания помехи
		TRX.VFO_A.AutoNotchFilter = false;	 //нотч-фильтр для вырезания помехи
		TRX.VFO_A.NotchFC = 1000;			 //частота среза нотч-фильтра
		TRX.VFO_A.DNR = false;				 //цифровое шумоподавление
		TRX.VFO_A.AGC = true;				 //AGC
		TRX.VFO_B.Freq = 14150000;			 //сохранённая частота VFO-B
		TRX.VFO_B.Mode = TRX_MODE_USB;		 //сохранённая мода VFO-B
		TRX.VFO_B.LPF_Filter_Width = 2700;	 //сохранённая ширина полосы VFO-B
		TRX.VFO_B.HPF_Filter_Width = 300;	 //сохранённая ширина полосы VFO-B
		TRX.VFO_B.ManualNotchFilter = false; //нотч-фильтр для вырезания помехи
		TRX.VFO_B.AutoNotchFilter = false;	 //нотч-фильтр для вырезания помехи
		TRX.VFO_B.NotchFC = 1000;			 //частота среза нотч-фильтра
		TRX.VFO_A.DNR = false;				 //цифровое шумоподавление
		TRX.VFO_A.AGC = true;				 //AGC
		TRX.current_vfo = false;			 // текущая VFO (false - A)
		TRX.ADC_Driver = false;				 //предусилитель (драйвер АЦП)
		TRX.LNA = false;					 //LNA (малошумящий усилитель)
		TRX.ATT = false;					 //аттенюатор
		TRX.FM_SQL_threshold = 4;			 //FM-шумодав
		TRX.Fast = true;					 //ускоренная смена частоты при вращении энкодера
		TRX.ADC_PGA = false;				 //ADC преамп
		for (uint8_t i = 0; i < BANDS_COUNT; i++)
		{
			TRX.BANDS_SAVED_SETTINGS[i].Freq = BANDS[i].startFreq + (BANDS[i].endFreq - BANDS[i].startFreq) / 2; //сохранённые частоты по диапазонам
			if (TRX.BANDS_SAVED_SETTINGS[i].Freq < 10000000)
				TRX.BANDS_SAVED_SETTINGS[i].Mode = TRX_MODE_LSB;
			else
				TRX.BANDS_SAVED_SETTINGS[i].Mode = TRX_MODE_USB;
			TRX.BANDS_SAVED_SETTINGS[i].LNA = false;
			TRX.BANDS_SAVED_SETTINGS[i].ATT = false;
			TRX.BANDS_SAVED_SETTINGS[i].ADC_Driver = false;
			TRX.BANDS_SAVED_SETTINGS[i].FM_SQL_threshold = 1;
			TRX.BANDS_SAVED_SETTINGS[i].ADC_PGA = false;
			TRX.BANDS_SAVED_SETTINGS[i].DNR = false;
			TRX.BANDS_SAVED_SETTINGS[i].AGC = true;
		}
		TRX.LPF = true;			  //ФНЧ
		TRX.BPF = true;			  //ДПФ
		TRX.FFT_Zoom = 1;		  //приближение спектра FFT
		TRX.AutoGain = false;	  //авто-управление предусилителем и аттенюатором
		TRX.CWDecoder = false;	  //автоматический декодер телеграфа
		TRX.InputType_MIC = true; //тип входа для передачи
		TRX.InputType_LINE = false;
		TRX.InputType_USB = false;
		TRX.CW_LPF_Filter = 700;					//дефолтное значение ширины фильтра CW
		TRX.CW_HPF_Filter = 0;						//дефолтное значение ширины фильтра CW
		TRX.SSB_LPF_Filter = 2700;					//дефолтное значение ширины фильтра SSB
		TRX.SSB_HPF_Filter = 300;					//дефолтное значение ширины фильтра SSB
		TRX.AM_LPF_Filter = 4000;					//дефолтное значение ширины фильтра AM
		TRX.FM_LPF_Filter = 15000;					//дефолтное значение ширины фильтра FM
		TRX.RF_Power = 20;							//выходная мощность (%)
		TRX.RX_AGC_speed = 3;						//скорость AGC на приём
		TRX.TX_AGC_speed = 3;						//скорость AGC на передачу
		TRX.BandMapEnabled = true;					//автоматическая смена моды по карте диапазонов
		TRX.FFT_Enabled = true;						//использовать спектр FFT
		TRX.CW_GENERATOR_SHIFT_HZ = 500;			//смещение гетеродина в CW моде
		TRX.Key_timeout = 500;						//время отпуская передачи после последнего знака на ключе
		TRX.FFT_Averaging = 4;						//усреднение FFT, чтобы сделать его более гладким
		TRX.WIFI_Enabled = true;					//активировать WiFi
		strcpy(TRX.WIFI_AP, "WIFI-AP");				//точка доступа WiFi
		strcpy(TRX.WIFI_PASSWORD, "WIFI-PASSWORD"); //пароль к точке WiFi
		TRX.WIFI_TIMEZONE = 3;						//часовой пояс (для синхронизации времени)
		TRX.SPEC_Begin = 1000;						//старт диапазона анализатора спектра
		TRX.SPEC_End = 30000;						//конец диапазона анализатора спектра
		TRX.SPEC_TopDBM = -60;						//пороги графика
		TRX.SPEC_BottomDBM = -130;					//пороги графика
		TRX.CW_SelfHear = true;						//самоконтоль CW
		TRX.ADC_RAND = false;						//ADC шифрование
		TRX.ADC_SHDN = false;						//ADC отключение
		TRX.ADC_DITH = false;						//ADC дизеринг
		TRX.FFT_Window = 1;
		TRX.Locked = false;				 //Блокировка управления
		TRX.CLAR = false;				 //Режим разноса частот (приём один VFO, передача другой)
		TRX.TWO_SIGNAL_TUNE = false;	 //Двухсигнальный генератор в режиме TUNE (1+2кГц)
		TRX.IF_Gain = 70;				 //Усиление ПЧ, dB (до всех обработок и AGC)
		TRX.CW_KEYER = true;			 //Автоматический ключ
		TRX.CW_KEYER_WPM = 30;			 //Скорость автоматического ключа
		TRX.S_METER_Style = false;		 //Вид S-метра (свечка или полоска)
		TRX.Debug_Console = false;		 //Вывод отладки в DEBUG/UART порт
		TRX.Dual_RX_Type = VFO_SEPARATE; //режим двойного приёмника
		TRX.FFT_Style = 1;				 //стиль отображения FFT
		TRX.ShiftEnabled = false;		 //активация режима SHIFT
		TRX.SHIFT_INTERVAL = 5000;		 //Диапазон расстройки ручкой SHIFT (5000 = -5000hz / +5000hz)
		TRX.DNR_SNR_THRESHOLD = 2;		 //Уровень цифрового шумоподавления
		TRX.DNR_AVERAGE = 3;
		TRX.DNR_MINIMAL = 18;
		TRX.NOISE_BLANKER = false;		//подавитель коротких импульсных помех NOISE BLANKER
		TRX.FRQ_STEP = 10;				//шаг перестройки частоты основным энкодером
		TRX.FRQ_FAST_STEP = 100;		//шаг перестройки частоты основным энкодером в режиме FAST
		TRX.FRQ_ENC_STEP = 25000;		//шаг перестройки частоты основным доп. энкодером
		TRX.FRQ_ENC_FAST_STEP = 100000; //шаг перестройки частоты основным доп. энкодером в режиме FAST
		TRX.AGC_GAIN_TARGET = -35; //Максимальное (целевое) усиление AGC
		TRX.WIFI_CAT_SERVER = true;	//Сервер для приёма CAT команд по WIFI
		TRX.MIC_GAIN = 3;			//Усиление микрофона

		TRX.ENDBit = 100; //Бит окончания успешной записи в eeprom
		sendToDebug_strln("[OK] Loaded default settings");
		SaveSettings();
	}
}

void LoadCalibration(void)
{
	EEPROM_PowerUp();
	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data((uint8_t *)&CALIBRATE, sizeof(CALIBRATE), 0, 0, true, false))
	{
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES)
		sendToDebug_strln("[ERR] Read EEPROM CALIBRATE multiple errors");

	if (CALIBRATE.flash_id != 190) //код проверки прошивки в eeprom, если не совпадает - используем дефолтные
	{
		sendToDebug_str("[ERR] CALIBRATE Flash check CODE: ");
		sendToDebug_uint8(CALIBRATE.flash_id, false);
		CALIBRATE.flash_id = 190; //код проверки прошивки в eeprom, если не совпадает - используем дефолтные

		CALIBRATE.ENCODER_INVERT = false; //инвертировать вращение влево-вправо у основного энкодера
		CALIBRATE.ENCODER2_INVERT = false; //инвертировать вращение влево-вправо у дополнительного энкодера
		CALIBRATE.ENCODER_DEBOUNCE = 1; //время для устранения дребезга контактов у основного энкодера, мс
		CALIBRATE.ENCODER2_DEBOUNCE = 50; //время для устранения дребезга контактов у дополнительного энкодера, мс
		CALIBRATE.ENCODER_SLOW_RATE = 25;					//замедление энкодера для больших разрешений
		CALIBRATE.ENCODER_ON_FALLING = false;	//энкодер срабатывает только на падение уровня A
		CALIBRATE.CIC_GAINER_val = 83;		//Смещение с выхода CIC
		CALIBRATE.CICFIR_GAINER_val = 54;	//Смещение с выхода CIC компенсатора
		CALIBRATE.TXCICFIR_GAINER_val = 56; //Смещение с выхода TX-CIC компенсатора
		CALIBRATE.DAC_GAINER_val = 30;		//Смещение DAC корректора
		//Калибровка максимальной выходной мощности на каждый диапазон
		CALIBRATE.rf_out_power[0] = 100;  // 0 mhz
		CALIBRATE.rf_out_power[1] = 51;	  // 1 mhz
		CALIBRATE.rf_out_power[2] = 36;	  // 2 mhz
		CALIBRATE.rf_out_power[3] = 40;	  // 3 mhz
		CALIBRATE.rf_out_power[4] = 48;	  // 4 mhz
		CALIBRATE.rf_out_power[5] = 58;	  // 5 mhz
		CALIBRATE.rf_out_power[6] = 67;	  // 6 mhz
		CALIBRATE.rf_out_power[7] = 79;	  // 7 mhz
		CALIBRATE.rf_out_power[8] = 85;	  // 8 mhz
		CALIBRATE.rf_out_power[9] = 91;	  // 9 mhz
		CALIBRATE.rf_out_power[10] = 98;  // 10 mhz
		CALIBRATE.rf_out_power[11] = 100;  // 11 mhz
		CALIBRATE.rf_out_power[12] = 100; // 12 mhz
		CALIBRATE.rf_out_power[13] = 100; // 13 mhz
		CALIBRATE.rf_out_power[14] = 100; // 14 mhz
		CALIBRATE.rf_out_power[15] = 100; // 15 mhz
		CALIBRATE.rf_out_power[16] = 97;  // 16 mhz
		CALIBRATE.rf_out_power[17] = 88;  // 17 mhz
		CALIBRATE.rf_out_power[18] = 78;  // 18 mhz
		CALIBRATE.rf_out_power[19] = 69;  // 19 mhz
		CALIBRATE.rf_out_power[20] = 60;  // 20 mhz
		CALIBRATE.rf_out_power[21] = 51;  // 21 mhz
		CALIBRATE.rf_out_power[22] = 43;  // 22 mhz
		CALIBRATE.rf_out_power[23] = 37;  // 23 mhz
		CALIBRATE.rf_out_power[24] = 34;  // 24 mhz
		CALIBRATE.rf_out_power[25] = 36;  // 25 mhz
		CALIBRATE.rf_out_power[26] = 41;  // 26 mhz
		CALIBRATE.rf_out_power[27] = 49;  // 27 mhz
		CALIBRATE.rf_out_power[28] = 55;  // 28 mhz
		CALIBRATE.rf_out_power[29] = 60; // 29 mhz
		CALIBRATE.rf_out_power[30] = 64; // 30 mhz
		CALIBRATE.rf_out_power[31] = 66; // 31+ mhz
		CALIBRATE.smeter_calibration = 0; //калибровка S-Метра, устанавливается при калибровке трансивера по S9 (LPF, BPF, ATT, PREAMP выключены)
		CALIBRATE.adc_offset = 0;		  //Калибровка смещения по входу ADC (по DC)
		CALIBRATE.att_db = -12;			  //подавление в аттенюаторе, dB
		CALIBRATE.lna_gain_db = 11;		  //усиление в МШУ предусилителе (LNA), dB
		//Данные по пропускной частоте с BPF фильтров (снимаются с помощью ГКЧ или выставляются по чувствительности), гЦ
		//Далее выставляются средние пограничные частоты срабатывания
		CALIBRATE.LPF_END = 33000000;															//LPH
		CALIBRATE.BPF_0_START = 135000000;														//UHF
		CALIBRATE.BPF_0_END = 150000000;														//UHF
		CALIBRATE.BPF_1_START = 1500000;														//1500000
		CALIBRATE.BPF_1_END = 2900000;															//3350000
		CALIBRATE.BPF_2_START = 2900000;														//2500000
		CALIBRATE.BPF_2_END = 4800000;															//5680000
		CALIBRATE.BPF_3_START = 4800000;														//4000000
		CALIBRATE.BPF_3_END = 7300000;															//8100000
		CALIBRATE.BPF_4_START = 7300000;														//6600000
		CALIBRATE.BPF_4_END = 12000000;															//13000000
		CALIBRATE.BPF_5_START = 12000000;														//11000000
		CALIBRATE.BPF_5_END = 19000000;															//20700000
		CALIBRATE.BPF_6_START = 19000000;														//17400000
		CALIBRATE.BPF_6_END = 30000000;															//31000000
		CALIBRATE.BPF_7_HPF = 30000000;															//HPF
		CALIBRATE.swr_trans_rate = 5.0f;														//SWR Transormator rate
		CALIBRATE.swr_trans_rate_shadow = (int32_t)(roundf(CALIBRATE.swr_trans_rate * 100.0f)); //SWR Transormator rate UINT shadow

		sendToDebug_strln("[OK] Loaded default calibrate settings");
		SaveCalibration();
	}
	EEPROM_PowerDown();	
}

VFO *CurrentVFO(void)
{
	if (!TRX.current_vfo)
		return &TRX.VFO_A;
	else
		return &TRX.VFO_B;
}

VFO *SecondaryVFO(void)
{
	if (!TRX.current_vfo)
		return &TRX.VFO_B;
	else
		return &TRX.VFO_A;
}

void SaveSettings(void)
{
	BKPSRAM_Enable();
	memcpy(BACKUP_SRAM_ADDR, &TRX, sizeof(TRX));
	memcpy(BACKUP_SRAM_ADDR + sizeof(TRX), &TRX, sizeof(TRX));
	SCB_CleanDCache_by_Addr(BACKUP_SRAM_ADDR, 1024 * 4);
	BKPSRAM_Disable();
	NeedSaveSettings = false;
	//sendToDebug_strln("[OK] Settings Saved");
}

void SaveCalibration(void)
{
	if (EEPROM_Busy)
		return;
	EEPROM_PowerUp();
	EEPROM_Busy = true;

	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Sector_Erase(sizeof(CALIBRATE), 0, 0, true, false))
	{
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES)
	{
		sendToDebug_strln("[ERR] Erase EEPROM calibrate multiple errors");
		EEPROM_Busy = false;
		return;
	}
	tryes = 0;
	SCB_CleanDCache();
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Write_Data((uint8_t *)&CALIBRATE, sizeof(CALIBRATE), 0, 0, true, false))
	{
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES)
	{
		sendToDebug_strln("[ERR] Write EEPROM calibrate multiple errors");
		EEPROM_Busy = false;
		return;
	}

	EEPROM_Busy = false;
	EEPROM_PowerDown();
	sendToDebug_strln("[OK] EEPROM Calibrations Saved");
	NeedSaveCalibration = false;
}

static bool EEPROM_Sector_Erase(uint16_t size, uint32_t start, uint8_t eeprom_bank, bool verify, bool force)
{
	if (!force && !EEPROM_Enabled)
		return true;
	if (!force && PERIPH_SPI_process)
		return false;
	else
		PERIPH_SPI_process = true;

	for (uint8_t page = 0; page <= (size / 0xFF); page++)
	{
		uint32_t BigAddress = start + page * 0xFF + eeprom_bank * W25Q16_SECTOR_SIZE;
		Address[0] = (BigAddress >> 16) & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = BigAddress & 0xFF;

		PERIPH_SPI_Transmit(&Write_Enable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Enable Command
		HAL_Delay(EEPROM_CO_DELAY);
		PERIPH_SPI_Transmit(&Sector_Erase, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Erase Chip Command
		HAL_Delay(EEPROM_CO_DELAY);
		PERIPH_SPI_Transmit(Address, NULL, 3, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Address ( The first address of flash module is 0x00000000 )
		HAL_Delay(EEPROM_ERASE_DELAY);
		PERIPH_SPI_Transmit(&Write_Disable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Disable Command
		HAL_Delay(EEPROM_CO_DELAY);
	}

	//verify
	if (verify)
	{
		EEPROM_Read_Data(verify_clone, size, start, eeprom_bank, false, true);
		for (uint16_t i = 0; i < size; i++)
			if (verify_clone[i] != 0xFF)
			{
				PERIPH_SPI_process = false;
				return false;
			}
	}
	PERIPH_SPI_process = false;
	return true;
}

static bool EEPROM_Write_Data(uint8_t *Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verify, bool force)
{
	if (!force && !EEPROM_Enabled)
		return true;
	if (!force && PERIPH_SPI_process)
		return false;
	else
		PERIPH_SPI_process = true;

	memcpy(write_clone, Buffer, size);
	for (uint16_t page = 0; page <= (size / 0xFF); page++)
	{
		PERIPH_SPI_Transmit(&Write_Enable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Enable Command
		HAL_Delay(EEPROM_CO_DELAY);

		uint32_t BigAddress = margin_left + page * 0xFF + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = (BigAddress >> 16) & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = BigAddress & 0xFF;
		uint16_t bsize = size - 0xFF * page;
		if (bsize > 0xFF)
			bsize = 0xFF;

		PERIPH_SPI_Transmit(&Page_Program, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Command
		HAL_Delay(EEPROM_CO_DELAY);
		PERIPH_SPI_Transmit(Address, NULL, 3, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Address ( The first address of flash module is 0x00000000 )
		HAL_Delay(EEPROM_AD_DELAY);
		PERIPH_SPI_Transmit((uint8_t *)(write_clone + 0xFF * page), NULL, (uint8_t)bsize, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Data
		HAL_Delay(EEPROM_WR_DELAY);
		PERIPH_SPI_Transmit(&Write_Disable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Disable Command
		HAL_Delay(EEPROM_CO_DELAY);
	}

	//verify
	if (verify)
	{
		EEPROM_Read_Data(verify_clone, size, margin_left, eeprom_bank, false, true);
		for (uint16_t i = 0; i < size; i++)
			if (verify_clone[i] != write_clone[i])
			{
				EEPROM_Sector_Erase(size, margin_left, eeprom_bank, true, true);
				PERIPH_SPI_process = false;
				return false;
			}
	}
	PERIPH_SPI_process = false;
	return true;
}

static bool EEPROM_Read_Data(uint8_t *Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verify, bool force)
{
	if (!force && !EEPROM_Enabled)
		return true;
	if (!force && PERIPH_SPI_process)
		return false;
	else
		PERIPH_SPI_process = true;

	for (uint16_t page = 0; page <= (size / 0xFF); page++)
	{
		uint32_t BigAddress = margin_left + page * 0xFF + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = (BigAddress >> 16) & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = BigAddress & 0xFF;
		uint16_t bsize = size - 0xFF * page;
		if (bsize > 0xFF)
			bsize = 0xFF;

		bool res = PERIPH_SPI_Transmit(&Read_Data, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Read Command
		HAL_Delay(EEPROM_CO_DELAY);
		if (!res)
		{
			EEPROM_Enabled = false;
			sendToDebug_strln("[ERR] EEPROM not found...");
			LCD_showError("EEPROM init error", true);
			PERIPH_SPI_process = false;
			return true;
		}

		PERIPH_SPI_Transmit(Address, NULL, 3, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Address
		HAL_Delay(EEPROM_AD_DELAY);
		PERIPH_SPI_Transmit(NULL, (uint8_t *)(Buffer + 0xFF * page), (uint8_t)bsize, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Read
		HAL_Delay(EEPROM_RD_DELAY);
	}

	//verify
	if (verify)
	{
		EEPROM_Read_Data(read_clone, size, margin_left, eeprom_bank, false, true);
		for (uint16_t i = 0; i < size; i++)
			if (read_clone[i] != Buffer[i])
			{
				PERIPH_SPI_process = false;
				return false;
			}
	}
	PERIPH_SPI_process = false;
	return true;
}

static void EEPROM_PowerDown(void)
{
	if (!EEPROM_Enabled)
		return;
	PERIPH_SPI_Transmit(&Power_Down, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Power_Down Command
	HAL_Delay(EEPROM_CO_DELAY);
}

static void EEPROM_PowerUp(void)
{
	if (!EEPROM_Enabled)
		return;
	PERIPH_SPI_Transmit(&Power_Up, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Power_Up Command
	HAL_Delay(EEPROM_CO_DELAY);
}

void BKPSRAM_Enable(void)
{
	__HAL_RCC_BKPRAM_CLK_ENABLE();
	HAL_PWREx_EnableBkUpReg();
	HAL_PWR_EnableBkUpAccess();
}

void BKPSRAM_Disable(void)
{
	HAL_PWR_DisableBkUpAccess();
}
