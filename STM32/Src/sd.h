#ifndef SD_H_
#define SD_H_
//--------------------------------------------------
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "fatfs.h"
//--------------------------------------------------
#define SD_CARD_SCAN_INTERVAL 1000
//--------------------------------------------------
/* Card type flags (CardType) */
#define CT_MMC 0x01				 /* MMC ver 3 */
#define CT_SD1 0x02				 /* SD ver 1 */
#define CT_SD2 0x04				 /* SD ver 2 */
#define CT_SDC (CT_SD1 | CT_SD2) /* SD */
#define CT_BLOCK 0x08			 /* Block addressing */
// Definitions for MMC/SDC command
#define CMD0 (0x40 + 0)	   // GO_IDLE_STATE
#define CMD1 (0x40 + 1)	   // SEND_OP_COND (MMC)
#define CMD8 (0x40 + 8)	   // SEND_IF_COND
#define CMD9 (0x40 + 9)	   // SEND_CSD
#define CMD12 (0x40 + 12)  /* STOP_TRANSMISSION */
#define CMD16 (0x40 + 16)  // SET_BLOCKLEN
#define CMD17 (0x40 + 17)  // READ_SINGLE_BLOCK
#define CMD18 (0x40 + 18)  /* READ_MULTIPLE_BLOCK */
#define CMD24 (0x40 + 24)  // WRITE_BLOCK
#define CMD25 (0x40 + 25)  /* WRITE_MULTIPLE_BLOCK */
#define CMD55 (0x40 + 55)  // APP_CMD
#define CMD58 (0x40 + 58)  // READ_OCR
#define ACMD41 (0xC0 + 41) // SEND_OP_COND (SDC)
#define ACMD23 (0xC0 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
//--------------------------------------------------
typedef struct sd_info
{
	volatile uint8_t type;
	DWORD SECTOR_COUNT;
	uint32_t capacity;
} sd_info_ptr;

typedef enum
{
	SDCOMM_IDLE,
	SDCOMM_LIST_ROOT,
	SDCOMM_FORMAT,
	SDCOMM_EXPORT_SETTINGS,
	SDCOMM_IMPORT_SETTINGS,
	SDCOMM_START_RECORD,
	SDCOMM_PROCESS_RECORD,
} SD_COMMAND;

extern sd_info_ptr sdinfo;
extern FATFS SDFatFs;
extern bool SD_RecordInProcess;
extern bool SD_CommandInProcess;
extern bool SD_underrun;
extern bool SD_NeedStopRecord;
extern bool SD_Present;
extern uint32_t SD_RecordBufferIndex;
extern BYTE SD_workbuffer_A[_MAX_SS];
extern BYTE SD_workbuffer_B[_MAX_SS];
extern BYTE SD_workbuffer_current;

//--------------------------------------------------
extern void SD_PowerOn(void);
extern uint8_t sd_ini(void);
extern void SPI_Release(void);
extern uint8_t SD_Read_Block(uint8_t *buff, uint32_t btr);
extern uint8_t SD_Write_Block(uint8_t *buff, uint8_t token);
extern uint8_t SPI_wait_ready(void);
extern uint8_t SD_cmd(uint8_t cmd, uint32_t arg);
extern void SD_Process(void);
extern bool SD_isIdle(void);
extern void SD_doCommand(SD_COMMAND command, bool force);
//--------------------------------------------------
#endif /* SD_H_ */
