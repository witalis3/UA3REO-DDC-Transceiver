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
//static uint8_t Erase_Chip = W25Q16_COMMAND_Erase_Chip;
static uint8_t Sector_Erase = W25Q16_COMMAND_Sector_Erase;
static uint8_t Page_Program = W25Q16_COMMAND_Page_Program;
static uint8_t Read_Data = W25Q16_COMMAND_Read_Data;
static uint8_t Power_Down = W25Q16_COMMAND_Power_Down;
static uint8_t Power_Up = W25Q16_COMMAND_Power_Up;

static uint8_t Address[3] = {0x00};
struct TRX_SETTINGS TRX = {0};
struct TRX_CALIBRATE CALIBRATE = {0};

static uint8_t write_clone[W25Q16_SECTOR_SIZE] = {0};
static uint8_t read_clone[W25Q16_SECTOR_SIZE] = {0};
static uint8_t verify_clone[W25Q16_SECTOR_SIZE] = {0};
static uint8_t settings_eeprom_bank = 0;

volatile bool NeedSaveSettings = false;
volatile bool NeedSaveCalibration = false;
volatile bool EEPROM_Busy = false;
static bool EEPROM_Enabled = true;

static bool EEPROM_Sector_Erase(uint16_t size, uint32_t start, uint8_t eeprom_bank, bool verify, bool force);
static bool EEPROM_Write_Data(uint8_t* Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verify, bool force);
static bool EEPROM_Read_Data(uint8_t* Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verif, bool force);
static void EEPROM_PowerDown(void);
static void EEPROM_PowerUp(void);

const char *MODE_DESCR[TRX_MODE_COUNT] = {
	"LSB",
	"USB",
	"CW_L",
	"CW_U",
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
	EEPROM_PowerUp();
	uint8_t tryes=0;
	while(tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data((uint8_t *)&TRX, sizeof(TRX), W25Q16_MARGIN_LEFT_SETTINGS, settings_eeprom_bank, true, false)) { tryes++; }
	if(tryes >= EEPROM_REPEAT_TRYES) sendToDebug_strln("[ERR] Read EEPROM multiple errors");
	settings_eeprom_bank++;

	//Проверка, что запись в eeprom была успешна, иначе используем второй банк
	if (TRX.ENDBit != 100)
	{
		tryes=0;
		while(tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data((uint8_t *)&TRX, sizeof(TRX), W25Q16_MARGIN_LEFT_SETTINGS, settings_eeprom_bank, true, false)) { tryes++; }
		if(tryes >= EEPROM_REPEAT_TRYES) sendToDebug_strln("[ERR] Read EEPROM multiple errors");
		settings_eeprom_bank = 0;

		if (TRX.ENDBit != 100)
			sendToDebug_strln("[ERR] EEPROM data error, loading default...");
		else
			sendToDebug_strln("[OK] EEPROM data succesfully loaded from BANK 2");
	}
	else
		sendToDebug_strln("[OK] EEPROM data succesfully loaded from BANK 1");

	TRX.ENDBit = 100;

	if (TRX.flash_id != 191 || clear) //code to trace new clean flash
	{
		sendToDebug_str("[ERR] Flash ID: ");
		sendToDebug_uint8(TRX.flash_id,false);
		TRX.flash_id = 190;		   //ID прошивки в eeprom, если не совпадает - используем дефолтные
		TRX.VFO_A.Freq = 7100000;	  //сохранённая частота VFO-A
		TRX.VFO_A.Mode = TRX_MODE_LSB; //сохранённая мода VFO-A
		TRX.VFO_A.LPF_Filter_Width = 2700; //сохранённая ширина полосы VFO-A
		TRX.VFO_A.HPF_Filter_Width = 300; //сохранённая ширина полосы VFO-A
		TRX.VFO_A.NotchFilter = false;																	  //нотч-фильтр для вырезания помехи
		TRX.VFO_A.NotchFC = 1000;																			  //частота среза нотч-фильтра
		TRX.VFO_A.DNR = false;			   //цифровое шумоподавление
		TRX.VFO_A.AGC = true;				   //AGC
		TRX.VFO_B.Freq = 14150000;	 //сохранённая частота VFO-B
		TRX.VFO_B.Mode = TRX_MODE_USB; //сохранённая мода VFO-B
		TRX.VFO_B.LPF_Filter_Width = 2700; //сохранённая ширина полосы VFO-B
		TRX.VFO_B.HPF_Filter_Width = 300; //сохранённая ширина полосы VFO-B
		TRX.VFO_B.NotchFilter = false;																	  //нотч-фильтр для вырезания помехи
		TRX.VFO_B.NotchFC = 1000;																			  //частота среза нотч-фильтра
		TRX.VFO_A.DNR = false;			   //цифровое шумоподавление
		TRX.VFO_A.AGC = true;				   //AGC
		TRX.current_vfo = false;	   // текущая VFO (false - A)
		TRX.ADC_Driver = false;			   //предусилитель (драйвер АЦП)
		TRX.LNA = false;					//LNA (малошумящий усилитель)
		TRX.ATT = false;			   //аттенюатор
		TRX.FM_SQL_threshold = 1; //FM-шумодав
		TRX.Fast = true;		  //ускоренная смена частоты при вращении энкодера
		TRX.ADC_PGA = false;						//ADC преамп
		for (uint8_t i = 0; i < BANDS_COUNT; i++)
		{
			TRX.BANDS_SAVED_SETTINGS[i].Freq = BANDS[i].startFreq + (BANDS[i].endFreq - BANDS[i].startFreq) / 2; //сохранённые частоты по диапазонам
			if(TRX.BANDS_SAVED_SETTINGS[i].Freq<10000000)
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
		TRX.LPF = true;				   //ФНЧ
		TRX.BPF = true;				   //ДПФ
		TRX.FFT_Zoom = 1;																			  //приближение спектра FFT
		TRX.AutoGain = false;																		  //авто-управление предусилителем и аттенюатором
		TRX.CWDecoder = false;																		  //автоматический декодер телеграфа
		TRX.InputType_MIC = true;	  //тип входа для передачи
		TRX.InputType_LINE = false;
		TRX.InputType_USB = false;
		TRX.CW_LPF_Filter = 500;	  //дефолтное значение ширины фильтра CW
		TRX.CW_HPF_Filter = 0;	  //дефолтное значение ширины фильтра CW
		TRX.SSB_LPF_Filter = 2700;	//дефолтное значение ширины фильтра SSB
		TRX.SSB_HPF_Filter = 300;	//дефолтное значение ширины фильтра SSB
		TRX.AM_LPF_Filter = 4000;	//дефолтное значение ширины фильтра AM
		TRX.FM_LPF_Filter = 15000;	//дефолтное значение ширины фильтра FM
		TRX.RF_Power = 20;		  //выходная мощность (%)
		TRX.RX_AGC_speed = 3;		   //скорость AGC на приём
		TRX.TX_AGC_speed = 3;		   //скорость AGC на передачу
		TRX.BandMapEnabled = true;	 //автоматическая смена моды по карте диапазонов
		TRX.FFT_Enabled = true;						//использовать спектр FFT
		TRX.CW_GENERATOR_SHIFT_HZ = 500;			//смещение гетеродина в CW моде
		TRX.ENCODER_SLOW_RATE = 35;					//замедление энкодера для больших разрешений
		TRX.LCD_Brightness = 60;					//яркость экрана
		TRX.Standby_Time = 180;						//время до гашения экрана по бездействию
		TRX.Key_timeout = 500;						//время отпуская передачи после последнего знака на ключе
		TRX.FFT_Averaging = 4;						//усреднение FFT, чтобы сделать его более гладким
		TRX.WIFI_Enabled = false;					//активировать WiFi
		strcpy(TRX.WIFI_AP, "WIFI-AP");				//точка доступа WiFi
		strcpy(TRX.WIFI_PASSWORD, "WIFI-PASSWORD"); //пароль к точке WiFi
		TRX.WIFI_TIMEZONE = 3;						//часовой пояс (для синхронизации времени)
		TRX.SPEC_Begin = 700;						//старт диапазона анализатора спектра
		TRX.SPEC_End = 800;							//старт диапазона анализатора спектра
		TRX.CW_SelfHear = true;						//самоконтоль CW
		TRX.ADC_RAND = false;						//ADC шифрование
		TRX.ADC_SHDN = false;						//ADC отключение
		TRX.ADC_DITH = false;						//ADC дизеринг
		TRX.FFT_Window = 1;
		TRX.Locked = false;			  //Блокировка управления
		TRX.CLAR = false;			  //Режим разноса частот (приём один VFO, передача другой)
		TRX.TWO_SIGNAL_TUNE = false;  //Двухсигнальный генератор в режиме TUNE (1+2кГц)
		TRX.IF_Gain = 70;			  //Усиление ПЧ, dB (до всех обработок и AGC)
		TRX.CW_KEYER = true;		  //Автоматический ключ
		TRX.CW_KEYER_WPM = 30;		  //Скорость автоматического ключа
		TRX.S_METER_Style = false;	//Вид S-метра (свечка или полоска)
		TRX.Debug_Console = false;  //Вывод отладки в DEBUG/UART порт
		TRX.Dual_RX_Type = VFO_SEPARATE; //режим двойного приёмника
		//сохранение старых значений семплов для DC фильтра. Несколько состояний для разных потребителей
		TRX.DC_Filter_State[0] = (DC_filter_state_type){.x_prev = 0, .y_prev = 0};
		TRX.DC_Filter_State[1] = (DC_filter_state_type){.x_prev = 0, .y_prev = 0};
		TRX.DC_Filter_State[2] = (DC_filter_state_type){.x_prev = 0, .y_prev = 0};
		TRX.DC_Filter_State[3] = (DC_filter_state_type){.x_prev = 0, .y_prev = 0};
		TRX.DC_Filter_State[4] = (DC_filter_state_type){.x_prev = 0, .y_prev = 0};
		TRX.DC_Filter_State[5] = (DC_filter_state_type){.x_prev = 0, .y_prev = 0};
		TRX.DC_Filter_State[6] = (DC_filter_state_type){.x_prev = 0, .y_prev = 0};
		TRX.DC_Filter_State[7] = (DC_filter_state_type){.x_prev = 0, .y_prev = 0};
		TRX.FFT_Style = 1; //стиль отображения FFT
		TRX.ShiftEnabled = false; //активация режима SHIFT
		
		TRX.ENDBit = 100;			  //Бит окончания успешной записи в eeprom
		sendToDebug_strln("[OK] Loaded default settings");
		SaveSettings();
	}
	EEPROM_PowerDown();
}

void LoadCalibration(void)
{
	EEPROM_PowerUp();
	uint8_t tryes=0;
	while(tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data((uint8_t *)&CALIBRATE, sizeof(CALIBRATE), W25Q16_MARGIN_LEFT_CALIBRATION, 0, true, false)) { tryes++; }
	if(tryes >= EEPROM_REPEAT_TRYES) sendToDebug_strln("[ERR] Read EEPROM CALIBRATE multiple errors");

	if (CALIBRATE.flash_id != 190) //code to trace new clean flash
	{
		sendToDebug_str("[ERR] CALIBRATE Flash ID: ");
		sendToDebug_uint8(CALIBRATE.flash_id,false);
		CALIBRATE.flash_id = 190;		   //ID прошивки в eeprom, если не совпадает - используем дефолтные
		
		CALIBRATE.CIC_GAINER_val = 88;	  //Смещение с выхода CIC
		CALIBRATE.CICFIR_GAINER_val = 54;   //Смещение с выхода CIC компенсатора
		CALIBRATE.TXCICFIR_GAINER_val = 56; //Смещение с выхода TX-CIC компенсатора
		CALIBRATE.DAC_GAINER_val = 30;	  //Смещение DAC корректора
		//Калибровка максимальной выходной мощности на каждый диапазон
		CALIBRATE.rf_out_power[0] = 100;  // 0 mhz
		CALIBRATE.rf_out_power[1] = 41;  // 1 mhz
		CALIBRATE.rf_out_power[2] = 32;  // 2 mhz
		CALIBRATE.rf_out_power[3] = 36;  // 3 mhz
		CALIBRATE.rf_out_power[4] = 43;  // 4 mhz
		CALIBRATE.rf_out_power[5] = 52;  // 5 mhz
		CALIBRATE.rf_out_power[6] = 61;  // 6 mhz
		CALIBRATE.rf_out_power[7] = 69;  // 7 mhz
		CALIBRATE.rf_out_power[8] = 76;  // 8 mhz
		CALIBRATE.rf_out_power[9] = 84;  // 9 mhz
		CALIBRATE.rf_out_power[10] = 90;  // 10 mhz
		CALIBRATE.rf_out_power[11] = 96;  // 11 mhz
		CALIBRATE.rf_out_power[12] = 100; // 12 mhz
		CALIBRATE.rf_out_power[13] = 100; // 13 mhz
		CALIBRATE.rf_out_power[14] = 100; // 14 mhz
		CALIBRATE.rf_out_power[15] = 100; // 15 mhz
		CALIBRATE.rf_out_power[16] = 98;  // 16 mhz
		CALIBRATE.rf_out_power[17] = 93;  // 17 mhz
		CALIBRATE.rf_out_power[18] = 86;  // 18 mhz
		CALIBRATE.rf_out_power[19] = 79;  // 19 mhz
		CALIBRATE.rf_out_power[20] = 72;  // 20 mhz
		CALIBRATE.rf_out_power[21] = 63;  // 21 mhz
		CALIBRATE.rf_out_power[22] = 54;  // 22 mhz
		CALIBRATE.rf_out_power[23] = 45;  // 23 mhz
		CALIBRATE.rf_out_power[24] = 41;  // 24 mhz
		CALIBRATE.rf_out_power[25] = 45;  // 25 mhz
		CALIBRATE.rf_out_power[26] = 52;  // 26 mhz
		CALIBRATE.rf_out_power[27] = 64;  // 27 mhz
		CALIBRATE.rf_out_power[28] = 78; // 28 mhz
		CALIBRATE.rf_out_power[29] = 100; // 29 mhz
		CALIBRATE.rf_out_power[30] = 100; // 30 mhz
		CALIBRATE.rf_out_power[31] = 100;  // 31+ mhz
		CALIBRATE.smeter_calibration = 0;	   //калибровка S-Метра, устанавливается при калибровке трансивера по S9 (LPF, BPF, ATT, PREAMP выключены)
		CALIBRATE.adc_offset = 25; //Калибровка смещения по входу ADC (по DC)
		CALIBRATE.att_db = -12;						//подавление в аттенюаторе, dB
		CALIBRATE.lna_gain_db = 11;				//усиление в МШУ предусилителе (LNA), dB
		//Данные по пропускной частоте с BPF фильтров (снимаются с помощью ГКЧ или выставляются по чувствительности), гЦ
		//Далее выставляются средние пограничные частоты срабатывания
		CALIBRATE.LPF_END = 33000000; //LPH
		CALIBRATE.BPF_0_START = 135000000; //UHF
		CALIBRATE.BPF_0_END = 150000000; //UHF
		CALIBRATE.BPF_1_START = 1500000; //1500000
		CALIBRATE.BPF_1_END = 2900000; //3350000
		CALIBRATE.BPF_2_START = 2900000; //2500000
		CALIBRATE.BPF_2_END = 4800000; //5680000
		CALIBRATE.BPF_3_START = 4800000; //4000000
		CALIBRATE.BPF_3_END = 7300000; //8100000
		CALIBRATE.BPF_4_START = 7300000; //6600000
		CALIBRATE.BPF_4_END = 12000000; //13000000
		CALIBRATE.BPF_5_START = 12000000; //11000000
		CALIBRATE.BPF_5_END = 19000000; //20700000
		CALIBRATE.BPF_6_START = 19000000; //17400000
		CALIBRATE.BPF_6_END = 30000000; //31000000
		CALIBRATE.BPF_7_HPF = 30000000; //HPF
		CALIBRATE.swr_trans_rate = 10.0f; //SWR Transormator rate
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
	if (EEPROM_Busy)
		return;
	EEPROM_PowerUp();
	NeedSaveSettings = false;
	EEPROM_Busy = true;
	
	uint8_t tryes=0;
	while(tryes < EEPROM_REPEAT_TRYES && !EEPROM_Write_Data((uint8_t *)&TRX, sizeof(TRX), W25Q16_MARGIN_LEFT_SETTINGS, settings_eeprom_bank, true, false)) { tryes++; }
	if(tryes >= EEPROM_REPEAT_TRYES) sendToDebug_strln("[ERR] Write EEPROM multiple errors");
	
	settings_eeprom_bank++;
	if (settings_eeprom_bank >= 2)
		settings_eeprom_bank = 0;
	
	tryes=0;
	while(tryes < EEPROM_REPEAT_TRYES && !EEPROM_Sector_Erase(sizeof(TRX), W25Q16_MARGIN_LEFT_SETTINGS, settings_eeprom_bank, true, false)){ tryes++; }
	if(tryes >= EEPROM_REPEAT_TRYES) sendToDebug_strln("[ERR] Erase EEPROM multiple errors");
	
	EEPROM_Busy = false;
	EEPROM_PowerDown();
	sendToDebug_strln("[OK] EEPROM Saved");
}

void SaveCalibration(void)
{
	if (EEPROM_Busy)
		return;
	EEPROM_PowerUp();
	NeedSaveCalibration = false;
	EEPROM_Busy = true;
	
	uint8_t tryes=0;	
	while(tryes < EEPROM_REPEAT_TRYES && !EEPROM_Sector_Erase(sizeof(CALIBRATE), W25Q16_MARGIN_LEFT_CALIBRATION, 0, true, false)){ tryes++; }
	if(tryes >= EEPROM_REPEAT_TRYES) sendToDebug_strln("[ERR] Erase EEPROM calibrate multiple errors");
	tryes=0;	
	while(tryes < EEPROM_REPEAT_TRYES && !EEPROM_Write_Data((uint8_t *)&CALIBRATE, sizeof(CALIBRATE), W25Q16_MARGIN_LEFT_CALIBRATION, 0, true, false)) { tryes++; }
	if(tryes >= EEPROM_REPEAT_TRYES) sendToDebug_strln("[ERR] Write EEPROM calibrate multiple errors");
	
	EEPROM_Busy = false;
	EEPROM_PowerDown();
	sendToDebug_strln("[OK] EEPROM Calibrations Saved");
}

static bool EEPROM_Sector_Erase(uint16_t size, uint32_t start, uint8_t eeprom_bank, bool verify, bool force)
{
	if (!force && !EEPROM_Enabled)
		return true;
	if(!force && PERIPH_SPI_process)
		return false;
	else
		PERIPH_SPI_process = true;
	
	for (uint8_t page = 0; page <= (size / 0xFF); page++)
	{
		uint32_t BigAddress = start + page * 0xFF + eeprom_bank * W25Q16_SECTOR_SIZE;
		Address[0] = BigAddress & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = (BigAddress >> 16) & 0xFF;
			 
		PERIPH_SPI_Transmit(&Write_Enable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Enable Command
		HAL_Delay(EEPROM_CO_DELAY);
		PERIPH_SPI_Transmit(&Sector_Erase, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Erase Chip Command
		HAL_Delay(EEPROM_CO_DELAY);
		PERIPH_SPI_Transmit(Address, NULL, sizeof(Address), W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Address ( The first address of flash module is 0x00000000 )
		HAL_Delay(EEPROM_ERASE_DELAY);
	}
	
	//sendToDebug_str("EEPROM erased bank "); sendToDebug_uint8(eeprom_bank, false);
	//verify
	if(verify)
	{
		EEPROM_Read_Data(verify_clone, size, start, eeprom_bank, false, true);
		for(uint16_t i = 0 ; i < size ; i++)
			if(verify_clone[i]!=0xFF)
			{
				//sendToDebug_str("[ERR] EEPROM Erase Error: ");
				//sendToDebug_uint16(i,false);
				//sendToDebug_uint8(verify_clone[i],false);
				PERIPH_SPI_process = false;
				return false;
			}
	}
	PERIPH_SPI_process = false;
	return true;
}

static bool EEPROM_Write_Data(uint8_t* Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verify, bool force)
{
	if (!force && !EEPROM_Enabled)
		return true;
	if(!force && PERIPH_SPI_process)
		return false;
	else
		PERIPH_SPI_process = true;
	
	memcpy(write_clone, Buffer, size);
	for (uint16_t page = 0; page <= (size / 0xFF); page++)
	{
		PERIPH_SPI_Transmit(&Write_Enable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Enable Command
		HAL_Delay(EEPROM_CO_DELAY);
		
		uint32_t BigAddress = margin_left + page * 0xFF + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = BigAddress & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = (BigAddress >> 16) & 0xFF;
		uint16_t bsize = size - 0xFF * page;
		if (bsize > 0xFF)
			bsize = 0xFF;
		
		PERIPH_SPI_Transmit(&Page_Program, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Command
		HAL_Delay(EEPROM_CO_DELAY);
		PERIPH_SPI_Transmit(Address, NULL, sizeof(Address), W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Address ( The first address of flash module is 0x00000000 )
		HAL_Delay(EEPROM_AD_DELAY);
		PERIPH_SPI_Transmit((uint8_t *)(write_clone + 0xFF * page), NULL, (uint8_t)bsize, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Data
		HAL_Delay(EEPROM_WR_DELAY);
	}
	
	//verify
	if(verify)
	{
		EEPROM_Read_Data(verify_clone, size, margin_left, eeprom_bank, false, true);
		for(uint16_t i = 0 ; i < size ; i++)
			if(verify_clone[i]!=write_clone[i])
			{
				//sendToDebug_strln("[ERR] EEPROM Write Error: ");
				//sendToDebug_uint16(i,false);
				//sendToDebug_newline();
				EEPROM_Sector_Erase(size, margin_left, eeprom_bank, true, true);
				PERIPH_SPI_process = false;
				return false;
			}
	}
	PERIPH_SPI_process = false;
	return true;
}

static bool EEPROM_Read_Data(uint8_t* Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verify, bool force)
{
	if (!force && !EEPROM_Enabled)
		return true;
	if(!force && PERIPH_SPI_process)
		return false;
	else
		PERIPH_SPI_process = true;
	
	for (uint16_t page = 0; page <= (size / 0xFF); page++)
	{
		uint32_t BigAddress = margin_left + page * 0xFF + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = BigAddress & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = (BigAddress >> 16) & 0xFF;
		uint16_t bsize = size - 0xFF * page;
		if (bsize > 0xFF)
			bsize = 0xFF;

		bool res = PERIPH_SPI_Transmit(&Read_Data, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Read Command
		HAL_Delay(EEPROM_CO_DELAY);
		if (!res)
		{
			sendToDebug_uint8(res, false);
			sendToDebug_uint32(hspi2.ErrorCode, false);
			EEPROM_Enabled = false;
			sendToDebug_strln("[ERR] EEPROM not found...");
			LCD_showError("EEPROM init error", true);
			PERIPH_SPI_process = false;
			return true;
		}

		PERIPH_SPI_Transmit(Address, NULL, sizeof(Address), W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Address
		HAL_Delay(EEPROM_AD_DELAY);
		PERIPH_SPI_Transmit(NULL, (uint8_t *)(Buffer + 0xFF * page), (uint8_t)bsize, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Read
		HAL_Delay(EEPROM_RD_DELAY);
	}
	
	//sendToDebug_str("EEPROM readed from bank "); sendToDebug_uint8(eeprom_bank, false);
	//verify
	if(verify)
	{
		//memcpy(write_clone, Buffer, size);
		EEPROM_Read_Data(read_clone, size, margin_left, eeprom_bank, false, true);
		for(uint16_t i = 0 ; i < size ; i++)
			if(read_clone[i]!=Buffer[i])
			{
				//sendToDebug_strln("[ERR] EEPROM Read Error: ");
				//sendToDebug_uint16(i,false);
				//sendToDebug_newline();
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
