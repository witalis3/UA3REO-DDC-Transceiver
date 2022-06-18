#include "sd.h"
#include "main.h"
#include "fatfs.h"
#include "functions.h"
#include "lcd.h"
#include "ff_gen_drv.h"
#include "user_diskio.h"
#include "system_menu.h"
#include "vocoder.h"
#include "filemanager.h"
#include "fpga.h"
#include "audio_filters.h"

SRAM FATFS SDFatFs = {0};
sd_info_ptr sdinfo = {
	.type = 0,
	.SECTOR_COUNT = 0,
	.BLOCK_SIZE = 512,
	.CAPACITY = 0,
};
extern Disk_drvTypeDef disk;
bool SD_RecordInProcess = false;
bool SD_RecordingCQmessage = false;
TRX_MODE rec_cqmessage_old_mode;
bool SD_PlayInProcess = false;
bool SD_PlayCQMessageInProcess = false;
bool SD_CommandInProcess = false;
bool SD_underrun = false;
bool SD_NeedStopRecord = false;
bool SD_NeedStopPlay = false;
bool SD_Play_Buffer_Ready = false;
uint32_t SD_Play_Buffer_Size = false;
uint32_t SD_RecordBufferIndex = 0;
bool SD_Present = false;
bool SD_BusyByUSB = false;
bool SD_USBCardReader = false;
uint32_t SD_Present_tryTime = 0;
bool SD_Mounted = false;
static SD_COMMAND SD_currentCommand = SDCOMM_IDLE;
uint32_t SDCOMM_WRITE_TO_FILE_partsize = 0;
void (*SDCOMM_WRITE_TO_FILE_callback)(void);

SRAM FIL File = {0};
SRAM static FILINFO fileInfo = {0};
SRAM static DIR dir = {0};
SRAM BYTE SD_workbuffer_A[_MAX_SS] = {0};
SRAM BYTE SD_workbuffer_B[_MAX_SS] = {0};
SRAM static WAV_header wav_hdr = {0};
BYTE SD_workbuffer_current = false; // false - fill A save B, true - fill B save A

static void SDCOMM_CHECKSD_handler(void);
static void SDCOMM_LISTROOT_handler(void);
static void SDCOMM_MKFS_handler(void);
static void SDCOMM_EXPORT_SETT_handler(void);
static void SDCOMM_IMPORT_SETT_handler(void);
static bool SD_WRITE_SETT_LINE(char *name, uint32_t *value, SystemMenuType type);
static bool SD_WRITE_SETT_STRING(char *name, char *value);
static void SDCOMM_PARSE_SETT_LINE(char *line);
static bool SDCOMM_CREATE_RECORD_FILE_handler(void);
static bool SDCOMM_OPEN_PLAY_FILE_handler(void);
static bool SDCOMM_WRITE_PACKET_RECORD_FILE_handler(void);
static void SDCOMM_LIST_DIRECTORY_handler(void);
static void SDCOMM_DELETE_FILE_handler(void);
static void SDCOMM_READ_PLAY_FILE_handler(void);
static void SDCOMM_WRITE_TO_FILE_handler(void);
static bool SDCOMM_CREATE_RECORD_FILE_main(char *filename, bool audio_rec);
static bool SDCOMM_CREATE_CQ_MESSAGE_FILE_handler(void);

bool SD_isIdle(void)
{
	if (SD_currentCommand == SDCOMM_IDLE)
		return true;
	else
		return false;
}

bool SD_doCommand(SD_COMMAND command, bool force)
{
	if (SD_Mounted)
	{
		if (SD_CommandInProcess && !force)
		{
			println("SD command ovverrun: ", SD_currentCommand);
			SD_underrun = true;
		}
		else
		{
			SD_CommandInProcess = true;
			SD_currentCommand = command;
			return true;
		}
		return false;
	}
	else
	{
		TRX_ptt_soft = false;
		TRX_ptt_change();

		if (LCD_systemMenuOpened)
			LCD_showInfo("SD card not found", true);
		else
			LCD_showTooltip("SD card not found");
		return false;
	}
}

void SD_Process(void)
{
	if (SD_BusyByUSB)
		return;

	// Init card
	if (!SD_Present && (HAL_GetTick() - SD_Present_tryTime) > SD_CARD_SCAN_INTERVAL)
	{
		SD_Present_tryTime = HAL_GetTick();
		SD_Mounted = false;

		disk.is_initialized[SDFatFs.drv] = false;
		if (disk_initialize(SDFatFs.drv) == RES_OK)
		{
			println("[OK] SD Card Inserted: ", (uint32_t)(sdinfo.CAPACITY / (uint64_t)1024 / (uint64_t)1024), "Mb");
			SD_Present = true;
			LCD_UpdateQuery.StatusInfoGUI = true;
		}
	}
	// Mount volume
	if (SD_Present && !SD_Mounted)
	{
		uint8_t res = f_mount(&SDFatFs, (TCHAR const *)USERPath, 1);
		if (res != FR_OK)
		{
			println("[ERR] SD cannot be mounted: code ", res);
			SD_Present = false;
		}
		else
		{
			//println("[OK] SD mounted");
			SD_Mounted = true;
		}
	}
	// Do actions
	if (SD_Mounted)
	{
		if (SD_currentCommand != SDCOMM_IDLE)
			SD_Present_tryTime = HAL_GetTick();
		switch (SD_currentCommand)
		{
		case SDCOMM_IDLE:
			// check SD card inserted if idle
			if ((SD_Present_tryTime < HAL_GetTick()) && (HAL_GetTick() - SD_Present_tryTime) > SD_CARD_SCAN_INTERVAL && !SD_RecordInProcess)
			{
				SD_doCommand(SDCOMM_CHECK_SD, false);
				return;
			}
			//
			break;
		case SDCOMM_CHECK_SD:
			SDCOMM_CHECKSD_handler();
			break;
		case SDCOMM_LIST_ROOT:
			SDCOMM_LISTROOT_handler();
			break;
		case SDCOMM_FORMAT:
			SDCOMM_MKFS_handler();
			break;
		case SDCOMM_EXPORT_SETTINGS:
			SDCOMM_EXPORT_SETT_handler();
			break;
		case SDCOMM_IMPORT_SETTINGS:
			SDCOMM_IMPORT_SETT_handler();
			break;
		case SDCOMM_START_RECORD:
			SDCOMM_CREATE_RECORD_FILE_handler();
			break;
		case SDCOMM_PROCESS_RECORD:
			SDCOMM_WRITE_PACKET_RECORD_FILE_handler();
			break;
		case SDCOMM_CREATE_CQ_MESSAGE_FILE:
			SDCOMM_CREATE_CQ_MESSAGE_FILE_handler();
			break;
		case SDCOMM_LIST_DIRECTORY:
			SDCOMM_LIST_DIRECTORY_handler();
			break;
		case SDCOMM_DELETE_FILE:
			SDCOMM_DELETE_FILE_handler();
			break;
		case SDCOMM_START_PLAY:
			SDCOMM_OPEN_PLAY_FILE_handler();
			break;
		case SDCOMM_PROCESS_PLAY:
			SDCOMM_READ_PLAY_FILE_handler();
			break;
		case SDCOMM_FLASH_BIN:
			SDCOMM_FLASH_BIN_handler();
			break;
		case SDCOMM_FLASH_JIC:
			SDCOMM_FLASH_JIC_handler(true);
			break;
		case SDCOMM_WRITE_TO_FILE:
			SDCOMM_WRITE_TO_FILE_handler();
			break;
		}
		SD_CommandInProcess = false;
		SD_currentCommand = SDCOMM_IDLE;
	}
}

static void SDCOMM_WRITE_TO_FILE_handler(void)
{
	if (f_open(&File, (TCHAR *)SD_workbuffer_A, FA_WRITE | FA_OPEN_APPEND) == FR_OK)
	{
		uint32_t byteswritten;
		FRESULT res = f_write(&File, SD_workbuffer_B, SDCOMM_WRITE_TO_FILE_partsize, (void *)&byteswritten);
		f_close(&File);
		if (res != FR_OK || byteswritten == 0)
		{
			println("SD file append error");
		}
		else
		{
			println("SD file data appended ", byteswritten);
			SDCOMM_WRITE_TO_FILE_callback();
		}
	}
	else
	{
		LCD_showTooltip("SD error");
		SD_PlayInProcess = false;
		SD_Present = false;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBar = true;
	}
}

static void SDCOMM_DELETE_FILE_handler(void)
{
	if (f_unlink((TCHAR *)SD_workbuffer_A) || f_unlink((TCHAR *)SD_workbuffer_A)) // two try'es
	{
		println("File deleted: ", (char *)SD_workbuffer_A);
		SD_doCommand(SDCOMM_LIST_DIRECTORY, true);
		SD_Process();
	}
	else
	{
		println("File delete error: ", (char *)SD_workbuffer_A);
	}
	LCD_redraw(false);
}

static void SDCOMM_LIST_DIRECTORY_handler(void)
{
	FILEMANAGER_files_count = 0;
	uint16_t FILEMANAGER_files_added = 0;
	dma_memset(FILEMANAGER_LISTING, 0, sizeof(FILEMANAGER_LISTING));
	if (f_opendir(&dir, FILEMANAGER_CurrentPath) == FR_OK)
	{
		while (f_readdir(&dir, &fileInfo) == FR_OK && fileInfo.fname[0])
		{
			if (fileInfo.fattrib & AM_DIR)
			{
				println("[DIR] ", fileInfo.fname);

				if (FILEMANAGER_files_startindex <= FILEMANAGER_files_count && FILEMANAGER_files_added < FILEMANAGER_LISTING_MAX_FILES)
				{
					strcat(FILEMANAGER_LISTING[FILEMANAGER_files_added], "[DIR] ");
					strncat(FILEMANAGER_LISTING[FILEMANAGER_files_added], fileInfo.fname, (FILEMANAGER_LISTING_MAX_FILELEN - 6));
					FILEMANAGER_files_added++;
				}
				FILEMANAGER_files_count++;
			}
		}
		f_closedir(&dir);

		f_opendir(&dir, FILEMANAGER_CurrentPath);
		while (f_readdir(&dir, &fileInfo) == FR_OK && fileInfo.fname[0])
		{
			if (!(fileInfo.fattrib & AM_DIR))
			{
				println("[FILE] ", fileInfo.fname);
				if (FILEMANAGER_files_startindex <= FILEMANAGER_files_count && FILEMANAGER_files_added < FILEMANAGER_LISTING_MAX_FILES)
				{
					strncat(FILEMANAGER_LISTING[FILEMANAGER_files_added], fileInfo.fname, (FILEMANAGER_LISTING_MAX_FILELEN));
					FILEMANAGER_files_added++;
				}
				FILEMANAGER_files_count++;
			}
		}
		f_closedir(&dir);
		println("read complete");
	}
	else
	{
		LCD_showInfo("SD error", true);
		SYSMENU_eventCloseAllSystemMenu();
		SD_Present = false;
	}
	LCD_UpdateQuery.SystemMenuRedraw = true;
}

static void SDCOMM_CHECKSD_handler(void)
{
	if (f_mount(&SDFatFs, (TCHAR const *)USERPath, 1) == FR_OK)
	{
		//println("[OK] Check SD");
	}
	else
	{
		//println("[ERR] Check SD");
		SD_RecordInProcess = false;
		SD_Present = false;
		LCD_UpdateQuery.StatusInfoGUI = true;
	}
}

static bool SDCOMM_CREATE_RECORD_FILE_handler(void)
{
	char filename[64] = {0};
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	sprintf(filename, "rec-%02d.%02d.%02d-%02d.%02d.%02d-%llu.wav", sDate.Date, sDate.Month, sDate.Year, sTime.Hours, sTime.Minutes, sTime.Seconds, CurrentVFO->Freq);
	println(filename);
	return SDCOMM_CREATE_RECORD_FILE_main(filename, true);
}

static bool SDCOMM_CREATE_CQ_MESSAGE_FILE_handler(void)
{
	return SDCOMM_CREATE_RECORD_FILE_main(SD_CQ_MESSAGE_FILE, false);
}

static bool SDCOMM_CREATE_RECORD_FILE_main(char *filename, bool audio_rec)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	if (f_open(&File, filename, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
	{
		dma_memset(SD_workbuffer_A, 0x00, sizeof(SD_workbuffer_A));
		dma_memset(&wav_hdr, 0x00, sizeof(wav_hdr));
		// RIFF header
		wav_hdr.riffsig = 0x46464952;
		wav_hdr.filesize = sizeof(wav_hdr);
		wav_hdr.wavesig = 0x45564157;

		// format chunk
		wav_hdr.fmtsig = 0x20746D66;
		// wav_hdr.fmtsize		= 16;  //PCM
		wav_hdr.fmtsize = 0x14; // IMA-ADPCM
		// wav_hdr.type			= 1; //PCM
		wav_hdr.type = 0x11; // IMA-ADPCM
		wav_hdr.nch = 1;	 // Mono
		wav_hdr.freq = 48000;
		// wav_hdr.rate			= 96000; //PCM 16bit
		wav_hdr.rate = (wav_hdr.freq * wav_hdr.nch * 256 / 505); // IMA-ADPCM byte rate, 48000 * Nch * 256 / 505
		// wav_hdr.block			= 2; //PCM
		wav_hdr.block = 256; // IMA-ADPCM block align, mono 256, stereo 512 */
		// wav_hdr.bits			= 16; //PCM
		wav_hdr.bits = 4;			  // IMA-ADPCM
		wav_hdr.bytes_extra_data = 2; // IMA-ADPCM bytes extra data
		wav_hdr.extra_data = 505;	  // IMA-ADPCM extra data

		// data chunk
		wav_hdr.datasig = 0x61746164;
		wav_hdr.datasize = 0;

		uint32_t byteswritten;
		f_write(&File, &wav_hdr, sizeof(wav_hdr), &byteswritten);

		if (audio_rec)
		{
			SD_RecordInProcess = true;
			LCD_UpdateQuery.StatusInfoBar = true;
			LCD_showTooltip("Start recording");
		}
		else
		{
			LCD_UpdateQuery.SystemMenuRedraw = true;
		}
		return true;
	}
	else
	{
		LCD_showTooltip("SD error");
		SD_RecordInProcess = false;
		SD_Present = false;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBar = true;
	}
	if (!audio_rec)
	{
		LCD_UpdateQuery.SystemMenuRedraw = true;
	}
	return false;
}

static bool SDCOMM_WRITE_PACKET_RECORD_FILE_handler(void)
{
	// reopen cq message
	static bool need_cqmess_reopen = true;
	if (SD_RecordingCQmessage)
	{
		if (need_cqmess_reopen)
		{
			f_open(&File, SD_CQ_MESSAGE_FILE, FA_WRITE | FA_OPEN_EXISTING);
			f_lseek(&File, sizeof(wav_hdr));
			need_cqmess_reopen = false;
		}
	}
	// write to SD
	uint32_t byteswritten;
	FRESULT res;
	if (SD_workbuffer_current)
		res = f_write(&File, SD_workbuffer_A, sizeof(SD_workbuffer_A), (void *)&byteswritten);
	else
		res = f_write(&File, SD_workbuffer_B, sizeof(SD_workbuffer_B), (void *)&byteswritten);
	if ((byteswritten == 0) || (res != FR_OK))
	{
		SD_Present = false;
		SD_NeedStopRecord = false;
		SD_RecordInProcess = false;
		if (SD_RecordingCQmessage)
		{
			TRX_setMode(rec_cqmessage_old_mode, CurrentVFO);
			SD_RecordingCQmessage = false;
		}
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBar = true;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		need_cqmess_reopen = true;
		LCD_showTooltip("SD error");
		return false;
	}
	else
	{
		// store size
		wav_hdr.datasize += byteswritten;
	}

	// stop record
	if (SD_NeedStopRecord)
	{
		SD_RecordInProcess = false;
		if (SD_RecordingCQmessage)
		{
			TRX_setMode(rec_cqmessage_old_mode, CurrentVFO);
			SD_RecordingCQmessage = false;
		}
		LCD_UpdateQuery.StatusInfoBar = true;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		need_cqmess_reopen = true;
		LCD_showTooltip("Stop recording");

		// update wav length
		f_lseek(&File, 0);
		f_write(&File, &wav_hdr, sizeof(wav_hdr), &byteswritten);

		f_close(&File);
		SD_NeedStopRecord = false;
		return true;
	}
	return true;
}

void SDCOMM_FLASH_BIN_handler(void)
{
	if (f_open(&File, (TCHAR *)SD_workbuffer_A, FA_READ | FA_OPEN_EXISTING) == FR_OK)
	{
		dma_memset(SD_workbuffer_A, 0x00, sizeof(SD_workbuffer_A));
		println("[FLASH] File Opened");
		TRX_Mute = true;
		WM8731_CleanBuffer();

		// SCB_DisableICache();
		// SCB_DisableDCache();
		HAL_FLASH_OB_Unlock();
		HAL_FLASH_Unlock();
		println("[FLASH] Unlocked");

		FLASH_OBProgramInitTypeDef OBInit;
		OBInit.OptionType = OPTIONBYTE_WRP;
		OBInit.Banks = FLASH_BANK_2;
		OBInit.WRPState = OB_WRPSTATE_DISABLE;
		OBInit.WRPSector = OB_WRP_SECTOR_ALL;
		HAL_FLASHEx_OBProgram(&OBInit);
		if (HAL_FLASH_OB_Launch() != HAL_OK)
		{
			println("[FLASH] WP disable error");
			return;
		}
		println("[FLASH] WP disabled");

		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
		FLASH_EraseInitTypeDef EraseInitStruct;
		EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		EraseInitStruct.Sector = 0;
		EraseInitStruct.NbSectors = 8;
		EraseInitStruct.Banks = FLASH_BANK_2;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_MASSERASE;
		uint32_t SectorError = 0;
		LCD_showInfo("Erasing...", false);
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
		{
			HAL_FLASH_Lock();
			LCD_showInfo("Flash erase error", true);
			return;
		}
		println("[FLASH] Erased");

		LCD_showInfo("Programming...", false);
		bool read_flag = true;
		uint32_t bytesreaded = 0;
		uint32_t LastPGAddress = 0x08100000;	// second bank
		const uint32_t flash_word_size = 8 * 4; // 8x 32bits words
		while (read_flag == true)
		{
			if (f_read(&File, SD_workbuffer_A, sizeof(SD_workbuffer_A), (void *)&bytesreaded) != FR_OK)
			{
				LCD_showInfo("File read error", true);
				return;
			}

			if (bytesreaded > 0)
			{
				for (uint32_t block_addr = 0; block_addr < bytesreaded; block_addr += flash_word_size)
				{
					// println("[FLASH] Programming: ", LastPGAddress);
					// print_flush();
					uint8_t res = HAL_FLASH_Program(0, LastPGAddress, (uint32_t)&SD_workbuffer_A[block_addr]);
					if (res != HAL_OK)
					{
						LCD_showInfo("Flashing error", true);
						// println("[FLASH] Flashing error: ", res, " ", HAL_FLASH_GetError());
						return;
					}
					// Check the written value
					if (*(uint32_t *)LastPGAddress != *(uint32_t *)(SD_workbuffer_A + block_addr))
					{
						LCD_showInfo("Flash Verify error", true);
						return;
					}
					// println("[FLASH] Block flashed: ", LastPGAddress);
					// print_flush();

					LastPGAddress += flash_word_size;
				}

				println("[FLASH] Flashed: ", LastPGAddress);
				print_flush();
			}
			else
			{
				read_flag = false;
			}
		}

		LCD_showInfo("Flashed", true);

		// First part finished, swap banks

		// Get the Dual boot configuration status
		HAL_FLASHEx_OBGetConfig(&OBInit);
		// Get FLASH_WRP_SECTORS write protection status
		OBInit.Banks = FLASH_BANK_1;
		HAL_FLASHEx_OBGetConfig(&OBInit);

		uint32_t i = 0;
		if ((OBInit.USERConfig & OB_SWAP_BANK_ENABLE) == OB_SWAP_BANK_DISABLE)
		{
			// Swap to bank2
			// Set OB SWAP_BANK_OPT to swap Bank2
			OBInit.OptionType = OPTIONBYTE_USER;
			OBInit.USERType = OB_USER_SWAP_BANK;
			OBInit.USERConfig = OB_SWAP_BANK_ENABLE;
			HAL_FLASHEx_OBProgram(&OBInit);
			// Launch Option bytes loading
			// HAL_FLASH_OB_Launch();
			HAL_SuspendTick();
			__disable_irq();   // Disable all interrupts
			SysTick->CTRL = 0; // Disable Systick timer
			SysTick->VAL = 0;
			SysTick->LOAD = 0;
			HAL_RCC_DeInit();		// Set the clock to the default state
			for (i = 0; i < 5; i++) // Clear Interrupt Enable Register & Interrupt Pending Register
			{
				NVIC->ICER[i] = 0xFFFFFFFF;
				NVIC->ICPR[i] = 0xFFFFFFFF;
			}
			SET_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTSTART);
			while (READ_BIT(FLASH->OPTSR_CUR, FLASH_OPTSR_OPT_BUSY) != 0U)
			{
			}
			// FLASH_OB_WaitForLastOperation(100);
			// HAL_NVIC_SystemReset();
			SCB->AIRCR = 0x05FA0004; // software reset
		}
		else
		{
			// Swap to bank1
			// Set OB SWAP_BANK_OPT to swap Bank1
			OBInit.OptionType = OPTIONBYTE_USER;
			OBInit.USERType = OB_USER_SWAP_BANK;
			OBInit.USERConfig = OB_SWAP_BANK_DISABLE;
			HAL_FLASHEx_OBProgram(&OBInit);
			// Launch Option bytes loading
			// HAL_FLASH_OB_Launch();
			HAL_SuspendTick();
			__disable_irq();   // Disable all interrupts
			SysTick->CTRL = 0; // Disable Systick timer
			SysTick->VAL = 0;
			SysTick->LOAD = 0;
			HAL_RCC_DeInit();		// Set the clock to the default state
			for (i = 0; i < 5; i++) // Clear Interrupt Enable Register & Interrupt Pending Register
			{
				NVIC->ICER[i] = 0xFFFFFFFF;
				NVIC->ICPR[i] = 0xFFFFFFFF;
			}
			SET_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTSTART);
			while (READ_BIT(FLASH->OPTSR_CUR, FLASH_OPTSR_OPT_BUSY) != 0U)
			{
			}
			// FLASH_OB_WaitForLastOperation(100);
			// HAL_NVIC_SystemReset();
			SCB->AIRCR = 0x05FA0004; // software reset
		}

		println("[FLASH] Banks swapped");
		LCD_showInfo("Finished...", true);
	}
	else
	{
		LCD_showInfo("SD error", true);
		SD_PlayInProcess = false;
		SD_Present = false;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBar = true;
	}
}

void SDCOMM_FLASH_JIC_handler(bool restart)
{
	if (f_open(&File, (TCHAR *)SD_workbuffer_A, FA_READ | FA_OPEN_EXISTING) == FR_OK)
	{
		dma_memset(SD_workbuffer_A, 0x00, sizeof(SD_workbuffer_A));
		println("[FLASH] File Opened");
		TRX_Mute = true;
		WM8731_CleanBuffer();

		FPGA_bus_stop = true;
		HAL_Delay(100);
		if (!FPGA_is_present())
			return;

		bool verify_error = true;
		while (verify_error)
		{
			LCD_showInfo("Erasing...", false);
			FPGA_spi_flash_erase();

			LCD_showInfo("Programming...", false);
			f_lseek(&File, FPGA_flash_file_offset);
			uint32_t fpga_flash_pos = 0;
			uint32_t bytesreaded = 0;
			bool read_in_progress = true;
			while (read_in_progress)
			{
				if (f_read(&File, SD_workbuffer_A, sizeof(SD_workbuffer_A), (void *)&bytesreaded) != FR_OK)
				{
					LCD_showInfo("File read error...", true);
					return;
				}

				if (bytesreaded > 0)
				{
					FPGA_spi_flash_write(fpga_flash_pos, (uint8_t *)SD_workbuffer_A, bytesreaded);
					fpga_flash_pos += bytesreaded;
					if (fpga_flash_pos >= FPGA_flash_size)
						break;
				}
				else
					read_in_progress = false;
			}

			LCD_showInfo("Verify...", false);
			f_lseek(&File, FPGA_flash_file_offset);
			fpga_flash_pos = 0;
			bytesreaded = 0;
			read_in_progress = true;
			while (read_in_progress)
			{
				if (f_read(&File, SD_workbuffer_A, sizeof(SD_workbuffer_A), (void *)&bytesreaded) != FR_OK)
				{
					LCD_showInfo("File read error...", true);
					return;
				}

				if (bytesreaded > 0)
				{
					verify_error = !FPGA_spi_flash_verify(fpga_flash_pos, (uint8_t *)SD_workbuffer_A, bytesreaded);
					fpga_flash_pos += bytesreaded;
					if (fpga_flash_pos >= FPGA_flash_size)
						break;
					if (verify_error)
						break;
				}
				else
					read_in_progress = false;
			}
		}

		f_close(&File);
		LCD_showInfo("Finished...", true);
		// HAL_NVIC_SystemReset();
		if (restart)
			SCB->AIRCR = 0x05FA0004; // software reset
	}
	else
	{
		LCD_showInfo("SD error", true);
		SD_PlayInProcess = false;
		SD_Present = false;
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBar = true;
	}
}

static bool SDCOMM_OPEN_PLAY_FILE_handler(void)
{
	if (f_open(&File, (TCHAR *)SD_workbuffer_A, FA_READ | FA_OPEN_EXISTING) == FR_OK)
	{
		dma_memset(SD_workbuffer_A, 0x00, sizeof(SD_workbuffer_A));
		dma_memset(&wav_hdr, 0x00, sizeof(wav_hdr));

		// read header
		uint32_t bytesreaded;
		FRESULT res = f_read(&File, &wav_hdr, sizeof(wav_hdr), &bytesreaded);
		// println((TCHAR*)SD_workbuffer_A);
		// println(bytesreaded, " ", res);

		SD_PlayInProcess = true;
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return true;
	}
	else
	{
		LCD_showTooltip("SD error");
		SD_PlayInProcess = false;
		SD_Present = false;
		TRX_ptt_soft = false;
		TRX_ptt_change();
		LCD_UpdateQuery.StatusInfoGUI = true;
		LCD_UpdateQuery.StatusInfoBar = true;
	}
	return false;
}

static void SDCOMM_READ_PLAY_FILE_handler(void)
{
	// read from SD
	uint32_t bytesreaded;
	FRESULT res;
	if (SD_workbuffer_current)
		res = f_read(&File, SD_workbuffer_A, sizeof(SD_workbuffer_A), (void *)&bytesreaded);
	else
		res = f_read(&File, SD_workbuffer_B, sizeof(SD_workbuffer_B), (void *)&bytesreaded);
	if ((bytesreaded == 0) || (res != FR_OK) || SD_NeedStopPlay)
	{
		// println(bytesreaded, " ", res, " ", (uint8_t)SD_NeedStopPlay);
		SD_PlayInProcess = false;
		if (SD_PlayCQMessageInProcess)
		{
			TRX_ptt_soft = false;
			TRX_ptt_change();
			SD_PlayCQMessageInProcess = false;
		}
		LCD_UpdateQuery.SystemMenuRedraw = true;
		println("Stop WAV playing");
		f_close(&File);
		SD_NeedStopPlay = false;
	}
	else
	{
		SD_Play_Buffer_Ready = true;
		SD_Play_Buffer_Size = bytesreaded;
	}
}

static bool SD_WRITE_SETT_LINE(char *name, uint32_t *value, SystemMenuType type)
{
	uint32_t byteswritten;
	char valbuff[64] = {0};
	float32_t tmp_float = 0;

	dma_memset(SD_workbuffer_A, 0x00, sizeof(SD_workbuffer_A));

	strcat((char *)SD_workbuffer_A, name);
	strcat((char *)SD_workbuffer_A, " = ");
	switch (type)
	{
	case SYSMENU_BOOLEAN:
		sprintf(valbuff, "%u", (uint8_t)*value);
		break;
	case SYSMENU_B4:
	case SYSMENU_UINT8:
	case SYSMENU_ATU_I:
	case SYSMENU_ATU_C:
		sprintf(valbuff, "%u", (uint8_t)*value);
		break;
	case SYSMENU_ENUM:
		sprintf(valbuff, "%u", (uint8_t)*value);
		break;
	case SYSMENU_ENUMR:
		sprintf(valbuff, "%u", (uint8_t)*value);
		break;
	case SYSMENU_UINT16:
		sprintf(valbuff, "%u", (uint16_t)*value);
		break;
	case SYSMENU_UINT32:
		sprintf(valbuff, "%u", (uint32_t)*value);
		break;
	case SYSMENU_UINT64:
		sprintf(valbuff, "%llu", (uint64_t)*value);
		break;
	case SYSMENU_INT8:
		sprintf(valbuff, "%d", (int8_t)*value);
		break;
	case SYSMENU_INT16:
		sprintf(valbuff, "%d", (int16_t)*value);
		break;
	case SYSMENU_INT32:
		sprintf(valbuff, "%d", (int32_t)*value);
		break;
	case SYSMENU_FLOAT32:
		dma_memcpy(&tmp_float, value, sizeof(float32_t));
		sprintf(valbuff, "%.6f", (double)tmp_float);
		break;
	case SYSMENU_FUNCBUTTON:
		sprintf(valbuff, "%u", (uint8_t)*value);
		break;
	case SYSMENU_RUN:
	case SYSMENU_UINT32R:
	case SYSMENU_MENU:
	case SYSMENU_INFOLINE:
		break;
	}
	strcat((char *)SD_workbuffer_A, valbuff);
	strcat((char *)SD_workbuffer_A, "\r\n");

	FRESULT res = f_write(&File, SD_workbuffer_A, strlen((char *)SD_workbuffer_A), (void *)&byteswritten);
	if ((byteswritten == 0) || (res != FR_OK))
	{
		SD_Present = false;
		return false;
	}
	return true;
}

static bool SD_WRITE_SETT_STRING(char *name, char *value)
{
	uint32_t byteswritten;

	dma_memset(SD_workbuffer_A, 0x00, sizeof(SD_workbuffer_A));

	strcat((char *)SD_workbuffer_A, name);
	strcat((char *)SD_workbuffer_A, " = ");
	strcat((char *)SD_workbuffer_A, value);
	strcat((char *)SD_workbuffer_A, "\r\n");

	FRESULT res = f_write(&File, SD_workbuffer_A, strlen((char *)SD_workbuffer_A), (void *)&byteswritten);
	if ((byteswritten == 0) || (res != FR_OK))
	{
		SD_Present = false;
		return false;
	}
	return true;
}

static void SDCOMM_EXPORT_SETT_handler(void)
{
	LCD_showInfo("Exporting...", false);
	if (f_open(&File, "wolf.ini", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
	{
		// TRX
		bool res = SD_WRITE_SETT_LINE("TRX.VFO_A.Freq", (uint32_t *)&TRX.VFO_A.Freq, SYSMENU_UINT64);
		if (res)
		{
			SD_WRITE_SETT_LINE("TRX.VFO_A.Mode", (uint32_t *)&TRX.VFO_A.Mode, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_A.LPF_RX_Filter_Width", (uint32_t *)&TRX.VFO_A.LPF_RX_Filter_Width, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_A.LPF_TX_Filter_Width", (uint32_t *)&TRX.VFO_A.LPF_TX_Filter_Width, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_A.HPF_RX_Filter_Width", (uint32_t *)&TRX.VFO_A.HPF_RX_Filter_Width, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_A.HPF_TX_Filter_Width", (uint32_t *)&TRX.VFO_A.HPF_TX_Filter_Width, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_A.ManualNotchFilter", (uint32_t *)&TRX.VFO_A.ManualNotchFilter, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VFO_A.AutoNotchFilter", (uint32_t *)&TRX.VFO_A.AutoNotchFilter, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VFO_A.NotchFC", (uint32_t *)&TRX.VFO_A.NotchFC, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_A.DNR_Type", (uint32_t *)&TRX.VFO_A.DNR_Type, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.VFO_A.AGC", (uint32_t *)&TRX.VFO_A.AGC, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VFO_A.SQL", (uint32_t *)&TRX.VFO_A.SQL, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VFO_A.FM_SQL_threshold_dbm", (uint32_t *)&TRX.VFO_A.FM_SQL_threshold_dbm, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.VFO_B.Freq", (uint32_t *)&TRX.VFO_B.Freq, SYSMENU_UINT64);
			SD_WRITE_SETT_LINE("TRX.VFO_B.Mode", (uint32_t *)&TRX.VFO_B.Mode, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_B.LPF_RX_Filter_Width", (uint32_t *)&TRX.VFO_B.LPF_RX_Filter_Width, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_B.LPF_TX_Filter_Width", (uint32_t *)&TRX.VFO_B.LPF_TX_Filter_Width, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_B.HPF_RX_Filter_Width", (uint32_t *)&TRX.VFO_B.HPF_RX_Filter_Width, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_B.HPF_TX_Filter_Width", (uint32_t *)&TRX.VFO_B.HPF_TX_Filter_Width, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_B.ManualNotchFilter", (uint32_t *)&TRX.VFO_B.ManualNotchFilter, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VFO_B.AutoNotchFilter", (uint32_t *)&TRX.VFO_B.AutoNotchFilter, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VFO_B.NotchFC", (uint32_t *)&TRX.VFO_B.NotchFC, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.VFO_B.DNR_Type", (uint32_t *)&TRX.VFO_B.DNR_Type, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.VFO_B.AGC", (uint32_t *)&TRX.VFO_B.AGC, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VFO_B.SQL", (uint32_t *)&TRX.VFO_B.SQL, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VFO_B.FM_SQL_threshold_dbm", (uint32_t *)&TRX.VFO_B.FM_SQL_threshold_dbm, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.selected_vfo", (uint32_t *)&TRX.selected_vfo, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ADC_Driver", (uint32_t *)&TRX.ADC_Driver, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.LNA", (uint32_t *)&TRX.LNA, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ATT", (uint32_t *)&TRX.ATT, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ATT_DB", (uint32_t *)&TRX.ATT_DB, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("TRX.ATT_STEP", (uint32_t *)&TRX.ATT_STEP, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.Fast", (uint32_t *)&TRX.Fast, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ADC_PGA", (uint32_t *)&TRX.ADC_PGA, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ANT_selected", (uint32_t *)&TRX.ANT_selected, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ANT_mode", (uint32_t *)&TRX.ANT_mode, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.RF_Filters", (uint32_t *)&TRX.RF_Filters, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.RF_Power", (uint32_t *)&TRX.RF_Power, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.ChannelMode", (uint32_t *)&TRX.ChannelMode, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.RIT_Enabled", (uint32_t *)&TRX.RIT_Enabled, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.XIT_Enabled", (uint32_t *)&TRX.XIT_Enabled, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FineRITTune", (uint32_t *)&TRX.FineRITTune, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.RIT_INTERVAL", (uint32_t *)&TRX.RIT_INTERVAL, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.XIT_INTERVAL", (uint32_t *)&TRX.XIT_INTERVAL, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.TWO_SIGNAL_TUNE", (uint32_t *)&TRX.TWO_SIGNAL_TUNE, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.SAMPLERATE_MAIN", (uint32_t *)&TRX.SAMPLERATE_MAIN, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.SAMPLERATE_FM", (uint32_t *)&TRX.SAMPLERATE_FM, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FRQ_STEP", (uint32_t *)&TRX.FRQ_STEP, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.FRQ_FAST_STEP", (uint32_t *)&TRX.FRQ_FAST_STEP, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.FRQ_ENC_STEP", (uint32_t *)&TRX.FRQ_ENC_STEP, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.FRQ_ENC_FAST_STEP", (uint32_t *)&TRX.FRQ_ENC_FAST_STEP, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.FRQ_CW_STEP_DIVIDER", (uint32_t *)&TRX.FRQ_CW_STEP_DIVIDER, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.Debug_Type", (uint32_t *)&TRX.Debug_Type, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.BandMapEnabled", (uint32_t *)&TRX.BandMapEnabled, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.InputType_MAIN", (uint32_t *)&TRX.InputType_MAIN, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.InputType_DIGI", (uint32_t *)&TRX.InputType_DIGI, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.AutoGain", (uint32_t *)&TRX.AutoGain, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.SPLIT_Enabled", (uint32_t *)&TRX.SPLIT_Enabled, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Dual_RX", (uint32_t *)&TRX.Dual_RX, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Encoder_Accelerate", (uint32_t *)&TRX.Encoder_Accelerate, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Dual_RX_Type", (uint32_t *)&TRX.Dual_RX_Type, SYSMENU_UINT8);
			SD_WRITE_SETT_STRING("TRX.CALLSIGN", TRX.CALLSIGN);
			SD_WRITE_SETT_STRING("TRX.LOCATOR", TRX.LOCATOR);
			SD_WRITE_SETT_LINE("TRX.Custom_Transverter_Enabled", (uint32_t *)&TRX.Custom_Transverter_Enabled, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Transverter_Offset_Mhz", (uint32_t *)&TRX.Transverter_Offset_Mhz, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.ATU_Enabled", (uint32_t *)&TRX.ATU_Enabled, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.TUNER_Enabled", (uint32_t *)&TRX.TUNER_Enabled, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Transverter_70cm", (uint32_t *)&TRX.Transverter_70cm, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Transverter_23cm", (uint32_t *)&TRX.Transverter_23cm, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Transverter_13cm", (uint32_t *)&TRX.Transverter_13cm, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Transverter_6cm", (uint32_t *)&TRX.Transverter_6cm, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Transverter_3cm", (uint32_t *)&TRX.Transverter_3cm, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.Auto_Input_Switch", (uint32_t *)&TRX.Auto_Input_Switch, SYSMENU_BOOLEAN);
			// AUDIO
			SD_WRITE_SETT_LINE("TRX.IF_Gain", (uint32_t *)&TRX.IF_Gain, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.AGC_GAIN_TARGET2", (uint32_t *)&TRX.AGC_GAIN_TARGET, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.MIC_GAIN", (uint32_t *)&TRX.MIC_GAIN, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.MIC_Boost", (uint32_t *)&TRX.MIC_Boost, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.MIC_NOISE_GATE", (uint32_t *)&TRX.MIC_NOISE_GATE, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.RX_EQ_LOW", (uint32_t *)&TRX.RX_EQ_LOW, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.RX_EQ_MID", (uint32_t *)&TRX.RX_EQ_MID, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.RX_EQ_HIG", (uint32_t *)&TRX.RX_EQ_HIG, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.MIC_EQ_LOW_SSB", (uint32_t *)&TRX.MIC_EQ_LOW_SSB, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.MIC_EQ_MID_SSB", (uint32_t *)&TRX.MIC_EQ_MID_SSB, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.MIC_EQ_HIG_SSB", (uint32_t *)&TRX.MIC_EQ_HIG_SSB, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.MIC_EQ_LOW_AMFM", (uint32_t *)&TRX.MIC_EQ_LOW_AMFM, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.MIC_EQ_MID_AMFM", (uint32_t *)&TRX.MIC_EQ_MID_AMFM, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.MIC_EQ_HIG_AMFM", (uint32_t *)&TRX.MIC_EQ_HIG_AMFM, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.MIC_REVERBER", (uint32_t *)&TRX.MIC_REVERBER, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.DNR1_SNR_THRESHOLD", (uint32_t *)&TRX.DNR1_SNR_THRESHOLD, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.DNR2_SNR_THRESHOLD", (uint32_t *)&TRX.DNR2_SNR_THRESHOLD, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.DNR_AVERAGE", (uint32_t *)&TRX.DNR_AVERAGE, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.DNR_MINIMAL", (uint32_t *)&TRX.DNR_MINIMAL, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.NOISE_BLANKER", (uint32_t *)&TRX.NOISE_BLANKER, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.RX_AGC_SSB_speed", (uint32_t *)&TRX.RX_AGC_SSB_speed, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.RX_AGC_CW_speed", (uint32_t *)&TRX.RX_AGC_CW_speed, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.RX_AGC_Max_gain", (uint32_t *)&TRX.RX_AGC_Max_gain, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.RX_AGC_Hold", (uint32_t *)&TRX.RX_AGC_Hold, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.TX_Compressor_speed_SSB", (uint32_t *)&TRX.TX_Compressor_speed_SSB, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.TX_Compressor_maxgain_SSB", (uint32_t *)&TRX.TX_Compressor_maxgain_SSB, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.TX_Compressor_speed_AMFM", (uint32_t *)&TRX.TX_Compressor_speed_AMFM, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.TX_Compressor_maxgain_AMFM", (uint32_t *)&TRX.TX_Compressor_maxgain_AMFM, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.CW_LPF_Filter", (uint32_t *)&TRX.CW_LPF_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.DIGI_LPF_Filter", (uint32_t *)&TRX.DIGI_LPF_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.SSB_LPF_RX_Filter", (uint32_t *)&TRX.SSB_LPF_RX_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.SSB_LPF_TX_Filter", (uint32_t *)&TRX.SSB_LPF_TX_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.SSB_HPF_RX_Filter", (uint32_t *)&TRX.SSB_HPF_RX_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.SSB_HPF_TX_Filter", (uint32_t *)&TRX.SSB_HPF_TX_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.AM_LPF_RX_Filter", (uint32_t *)&TRX.AM_LPF_RX_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.AM_LPF_TX_Filter", (uint32_t *)&TRX.AM_LPF_TX_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.FM_LPF_RX_Filter", (uint32_t *)&TRX.FM_LPF_RX_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.FM_LPF_TX_Filter", (uint32_t *)&TRX.FM_LPF_TX_Filter, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.Beeper", (uint32_t *)&TRX.Beeper, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.CTCSS_Freq", (uint32_t *)&TRX.CTCSS_Freq, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("TRX.SELFHEAR_Volume", (uint32_t *)&TRX.SELFHEAR_Volume, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FM_Stereo", (uint32_t *)&TRX.FM_Stereo, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.AGC_Spectral", (uint32_t *)&TRX.AGC_Spectral, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VAD_THRESHOLD", (uint32_t *)&TRX.VAD_THRESHOLD, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.VOX", (uint32_t *)&TRX.VOX, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.VOX_TIMEOUT", (uint32_t *)&TRX.VOX_TIMEOUT, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.VOX_THRESHOLD", (uint32_t *)&TRX.VOX_THRESHOLD, SYSMENU_INT8);
			// CW
			SD_WRITE_SETT_LINE("TRX.CW_Pitch", (uint32_t *)&TRX.CW_Pitch, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.CW_Key_timeout", (uint32_t *)&TRX.CW_Key_timeout, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.CW_SelfHear", (uint32_t *)&TRX.CW_SelfHear, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.CW_KEYER", (uint32_t *)&TRX.CW_KEYER, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.CW_KEYER_WPM", (uint32_t *)&TRX.CW_KEYER_WPM, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.CW_GaussFilter", (uint32_t *)&TRX.CW_GaussFilter, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.CW_DotToDashRate", (uint32_t *)&TRX.CW_DotToDashRate, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("TRX.CW_Iambic", (uint32_t *)&TRX.CW_Iambic, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.CW_Invert", (uint32_t *)&TRX.CW_Invert, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.CW_PTT_Type", (uint32_t *)&TRX.CW_PTT_Type, SYSMENU_UINT8);
			// SCREEN
			SD_WRITE_SETT_LINE("TRX.ColorThemeId", (uint32_t *)&TRX.ColorThemeId, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.LayoutThemeId", (uint32_t *)&TRX.LayoutThemeId, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_Enabled", (uint32_t *)&TRX.FFT_Enabled, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_Zoom", (uint32_t *)&TRX.FFT_Zoom, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_ZoomCW", (uint32_t *)&TRX.FFT_ZoomCW, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.LCD_Brightness", (uint32_t *)&TRX.LCD_Brightness, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.LCD_SleepTimeout", (uint32_t *)&TRX.LCD_SleepTimeout, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.FFT_Speed", (uint32_t *)&TRX.FFT_Speed, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_Sensitivity", (uint32_t *)&TRX.FFT_Sensitivity, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_Averaging", (uint32_t *)&TRX.FFT_Averaging, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_Window", (uint32_t *)&TRX.FFT_Window, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_Height", (uint32_t *)&TRX.FFT_Height, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_Style", (uint32_t *)&TRX.FFT_Style, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_BW_Style", (uint32_t *)&TRX.FFT_BW_Style, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_Color", (uint32_t *)&TRX.FFT_Color, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.WTF_Color", (uint32_t *)&TRX.WTF_Color, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_Compressor", (uint32_t *)&TRX.FFT_Compressor, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WTF_Moving", (uint32_t *)&TRX.WTF_Moving, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_FreqGrid", (uint32_t *)&TRX.FFT_FreqGrid, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.FFT_dBmGrid", (uint32_t *)&TRX.FFT_dBmGrid, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_Background", (uint32_t *)&TRX.FFT_Background, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_Lens", (uint32_t *)&TRX.FFT_Lens, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_HoldPeaks", (uint32_t *)&TRX.FFT_HoldPeaks, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_3D", (uint32_t *)&TRX.FFT_3D, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.FFT_Automatic", (uint32_t *)&TRX.FFT_Automatic, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_ManualBottom", (uint32_t *)&TRX.FFT_ManualBottom, SYSMENU_INT16);
			SD_WRITE_SETT_LINE("TRX.FFT_ManualTop", (uint32_t *)&TRX.FFT_ManualTop, SYSMENU_INT16);
			SD_WRITE_SETT_LINE("TRX.FFT_DXCluster", (uint32_t *)&TRX.FFT_DXCluster, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_DXCluster_Azimuth", (uint32_t *)&TRX.FFT_DXCluster_Azimuth, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_DXCluster_Timeout", (uint32_t *)&TRX.FFT_DXCluster_Timeout, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.Show_Sec_VFO", (uint32_t *)&TRX.Show_Sec_VFO, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.FFT_Scale_Type", (uint32_t *)&TRX.FFT_Scale_Type, SYSMENU_UINT8);
			// DECODER
			SD_WRITE_SETT_LINE("TRX.CW_Decoder", (uint32_t *)&TRX.CW_Decoder, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.CW_Decoder_Threshold", (uint32_t *)&TRX.CW_Decoder_Threshold, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.RDS_Decoder", (uint32_t *)&TRX.RDS_Decoder, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.RTTY_Speed", (uint32_t *)&TRX.RTTY_Speed, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.RTTY_Shift", (uint32_t *)&TRX.RTTY_Shift, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.RTTY_Freq", (uint32_t *)&TRX.RTTY_Freq, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("TRX.RTTY_StopBits", (uint32_t *)&TRX.RTTY_StopBits, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("TRX.RTTY_InvertBits", (uint32_t *)&TRX.RTTY_InvertBits, SYSMENU_BOOLEAN);
			// ADC
			SD_WRITE_SETT_LINE("TRX.ADC_Driver", (uint32_t *)&TRX.ADC_Driver, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ADC_PGA", (uint32_t *)&TRX.ADC_PGA, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ADC_RAND", (uint32_t *)&TRX.ADC_RAND, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ADC_SHDN", (uint32_t *)&TRX.ADC_SHDN, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.ADC_DITH", (uint32_t *)&TRX.ADC_DITH, SYSMENU_BOOLEAN);
			// WIFI
			SD_WRITE_SETT_LINE("TRX.WIFI_Enabled", (uint32_t *)&TRX.WIFI_Enabled, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WIFI_TIMEZONE", (uint32_t *)&TRX.WIFI_TIMEZONE, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("TRX.WIFI_CAT_SERVER", (uint32_t *)&TRX.WIFI_CAT_SERVER, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_STRING("TRX.WIFI_AP1", TRX.WIFI_AP1);
			SD_WRITE_SETT_STRING("TRX.WIFI_AP2", TRX.WIFI_AP2);
			SD_WRITE_SETT_STRING("TRX.WIFI_AP3", TRX.WIFI_AP3);
			SD_WRITE_SETT_STRING("TRX.WIFI_PASSWORD1", TRX.WIFI_PASSWORD1);
			SD_WRITE_SETT_STRING("TRX.WIFI_PASSWORD2", TRX.WIFI_PASSWORD2);
			SD_WRITE_SETT_STRING("TRX.WIFI_PASSWORD3", TRX.WIFI_PASSWORD3);
			// SERVICES
			SD_WRITE_SETT_LINE("TRX.SWR_CUSTOM_Begin", (uint32_t *)&TRX.SWR_CUSTOM_Begin, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.SWR_CUSTOM_End", (uint32_t *)&TRX.SWR_CUSTOM_End, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.SPEC_Begin", (uint32_t *)&TRX.SPEC_Begin, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.SPEC_End", (uint32_t *)&TRX.SPEC_End, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("TRX.SPEC_TopDBM", (uint32_t *)&TRX.SPEC_TopDBM, SYSMENU_INT16);
			SD_WRITE_SETT_LINE("TRX.SPEC_BottomDBM", (uint32_t *)&TRX.SPEC_BottomDBM, SYSMENU_INT16);
			SD_WRITE_SETT_LINE("TRX.WSPR_FREQ_OFFSET", (uint32_t *)&TRX.WSPR_FREQ_OFFSET, SYSMENU_INT16);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_160", (uint32_t *)&TRX.WSPR_BANDS_160, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_80", (uint32_t *)&TRX.WSPR_BANDS_80, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_40", (uint32_t *)&TRX.WSPR_BANDS_40, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_30", (uint32_t *)&TRX.WSPR_BANDS_30, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_20", (uint32_t *)&TRX.WSPR_BANDS_20, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_17", (uint32_t *)&TRX.WSPR_BANDS_17, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_15", (uint32_t *)&TRX.WSPR_BANDS_15, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_12", (uint32_t *)&TRX.WSPR_BANDS_12, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_10", (uint32_t *)&TRX.WSPR_BANDS_10, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_6", (uint32_t *)&TRX.WSPR_BANDS_6, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("TRX.WSPR_BANDS_2", (uint32_t *)&TRX.WSPR_BANDS_2, SYSMENU_BOOLEAN);
			// CALIBRATION
			SD_WRITE_SETT_LINE("CALIBRATE.ENCODER_INVERT", (uint32_t *)&CALIBRATE.ENCODER_INVERT, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.ENCODER2_INVERT", (uint32_t *)&CALIBRATE.ENCODER2_INVERT, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.ENCODER_DEBOUNCE", (uint32_t *)&CALIBRATE.ENCODER_DEBOUNCE, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.ENCODER2_DEBOUNCE", (uint32_t *)&CALIBRATE.ENCODER2_DEBOUNCE, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.ENCODER_SLOW_RATE", (uint32_t *)&CALIBRATE.ENCODER_SLOW_RATE, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.ENCODER_ON_FALLING", (uint32_t *)&CALIBRATE.ENCODER_ON_FALLING, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.ENCODER_ACCELERATION", (uint32_t *)&CALIBRATE.ENCODER_ACCELERATION, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.RF_unit_type", (uint32_t *)&CALIBRATE.RF_unit_type, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.TangentType", (uint32_t *)&CALIBRATE.TangentType, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.CICFIR_GAINER_48K_3", (uint32_t *)&CALIBRATE.CICFIR_GAINER_48K_val, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.CICFIR_GAINER_96K_3", (uint32_t *)&CALIBRATE.CICFIR_GAINER_96K_val, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.CICFIR_GAINER_192K_3", (uint32_t *)&CALIBRATE.CICFIR_GAINER_192K_val, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.CICFIR_GAINER_384K_3", (uint32_t *)&CALIBRATE.CICFIR_GAINER_384K_val, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.DAC_GAINER_3", (uint32_t *)&CALIBRATE.DAC_GAINER_val, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.TXCICFIR_GAINER_3", (uint32_t *)&CALIBRATE.TXCICFIR_GAINER_val, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_2200m", (uint32_t *)&CALIBRATE.rf_out_power_2200m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_160m", (uint32_t *)&CALIBRATE.rf_out_power_160m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_80m", (uint32_t *)&CALIBRATE.rf_out_power_80m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_40m", (uint32_t *)&CALIBRATE.rf_out_power_40m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_30m", (uint32_t *)&CALIBRATE.rf_out_power_30m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_20m", (uint32_t *)&CALIBRATE.rf_out_power_20m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_17m", (uint32_t *)&CALIBRATE.rf_out_power_17m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_15m", (uint32_t *)&CALIBRATE.rf_out_power_15m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_12m", (uint32_t *)&CALIBRATE.rf_out_power_12m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_cb", (uint32_t *)&CALIBRATE.rf_out_power_cb, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_10m", (uint32_t *)&CALIBRATE.rf_out_power_10m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_6m", (uint32_t *)&CALIBRATE.rf_out_power_6m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.rf_out_power_2m", (uint32_t *)&CALIBRATE.rf_out_power_2m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.smeter_calibration_hf", (uint32_t *)&CALIBRATE.smeter_calibration_hf, SYSMENU_INT16);
			SD_WRITE_SETT_LINE("CALIBRATE.smeter_calibration_vhf", (uint32_t *)&CALIBRATE.smeter_calibration_vhf, SYSMENU_INT16);
			SD_WRITE_SETT_LINE("CALIBRATE.adc_offset", (uint32_t *)&CALIBRATE.adc_offset, SYSMENU_INT16);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_LPF_END", (uint32_t *)&CALIBRATE.RFU_LPF_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_0_START", (uint32_t *)&CALIBRATE.RFU_BPF_0_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_0_END", (uint32_t *)&CALIBRATE.RFU_BPF_0_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_1_START", (uint32_t *)&CALIBRATE.RFU_BPF_1_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_1_END", (uint32_t *)&CALIBRATE.RFU_BPF_1_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_2_START", (uint32_t *)&CALIBRATE.RFU_BPF_2_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_2_END", (uint32_t *)&CALIBRATE.RFU_BPF_2_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_3_START", (uint32_t *)&CALIBRATE.RFU_BPF_3_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_3_END", (uint32_t *)&CALIBRATE.RFU_BPF_3_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_4_START", (uint32_t *)&CALIBRATE.RFU_BPF_4_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_4_END", (uint32_t *)&CALIBRATE.RFU_BPF_4_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_5_START", (uint32_t *)&CALIBRATE.RFU_BPF_5_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_5_END", (uint32_t *)&CALIBRATE.RFU_BPF_5_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_6_START", (uint32_t *)&CALIBRATE.RFU_BPF_6_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_6_END", (uint32_t *)&CALIBRATE.RFU_BPF_6_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_7_START", (uint32_t *)&CALIBRATE.RFU_BPF_7_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_7_END", (uint32_t *)&CALIBRATE.RFU_BPF_7_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_8_START", (uint32_t *)&CALIBRATE.RFU_BPF_8_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_BPF_8_END", (uint32_t *)&CALIBRATE.RFU_BPF_8_END, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.RFU_HPF_START", (uint32_t *)&CALIBRATE.RFU_HPF_START, SYSMENU_UINT32);
			SD_WRITE_SETT_LINE("CALIBRATE.SWR_FWD_Calibration_HF", (uint32_t *)&CALIBRATE.SWR_FWD_Calibration_HF, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("CALIBRATE.SWR_REF_Calibration_HF", (uint32_t *)&CALIBRATE.SWR_REF_Calibration_HF, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("CALIBRATE.SWR_FWD_Calibration_6M", (uint32_t *)&CALIBRATE.SWR_FWD_Calibration_6M, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("CALIBRATE.SWR_REF_Calibration_6M", (uint32_t *)&CALIBRATE.SWR_REF_Calibration_6M, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("CALIBRATE.SWR_FWD_Calibration_VHF", (uint32_t *)&CALIBRATE.SWR_FWD_Calibration_VHF, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("CALIBRATE.SWR_REF_Calibration_VHF", (uint32_t *)&CALIBRATE.SWR_REF_Calibration_VHF, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("CALIBRATE.MAX_RF_POWER", (uint32_t *)&CALIBRATE.MAX_RF_POWER, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.VCXO_correction", (uint32_t *)&CALIBRATE.VCXO_correction, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("CALIBRATE.FAN_MEDIUM_START", (uint32_t *)&CALIBRATE.FAN_MEDIUM_START, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.FAN_MEDIUM_STOP", (uint32_t *)&CALIBRATE.FAN_MEDIUM_STOP, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.FAN_FULL_START", (uint32_t *)&CALIBRATE.FAN_FULL_START, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.TRX_MAX_RF_TEMP", (uint32_t *)&CALIBRATE.TRX_MAX_RF_TEMP, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.TRX_MAX_SWR", (uint32_t *)&CALIBRATE.TRX_MAX_SWR, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.FM_DEVIATION_SCALE", (uint32_t *)&CALIBRATE.FM_DEVIATION_SCALE, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.SSB_POWER_ADDITION", (uint32_t *)&CALIBRATE.SSB_POWER_ADDITION, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.AM_MODULATION_INDEX", (uint32_t *)&CALIBRATE.AM_MODULATION_INDEX, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.TUNE_MAX_POWER", (uint32_t *)&CALIBRATE.TUNE_MAX_POWER, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.RTC_Coarse_Calibration", (uint32_t *)&CALIBRATE.RTC_Coarse_Calibration, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.RTC_Calibration", (uint32_t *)&CALIBRATE.RTC_Calibration, SYSMENU_INT16);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_2200m", (uint32_t *)&CALIBRATE.EXT_2200m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_160m", (uint32_t *)&CALIBRATE.EXT_160m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_80m", (uint32_t *)&CALIBRATE.EXT_80m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_60m", (uint32_t *)&CALIBRATE.EXT_60m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_40m", (uint32_t *)&CALIBRATE.EXT_40m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_30m", (uint32_t *)&CALIBRATE.EXT_30m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_20m", (uint32_t *)&CALIBRATE.EXT_20m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_17m", (uint32_t *)&CALIBRATE.EXT_17m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_15m", (uint32_t *)&CALIBRATE.EXT_15m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_12m", (uint32_t *)&CALIBRATE.EXT_12m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_CB", (uint32_t *)&CALIBRATE.EXT_CB, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_10m", (uint32_t *)&CALIBRATE.EXT_10m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_6m", (uint32_t *)&CALIBRATE.EXT_6m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_FM", (uint32_t *)&CALIBRATE.EXT_FM, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_2m", (uint32_t *)&CALIBRATE.EXT_2m, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_70cm", (uint32_t *)&CALIBRATE.EXT_70cm, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_TRANSV_70cm", (uint32_t *)&CALIBRATE.EXT_TRANSV_70cm, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_TRANSV_23cm", (uint32_t *)&CALIBRATE.EXT_TRANSV_23cm, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_TRANSV_13cm", (uint32_t *)&CALIBRATE.EXT_TRANSV_13cm, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_TRANSV_6cm", (uint32_t *)&CALIBRATE.EXT_TRANSV_6cm, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.EXT_TRANSV_3cm", (uint32_t *)&CALIBRATE.EXT_TRANSV_3cm, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_NOTHAM", (uint32_t *)&CALIBRATE.NOTX_NOTHAM, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_2200m", (uint32_t *)&CALIBRATE.NOTX_2200m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_160m", (uint32_t *)&CALIBRATE.NOTX_160m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_80m", (uint32_t *)&CALIBRATE.NOTX_80m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_60m", (uint32_t *)&CALIBRATE.NOTX_60m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_40m", (uint32_t *)&CALIBRATE.NOTX_40m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_30m", (uint32_t *)&CALIBRATE.NOTX_30m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_20m", (uint32_t *)&CALIBRATE.NOTX_20m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_17m", (uint32_t *)&CALIBRATE.NOTX_17m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_15m", (uint32_t *)&CALIBRATE.NOTX_15m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_12m", (uint32_t *)&CALIBRATE.NOTX_12m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_CB", (uint32_t *)&CALIBRATE.NOTX_CB, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_10m", (uint32_t *)&CALIBRATE.NOTX_10m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_6m", (uint32_t *)&CALIBRATE.NOTX_6m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_FM", (uint32_t *)&CALIBRATE.NOTX_FM, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_2m", (uint32_t *)&CALIBRATE.NOTX_2m, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.NOTX_70cm", (uint32_t *)&CALIBRATE.NOTX_70cm, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.ENABLE_60m_band", (uint32_t *)&CALIBRATE.ENABLE_60m_band, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.ENABLE_marine_band", (uint32_t *)&CALIBRATE.ENABLE_marine_band, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.OTA_update", (uint32_t *)&CALIBRATE.OTA_update, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.TX_StartDelay", (uint32_t *)&CALIBRATE.TX_StartDelay, SYSMENU_UINT16);
			SD_WRITE_SETT_LINE("CALIBRATE.LCD_Rotate", (uint32_t *)&CALIBRATE.LCD_Rotate, SYSMENU_BOOLEAN);
			SD_WRITE_SETT_LINE("CALIBRATE.INA226_EN", (uint32_t *)&CALIBRATE.INA226_EN, SYSMENU_BOOLEAN); // Tisho
			SD_WRITE_SETT_LINE("CALIBRATE.INA226_CurCalc", (uint32_t *)&CALIBRATE.INA226_CurCalc, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("CALIBRATE.PWR_VLT_Calibration", (uint32_t *)&CALIBRATE.PWR_VLT_Calibration, SYSMENU_FLOAT32);
			SD_WRITE_SETT_LINE("CALIBRATE.ATU_AVERAGING", (uint32_t *)&CALIBRATE.ATU_AVERAGING, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.LNA_compensation", (uint32_t *)&CALIBRATE.LNA_compensation, SYSMENU_INT8);
			SD_WRITE_SETT_LINE("CALIBRATE.CAT_Type", (uint32_t *)&CALIBRATE.CAT_Type, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.TwoSignalTune_Balance", (uint32_t *)&CALIBRATE.TwoSignalTune_Balance, SYSMENU_UINT8);
			SD_WRITE_SETT_LINE("CALIBRATE.LinearPowerControl", (uint32_t *)&CALIBRATE.LinearPowerControl, SYSMENU_BOOLEAN);
			// Bands settings
			char buff[64] = {0};
			for (uint8_t i = 0; i < BANDS_COUNT; i++)
			{
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].Freq", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].Freq, SYSMENU_UINT64);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].Mode", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].Mode, SYSMENU_UINT8);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].LNA", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].LNA, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ATT", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].ATT, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ATT_DB", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].ATT_DB, SYSMENU_FLOAT32);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ANT_selected", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].ANT_selected, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ANT_mode", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].ANT_mode, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ADC_Driver", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].ADC_Driver, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].SQL", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].SQL, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].FM_SQL_threshold_dbm", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].FM_SQL_threshold_dbm, SYSMENU_INT8);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ADC_PGA", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].ADC_PGA, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].DNR_Type", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].DNR_Type, SYSMENU_UINT8);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].AGC", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].AGC, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].SAMPLERATE", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&TRX.BANDS_SAVED_SETTINGS[i].SAMPLERATE, SYSMENU_UINT8);
			}

			// Memory channels settings
			for (uint8_t i = 0; i < MEMORY_CHANNELS_COUNT; i++)
			{
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].Freq", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].Freq, SYSMENU_UINT64);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].Mode", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].Mode, SYSMENU_UINT8);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].LNA", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].LNA, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ATT", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].ATT, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ATT_DB", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].ATT_DB, SYSMENU_FLOAT32);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ANT_selected", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].ANT_selected, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ANT_mode", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].ANT_mode, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ADC_Driver", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].ADC_Driver, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].SQL", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].SQL, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].FM_SQL_threshold_dbm", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].FM_SQL_threshold_dbm, SYSMENU_INT8);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ADC_PGA", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].ADC_PGA, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].DNR_Type", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].DNR_Type, SYSMENU_UINT8);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].AGC", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].AGC, SYSMENU_BOOLEAN);
				sprintf(buff, "TRX.MEMORY_CHANNELS[%d].SAMPLERATE", i);
				SD_WRITE_SETT_LINE(buff, (uint32_t *)&CALIBRATE.MEMORY_CHANNELS[i].SAMPLERATE, SYSMENU_UINT8);
			}
		}

		if (!res)
			LCD_showInfo("SD error", true);
		else
			LCD_showInfo("Settings export complete", true);
	}
	else
	{
		LCD_showInfo("SD error", true);
		SD_Present = false;
	}
	f_close(&File);
}

static void SDCOMM_PARSE_SETT_LINE(char *line)
{
	static IRAM2 char name[83] = {0};
	static IRAM2 char value[83] = {0};
	char *istr = strstr((char *)line, " = ");
	uint16_t name_len = (uint16_t)((uint32_t)istr - (uint32_t)line);
	dma_memset(name, 0x00, sizeof(name));
	dma_memset(value, 0x00, sizeof(value));
	if (name_len > 82)
	{
		LCD_showInfo("Line name length error", true);
		// println("E ", line);
		return;
	}
	if (strlen((char *)line + name_len + 3) > 82)
	{
		LCD_showInfo("Line value length error", true);
		return;
	}
	strncpy(name, (char *)line, name_len);
	strcpy(value, (char *)line + name_len + 3);

	uint32_t uint64val = atoll(value);
	uint32_t uintval = atol(value);
	int32_t intval = atol(value);
	float32_t floatval = atof(value);
	bool bval = false;
	if (uintval > 0)
		bval = true;

	// println("IMP: ",name, " = ", value);
	// print_flush();

	// TRX
	if (strcmp(name, "TRX.VFO_A.Freq") == 0)
		TRX.VFO_A.Freq = uint64val;
	if (strcmp(name, "TRX.VFO_A.Mode") == 0)
		TRX.VFO_A.Mode = uintval;
	if (strcmp(name, "TRX.VFO_A.LPF_RX_Filter_Width") == 0)
		TRX.VFO_A.LPF_RX_Filter_Width = uintval;
	if (strcmp(name, "TRX.VFO_A.LPF_TX_Filter_Width") == 0)
		TRX.VFO_A.LPF_TX_Filter_Width = uintval;
	if (strcmp(name, "TRX.VFO_A.HPF_RX_Filter_Width") == 0)
		TRX.VFO_A.HPF_RX_Filter_Width = uintval;
	if (strcmp(name, "TRX.VFO_A.HPF_TX_Filter_Width") == 0)
		TRX.VFO_A.HPF_TX_Filter_Width = uintval;
	if (strcmp(name, "TRX.VFO_A.ManualNotchFilter") == 0)
		TRX.VFO_A.ManualNotchFilter = bval;
	if (strcmp(name, "TRX.VFO_A.AutoNotchFilter") == 0)
		TRX.VFO_A.AutoNotchFilter = bval;
	if (strcmp(name, "TRX.VFO_A.NotchFC") == 0)
		TRX.VFO_A.NotchFC = uintval;
	if (strcmp(name, "TRX.VFO_A.DNR_Type") == 0)
		TRX.VFO_A.DNR_Type = (uint8_t)uintval;
	if (strcmp(name, "TRX.VFO_A.AGC") == 0)
		TRX.VFO_A.AGC = bval;
	if (strcmp(name, "TRX.VFO_A.SQL") == 0)
		TRX.VFO_A.SQL = bval;
	if (strcmp(name, "TRX.VFO_A.FM_SQL_threshold_dbm") == 0)
		TRX.VFO_A.FM_SQL_threshold_dbm = (int8_t)intval;
	if (strcmp(name, "TRX.VFO_B.Freq") == 0)
		TRX.VFO_B.Freq = uint64val;
	if (strcmp(name, "TRX.VFO_B.Mode") == 0)
		TRX.VFO_B.Mode = uintval;
	if (strcmp(name, "TRX.VFO_B.LPF_RX_Filter_Width") == 0)
		TRX.VFO_B.LPF_RX_Filter_Width = uintval;
	if (strcmp(name, "TRX.VFO_B.LPF_TX_Filter_Width") == 0)
		TRX.VFO_B.LPF_TX_Filter_Width = uintval;
	if (strcmp(name, "TRX.VFO_B.HPF_RX_Filter_Width") == 0)
		TRX.VFO_B.HPF_RX_Filter_Width = uintval;
	if (strcmp(name, "TRX.VFO_B.HPF_TX_Filter_Width") == 0)
		TRX.VFO_B.HPF_TX_Filter_Width = uintval;
	if (strcmp(name, "TRX.VFO_B.ManualNotchFilter") == 0)
		TRX.VFO_B.ManualNotchFilter = bval;
	if (strcmp(name, "TRX.VFO_B.AutoNotchFilter") == 0)
		TRX.VFO_B.AutoNotchFilter = bval;
	if (strcmp(name, "TRX.VFO_B.NotchFC") == 0)
		TRX.VFO_B.NotchFC = uintval;
	if (strcmp(name, "TRX.VFO_B.DNR_Type") == 0)
		TRX.VFO_B.DNR_Type = (uint8_t)uintval;
	if (strcmp(name, "TRX.VFO_B.AGC") == 0)
		TRX.VFO_B.AGC = bval;
	if (strcmp(name, "TRX.VFO_B.SQL") == 0)
		TRX.VFO_B.SQL = bval;
	if (strcmp(name, "TRX.VFO_B.FM_SQL_threshold_dbm") == 0)
		TRX.VFO_B.FM_SQL_threshold_dbm = (int8_t)intval;
	if (strcmp(name, "TRX.selected_vfo") == 0)
		TRX.selected_vfo = bval;
	if (strcmp(name, "TRX.LNA") == 0)
		TRX.LNA = bval;
	if (strcmp(name, "TRX.ATT") == 0)
		TRX.ATT = bval;
	if (strcmp(name, "TRX.ATT_DB") == 0)
		TRX.ATT_DB = floatval;
	if (strcmp(name, "TRX.ATT_STEP") == 0)
		TRX.ATT_STEP = (uint8_t)uintval;
	if (strcmp(name, "TRX.Fast") == 0)
		TRX.Fast = bval;
	if (strcmp(name, "TRX.ANT_selected") == 0)
		TRX.ANT_selected = bval;
	if (strcmp(name, "TRX.ANT_mode") == 0)
		TRX.ANT_mode = bval;
	if (strcmp(name, "TRX.RF_Filters") == 0)
		TRX.RF_Filters = bval;
	if (strcmp(name, "TRX.ChannelMode") == 0)
		TRX.ChannelMode = bval;
	if (strcmp(name, "TRX.RF_Power") == 0)
		TRX.RF_Power = (uint8_t)uintval;
	if (strcmp(name, "TRX.RIT_Enabled") == 0)
		TRX.RIT_Enabled = bval;
	if (strcmp(name, "TRX.XIT_Enabled") == 0)
		TRX.XIT_Enabled = bval;
	if (strcmp(name, "TRX.FineRITTune") == 0)
		TRX.FineRITTune = bval;
	if (strcmp(name, "TRX.RIT_INTERVAL") == 0)
		TRX.RIT_INTERVAL = (uint16_t)uintval;
	if (strcmp(name, "TRX.XIT_INTERVAL") == 0)
		TRX.XIT_INTERVAL = (uint16_t)uintval;
	if (strcmp(name, "TRX.TWO_SIGNAL_TUNE") == 0)
		TRX.TWO_SIGNAL_TUNE = bval;
	if (strcmp(name, "TRX.SAMPLERATE_MAIN") == 0)
		TRX.SAMPLERATE_MAIN = (uint8_t)uintval;
	if (strcmp(name, "TRX.SAMPLERATE_FM") == 0)
		TRX.SAMPLERATE_FM = (uint8_t)uintval;
	if (strcmp(name, "TRX.FRQ_STEP") == 0)
		TRX.FRQ_STEP = uintval;
	if (strcmp(name, "TRX.FRQ_FAST_STEP") == 0)
		TRX.FRQ_FAST_STEP = uintval;
	if (strcmp(name, "TRX.FRQ_ENC_STEP") == 0)
		TRX.FRQ_ENC_STEP = uintval;
	if (strcmp(name, "TRX.FRQ_ENC_FAST_STEP") == 0)
		TRX.FRQ_ENC_FAST_STEP = uintval;
	if (strcmp(name, "TRX.FRQ_CW_STEP_DIVIDER") == 0)
		TRX.FRQ_CW_STEP_DIVIDER = (uint8_t)uintval;
	if (strcmp(name, "TRX.Debug_Type") == 0)
		TRX.Debug_Type = (uint8_t)uintval;
	if (strcmp(name, "TRX.BandMapEnabled") == 0)
		TRX.BandMapEnabled = bval;
	if (strcmp(name, "TRX.InputType_MAIN") == 0)
		TRX.InputType_MAIN = (uint8_t)uintval;
	if (strcmp(name, "TRX.InputType_DIGI") == 0)
		TRX.InputType_DIGI = (uint8_t)uintval;
	if (strcmp(name, "TRX.AutoGain") == 0)
		TRX.AutoGain = bval;
	if (strcmp(name, "TRX.SPLIT_Enabled") == 0)
		TRX.SPLIT_Enabled = bval;
	if (strcmp(name, "TRX.Dual_RX") == 0)
		TRX.Dual_RX = bval;
	if (strcmp(name, "TRX.Encoder_Accelerate") == 0)
		TRX.Encoder_Accelerate = bval;
	if (strcmp(name, "TRX.Dual_RX_Type") == 0)
		TRX.Dual_RX_Type = (DUAL_RX_TYPE)uintval;
	if (strcmp(name, "TRX.CALLSIGN") == 0)
	{
		dma_memset(TRX.CALLSIGN, 0x00, sizeof(TRX.CALLSIGN));
		uint32_t lens = strlen(value);
		if (lens > sizeof(TRX.CALLSIGN))
			lens = sizeof(TRX.CALLSIGN);
		strncpy(TRX.CALLSIGN, value, lens);
	}
	if (strcmp(name, "TRX.LOCATOR") == 0)
	{
		dma_memset(TRX.LOCATOR, 0x00, sizeof(TRX.LOCATOR));
		uint32_t lens = strlen(value);
		if (lens > sizeof(TRX.LOCATOR))
			lens = sizeof(TRX.LOCATOR);
		strncpy(TRX.LOCATOR, value, lens);
	}
	if (strcmp(name, "TRX.Custom_Transverter_Enabled") == 0)
		TRX.Custom_Transverter_Enabled = bval;
	if (strcmp(name, "TRX.Transverter_Offset_Mhz") == 0)
		TRX.Transverter_Offset_Mhz = (uint16_t)uintval;
	if (strcmp(name, "TRX.ATU_Enabled") == 0)
		TRX.ATU_Enabled = bval;
	if (strcmp(name, "TRX.TUNER_Enabled") == 0)
		TRX.TUNER_Enabled = bval;
	if (strcmp(name, "TRX.Transverter_70cm") == 0)
		TRX.Transverter_70cm = bval;
	if (strcmp(name, "TRX.Transverter_23cm") == 0)
		TRX.Transverter_23cm = bval;
	if (strcmp(name, "TRX.Transverter_13cm") == 0)
		TRX.Transverter_13cm = bval;
	if (strcmp(name, "TRX.Transverter_6cm") == 0)
		TRX.Transverter_6cm = bval;
	if (strcmp(name, "TRX.Transverter_3cm") == 0)
		TRX.Transverter_3cm = bval;
	if (strcmp(name, "TRX.Auto_Input_Switch") == 0)
		TRX.Auto_Input_Switch = bval;
	// AUDIO
	if (strcmp(name, "TRX.IF_Gain") == 0)
		TRX.IF_Gain = (uint8_t)uintval;
	if (strcmp(name, "TRX.AGC_GAIN_TARGET2") == 0)
		TRX.AGC_GAIN_TARGET = (int8_t)intval;
	if (strcmp(name, "TRX.MIC_GAIN") == 0)
		TRX.MIC_GAIN = (uint8_t)uintval;
	if (strcmp(name, "TRX.MIC_Boost") == 0)
		TRX.MIC_Boost = bval;
	if (strcmp(name, "TRX.MIC_NOISE_GATE") == 0)
		TRX.MIC_NOISE_GATE = (int8_t)intval;
	if (strcmp(name, "TRX.RX_EQ_LOW") == 0)
		TRX.RX_EQ_LOW = (int8_t)intval;
	if (strcmp(name, "TRX.RX_EQ_MID") == 0)
		TRX.RX_EQ_MID = (int8_t)intval;
	if (strcmp(name, "TRX.RX_EQ_HIG") == 0)
		TRX.RX_EQ_HIG = (int8_t)intval;
	if (strcmp(name, "TRX.MIC_EQ_LOW_SSB") == 0)
		TRX.MIC_EQ_LOW_SSB = (int8_t)intval;
	if (strcmp(name, "TRX.MIC_EQ_MID_SSB") == 0)
		TRX.MIC_EQ_MID_SSB = (int8_t)intval;
	if (strcmp(name, "TRX.MIC_EQ_HIG_SSB") == 0)
		TRX.MIC_EQ_HIG_SSB = (int8_t)intval;
	if (strcmp(name, "TRX.MIC_EQ_LOW_AMFM") == 0)
		TRX.MIC_EQ_LOW_AMFM = (int8_t)intval;
	if (strcmp(name, "TRX.MIC_EQ_MID_AMFM") == 0)
		TRX.MIC_EQ_MID_AMFM = (int8_t)intval;
	if (strcmp(name, "TRX.MIC_EQ_HIG_AMFM") == 0)
		TRX.MIC_EQ_HIG_AMFM = (int8_t)intval;
	if (strcmp(name, "TRX.MIC_REVERBER") == 0)
		TRX.MIC_REVERBER = (uint8_t)uintval;
	if (strcmp(name, "TRX.DNR1_SNR_THRESHOLD") == 0)
		TRX.DNR1_SNR_THRESHOLD = (uint8_t)uintval;
	if (strcmp(name, "TRX.DNR2_SNR_THRESHOLD") == 0)
		TRX.DNR2_SNR_THRESHOLD = (uint8_t)uintval;
	if (strcmp(name, "TRX.DNR_AVERAGE") == 0)
		TRX.DNR_AVERAGE = (uint8_t)uintval;
	if (strcmp(name, "TRX.DNR_MINIMAL") == 0)
		TRX.DNR_MINIMAL = (uint8_t)uintval;
	if (strcmp(name, "TRX.NOISE_BLANKER") == 0)
		TRX.NOISE_BLANKER = uintval;
	if (strcmp(name, "TRX.RX_AGC_SSB_speed") == 0)
		TRX.RX_AGC_SSB_speed = (uint8_t)uintval;
	if (strcmp(name, "TRX.RX_AGC_CW_speed") == 0)
		TRX.RX_AGC_CW_speed = (uint8_t)uintval;
	if (strcmp(name, "TRX.RX_AGC_Max_gain") == 0)
		TRX.RX_AGC_Max_gain = (uint8_t)uintval;
	if (strcmp(name, "TRX.RX_AGC_Hold") == 0)
		TRX.RX_AGC_Hold = (uint16_t)uintval;
	if (strcmp(name, "TRX.TX_Compressor_speed_SSB") == 0)
		TRX.TX_Compressor_speed_SSB = (uint8_t)uintval;
	if (strcmp(name, "TRX.TX_Compressor_maxgain_SSB") == 0)
		TRX.TX_Compressor_maxgain_SSB = (uint8_t)uintval;
	if (strcmp(name, "TRX.TX_Compressor_speed_AMFM") == 0)
		TRX.TX_Compressor_speed_AMFM = (uint8_t)uintval;
	if (strcmp(name, "TRX.TX_Compressor_maxgain_AMFM") == 0)
		TRX.TX_Compressor_maxgain_AMFM = (uint8_t)uintval;
	if (strcmp(name, "TRX.CW_LPF_Filter") == 0)
		TRX.CW_LPF_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.DIGI_LPF_Filter") == 0)
		TRX.DIGI_LPF_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.SSB_LPF_RX_Filter") == 0)
		TRX.SSB_LPF_RX_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.SSB_LPF_TX_Filter") == 0)
		TRX.SSB_LPF_TX_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.SSB_HPF_RX_Filter") == 0)
		TRX.SSB_HPF_RX_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.SSB_HPF_TX_Filter") == 0)
		TRX.SSB_HPF_TX_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.AM_LPF_RX_Filter") == 0)
		TRX.AM_LPF_RX_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.AM_LPF_TX_Filter") == 0)
		TRX.AM_LPF_TX_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.FM_LPF_RX_Filter") == 0)
		TRX.FM_LPF_RX_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.FM_LPF_TX_Filter") == 0)
		TRX.FM_LPF_TX_Filter = (uint16_t)uintval;
	if (strcmp(name, "TRX.Beeper") == 0)
		TRX.Beeper = uintval;
	if (strcmp(name, "TRX.CTCSS_Freq") == 0)
		TRX.CTCSS_Freq = floatval;
	if (strcmp(name, "TRX.SELFHEAR_Volume") == 0)
		TRX.SELFHEAR_Volume = (uint8_t)uintval;
	if (strcmp(name, "TRX.FM_Stereo") == 0)
		TRX.FM_Stereo = bval;
	if (strcmp(name, "TRX.AGC_Spectral") == 0)
		TRX.AGC_Spectral = bval;
	if (strcmp(name, "TRX.VAD_THRESHOLD") == 0)
		TRX.VAD_THRESHOLD = (uint8_t)uintval;
	if (strcmp(name, "TRX.VOX") == 0)
		TRX.VOX = bval;
	if (strcmp(name, "TRX.VOX_TIMEOUT") == 0)
		TRX.VOX_TIMEOUT = (uint16_t)uintval;
	if (strcmp(name, "TRX.VOX_THRESHOLD") == 0)
		TRX.VOX_THRESHOLD = (int8_t)intval;
	// CW
	if (strcmp(name, "TRX.CW_Pitch") == 0)
		TRX.CW_Pitch = (uint16_t)uintval;
	if (strcmp(name, "TRX.CW_Key_timeout") == 0)
		TRX.CW_Key_timeout = (uint16_t)uintval;
	if (strcmp(name, "TRX.CW_SelfHear") == 0)
		TRX.CW_SelfHear = (uint16_t)uintval;
	if (strcmp(name, "TRX.CW_KEYER") == 0)
		TRX.CW_KEYER = uintval;
	if (strcmp(name, "TRX.CW_KEYER_WPM") == 0)
		TRX.CW_KEYER_WPM = (uint16_t)uintval;
	if (strcmp(name, "TRX.CW_GaussFilter") == 0)
		TRX.CW_GaussFilter = uintval;
	if (strcmp(name, "TRX.CW_DotToDashRate") == 0)
		TRX.CW_DotToDashRate = floatval;
	if (strcmp(name, "TRX.CW_Iambic") == 0)
		TRX.CW_Iambic = bval;
	if (strcmp(name, "TRX.CW_Invert") == 0)
		TRX.CW_Invert = bval;
	if (strcmp(name, "TRX.CW_PTT_Type") == 0)
		TRX.CW_PTT_Type = (uint8_t)uintval;
	// SCREEN
	if (strcmp(name, "TRX.ColorThemeId") == 0)
		TRX.ColorThemeId = (uint8_t)uintval;
	if (strcmp(name, "TRX.LayoutThemeId") == 0)
		TRX.LayoutThemeId = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_Enabled") == 0)
		TRX.FFT_Enabled = uintval;
	if (strcmp(name, "TRX.FFT_Zoom") == 0)
		TRX.FFT_Zoom = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_ZoomCW") == 0)
		TRX.FFT_ZoomCW = (uint8_t)uintval;
	if (strcmp(name, "TRX.LCD_Brightness") == 0)
		TRX.LCD_Brightness = (uint8_t)uintval;
	if (strcmp(name, "TRX.LCD_SleepTimeout") == 0)
		TRX.LCD_SleepTimeout = (uint16_t)uintval;
	if (strcmp(name, "TRX.FFT_Speed") == 0)
		TRX.FFT_Speed = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_Sensitivity") == 0)
		TRX.FFT_Sensitivity = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_Averaging") == 0)
		TRX.FFT_Averaging = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_Window") == 0)
		TRX.FFT_Window = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_Height") == 0)
		TRX.FFT_Height = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_Style") == 0)
		TRX.FFT_Style = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_BW_Style") == 0)
		TRX.FFT_BW_Style = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_Color") == 0)
		TRX.FFT_Color = (uint8_t)uintval;
	if (strcmp(name, "TRX.WTF_Color") == 0)
		TRX.WTF_Color = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_Compressor") == 0)
		TRX.FFT_Compressor = uintval;
	if (strcmp(name, "TRX.WTF_Moving") == 0)
		TRX.WTF_Moving = uintval;
	if (strcmp(name, "TRX.FFT_FreqGrid") == 0)
		TRX.FFT_FreqGrid = (int8_t)intval;
	if (strcmp(name, "TRX.FFT_dBmGrid") == 0)
		TRX.FFT_dBmGrid = bval;
	if (strcmp(name, "TRX.FFT_Background") == 0)
		TRX.FFT_Background = uintval;
	if (strcmp(name, "TRX.FFT_Lens") == 0)
		TRX.FFT_Lens = uintval;
	if (strcmp(name, "TRX.FFT_HoldPeaks") == 0)
		TRX.FFT_HoldPeaks = uintval;
	if (strcmp(name, "TRX.FFT_3D") == 0)
		TRX.FFT_3D = (uint8_t)uintval;
	if (strcmp(name, "TRX.FFT_Automatic") == 0)
		TRX.FFT_Automatic = uintval;
	if (strcmp(name, "TRX.FFT_ManualBottom") == 0)
		TRX.FFT_ManualBottom = (int16_t)intval;
	if (strcmp(name, "TRX.FFT_ManualTop") == 0)
		TRX.FFT_ManualTop = (int16_t)intval;
	if (strcmp(name, "TRX.FFT_DXCluster") == 0)
		TRX.FFT_DXCluster = bval;
	if (strcmp(name, "TRX.FFT_DXCluster_Azimuth") == 0)
		TRX.FFT_DXCluster_Azimuth = bval;
	if (strcmp(name, "TRX.FFT_DXCluster_Timeout") == 0)
		TRX.FFT_DXCluster_Timeout = (uint8_t)uintval;
	if (strcmp(name, "TRX.Show_Sec_VFO") == 0)
		TRX.Show_Sec_VFO = bval;
	if (strcmp(name, "TRX.FFT_Scale_Type") == 0)
		TRX.FFT_Scale_Type = (uint8_t)uintval;
	// DECODERS
	if (strcmp(name, "TRX.CW_Decoder") == 0)
		TRX.CW_Decoder = bval;
	if (strcmp(name, "TRX.CW_Decoder_Threshold") == 0)
		TRX.CW_Decoder_Threshold = (uint8_t)uintval;
	if (strcmp(name, "TRX.RDS_Decoder") == 0)
		TRX.RDS_Decoder = bval;
	if (strcmp(name, "TRX.RTTY_Speed") == 0)
		TRX.RTTY_Speed = uintval;
	if (strcmp(name, "TRX.RTTY_Shift") == 0)
		TRX.RTTY_Shift = uintval;
	if (strcmp(name, "TRX.RTTY_Freq") == 0)
		TRX.RTTY_Freq = uintval;
	if (strcmp(name, "TRX.RTTY_StopBits") == 0)
		TRX.RTTY_StopBits = (uint8_t)uintval;
	if (strcmp(name, "TRX.RTTY_InvertBits") == 0)
		TRX.RTTY_InvertBits = bval;
	// ADC
	if (strcmp(name, "TRX.ADC_Driver") == 0)
		TRX.ADC_Driver = uintval;
	if (strcmp(name, "TRX.ADC_PGA") == 0)
		TRX.ADC_PGA = uintval;
	if (strcmp(name, "TRX.ADC_RAND") == 0)
		TRX.ADC_RAND = uintval;
	if (strcmp(name, "TRX.ADC_SHDN") == 0)
		TRX.ADC_SHDN = uintval;
	if (strcmp(name, "TRX.ADC_DITH") == 0)
		TRX.ADC_DITH = uintval;
	// WIFI
	if (strcmp(name, "TRX.WIFI_Enabled") == 0)
		TRX.WIFI_Enabled = uintval;
	if (strcmp(name, "TRX.WIFI_TIMEZONE") == 0)
		TRX.WIFI_TIMEZONE = (int8_t)intval;
	if (strcmp(name, "TRX.WIFI_CAT_SERVER") == 0)
		TRX.WIFI_CAT_SERVER = uintval;
	if (strcmp(name, "TRX.WIFI_AP1") == 0)
	{
		dma_memset(TRX.WIFI_AP1, 0x00, sizeof(TRX.WIFI_AP1));
		uint32_t lens = strlen(value);
		if (lens > sizeof(TRX.WIFI_AP1))
			lens = sizeof(TRX.WIFI_AP1);
		strncpy(TRX.WIFI_AP1, value, lens);
	}
	if (strcmp(name, "TRX.WIFI_AP2") == 0)
	{
		dma_memset(TRX.WIFI_AP2, 0x00, sizeof(TRX.WIFI_AP2));
		uint32_t lens = strlen(value);
		if (lens > sizeof(TRX.WIFI_AP2))
			lens = sizeof(TRX.WIFI_AP2);
		strncpy(TRX.WIFI_AP2, value, lens);
	}
	if (strcmp(name, "TRX.WIFI_AP3") == 0)
	{
		dma_memset(TRX.WIFI_AP3, 0x00, sizeof(TRX.WIFI_AP3));
		uint32_t lens = strlen(value);
		if (lens > sizeof(TRX.WIFI_AP3))
			lens = sizeof(TRX.WIFI_AP3);
		strncpy(TRX.WIFI_AP3, value, lens);
	}
	if (strcmp(name, "TRX.WIFI_PASSWORD1") == 0)
	{
		dma_memset(TRX.WIFI_PASSWORD1, 0x00, sizeof(TRX.WIFI_PASSWORD1));
		uint32_t lens = strlen(value);
		if (lens > sizeof(TRX.WIFI_PASSWORD1))
			lens = sizeof(TRX.WIFI_PASSWORD1);
		strncpy(TRX.WIFI_PASSWORD1, value, lens);
	}
	if (strcmp(name, "TRX.WIFI_PASSWORD2") == 0)
	{
		dma_memset(TRX.WIFI_PASSWORD2, 0x00, sizeof(TRX.WIFI_PASSWORD2));
		uint32_t lens = strlen(value);
		if (lens > sizeof(TRX.WIFI_PASSWORD2))
			lens = sizeof(TRX.WIFI_PASSWORD2);
		strncpy(TRX.WIFI_PASSWORD2, value, lens);
	}
	if (strcmp(name, "TRX.WIFI_PASSWORD3") == 0)
	{
		dma_memset(TRX.WIFI_PASSWORD3, 0x00, sizeof(TRX.WIFI_PASSWORD3));
		uint32_t lens = strlen(value);
		if (lens > sizeof(TRX.WIFI_PASSWORD3))
			lens = sizeof(TRX.WIFI_PASSWORD3);
		strncpy(TRX.WIFI_PASSWORD3, value, lens);
	}
	// SERVICES
	if (strcmp(name, "TRX.SWR_CUSTOM_Begin") == 0)
		TRX.SWR_CUSTOM_Begin = uintval;
	if (strcmp(name, "TRX.SWR_CUSTOM_End") == 0)
		TRX.SWR_CUSTOM_End = uintval;
	if (strcmp(name, "TRX.SPEC_Begin") == 0)
		TRX.SPEC_Begin = uintval;
	if (strcmp(name, "TRX.SPEC_End") == 0)
		TRX.SPEC_End = uintval;
	if (strcmp(name, "TRX.SPEC_TopDBM") == 0)
		TRX.SPEC_TopDBM = (int16_t)intval;
	if (strcmp(name, "TRX.SPEC_BottomDBM") == 0)
		TRX.SPEC_BottomDBM = (int16_t)intval;
	if (strcmp(name, "TRX.WSPR_FREQ_OFFSET") == 0)
		TRX.WSPR_FREQ_OFFSET = (int16_t)intval;
	if (strcmp(name, "TRX.WSPR_BANDS_160") == 0)
		TRX.WSPR_BANDS_160 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_80") == 0)
		TRX.WSPR_BANDS_80 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_40") == 0)
		TRX.WSPR_BANDS_40 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_30") == 0)
		TRX.WSPR_BANDS_30 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_20") == 0)
		TRX.WSPR_BANDS_20 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_17") == 0)
		TRX.WSPR_BANDS_17 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_15") == 0)
		TRX.WSPR_BANDS_15 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_12") == 0)
		TRX.WSPR_BANDS_12 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_10") == 0)
		TRX.WSPR_BANDS_10 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_6") == 0)
		TRX.WSPR_BANDS_6 = uintval;
	if (strcmp(name, "TRX.WSPR_BANDS_2") == 0)
		TRX.WSPR_BANDS_2 = uintval;
	// CALIBRATION
	if (strcmp(name, "CALIBRATE.ENCODER_INVERT") == 0)
		CALIBRATE.ENCODER_INVERT = bval;
	if (strcmp(name, "CALIBRATE.ENCODER2_INVERT") == 0)
		CALIBRATE.ENCODER2_INVERT = bval;
	if (strcmp(name, "CALIBRATE.ENCODER_DEBOUNCE") == 0)
		CALIBRATE.ENCODER_DEBOUNCE = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.ENCODER2_DEBOUNCE") == 0)
		CALIBRATE.ENCODER2_DEBOUNCE = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.ENCODER_SLOW_RATE") == 0)
		CALIBRATE.ENCODER_SLOW_RATE = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.ENCODER_ON_FALLING") == 0)
		CALIBRATE.ENCODER_ON_FALLING = bval;
	if (strcmp(name, "CALIBRATE.ENCODER_ACCELERATION") == 0)
		CALIBRATE.ENCODER_ACCELERATION = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.RF_unit_type") == 0)
		CALIBRATE.RF_unit_type = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.TangentType") == 0)
		CALIBRATE.TangentType = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.CICFIR_GAINER_48K_3") == 0)
		CALIBRATE.CICFIR_GAINER_48K_val = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.CICFIR_GAINER_96K_3") == 0)
		CALIBRATE.CICFIR_GAINER_96K_val = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.CICFIR_GAINER_192K_3") == 0)
		CALIBRATE.CICFIR_GAINER_192K_val = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.CICFIR_GAINER_384K_3") == 0)
		CALIBRATE.CICFIR_GAINER_384K_val = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.TXCICFIR_GAINER_3") == 0)
		CALIBRATE.TXCICFIR_GAINER_val = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.DAC_GAINER_3") == 0)
		CALIBRATE.DAC_GAINER_val = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_2200m") == 0)
		CALIBRATE.rf_out_power_2200m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_160m") == 0)
		CALIBRATE.rf_out_power_160m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_80m") == 0)
		CALIBRATE.rf_out_power_80m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_40m") == 0)
		CALIBRATE.rf_out_power_40m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_30m") == 0)
		CALIBRATE.rf_out_power_30m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_20m") == 0)
		CALIBRATE.rf_out_power_20m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_17m") == 0)
		CALIBRATE.rf_out_power_17m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_15m") == 0)
		CALIBRATE.rf_out_power_15m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_12m") == 0)
		CALIBRATE.rf_out_power_12m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_cb") == 0)
		CALIBRATE.rf_out_power_cb = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_10m") == 0)
		CALIBRATE.rf_out_power_10m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_6m") == 0)
		CALIBRATE.rf_out_power_6m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.rf_out_power_2m") == 0)
		CALIBRATE.rf_out_power_2m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.smeter_calibration_hf") == 0)
		CALIBRATE.smeter_calibration_hf = (int16_t)intval;
	if (strcmp(name, "CALIBRATE.smeter_calibration_vhf") == 0)
		CALIBRATE.smeter_calibration_vhf = (int16_t)intval;
	if (strcmp(name, "CALIBRATE.adc_offset") == 0)
		CALIBRATE.adc_offset = (int16_t)intval;
	if (strcmp(name, "CALIBRATE.RFU_LPF_END") == 0)
		CALIBRATE.RFU_LPF_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_0_START") == 0)
		CALIBRATE.RFU_BPF_0_START = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_0_END") == 0)
		CALIBRATE.RFU_BPF_0_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_1_START") == 0)
		CALIBRATE.RFU_BPF_1_START = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_1_END") == 0)
		CALIBRATE.RFU_BPF_1_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_2_START") == 0)
		CALIBRATE.RFU_BPF_2_START = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_2_END") == 0)
		CALIBRATE.RFU_BPF_2_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_3_START") == 0)
		CALIBRATE.RFU_BPF_3_START = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_3_END") == 0)
		CALIBRATE.RFU_BPF_3_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_4_START") == 0)
		CALIBRATE.RFU_BPF_4_START = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_4_END") == 0)
		CALIBRATE.RFU_BPF_4_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_5_START") == 0)
		CALIBRATE.RFU_BPF_5_START = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_5_END") == 0)
		CALIBRATE.RFU_BPF_5_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_6_START") == 0)
		CALIBRATE.RFU_BPF_6_START = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_6_END") == 0)
		CALIBRATE.RFU_BPF_6_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_7_START") == 0)
		CALIBRATE.RFU_BPF_7_START = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_7_END") == 0)
		CALIBRATE.RFU_BPF_7_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_8_START") == 0)
		CALIBRATE.RFU_BPF_8_START = uintval;
	if (strcmp(name, "CALIBRATE.RFU_BPF_8_END") == 0)
		CALIBRATE.RFU_BPF_8_END = uintval;
	if (strcmp(name, "CALIBRATE.RFU_HPF_START") == 0)
		CALIBRATE.RFU_HPF_START = uintval;
	if (strcmp(name, "CALIBRATE.SWR_FWD_Calibration_HF") == 0)
		CALIBRATE.SWR_FWD_Calibration_HF = floatval;
	if (strcmp(name, "CALIBRATE.SWR_REF_Calibration_HF") == 0)
		CALIBRATE.SWR_REF_Calibration_HF = floatval;
	if (strcmp(name, "CALIBRATE.SWR_FWD_Calibration_6M") == 0)
		CALIBRATE.SWR_FWD_Calibration_6M = floatval;
	if (strcmp(name, "CALIBRATE.SWR_REF_Calibration_6M") == 0)
		CALIBRATE.SWR_REF_Calibration_6M = floatval;
	if (strcmp(name, "CALIBRATE.SWR_FWD_Calibration_VHF") == 0)
		CALIBRATE.SWR_FWD_Calibration_VHF = floatval;
	if (strcmp(name, "CALIBRATE.SWR_REF_Calibration_VHF") == 0)
		CALIBRATE.SWR_REF_Calibration_VHF = floatval;
	if (strcmp(name, "CALIBRATE.MAX_RF_POWER") == 0)
		CALIBRATE.MAX_RF_POWER = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.VCXO_correction") == 0)
		CALIBRATE.VCXO_correction = (int8_t)intval;
	if (strcmp(name, "CALIBRATE.FAN_MEDIUM_START") == 0)
		CALIBRATE.FAN_MEDIUM_START = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.FAN_MEDIUM_STOP") == 0)
		CALIBRATE.FAN_MEDIUM_STOP = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.FAN_FULL_START") == 0)
		CALIBRATE.FAN_FULL_START = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.TRX_MAX_RF_TEMP") == 0)
		CALIBRATE.TRX_MAX_RF_TEMP = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.TRX_MAX_SWR") == 0)
		CALIBRATE.TRX_MAX_SWR = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.FM_DEVIATION_SCALE") == 0)
		CALIBRATE.FM_DEVIATION_SCALE = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.SSB_POWER_ADDITION") == 0)
		CALIBRATE.SSB_POWER_ADDITION = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.AM_MODULATION_INDEX") == 0)
		CALIBRATE.AM_MODULATION_INDEX = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.TUNE_MAX_POWER") == 0)
		CALIBRATE.TUNE_MAX_POWER = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.RTC_Coarse_Calibration") == 0)
		CALIBRATE.RTC_Coarse_Calibration = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.RTC_Calibration") == 0)
		CALIBRATE.RTC_Calibration = intval;

	if (strcmp(name, "CALIBRATE.EXT_2200m") == 0)
		CALIBRATE.EXT_2200m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_160m") == 0)
		CALIBRATE.EXT_160m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_80m") == 0)
		CALIBRATE.EXT_80m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_60m") == 0)
		CALIBRATE.EXT_60m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_40m") == 0)
		CALIBRATE.EXT_40m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_30m") == 0)
		CALIBRATE.EXT_30m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_20m") == 0)
		CALIBRATE.EXT_20m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_17m") == 0)
		CALIBRATE.EXT_17m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_15m") == 0)
		CALIBRATE.EXT_15m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_12m") == 0)
		CALIBRATE.EXT_12m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_CB") == 0)
		CALIBRATE.EXT_CB = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_10m") == 0)
		CALIBRATE.EXT_10m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_6m") == 0)
		CALIBRATE.EXT_6m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_FM") == 0)
		CALIBRATE.EXT_FM = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_2m") == 0)
		CALIBRATE.EXT_2m = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_70cm") == 0)
		CALIBRATE.EXT_70cm = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_TRANSV_70cm") == 0)
		CALIBRATE.EXT_TRANSV_70cm = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_TRANSV_23cm") == 0)
		CALIBRATE.EXT_TRANSV_23cm = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_TRANSV_13cm") == 0)
		CALIBRATE.EXT_TRANSV_13cm = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_TRANSV_6cm") == 0)
		CALIBRATE.EXT_TRANSV_6cm = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.EXT_TRANSV_3cm") == 0)
		CALIBRATE.EXT_TRANSV_3cm = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.NOTX_NOTHAM") == 0)
		CALIBRATE.NOTX_NOTHAM = bval;
	if (strcmp(name, "CALIBRATE.NOTX_2200m") == 0)
		CALIBRATE.NOTX_2200m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_160m") == 0)
		CALIBRATE.NOTX_160m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_80m") == 0)
		CALIBRATE.NOTX_80m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_60m") == 0)
		CALIBRATE.NOTX_60m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_40m") == 0)
		CALIBRATE.NOTX_40m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_30m") == 0)
		CALIBRATE.NOTX_30m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_20m") == 0)
		CALIBRATE.NOTX_20m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_17m") == 0)
		CALIBRATE.NOTX_17m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_15m") == 0)
		CALIBRATE.NOTX_15m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_12m") == 0)
		CALIBRATE.NOTX_12m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_CB") == 0)
		CALIBRATE.NOTX_CB = bval;
	if (strcmp(name, "CALIBRATE.NOTX_10m") == 0)
		CALIBRATE.NOTX_10m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_6m") == 0)
		CALIBRATE.NOTX_6m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_2m") == 0)
		CALIBRATE.NOTX_2m = bval;
	if (strcmp(name, "CALIBRATE.NOTX_70cm") == 0)
		CALIBRATE.NOTX_70cm = bval;
	if (strcmp(name, "CALIBRATE.ENABLE_60m_band") == 0)
		CALIBRATE.ENABLE_60m_band = bval;
	if (strcmp(name, "CALIBRATE.ENABLE_marine_band") == 0)
		CALIBRATE.ENABLE_marine_band = bval;
	if (strcmp(name, "CALIBRATE.OTA_update") == 0)
		CALIBRATE.OTA_update = bval;
	if (strcmp(name, "CALIBRATE.TX_StartDelay") == 0)
		CALIBRATE.TX_StartDelay = uintval;
	if (strcmp(name, "CALIBRATE.LCD_Rotate") == 0)
		CALIBRATE.LCD_Rotate = bval;
	if (strcmp(name, "CALIBRATE.INA226_EN") == 0) // Tisho
		CALIBRATE.INA226_EN = bval;
	if (strcmp(name, "CALIBRATE.INA226_CurCalc") == 0)
		CALIBRATE.INA226_CurCalc = floatval;
	if (strcmp(name, "CALIBRATE.PWR_VLT_Calibration") == 0)
		CALIBRATE.PWR_VLT_Calibration = floatval;
	if (strcmp(name, "CALIBRATE.ATU_AVERAGING") == 0)
		CALIBRATE.ATU_AVERAGING = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.LNA_compensation") == 0)
		CALIBRATE.LNA_compensation = (int8_t)intval;
	if (strcmp(name, "CALIBRATE.CAT_Type") == 0)
		CALIBRATE.CAT_Type = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.TwoSignalTune_Balance") == 0)
		CALIBRATE.TwoSignalTune_Balance = (uint8_t)uintval;
	if (strcmp(name, "CALIBRATE.LinearPowerControl") == 0)
		CALIBRATE.LinearPowerControl = bval;

	// Bands settings
	char buff[64] = {0};
	for (uint8_t i = 0; i < BANDS_COUNT; i++)
	{
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].Freq", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].Freq = uint64val;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].Mode", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].Mode = (uint8_t)uintval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].LNA", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].LNA = bval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ATT", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].ATT = bval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ATT_DB", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].ATT_DB = floatval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ANT_selected", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].ANT_selected = bval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ANT_mode", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].ANT_mode = bval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ADC_Driver", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].ADC_Driver = bval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].SQL", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].SQL = bval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].FM_SQL_threshold_dbm", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].FM_SQL_threshold_dbm = (int8_t)intval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].ADC_PGA", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].ADC_PGA = bval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].DNR_Type", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].DNR_Type = (uint8_t)uintval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].AGC", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].AGC = bval;
		sprintf(buff, "TRX.BANDS_SAVED_SETTINGS[%d].SAMPLERATE", i);
		if (strcmp(name, buff) == 0)
			TRX.BANDS_SAVED_SETTINGS[i].SAMPLERATE = (uint8_t)uintval;
	}

	// Memory channels settings
	for (uint8_t i = 0; i < MEMORY_CHANNELS_COUNT; i++)
	{
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].Freq", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].Freq = uint64val;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].Mode", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].Mode = (uint8_t)uintval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].LNA", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].LNA = bval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ATT", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].ATT = bval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ATT_DB", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].ATT_DB = floatval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ANT_selected", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].ANT_selected = bval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ANT_mode", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].ANT_mode = bval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ADC_Driver", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].ADC_Driver = bval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].SQL", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].SQL = bval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].FM_SQL_threshold_dbm", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].FM_SQL_threshold_dbm = (int8_t)intval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].ADC_PGA", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].ADC_PGA = bval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].DNR_Type", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].DNR_Type = (uint8_t)uintval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].AGC", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].AGC = bval;
		sprintf(buff, "TRX.MEMORY_CHANNELS[%d].SAMPLERATE", i);
		if (strcmp(name, buff) == 0)
			CALIBRATE.MEMORY_CHANNELS[i].SAMPLERATE = (uint8_t)uintval;
	}
}

static void SDCOMM_IMPORT_SETT_handler(void)
{
	char readedLine[83] = {0};
	LCD_showInfo("Importing...", false);
	if (f_open(&File, "wolf.ini", FA_READ) == FR_OK)
	{
		uint32_t bytesread = 1;
		uint32_t file_offset = 1;
		while (bytesread != 0)
		{
			dma_memset(SD_workbuffer_A, 0x00, sizeof(SD_workbuffer_A));
			FRESULT res = f_read(&File, SD_workbuffer_A, sizeof(SD_workbuffer_A), (void *)&bytesread);
			if (res != FR_OK)
			{
				f_close(&File);
				if (res == FR_DISK_ERR)
					LCD_showInfo("Disk error", true);
				else if (res == FR_INT_ERR)
					LCD_showInfo("Int error", true);
				else if (res == FR_NOT_READY)
					LCD_showInfo("Not Ready error", true);
				else if (res == FR_INVALID_OBJECT)
					LCD_showInfo("Invalid Object error", true);
				else if (res == FR_TIMEOUT)
					LCD_showInfo("Timeout error", true);
				else
					LCD_showInfo("Unknown error", true);
				SD_Present = false;
				return;
			}
			uint16_t start_index = 0;
			if (bytesread != 0)
			{
				// sendToDebug_str((char*)workbuffer);
				char *istr = strstr((char *)SD_workbuffer_A + start_index, "\r\n"); // look for the end of the line
				while (istr != NULL && start_index < sizeof(SD_workbuffer_A))
				{
					uint16_t len = (uint16_t)((uint32_t)istr - ((uint32_t)SD_workbuffer_A + start_index));
					if (len <= 80)
					{
						dma_memset(readedLine, 0x00, sizeof(readedLine));
						strncpy(readedLine, (char *)SD_workbuffer_A + start_index, len);
						start_index += len + 2;
						file_offset += len + 2;
						istr = strstr((char *)SD_workbuffer_A + start_index, "\r\n"); // look for the end of the line
						SDCOMM_PARSE_SETT_LINE(readedLine);
					}
					else
					{
						LCD_showInfo("Line length error", true);
						break;
					}
				}
				// tailing
				f_lseek(&File, file_offset);
				// println("Estimate: ", bytesread - start_index);
			}
		}
		// reinit
		COLOR = &COLOR_THEMES[TRX.ColorThemeId];
		LAYOUT = &LAYOUT_THEMES[TRX.LayoutThemeId];
		FFT_Init();
		NeedReinitAudioFiltersClean = true;
		TRX_setFrequency(CurrentVFO->Freq, CurrentVFO);
		TRX_setFrequency(SecondaryVFO->Freq, SecondaryVFO);
		TRX_setMode(CurrentVFO->Mode, CurrentVFO);
		TRX_setMode(SecondaryVFO->Mode, SecondaryVFO);
	}
	else
	{
		f_close(&File);
		LCD_showInfo("SD error", true);
		SD_Present = false;
		return;
	}
	f_close(&File);
	NeedSaveSettings = true;
	LCD_showInfo("Settings import complete", true);
}

static void SDCOMM_MKFS_handler(void)
{
	LCD_showInfo("Start formatting...", false);
	FRESULT res = f_mkfs((TCHAR const *)USERPath, FM_FAT32, 0, SD_workbuffer_A, sizeof SD_workbuffer_A);
	if (res == FR_OK)
	{
		LCD_showInfo("SD Format complete", true);
	}
	else
	{
		LCD_showInfo("SD Format error", true);
		SD_Present = false;
	}
}

static void SDCOMM_LISTROOT_handler(void)
{
	if (f_opendir(&dir, "/") == FR_OK)
	{
		while (1)
		{
			if (f_readdir(&dir, &fileInfo) == FR_OK && fileInfo.fname[0])
			{
				char *fn = fileInfo.fname;
				if (fileInfo.fattrib & AM_DIR)
				{
					print("[DIR]  ");
				}
				if (strlen(fn))
				{
					print(fn);
					// sendToDebug_uint32(strlen(fn),false);
				}
				else
				{
					// sendToDebug_str(fileInfo.fname);
					// sendToDebug_uint32(strlen((char*)fileInfo.fname),false);
				}
			}
			else
				break;
			println("");
		}
		f_closedir(&dir);
		println("read complete");
	}
	else
	{
		SD_Present = false;
	}
}

//-----------------------------------------------
static uint8_t SPIx_WriteRead(uint8_t Byte)
{
	uint8_t SPIx_receivedByte = 0;

	if (!SPI_Transmit(&Byte, &SPIx_receivedByte, 1, SD_CS_GPIO_Port, SD_CS_Pin, false, SPI_SD_PRESCALER, false))
		println("SD SPI R Err");

	return SPIx_receivedByte;
}

static void SPI_SendByte(uint8_t bt)
{
	SPIx_WriteRead(bt);
}

static uint8_t SPI_ReceiveByte(void)
{
	return SPIx_WriteRead(0xFF);
}

void SPI_Release(void)
{
	SPIx_WriteRead(0xFF);
}

uint8_t SPI_wait_ready(void)
{
	uint8_t res;
	uint16_t cnt;
	cnt = 0;
	do
	{ // BUSY
		res = SPI_ReceiveByte();
		cnt++;
	} while ((res != 0xFF) && (cnt < 0xFFFF));
	if (cnt >= 0xFFFF)
		return 1;
	return res;
}

uint8_t SD_cmd(uint8_t cmd, uint32_t arg)
{
	uint8_t n, res;
	// ACMD<n> is the command sequense of CMD55-CMD<n>
	if (cmd & 0x80)
	{
		cmd &= 0x7F;
		res = SD_cmd(CMD55, 0);
		if (res > 1)
			return res;
	}
	// Select the card
	// HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
	SPI_ReceiveByte();
	// HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
	SPI_ReceiveByte();
	// Send a command packet
	SPI_SendByte(cmd);					// Start + Command index
	SPI_SendByte((uint8_t)(arg >> 24)); // Argument[31..24]
	SPI_SendByte((uint8_t)(arg >> 16)); // Argument[23..16]
	SPI_SendByte((uint8_t)(arg >> 8));	// Argument[15..8]
	SPI_SendByte((uint8_t)(arg));		// Argument[7..0]
	n = 0x01;							// Dummy CRC + Stop

	uint8_t crcval = 0x00U;
	crcval = sd_crc7_byte(crcval, cmd);
	crcval = sd_crc7_byte(crcval, (uint8_t)(arg >> 24));
	crcval = sd_crc7_byte(crcval, (uint8_t)(arg >> 16));
	crcval = sd_crc7_byte(crcval, (uint8_t)(arg >> 8));
	crcval = sd_crc7_byte(crcval, (uint8_t)(arg));
	n = (crcval << 1) | 0x01U;
	// println("CMD CRC: ",n);

	if (cmd == CMD0)
	{
		n = 0x95;
	} // Valid CRC for CMD0(0)
	if (cmd == CMD8)
	{
		n = 0x87;
	} // Valid CRC for CMD8(0x1AA)
	SPI_SendByte(n);
	// Receive a command response
	n = 10; // Wait for a valid response in timeout of 10 attempts
	do
	{
		res = SPI_ReceiveByte();
	} while ((res & 0x80) && --n);
	if (n == 0 && SD_Present)
		println("SD CMD timeout");
	// println("SD CMD ", cmd, " RES ", res);
	return res;
}

void SD_PowerOn(void)
{
	HAL_Delay(20);
}

SRAM uint8_t SD_Read_Block_tmp[SD_MAXBLOCK_SIZE] = {0};
uint8_t SD_Read_Block(uint8_t *buff, uint32_t btr)
{
	//println("SD_Read_Block");
	uint8_t result;
	uint16_t cnt;
	SPI_Release(); // FF token
	cnt = 0;
	do
	{
		result = SPI_ReceiveByte();
		cnt++;
	} while ((result != 0xFE) && (cnt < 0xFFFF));
	if (cnt >= 0xFFFF)
	{
		println("SD R Token Err", true);
		return 0;
	}

	dma_memset(buff, 0xFF, btr);
	// for (cnt = 0; cnt < btr; cnt++)
	//   buff[cnt] = SPI_ReceiveByte();
	if (!SPI_Transmit(NULL, SD_Read_Block_tmp, btr, SD_CS_GPIO_Port, SD_CS_Pin, false, SPI_SD_PRESCALER, true))
	{
		println("SD SPI R Err");
		return 0;
	}
	dma_memcpy(buff, SD_Read_Block_tmp, btr);

	// CRC check
	uint32_t crcval = 0x0000U;
	for (uint16_t i = 0; i < btr; i++)
		crcval = sd_crc16_byte(crcval, buff[i]);

	// SPI_Release();
	// SPI_Release();
	uint16_t crc = (SPI_ReceiveByte() << 8) | (SPI_ReceiveByte() << 0);

	if (crcval != crc)
	{
		println(crcval, " -> ", crc, " CRC R ERR");
		return 0;
	}
	// println(crcval, " -> ", crc);
	return 1;
}

SRAM uint8_t SD_Write_Block_tmp[SD_MAXBLOCK_SIZE] = {0};
uint8_t SD_Write_Block(uint8_t *buff, uint8_t token, bool dma)
{
	//println("SD_Write_Block");
	uint8_t result;
	uint16_t cnt;
	SPI_wait_ready(); /* Wait for card ready */
	SPI_SendByte(token);
	if (token != 0xFD)
	{ /* Send data if token is other than StopTran */
		// for (cnt = 0; cnt < sdinfo.BLOCK_SIZE; cnt++)
		// SPI_SendByte(buff[cnt]);

		if (dma)
		{
			dma_memcpy(SD_Write_Block_tmp, buff, sizeof(SD_Write_Block_tmp));
			if (!SPI_Transmit(SD_Write_Block_tmp, NULL, sdinfo.BLOCK_SIZE, SD_CS_GPIO_Port, SD_CS_Pin, false, SPI_SD_PRESCALER, dma))
			{
				println("SD SPI W Err");
				return 0;
			}
		}
		else
		{
			if (!SPI_Transmit(buff, NULL, sdinfo.BLOCK_SIZE, SD_CS_GPIO_Port, SD_CS_Pin, false, SPI_SD_PRESCALER, dma))
			{
				println("SD SPI W Err");
				return 0;
			}
		}

		// CRC check
		uint32_t crcval = 0x0000U;
		for (uint16_t i = 0; i < SD_MAXBLOCK_SIZE; i++)
			crcval = sd_crc16_byte(crcval, buff[i]);
		SPI_SendByte((crcval >> 8) & 0xFF);
		SPI_SendByte((crcval >> 0) & 0xFF);
		// SPI_Release();
		// SPI_Release();
		result = SPI_ReceiveByte();
		if ((result & 0x05) != 0x05)
		{
			println(crcval, " CRC W ERR");
			return 0;
		}
		cnt = 0;
		do
		{ // BUSY
			result = SPI_ReceiveByte();
			cnt++;
		} while ((result != 0xFF) && (cnt < 0xFFFF));

		if (cnt >= 0xFFFF)
		{
			println(crcval, " SD W BUSY");
			return 0;
		}
	}
	return 1;
}

uint8_t sd_ini(void)
{
	uint8_t i, cmd;
	int16_t tmr;
	uint32_t temp;
	sd_crc_generate_table();

	sdinfo.type = 0;
	uint8_t ocr[4];
	uint8_t csd[16];
	temp = hspi2.Init.BaudRatePrescaler;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128; // 156.25 kbbs (96 kbps)
	HAL_SPI_Init(&hspi2);
	// HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
	for (i = 0; i < 10; i++)
		SPI_Release();
	hspi2.Init.BaudRatePrescaler = temp;
	HAL_SPI_Init(&hspi2);
	// HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
	if (SD_cmd(CMD0, 0) == 1) // Enter Idle state
	{
		SPI_Release();
		SD_cmd(ACMD42, 0); // disable pull-up on CD line

		// OCR
		if (SD_cmd(CMD8, 0x1AA) == 1) // SDv2
		{
			for (i = 0; i < 4; i++)
				ocr[i] = SPI_ReceiveByte();
			// sendToDebug_strln("SDv2");
			// sprintf(sd_str_buff,"OCR: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",ocr[0],ocr[1],ocr[2],ocr[3]);
			// sendToDebug_str(sd_str_buff);
			//  Get trailing return value of R7 resp
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) // The card can work at vdd range of 2.7-3.6V
			{
				for (tmr = 12000; tmr && SD_cmd(ACMD41, 1UL << 30); tmr--)
					; // Wait for leaving idle state (ACMD41 with HCS bit)
				if (tmr && SD_cmd(CMD58, 0) == 0)
				{ // Check CCS bit in the OCR
					for (i = 0; i < 4; i++)
						ocr[i] = SPI_ReceiveByte();
					// sprintf(sd_str_buff,"OCR: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",ocr[0],ocr[1],ocr[2],ocr[3]);
					// sendToDebug_str(sd_str_buff);
					sdinfo.type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; // SDv2 (HC or SC)
				}
			}
		}
		else // SDv1 or MMCv3
		{
			if (SD_cmd(ACMD41, 0) <= 1)
			{
				sdinfo.type = CT_SD1;
				cmd = ACMD41; // SDv1
							  // sendToDebug_strln("SDv1");
			}
			else
			{
				sdinfo.type = CT_MMC;
				cmd = CMD1; // MMCv3
							// sendToDebug_strln("MMCv3");
			}
			for (tmr = 25000; tmr && SD_cmd(cmd, 0); tmr--)
				; // Wait for leaving idle state
			sdinfo.BLOCK_SIZE = 512;
			if (!tmr || SD_cmd(CMD16, sdinfo.BLOCK_SIZE) != 0) // Set R/W block length to 512
				sdinfo.type = 0;
		}

		// GET_SECTOR_COUNT // Get drive capacity in unit of sector (DWORD)
		if ((SD_cmd(CMD9, 0) == 0))
		{
			sdinfo.BLOCK_SIZE = 512;

			SPI_ReceiveByte();

			for (i = 0; i < 16; i++)
			{
				csd[i] = SPI_ReceiveByte();
				if (i == 0 && csd[i] >= 0xF0)
					csd[i] = SPI_ReceiveByte(); // repeat (clean buff)
			}

			/*char sd_str_buff[60]={0};
			sprintf(sd_str_buff,"CSD: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",csd[0],csd[1],csd[2],csd[3]);
			print(sd_str_buff);
			sprintf(sd_str_buff,"CSD: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",csd[4],csd[5],csd[6],csd[7]);
			print(sd_str_buff);
			sprintf(sd_str_buff,"CSD: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",csd[8],csd[9],csd[10],csd[11]);
			print(sd_str_buff);
			sprintf(sd_str_buff,"CSD: 0x%02X 0x%02X 0x%02X 0x%02X\r\n",csd[12],csd[13],csd[14],csd[15]);
			print(sd_str_buff);*/

			if ((csd[0] >> 6) == 1) // SDC ver 2.00 // High Capacity - CSD Version 2.0
			{
				uint32_t C_SIZE = (DWORD)csd[9] + ((DWORD)csd[8] << 8) + ((DWORD)(csd[7] & 0x3F) << 16) + 1;
				// println(C_SIZE); //7674 in 4gb, 15248 in 8gb
				sdinfo.SECTOR_COUNT = C_SIZE << 10;
				// println("SDHC sector count: ", sdinfo.SECTOR_COUNT);
			}
			if (sdinfo.SECTOR_COUNT == 0) // Standard Capacity - CSD Version 1.0
			{
				uint32_t csize = ((csd[5] & 0x03) << 10) + ((WORD)csd[6] << 2) + ((WORD)(csd[7] & 0xC0) >> 6);
				// println("csize: ", csize);
				BYTE READ_BL_LEN = (csd[5] & 0xF0) >> 4;
				uint32_t BLOCK_LEN = pow(2, READ_BL_LEN);
				// println("BLOCK_LEN: ", BLOCK_LEN);
				BYTE C_SIZE_MULT = (csd[9] & 0x03) | ((csd[10] & 0x80) >> 5);
				uint32_t MULT = pow(2, (C_SIZE_MULT + 2));
				// println("MULT: ", MULT);
				uint32_t BLOCKNR = (csize + 1) * MULT;
				// println("BLOCKNR: ", BLOCKNR);
				uint32_t SECTOR_COUNT = BLOCKNR * BLOCK_LEN / sdinfo.BLOCK_SIZE;
				// println("SECTOR_COUNT: ", SECTOR_COUNT);
				sdinfo.SECTOR_COUNT = SECTOR_COUNT;
				// println("SDSC sector count: ", sdinfo.SECTOR_COUNT);
			}
			if ((csd[0] >> 6) != 1 || sdinfo.SECTOR_COUNT == 0)
			{ // SDC ver 1.XX or MMC ver 3
				BYTE n = (BYTE)((csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2);
				DWORD csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				sdinfo.SECTOR_COUNT = csize << (n - 9);
				// println("SDC1 sector count: ", sdinfo.SECTOR_COUNT);
			}
			sdinfo.CAPACITY = (uint64_t)sdinfo.SECTOR_COUNT * (uint64_t)sdinfo.BLOCK_SIZE;
		}
		//
	}
	else
	{
		return 1;
	}
	// sprintf(sd_str_buff, "Type SD: 0x%02X\r\n",sdinfo.type);
	// sendToDebug_str(sd_str_buff);
	return 0;
}
