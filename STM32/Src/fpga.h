#ifndef FPGA_h
#define FPGA_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include "fft.h"
#include "audio_processor.h"
#include "settings.h"

#define FPGA_flash_size 0x200000
#define FPGA_flash_file_offset (0xA0 - 1)
#define FPGA_sector_size (64 * 1024)
#define FPGA_page_size 256
#define FPGA_FLASH_COMMAND_DELAY for(uint32_t wait = 0; wait < 50; wait++) __asm("nop"); //50
#define FPGA_FLASH_WRITE_DELAY for(uint32_t wait = 0; wait < 500; wait++) __asm("nop"); //500
#define FPGA_FLASH_READ_DELAY for(uint32_t wait = 0; wait < 50; wait++) __asm("nop"); //50

//Micron M25P80 Serial Flash COMMANDS:
#define M25P80_WRITE_ENABLE 0x06
#define M25P80_WRITE_DISABLE 0x04
#define M25P80_READ_IDENTIFICATION 0x9F
#define M25P80_READ_IDENTIFICATION2 0x9E
#define M25P80_READ_STATUS_REGISTER 0x05
#define M25P80_WRITE_STATUS_REGISTER 0x01
#define M25P80_READ_DATA_BYTES 0x03
#define M25P80_READ_DATA_BYTES_at_HIGHER_SPEED 0x0B
#define M25P80_PAGE_PROGRAM 0x02
#define M25P80_SECTOR_ERASE 0xD8
#define M25P80_BULK_ERASE 0xC7
#define M25P80_DEEP_POWER_DOWN 0xB9
#define M25P80_RELEASE_from_DEEP_POWER_DOWN 0xAB

//Public variables
extern volatile uint32_t FPGA_samples;                                     //счетчик числа семплов при обмене с FPGA
extern volatile bool FPGA_Buffer_underrun;                                 //флаг недостатка данных из FPGA
extern volatile bool FPGA_NeedSendParams;                                  //флаг необходимости отправить параметры в FPGA
extern volatile bool FPGA_NeedGetParams;                                   //флаг необходимости получить параметры из FPGA
extern volatile bool FPGA_NeedRestart;                                     //флаг необходимости рестарта модулей FPGA
extern volatile float32_t FPGA_Audio_Buffer_RX1_Q[FPGA_RX_IQ_BUFFER_SIZE]; //буфферы FPGA
extern volatile float32_t FPGA_Audio_Buffer_RX1_I[FPGA_RX_IQ_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_Buffer_RX2_Q[FPGA_RX_IQ_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_Buffer_RX2_I[FPGA_RX_IQ_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_SendBuffer_Q[FPGA_TX_IQ_BUFFER_SIZE];
extern volatile float32_t FPGA_Audio_SendBuffer_I[FPGA_TX_IQ_BUFFER_SIZE];
extern uint_fast16_t FPGA_Audio_RXBuffer_Index; //текущий индекс в буфферах FPGA
extern uint_fast16_t FPGA_Audio_TXBuffer_Index; //текущий индекс в буфферах FPGA
extern bool FPGA_Audio_Buffer_State;          //состояние буффера, заполнена половина или целиком true - compleate ; false - half

//Public methods
extern void FPGA_Init(void);                //инициализация обмена с FPGA
extern void FPGA_fpgadata_iqclock(void);    //обмен IQ данными с FPGA
extern void FPGA_fpgadata_stuffclock(void); //обмен параметрами с FPGA
extern void FPGA_restart(void);             //перезапуск модулей FPGA

#endif
