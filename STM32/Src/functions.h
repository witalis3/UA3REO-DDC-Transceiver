#ifndef Functions_h
#define Functions_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "profiler.h"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "arm_math.h"
#pragma GCC diagnostic pop

#define TRX_MODE_LSB 0
#define TRX_MODE_USB 1
#define TRX_MODE_CW_L 2
#define TRX_MODE_CW_U 3
#define TRX_MODE_NFM 4
#define TRX_MODE_WFM 5
#define TRX_MODE_AM 6
#define TRX_MODE_DIGI_L 7
#define TRX_MODE_DIGI_U 8
#define TRX_MODE_IQ 9
#define TRX_MODE_LOOPBACK 10
#define TRX_MODE_NO_TX 11
#define TRX_MODE_COUNT 12

//#define IROM1 __attribute__((section(".ER_IROM1")))          // 2mb FLASH
//#define ITCM __attribute__((section(".ITCM")))            // 64kb ITCM
#define DTCM __attribute__((section(".IRAM1")))              // 128kb DTCM
#define IRAM2 __attribute__((section(".IRAM2")))             // 512kb AXI SRAM
#define SRAM1 __attribute__((section(".SRAM1")))             // SRAM1+SRAM2 128kb+128kb D-Cache disabled in MPU
//#define SRAM2 __attribute__((section(".SRAM2")))             // 128kb D-Cache disabled in MPU
//#define SRAM3 __attribute__((section(".SRAM3")))             // 32kb
//#define SRAM4 __attribute__((section(".SRAM4")))             // 64kb
//#define BACKUP_SRAM __attribute__((section(".BACKUP_SRAM"))) // 4kb Backup SRAM

//UINT from BINARY STRING
#define HEX__(n) 0x##n##LU
#define B8__(x) ((x & 0x0000000FLU) ? 1 : 0) + ((x & 0x000000F0LU) ? 2 : 0) + ((x & 0x00000F00LU) ? 4 : 0) + ((x & 0x0000F000LU) ? 8 : 0) + ((x & 0x000F0000LU) ? 16 : 0) + ((x & 0x00F00000LU) ? 32 : 0) + ((x & 0x0F000000LU) ? 64 : 0) + ((x & 0xF0000000LU) ? 128 : 0)
#define B8(d) ((unsigned char)B8__(HEX__(d)))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define SQRT2 1.41421356237f
#define ARRLENTH(x)  (sizeof(x) / sizeof((x)[0]))
#define MINI_DELAY for(uint_fast16_t wait_i = 0; wait_i < 100; wait_i++) __asm("nop");
		
typedef struct {
    float32_t Load;      /*!< CPU load percentage */
    uint32_t WCNT;   /*!< Number of working cycles in one period. Meant for private use */
    uint32_t SCNT;   /*!< Number of sleeping cycles in one period. Meant for private use */
		uint32_t SINC;
} CPULOAD_t;
		
extern UART_HandleTypeDef huart1;
extern CPULOAD_t CPU_LOAD;

extern void CPULOAD_Init(void);
extern void CPULOAD_GoToSleepMode(void);
extern void CPULOAD_WakeUp(void);
extern void CPULOAD_Calc(void);
extern uint32_t getPhraseFromFrequency(int32_t freq);
extern void addSymbols(char *dest, char *str, uint_fast8_t length, char *symbol, bool toEnd);
extern void sendToDebug_str(char *str);
extern void sendToDebug_strln(char *data);
extern void sendToDebug_str2(char *data1, char *data2);
extern void sendToDebug_str3(char *data1, char *data2, char *data3);
extern void sendToDebug_newline(void);
extern void sendToDebug_flush(void);
extern void sendToDebug_uint8(uint8_t data, bool _inline);
extern void sendToDebug_uint16(uint16_t data, bool _inline);
extern void sendToDebug_uint32(uint32_t data, bool _inline);
extern void sendToDebug_int8(int8_t data, bool _inline);
extern void sendToDebug_int16(int16_t data, bool _inline);
extern void sendToDebug_int32(int32_t data, bool _inline);
extern void sendToDebug_float32(float32_t data, bool _inline);
extern void sendToDebug_hex(uint8_t data, bool _inline);
//extern void delay_us(uint32_t us);
extern float32_t log10f_fast(float32_t X);
extern void readHalfFromCircleBuffer32(uint32_t *source, uint32_t *dest, uint32_t index, uint32_t length);
extern void readHalfFromCircleUSBBuffer24Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length);
extern void dma_memcpy32(uint32_t dest, uint32_t src, uint32_t len);
extern float32_t db2rateV(float32_t i);
extern float32_t db2rateP(float32_t i);
extern float32_t rate2dbV(float32_t i);
extern float32_t rate2dbP(float32_t i);
extern float32_t volume2rate(float32_t i);
extern void shiftTextLeft(char *string, uint_fast16_t shiftLength);
extern float32_t getMaxTXAmplitudeOnFreq(uint32_t freq);
extern float32_t generateSin(float32_t amplitude, uint32_t index, uint32_t samplerate, uint32_t freq);
extern int32_t convertToSPIBigEndian(int32_t in);
extern void arm_quick_sort_f32(float32_t * pSrc, float32_t * pDst, uint32_t blockSize, uint8_t dir);
	
#endif
