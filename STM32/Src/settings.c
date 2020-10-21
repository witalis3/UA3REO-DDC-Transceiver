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
#include "front_unit.h"

char version_string[19] = "2.0.2"; //1.2.3-yymmdd.hhmm (concatinate)

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

void InitSettings(void)
{
	static bool already_inited = false;
	if(already_inited) return;
	already_inited = true;
	
	//concat build date to version -yymmdd.hhmm
	uint8_t cur_len = (uint8_t)strlen(version_string);
	strcat(version_string, "-");
	version_string[++cur_len] = BUILD_YEAR_CH2;
	version_string[++cur_len] = BUILD_YEAR_CH3;
	version_string[++cur_len] = BUILD_MONTH_CH0;
	version_string[++cur_len] = BUILD_MONTH_CH1;
	version_string[++cur_len] = BUILD_DAY_CH0;
	version_string[++cur_len] = BUILD_DAY_CH1;
	version_string[++cur_len] = '.';
	version_string[++cur_len] = BUILD_HOUR_CH0;
	version_string[++cur_len] = BUILD_HOUR_CH1;
	version_string[++cur_len] = BUILD_MIN_CH0;
	version_string[++cur_len] = BUILD_MIN_CH1;
	version_string[++cur_len] = '\0';
	sendToDebug_strln(version_string);
}

void LoadSettings(bool clear)
{
	BKPSRAM_Enable();
	memcpy(&TRX, BACKUP_SRAM_ADDR, sizeof(TRX));
	if (clear)
		memset(&TRX, 0x00, sizeof(TRX));
	// Check, the data in the backup sram is correct, otherwise we use the second bank
	if (TRX.ENDBit != 100)
		sendToDebug_strln("[ERR] BACKUP SRAM incorrect");
	else
		sendToDebug_strln("[OK] BACKUP SRAM data succesfully loaded");
	BKPSRAM_Disable();

	if (TRX.flash_id != SETT_VERSION || clear || TRX.ENDBit != 100) // code to trace new clean flash
	{
		sendToDebug_str("[ERR] Flash ID:");
		sendToDebug_uint8(TRX.flash_id, false);
		TRX.flash_id = SETT_VERSION;		 // Firmware ID in SRAM, if it doesn't match, use the default
		TRX.VFO_A.Freq = 7100000;			 // stored VFO-A frequency
		TRX.VFO_A.Mode = TRX_MODE_LSB;		 // saved VFO-A mode
		TRX.VFO_A.LPF_Filter_Width = 2700;	 // saved bandwidth for VFO-A
		TRX.VFO_A.HPF_Filter_Width = 300;	 // saved bandwidth for VFO-A
		TRX.VFO_A.ManualNotchFilter = false; // notch filter to cut out noise
		TRX.VFO_A.AutoNotchFilter = false;	 // notch filter to cut out noise
		TRX.VFO_A.NotchFC = 1000;			 // cutoff frequency of the notch filter
		TRX.VFO_A.DNR = false;				 // digital noise reduction
		TRX.VFO_A.AGC = true;				 // AGC
		TRX.VFO_B.Freq = 14150000;			 // stored VFO-B frequency
		TRX.VFO_B.Mode = TRX_MODE_USB;		 // saved VFO-B mode
		TRX.VFO_B.LPF_Filter_Width = 2700;	 // saved bandwidth for VFO-B
		TRX.VFO_B.HPF_Filter_Width = 300;	 // saved bandwidth for VFO-B
		TRX.VFO_B.ManualNotchFilter = false; // notch filter to cut out noise
		TRX.VFO_B.AutoNotchFilter = false;	 // notch filter to cut out noise
		TRX.VFO_B.NotchFC = 1000;			 // cutoff frequency of the notch filter
		TRX.VFO_A.DNR = false;				 // digital noise reduction
		TRX.VFO_A.AGC = true;				 // AGC
		TRX.current_vfo = false;			 // current VFO (false - A)
		TRX.ADC_Driver = false;				 // preamplifier (ADC driver)
		TRX.LNA = false;					 // LNA (Low Noise Amplifier)
		TRX.ATT = false;					 // attenuator
		TRX.ATT_DB = 12.0f;					 // suppress the attenuator
		TRX.ATT_STEP = 6.0f;				 // step of tuning the attenuator
		TRX.FM_SQL_threshold = 4;			 // FM noise reduction
		TRX.Fast = true;					 // accelerated frequency change when the encoder rotates
		TRX.ADC_PGA = false;				 // ADC preamp
		TRX.ANT = false;		  // ANT-1
		for (uint8_t i = 0; i < BANDS_COUNT; i++)
		{
			TRX.BANDS_SAVED_SETTINGS[i].Freq = BANDS[i].startFreq + (BANDS[i].endFreq - BANDS[i].startFreq) / 2; // saved frequencies by bands
			TRX.BANDS_SAVED_SETTINGS[i].Mode = (uint8_t)getModeFromFreq(TRX.BANDS_SAVED_SETTINGS[i].Freq);
			TRX.BANDS_SAVED_SETTINGS[i].LNA = TRX.LNA;
			TRX.BANDS_SAVED_SETTINGS[i].ATT = TRX.ATT;
			TRX.BANDS_SAVED_SETTINGS[i].ATT_DB = TRX.ATT_STEP;
			TRX.BANDS_SAVED_SETTINGS[i].ANT = TRX.ANT;
			TRX.BANDS_SAVED_SETTINGS[i].ADC_Driver = TRX.ADC_Driver;
			TRX.BANDS_SAVED_SETTINGS[i].FM_SQL_threshold = TRX.FM_SQL_threshold;
			TRX.BANDS_SAVED_SETTINGS[i].ADC_PGA = TRX.ADC_PGA;
			TRX.BANDS_SAVED_SETTINGS[i].DNR = false;
			TRX.BANDS_SAVED_SETTINGS[i].AGC = true;
		}
		TRX.RF_Filters = true;	  // LPF / HPF / BPF
		TRX.FFT_Zoom = 2;		  // approximation of the FFT spectrum
		TRX.AutoGain = false;	  // auto-control preamp and attenuator
		TRX.CWDecoder = false;	  // automatic telegraph decoder
		TRX.InputType_MIC = true; // type of input to transfer
		TRX.InputType_LINE = false;
		TRX.InputType_USB = false;
		TRX.CW_LPF_Filter = 700;					// default value of CW filter width
		TRX.CW_HPF_Filter = 0;						// default value of CW filter width
		TRX.SSB_LPF_Filter = 2700;					// default value of SSB filter width
		TRX.SSB_HPF_Filter = 300;					// default value of SSB filter width
		TRX.AM_LPF_Filter = 4000;					// default value of AM filter width
		TRX.FM_LPF_Filter = 15000;					// default value of the FM filter width
		TRX.RF_Power = 20;							//output power (%)
		TRX.RX_AGC_speed = 3;						// AGC receive rate
		TRX.TX_AGC_speed = 3;						// AGC transfer rate
		TRX.BandMapEnabled = true;					// automatic change of mode according to the range map
		TRX.FFT_Enabled = true;						// use FFT spectrum
		TRX.CW_GENERATOR_SHIFT_HZ = 500;			// LO offset in CW mode
		TRX.CW_Key_timeout = 500;						// time of releasing transmission after the last character on the key
		TRX.FFT_Averaging = 4;						// averaging the FFT to make it smoother
		TRX.WIFI_Enabled = true;					// activate WiFi
		strcpy(TRX.WIFI_AP, "WIFI-AP");				// WiFi hotspot
		strcpy(TRX.WIFI_PASSWORD, "WIFI-PASSWORD"); // password to the WiFi point
		TRX.WIFI_TIMEZONE = 3;						// time zone (for time synchronization)
		TRX.SPEC_Begin = 1000;						// start spectrum analyzer range
		TRX.SPEC_End = 30000;						// end of spectrum analyzer range
		TRX.SPEC_TopDBM = -60;						// chart thresholds
		TRX.SPEC_BottomDBM = -130;					// chart thresholds
		TRX.CW_SelfHear = true;						// self-control CW
		TRX.ADC_RAND = true;						// ADC encryption (xor randomizer)
		TRX.ADC_SHDN = false;						// ADC disable
		TRX.ADC_DITH = true;						// ADC dither
		TRX.FFT_Window = 1;
		TRX.Locked = false;				 // Lock control
		TRX.CLAR = false;				 // Split frequency mode (receive one VFO, transmit another)
		TRX.TWO_SIGNAL_TUNE = false;	 // Two-signal generator in TUNE mode (1 + 2kHz)
		TRX.IF_Gain = 70;				 // IF gain, dB (before all processing and AGC)
		TRX.CW_KEYER = true;			 // Automatic key
		TRX.CW_KEYER_WPM = 30;			 // Automatic key speed
		TRX.Debug_Console = false;		 // Debug output to DEBUG / UART port
		TRX.Dual_RX = false;					//Dual RX feature
		TRX.Dual_RX_Type = VFO_A_PLUS_B; // dual receiver mode
		TRX.FFT_Color = 1;				 // FFT display color
		TRX.FFT_Height = 2;				 // FFT display height
		TRX.ShiftEnabled = false;		 // activate the SHIFT mode
		TRX.SHIFT_INTERVAL = 5000;		 // Detune range with the SHIFT knob (5000 = -5000hz / + 5000hz)
		TRX.DNR_SNR_THRESHOLD = 50;		 // Digital noise reduction level
		TRX.DNR_AVERAGE = 2;			 // DNR averaging when looking for average magnitude
		TRX.DNR_MINIMAL = 98;			 // DNR averaging when searching for minimum magnitude
		TRX.NOISE_BLANKER = true;		 // suppressor of short impulse noise NOISE BLANKER
		TRX.FRQ_STEP = 10;				 // frequency tuning step by the main encoder
		TRX.FRQ_FAST_STEP = 100;		 // frequency tuning step by the main encoder in FAST mode
		TRX.FRQ_ENC_STEP = 25000;		 // frequency tuning step by main add. encoder
		TRX.FRQ_ENC_FAST_STEP = 100000;	 // frequency tuning step by main add. encoder in FAST mode
		TRX.AGC_GAIN_TARGET = -35;		 // Maximum (target) AGC gain
		TRX.WIFI_CAT_SERVER = true;		 // Server for receiving CAT commands via WIFI
		TRX.MIC_GAIN = 3;				 // Microphone gain
		TRX.RX_EQ_LOW = 0;				 // Receiver Equalizer (Low)
		TRX.RX_EQ_MID = 0;				 // Receiver EQ (mids)
		TRX.RX_EQ_HIG = 0;				 // Receiver EQ (high)
		TRX.MIC_EQ_LOW = 0;				 // Mic EQ (Low)
		TRX.MIC_EQ_MID = 0;				 // Mic Equalizer (Mids)
		TRX.MIC_EQ_HIG = 0;				 // Mic EQ (high)
		TRX.FFT_Speed = 3;				 // FFT Speed

		TRX.ENDBit = 100; // Bit for the end of a successful write to eeprom
		sendToDebug_strln("[OK] Loaded default settings");
		SaveSettings();
	}
}

void LoadCalibration(bool clear)
{
	EEPROM_PowerUp();
	uint8_t tryes = 0;
	while (tryes < EEPROM_REPEAT_TRYES && !EEPROM_Read_Data((uint8_t *)&CALIBRATE, sizeof(CALIBRATE), 0, 0, true, false))
	{
		tryes++;
	}
	if (tryes >= EEPROM_REPEAT_TRYES)
		sendToDebug_strln("[ERR] Read EEPROM CALIBRATE multiple errors");

	if (CALIBRATE.flash_id != CALIB_VERSION || clear) // code for checking the firmware in the eeprom, if it does not match, we use the default
	{
		sendToDebug_str("[ERR] CALIBRATE Flash check CODE:");
		sendToDebug_uint8(CALIBRATE.flash_id, false);
		CALIBRATE.flash_id = CALIB_VERSION; // code for checking the firmware in the eeprom, if it does not match, we use the default

		CALIBRATE.ENCODER_INVERT = false;														// invert left-right rotation of the main encoder
		CALIBRATE.ENCODER2_INVERT = false;														// invert left-right rotation of the optional encoder
		CALIBRATE.ENCODER_DEBOUNCE = 0;															// time to eliminate contact bounce at the main encoder, ms
		CALIBRATE.ENCODER2_DEBOUNCE = 50;														// time to eliminate contact bounce at the additional encoder, ms
		CALIBRATE.ENCODER_SLOW_RATE = 25;														// slow down the encoder for high resolutions
		CALIBRATE.ENCODER_ON_FALLING = false;													// encoder only triggers when level A falls
		CALIBRATE.CIC_GAINER_val = 83;															// Offset from CIC output
		CALIBRATE.CICFIR_GAINER_val = 54;														// Offset from the output of the CIC compensator
		CALIBRATE.TXCICFIR_GAINER_val = 57;														// Offset from the TX-CIC output of the compensator
		CALIBRATE.DAC_GAINER_val = 30;															// DAC offset offset
																								// Calibrate the maximum output power for each band
		CALIBRATE.rf_out_power_lf = 86;														// <2mhz
		CALIBRATE.rf_out_power_hf_low = 56;														// <5mhz
		CALIBRATE.rf_out_power_hf = 41;														// <30mhz
		CALIBRATE.rf_out_power_hf_high = 56;														// <99.84mhz
		CALIBRATE.rf_out_power_vhf = 85;															// 99.84-199.68mhz
		CALIBRATE.smeter_calibration = 0;														// S-Meter calibration, set when calibrating the transceiver to S9 (LPF, BPF, ATT, PREAMP off)
		CALIBRATE.adc_offset = 0;																// Calibrate the offset at the ADC input (DC)
																								// Bandwidth frequency data from BPF filters (taken with GKCH or set by sensitivity), Hz
																								// Next, the average border response frequencies are set
		CALIBRATE.LPF_END = 60000 * 1000;														//LPF
		CALIBRATE.BPF_0_START = 135 * 1000 * 1000;												//2m U14-RF3
		CALIBRATE.BPF_0_END = 150 * 1000 * 1000;												//2m
		CALIBRATE.BPF_1_START = 1500 * 1000;													//160m U16-RF2
		CALIBRATE.BPF_1_END = 2400 * 1000;														//160m
		CALIBRATE.BPF_2_START = 2400 * 1000;													//80m U16-RF4
		CALIBRATE.BPF_2_END = 4700 * 1000;														//80m
		CALIBRATE.BPF_3_START = 4700 * 1000;													//40m U16-RF1
		CALIBRATE.BPF_3_END = 7200 * 1000;														//40m
		CALIBRATE.BPF_4_START = 7200 * 1000;													//30m U16-RF3
		CALIBRATE.BPF_4_END = 11500 * 1000;														//30m
		CALIBRATE.BPF_5_START = 11500 * 1000;													//20,17m U14-RF2
		CALIBRATE.BPF_5_END = 21000 * 1000;														//20,17m
		CALIBRATE.BPF_6_START = 21000 * 1000;													//15,12,10,6m U14-RF4
		CALIBRATE.BPF_6_END = 64000 * 1000;														//15,12,10,6m
		CALIBRATE.BPF_HPF = 60000 * 1000;														//HPF U14-RF1
		CALIBRATE.swr_trans_rate = 11.0f;														//SWR Transormator rate
		CALIBRATE.swr_trans_rate_shadow = (int32_t)(roundf(CALIBRATE.swr_trans_rate * 100.0f)); //SWR Transormator rate UINT shadow
		CALIBRATE.VCXO_correction = 0;															//VCXO Frequency offset

		sendToDebug_strln("[OK] Loaded default calibrate settings");
		SaveCalibration();
	}
	EEPROM_PowerDown();
}

inline VFO *CurrentVFO(void)
{
	if (!TRX.current_vfo)
		return &TRX.VFO_A;
	else
		return &TRX.VFO_B;
}

inline VFO *SecondaryVFO(void)
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
	SCB_CleanDCache_by_Addr((uint32_t *)&TRX, sizeof(TRX));
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
	if (!force && SPI_process)
		return false;
	else
		SPI_process = true;

	for (uint8_t page = 0; page <= (size / 0xFF); page++)
	{
		uint32_t BigAddress = start + page * 0xFF + eeprom_bank * W25Q16_SECTOR_SIZE;
		Address[0] = (BigAddress >> 16) & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = BigAddress & 0xFF;

		SPI_Transmit(&Write_Enable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Enable Command
		HAL_Delay(EEPROM_CO_DELAY);
		SPI_Transmit(&Sector_Erase, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Erase Chip Command
		HAL_Delay(EEPROM_CO_DELAY);
		SPI_Transmit(Address, NULL, 3, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Address ( The first address of flash module is 0x00000000 )
		HAL_Delay(EEPROM_ERASE_DELAY);
		SPI_Transmit(&Write_Disable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Disable Command
		HAL_Delay(EEPROM_CO_DELAY);
	}

	//verify
	if (verify)
	{
		EEPROM_Read_Data(verify_clone, size, start, eeprom_bank, false, true);
		for (uint16_t i = 0; i < size; i++)
			if (verify_clone[i] != 0xFF)
			{
				SPI_process = false;
				return false;
			}
	}
	SPI_process = false;
	return true;
}

static bool EEPROM_Write_Data(uint8_t *Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verify, bool force)
{
	if (!force && !EEPROM_Enabled)
		return true;
	if (!force && SPI_process)
		return false;
	else
		SPI_process = true;

	memcpy(write_clone, Buffer, size);
	for (uint16_t page = 0; page <= (size / 0xFF); page++)
	{
		SPI_Transmit(&Write_Enable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Enable Command
		HAL_Delay(EEPROM_CO_DELAY);

		uint32_t BigAddress = margin_left + page * 0xFF + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = (BigAddress >> 16) & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = BigAddress & 0xFF;
		uint16_t bsize = size - 0xFF * page;
		if (bsize > 0xFF)
			bsize = 0xFF;

		SPI_Transmit(&Page_Program, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Command
		HAL_Delay(EEPROM_CO_DELAY);
		SPI_Transmit(Address, NULL, 3, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Address ( The first address of flash module is 0x00000000 )
		HAL_Delay(EEPROM_AD_DELAY);
		SPI_Transmit((uint8_t *)(write_clone + 0xFF * page), NULL, (uint8_t)bsize, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Data
		HAL_Delay(EEPROM_WR_DELAY);
		SPI_Transmit(&Write_Disable, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Write Disable Command
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
				SPI_process = false;
				return false;
			}
	}
	SPI_process = false;
	return true;
}

static bool EEPROM_Read_Data(uint8_t *Buffer, uint16_t size, uint32_t margin_left, uint8_t eeprom_bank, bool verify, bool force)
{
	if (!force && !EEPROM_Enabled)
		return true;
	if (!force && SPI_process)
		return false;
	else
		SPI_process = true;

	for (uint16_t page = 0; page <= (size / 0xFF); page++)
	{
		uint32_t BigAddress = margin_left + page * 0xFF + (eeprom_bank * W25Q16_SECTOR_SIZE);
		Address[0] = (BigAddress >> 16) & 0xFF;
		Address[1] = (BigAddress >> 8) & 0xFF;
		Address[2] = BigAddress & 0xFF;
		uint16_t bsize = size - 0xFF * page;
		if (bsize > 0xFF)
			bsize = 0xFF;

		bool res = SPI_Transmit(&Read_Data, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Read Command
		HAL_Delay(EEPROM_CO_DELAY);
		if (!res)
		{
			EEPROM_Enabled = false;
			sendToDebug_strln("[ERR] EEPROM not found...");
			LCD_showError("EEPROM init error", true);
			SPI_process = false;
			return true;
		}

		SPI_Transmit(Address, NULL, 3, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, true); // Write Address
		HAL_Delay(EEPROM_AD_DELAY);
		SPI_Transmit(NULL, (uint8_t *)(Buffer + 0xFF * page), (uint8_t)bsize, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Read
		HAL_Delay(EEPROM_RD_DELAY);
	}

	//verify
	if (verify)
	{
		EEPROM_Read_Data(read_clone, size, margin_left, eeprom_bank, false, true);
		for (uint16_t i = 0; i < size; i++)
			if (read_clone[i] != Buffer[i])
			{
				SPI_process = false;
				return false;
			}
	}
	SPI_process = false;
	return true;
}

static void EEPROM_PowerDown(void)
{
	if (!EEPROM_Enabled)
		return;
	SPI_Transmit(&Power_Down, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Power_Down Command
	HAL_Delay(EEPROM_CO_DELAY);
}

static void EEPROM_PowerUp(void)
{
	if (!EEPROM_Enabled)
		return;
	SPI_Transmit(&Power_Up, NULL, 1, W26Q16_CS_GPIO_Port, W26Q16_CS_Pin, false); // Power_Up Command
	HAL_Delay(EEPROM_CO_DELAY);
}

void BKPSRAM_Enable(void)
{
	//__HAL_RCC_BKPRAM_CLK_ENABLE();
	HAL_PWREx_EnableBkUpReg();
	HAL_PWR_EnableBkUpAccess();
}

void BKPSRAM_Disable(void)
{
	HAL_PWR_DisableBkUpAccess();
}
