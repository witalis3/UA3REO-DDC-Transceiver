#include "settings.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include "trx_manager.h"
#include "lcd.h"
#include "fpga.h"
#include "main.h"
#include "bands.h"

//W25Q16
static uint8_t Write_Enable = W25Q16_COMMAND_Write_Enable;
//static uint8_t Erase_Chip = W25Q16_COMMAND_Erase_Chip;
static uint8_t Sector_Erase = W25Q16_COMMAND_Sector_Erase;
static uint8_t Page_Program = W25Q16_COMMAND_Page_Program;
static uint8_t Read_Data = W25Q16_COMMAND_Read_Data;
static uint8_t Address[3] = { 0x00 };
struct TRX_SETTINGS TRX = { 0 };
static uint8_t eeprom_bank = 0;
	
volatile bool NeedSaveSettings = false;
volatile bool EEPROM_Busy = false;

static void Flash_Sector_Erase(void);
static void Flash_Write_Data(void);
static void Flash_Read_Data(void);

struct t_CALIBRATE CALIBRATE = {
	.rf_out_power = {
		33, // 0 mhz
		20, // 1 mhz
		24, // 2 mhz
		42, // 3 mhz
		43, // 4 mhz
		54, // 5 mhz
		63, // 6 mhz
		72, // 7 mhz
		80, // 8 mhz
		86, // 9 mhz
		94, // 10 mhz
		97, // 11 mhz
		100, // 12 mhz
		100, // 13 mhz
		100, // 14 mhz
		100, // 15 mhz
		96, // 16 mhz
		90, // 17 mhz
		83, // 18 mhz
		72, // 19 mhz
		61, // 20 mhz
		50, // 21 mhz
		41, // 22 mhz
		38, // 23 mhz
		43, // 24 mhz
		57, // 25 mhz
		72, // 26 mhz
		88, // 27 mhz
		100, // 28 mhz
		100, // 29 mhz
		100, // 30 mhz
		100    // 31+ mhz
	},
	.adc_calibration = 3.5f,
	.swr_meter_Rtop = 0.1f,
	.swr_meter_Rbottom = 510.0f,
	.swr_meter_fwd_diff = 0.0f,
	.swr_meter_ref_diff = -0.055f,
	.swr_meter_diode_drop = 0.62f,
	.swr_meter_trans_rate = 10.0f,
	.swr_meter_ref_sub = 0.0f,
};

void LoadSettings(bool clear)
{
	Flash_Read_Data();
	eeprom_bank++;
	
	//Проверка, что запись в eeprom была успешна, иначе используем второй банк
	if(TRX.ENDBit != 100)
	{
		Flash_Read_Data();
		eeprom_bank = 0;
		
		if(TRX.ENDBit != 100)
			sendToDebug_str("EEPROM error, loading default...");
	}
	TRX.ENDBit = 100;

	if (TRX.clean_flash != 185 || clear) //code to trace new clean flash
	{
		TRX.clean_flash = 185; //ID прошивки в eeprom, если не совпадает - используем дефолтные
		TRX.VFO_A.Freq = 7100000; //сохранённая частота VFO-A
		TRX.VFO_A.Mode = TRX_MODE_LSB; //сохранённая мода VFO-A
		TRX.VFO_A.Filter_Width = 2700; //сохранённая ширина полосы VFO-A
		TRX.VFO_B.Freq = 14150000; //сохранённая частота VFO-B
		TRX.VFO_B.Mode = TRX_MODE_USB; //сохранённая мода VFO-B
		TRX.VFO_B.Filter_Width = 2700; //сохранённая ширина полосы VFO-B
		TRX.current_vfo = false; // текущая VFO (false - A)
		TRX.Preamp = false; //предусилитель
		TRX.AGC = true; //AGC
		TRX.ATT = false; //аттенюатор
		TRX.LPF = true; //ФНЧ
		TRX.BPF = true; //ДПФ
		TRX.DNR = false; //цифровое шумоподавление
		TRX.AGC_speed = 3; //скорость AGC
		TRX.BandMapEnabled = true; //автоматическая смена моды по карте диапазонов
		TRX.Volume = 20; //громкость
		TRX.InputType_MIC = true; //тип входа для передачи
		TRX.InputType_LINE = false;
		TRX.InputType_USB = false;
		TRX.Fast = true; //ускоренная смена частоты при вращении энкодера
		TRX.CW_Filter = 500; //дефолтное значение ширины фильтра CW
		TRX.SSB_Filter = 2700; //дефолтное значение ширины фильтра SSB
		TRX.FM_Filter = 15000; //дефолтное значение ширины фильтра FM
		TRX.RF_Power = 20; //выходная мощность (%)
		TRX.FM_SQL_threshold = 1; //FM-шумодав
		TRX.RF_Gain = 50; //усиление ВЧ
		for (uint8_t i = 0; i < BANDS_COUNT; i++) TRX.saved_freq[i] = BANDS[i].startFreq + (BANDS[i].endFreq - BANDS[i].startFreq) / 2; //сохранённые частоты по диапазонам
		TRX.FFT_Zoom = 1; //приближение спектра FFT
		TRX.AutoGain = false; //авто-управление предусилителем и аттенюатором
		TRX.NotchFilter = false; //нотч-фильтр для вырезания помехи
		TRX.NotchFC = 1000; //частота среза нотч-фильтра
		TRX.CWDecoder = false; //автоматический декодер телеграфа
		//system settings
		TRX.FFT_Enabled = true; //использовать спектр FFT
		TRX.CW_GENERATOR_SHIFT_HZ = 500; //смещение гетеродина в CW моде
		TRX.ENCODER_SLOW_RATE = 35; //замедление энкодера для больших разрешений
		TRX.LCD_Brightness = 60; //яркость экрана
		TRX.Standby_Time = 180; //время до гашения экрана по бездействию
		TRX.Beeping = true; //звуки нажатия на кнопки
		TRX.Key_timeout = 500; //время отпуская передачи после последнего знака на ключе
		TRX.FFT_Averaging = 4; //усреднение FFT, чтобы сделать его более гладким
		TRX.SSB_HPF_pass = 300; //частота среза ФВЧ в SSB моде
		TRX.WIFI_Enabled = false; //активировать WiFi
		strcpy(TRX.WIFI_AP, "WIFI-AP"); //точка доступа WiFi
		strcpy(TRX.WIFI_PASSWORD, "WIFI-PASSWORD"); //пароль к точке WiFi
		TRX.WIFI_TIMEZONE = 3; //часовой пояс (для синхронизации времени)
		TRX.SPEC_Begin = 700; //старт диапазона анализатора спектра
		TRX.SPEC_End = 800; //старт диапазона анализатора спектра
		TRX.CW_SelfHear = true; //самоконтоль CW
		TRX.ADC_PGA = false; //ADC преамп
		TRX.ADC_RAND = false; //ADC шифрование
		TRX.ADC_SHDN = false; //ADC отключение
		TRX.ADC_DITH = false; //ADC дизеринг
		TRX.FFT_Window = 1;
		TRX.Locked = false; //Блокировка управления
		TRX.CLAR = false; //Режим разноса частот (приём один VFO, передача другой)
		TRX.ENDBit = 100; //Бит окончания успешной записи в eeprom
		SaveSettings();
	}
}

VFO *CurrentVFO(void)
{
	if (!TRX.current_vfo)
		return &TRX.VFO_A;
	else
		return &TRX.VFO_B;
}

void SaveSettings(void)
{
	if(EEPROM_Busy) return;
	NeedSaveSettings = false;
	EEPROM_Busy = true;
	Flash_Write_Data();
	HAL_Delay(EEPROM_OP_DELAY);
	HAL_Delay(EEPROM_OP_DELAY);
	eeprom_bank++;
	if(eeprom_bank >= 2)
		eeprom_bank = 0;
	Flash_Sector_Erase();
	EEPROM_Busy = false;
}

static void Flash_Sector_Erase(void)
{
	//if(eeprom_bank==0) sendToDebug_str("ERASE EEPROM BANK 1\r\n");
	//if(eeprom_bank==1) sendToDebug_str("ERASE EEPROM BANK 2\r\n");
	
	uint32_t BigAddress = eeprom_bank * W25Q16_SECTOR_SIZE;
	Address[0] = BigAddress & 0xFF;
	Address[1] = (BigAddress >> 8) & 0xFF;
	Address[2] = (BigAddress >> 16) & 0xFF;
	
	HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_RESET);     // CS to low
	HAL_SPI_Transmit(&hspi1, &Write_Enable, 1, HAL_MAX_DELAY); // Write Enable Command
	HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_SET);       // CS to high
	HAL_Delay(EEPROM_OP_DELAY);
	HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_RESET);     // CS to low
	HAL_SPI_Transmit(&hspi1, &Sector_Erase, 1, HAL_MAX_DELAY);   // Erase Chip Command
	HAL_SPI_Transmit(&hspi1, Address, sizeof(Address), HAL_MAX_DELAY);      // Write Address ( The first address of flash module is 0x00000000 )
	HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_SET);       // CS to high
	HAL_Delay(EEPROM_OP_DELAY);
}

static void Flash_Write_Data(void)
{
	//if(eeprom_bank==0) sendToDebug_str("SAVE EEPROM BANK 1\r\n");
	//if(eeprom_bank==1) sendToDebug_str("SAVE EEPROM BANK 2\r\n");
	
	for (uint8_t page = 0; page <= (sizeof(TRX) / 0xFF); page++)
	{
		HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_RESET);     // CS to low
		HAL_SPI_Transmit(&hspi1, &Write_Enable, 1, HAL_MAX_DELAY); // Write Enable Command
		HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_SET);       // CS to high

		uint32_t BigAddress = page + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = BigAddress & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = (BigAddress >> 16) & 0xFF;
		uint16_t size = sizeof(TRX) - 0xFF * page;
		if (size > 0xFF) size = 0xFF;

		HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_RESET);     // CS to low
		HAL_SPI_Transmit(&hspi1, &Page_Program, 1, HAL_MAX_DELAY); // Page Program Command
		HAL_SPI_Transmit(&hspi1, Address, sizeof(Address), HAL_MAX_DELAY);      // Write Address ( The first address of flash module is 0x00000000 )
		HAL_SPI_Transmit(&hspi1, (uint8_t*)((uint32_t)&TRX + 0xFF * page), size, HAL_MAX_DELAY);       // Write
		HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_SET);       // CS to high
		HAL_Delay(EEPROM_OP_DELAY);
	}
}

static void Flash_Read_Data(void)
{
	//if(eeprom_bank==0) sendToDebug_str("LOAD EEPROM BANK 1\r\n");
	//if(eeprom_bank==1) sendToDebug_str("LOAD EEPROM BANK 2\r\n");
	
	for (uint8_t page = 0; page <= (sizeof(TRX) / 0xFF); page++)
	{
		uint32_t BigAddress = page + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = BigAddress & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = (BigAddress >> 16) & 0xFF;
		uint16_t size = sizeof(TRX) - 0xFF * page;
		if (size > 0xFF) size = 0xFF;

		HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_RESET);     // CS to low
		HAL_SPI_Transmit(&hspi1, &Read_Data, 1, HAL_MAX_DELAY);  // Read Command
		HAL_SPI_Transmit(&hspi1, Address, sizeof(Address), HAL_MAX_DELAY);    // Write Address
		HAL_SPI_Receive(&hspi1, (uint8_t*)((uint32_t)&TRX + 0xFF * page), size, HAL_MAX_DELAY);      // Read
		HAL_GPIO_WritePin(W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, GPIO_PIN_SET);       // CS to high
		HAL_Delay(EEPROM_OP_DELAY);
	}
}
