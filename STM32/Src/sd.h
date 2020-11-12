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
#define CT_MMC 0x01 /* MMC ver 3 */
#define CT_SD1 0x02 /* SD ver 1 */
#define CT_SD2 0x04 /* SD ver 2 */
#define CT_SDC (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK 0x08 /* Block addressing */
//--------------------------------------------------
typedef struct sd_info {
  volatile uint8_t type;//тип карты
} sd_info_ptr;

extern sd_info_ptr sdinfo;
extern FATFS SDFatFs;

//--------------------------------------------------
void SD_PowerOn(void);
uint8_t sd_ini(void);
void SPI_Release(void);
uint8_t SD_Read_Block (uint8_t *buff, uint32_t lba);
uint8_t SD_Write_Block (uint8_t *buff, uint32_t lba);
uint8_t SPI_wait_ready(void);
void SD_Process(void);
//--------------------------------------------------
#endif /* SD_H_ */
