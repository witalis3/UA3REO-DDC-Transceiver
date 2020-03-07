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
static uint8_t Address[3] = {0x00};
struct TRX_SETTINGS TRX = {0};
static uint8_t eeprom_bank = 0;

volatile bool NeedSaveSettings = false;
volatile bool EEPROM_Busy = false;
static bool EEPROM_Enabled = true;

static void Flash_Sector_Erase(void);
static void Flash_Write_Data(void);
static void Flash_Read_Data(void);

struct t_CALIBRATE CALIBRATE = {
	.rf_out_power = {
		//калибровка выходной мощности на каждый мегагерц
		33,  // 0 mhz
		20,  // 1 mhz
		24,  // 2 mhz
		42,  // 3 mhz
		43,  // 4 mhz
		54,  // 5 mhz
		63,  // 6 mhz
		72,  // 7 mhz
		80,  // 8 mhz
		86,  // 9 mhz
		94,  // 10 mhz
		97,  // 11 mhz
		100, // 12 mhz
		100, // 13 mhz
		100, // 14 mhz
		100, // 15 mhz
		96,  // 16 mhz
		90,  // 17 mhz
		83,  // 18 mhz
		72,  // 19 mhz
		61,  // 20 mhz
		50,  // 21 mhz
		41,  // 22 mhz
		38,  // 23 mhz
		43,  // 24 mhz
		57,  // 25 mhz
		72,  // 26 mhz
		88,  // 27 mhz
		100, // 28 mhz
		100, // 29 mhz
		100, // 30 mhz
		100  // 31+ mhz
	},
	.smeter_calibration = 0.1f,	   //калибровка S-Метра, устанавливается при калибровке трансивера по S9 (LPF, BPF, ATT, PREAMP выключены)
	.swr_meter_Rtop = 0.1f,		   //Верхнее плечо делителя напряжения КСВ метра, ом
	.swr_meter_Rbottom = 510.0f,   //Нижнее плечо делителя напряжения КСВ метра, ом
	.swr_meter_fwd_diff = 0.0f,	//Разница напряжения FWD получаемым АЦП с реальным, в
	.swr_meter_ref_diff = -0.055f, //Разница напряжения REF получаемым АЦП с реальным, в
	.swr_meter_diode_drop = 0.62f, //Падение напряжения на диодах
	.swr_meter_trans_rate = 10.0f, //Коэффициент трансформации КСВ метра
	.swr_meter_ref_sub = 0.0f,	 //% вычитаемого FWD из REF
	.adc_offset = 52, //Калибровка смещения по входу ADC (по DC)
	.att_db = -12.3f,						//подавление в аттенюаторе, dB
	.lna_gain_db = 11.0f,				//усиление в МШУ предусилителе (LNA), dB
	//Данные по пропускной частоте с BPF фильтров (снимаются с помощью ГКЧ или выставляются по чувствительности), гЦ
	//Далее выставляются средние пограничные частоты срабатывания
	.LPF_END = 33000000, //LPH
	.BPF_0_START = 135000000, //UHF
	.BPF_0_END = 150000000, //UHF
	.BPF_1_START = 1500000, //1500000
	.BPF_1_END = 2900000, //3350000
	.BPF_2_START = 2900000, //2500000
	.BPF_2_END = 4840000, //5680000
	.BPF_3_START = 4840000, //4000000
	.BPF_3_END = 7350000, //8100000
	.BPF_4_START = 7350000, //6600000
	.BPF_4_END = 12000000, //13000000
	.BPF_5_START = 12000000, //11000000
	.BPF_5_END = 19000000, //20700000
	.BPF_6_START = 19000000, //17400000
	.BPF_6_END = 30000000, //31000000
	.BPF_7_HPF = 30000000, //HPF
};

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
	Flash_Read_Data();
	eeprom_bank++;

	//Проверка, что запись в eeprom была успешна, иначе используем второй банк
	if (TRX.ENDBit != 100)
	{
		Flash_Read_Data();
		eeprom_bank = 0;

		if (TRX.ENDBit != 100)
			sendToDebug_strln("[ERR] EEPROM data error, loading default...");
		else
			sendToDebug_strln("[OK] EEPROM data succesfully loaded from BANK 2");
	}
	else
		sendToDebug_strln("[OK] EEPROM data succesfully loaded from BANK 1");
	
	TRX.ENDBit = 100;

	if (TRX.clean_flash != 191 || clear) //code to trace new clean flash
	{
		TRX.clean_flash = 191;		   //ID прошивки в eeprom, если не совпадает - используем дефолтные
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
		TRX.CIC_GAINER_val = 88;	  //Смещение с выхода CIC
		TRX.CICFIR_GAINER_val = 54;   //Смещение с выхода CIC компенсатора
		TRX.TXCICFIR_GAINER_val = 32; //Смещение с выхода TX-CIC компенсатора
		TRX.DAC_GAINER_val = 30;	  //Смещение DAC корректора
		TRX.IF_Gain = 60;			  //Усиление ПЧ, dB (до всех обработок и AGC)
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
		
		TRX.ENDBit = 100;			  //Бит окончания успешной записи в eeprom
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
	NeedSaveSettings = false;
	EEPROM_Busy = true;
	Flash_Write_Data();
	HAL_Delay(EEPROM_OP_DELAY);
	HAL_Delay(EEPROM_OP_DELAY);
	eeprom_bank++;
	if (eeprom_bank >= 2)
		eeprom_bank = 0;
	Flash_Sector_Erase();
	EEPROM_Busy = false;
	sendToDebug_strln("EEPROM Saved");
}

static void Flash_Sector_Erase(void)
{
	if (!EEPROM_Enabled)
		return;
	
	for (uint8_t page = 0; page <= (sizeof(TRX) / 0xFF); page++)
	{
		uint32_t BigAddress = W25Q16_MARGIN_LEFT + page * 0xFF + eeprom_bank * W25Q16_SECTOR_SIZE;
		Address[0] = BigAddress & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = (BigAddress >> 16) & 0xFF;
			 
		PERIPH_SPI_Transmit(&Write_Enable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Enable Command
		HAL_Delay(EEPROM_OP_DELAY);
		PERIPH_SPI_Transmit(&Sector_Erase, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Erase Chip Command
		PERIPH_SPI_Transmit(Address, NULL, sizeof(Address), W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Address ( The first address of flash module is 0x00000000 )
		HAL_Delay(EEPROM_OP_DELAY);
	}
	//sendToDebug_str("EEPROM erased bank "); sendToDebug_uint8(eeprom_bank, false);
}

static void Flash_Write_Data(void)
{
	if (!EEPROM_Enabled)
		return;
	
	for (uint8_t page = 0; page <= (sizeof(TRX) / 0xFF); page++)
	{
		PERIPH_SPI_Transmit(&Write_Enable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Enable Command

		uint32_t BigAddress = W25Q16_MARGIN_LEFT + page * 0xFF + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = BigAddress & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = (BigAddress >> 16) & 0xFF;
		uint16_t size = sizeof(TRX) - 0xFF * page;
		if (size > 0xFF)
			size = 0xFF;

		PERIPH_SPI_Transmit(&Page_Program, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Enable Command
		PERIPH_SPI_Transmit(Address, NULL, sizeof(Address), W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Address ( The first address of flash module is 0x00000000 )
		PERIPH_SPI_Transmit((uint8_t *)((uint32_t)&TRX + 0xFF * page), NULL, size, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Address ( The first address of flash module is 0x00000000 )
		HAL_Delay(EEPROM_OP_DELAY);
	}
	//sendToDebug_str("EEPROM saved to bank "); sendToDebug_uint8(eeprom_bank, false);
}

static void Flash_Read_Data(void)
{
	if (!EEPROM_Enabled)
		return;
	
	for (uint8_t page = 0; page <= (sizeof(TRX) / 0xFF); page++)
	{
		uint32_t BigAddress = W25Q16_MARGIN_LEFT + page * 0xFF + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = BigAddress & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = (BigAddress >> 16) & 0xFF;
		uint16_t size = sizeof(TRX) - 0xFF * page;
		if (size > 0xFF)
			size = 0xFF;

		bool res = PERIPH_SPI_Transmit(&Read_Data, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Read Command
		if (!res)
		{
			sendToDebug_uint8(res, false);
			sendToDebug_uint8(hspi2.ErrorCode, false);
			EEPROM_Enabled = false;
			sendToDebug_strln("[ERR] EEPROM not found...");
			LCD_showError("EEPROM init error", true);
			return;
		}
		PERIPH_SPI_Transmit(Address, NULL, sizeof(Address), W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Address
		PERIPH_SPI_Transmit(NULL, (uint8_t *)((uint32_t)&TRX + 0xFF * page), size, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Read
		HAL_Delay(EEPROM_OP_DELAY);
	}
	//sendToDebug_str("EEPROM readed from bank "); sendToDebug_uint8(eeprom_bank, false);
}
