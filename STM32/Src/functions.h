#ifndef Functions_h
#define Functions_h

__asm(".global __use_no_heap\n\t");

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <arm_math.h>
#include "profiler.h"
#include "print.h"
#include "settings.h"

#define ITCM __attribute__((section(".ITCM"))) __attribute__((aligned(32)))   // 64kb ITCM
#define IRAM2 __attribute__((section(".IRAM"))) __attribute__((aligned(32)))  // 512kb AXI SRAM
#define SRAM __attribute__((section(".SRAM"))) __attribute__((aligned(32)))   // SRAM1+SRAM2+SRAM3 128kb+128kb+32kb
#define SRAM4 __attribute__((section(".SRAM4"))) __attribute__((aligned(32))) // SRAM4 64kb
#define BACKUP_SRAM_BANK1_ADDR (uint32_t *)0x38800000
#define BACKUP_SRAM_BANK2_ADDR (uint32_t *)0x38800800
//#define ALIGN_32BIT __attribute__((aligned(32)))

// UINT from BINARY STRING
#define HEX_(n) 0x##n##LU
#define B8_(x) (((x) & 0x0000000FLU) ? 1 : 0) + (((x) & 0x000000F0LU) ? 2 : 0) + (((x) & 0x00000F00LU) ? 4 : 0) + (((x) & 0x0000F000LU) ? 8 : 0) + (((x) & 0x000F0000LU) ? 16 : 0) + (((x) & 0x00F00000LU) ? 32 : 0) + (((x) & 0x0F000000LU) ? 64 : 0) + (((x) & 0xF0000000LU) ? 128 : 0)
#define B8(d) ((unsigned char)B8_(HEX_(d)))

#define BYTE_TO_BINARY(byte)     \
  (byte & 0x80 ? '1' : '0'),     \
      (byte & 0x40 ? '1' : '0'), \
      (byte & 0x20 ? '1' : '0'), \
      (byte & 0x10 ? '1' : '0'), \
      (byte & 0x08 ? '1' : '0'), \
      (byte & 0x04 ? '1' : '0'), \
      (byte & 0x02 ? '1' : '0'), \
      (byte & 0x01 ? '1' : '0')
#define BIT16_TO_BINARY(byte)      \
  (byte & 0x8000 ? '1' : '0'),     \
      (byte & 0x4000 ? '1' : '0'), \
      (byte & 0x2000 ? '1' : '0'), \
      (byte & 0x1000 ? '1' : '0'), \
      (byte & 0x0800 ? '1' : '0'), \
      (byte & 0x0400 ? '1' : '0'), \
      (byte & 0x0200 ? '1' : '0'), \
      (byte & 0x0100 ? '1' : '0'), \
      (byte & 0x0080 ? '1' : '0'), \
      (byte & 0x0040 ? '1' : '0'), \
      (byte & 0x0020 ? '1' : '0'), \
      (byte & 0x0010 ? '1' : '0'), \
      (byte & 0x0008 ? '1' : '0'), \
      (byte & 0x0004 ? '1' : '0'), \
      (byte & 0x0002 ? '1' : '0'), \
      (byte & 0x0001 ? '1' : '0')
#define BIT26_TO_BINARY(byte)         \
  (byte & 0x2000000 ? '1' : '0'),     \
      (byte & 0x1000000 ? '1' : '0'), \
      (byte & 0x800000 ? '1' : '0'),  \
      (byte & 0x400000 ? '1' : '0'),  \
      (byte & 0x200000 ? '1' : '0'),  \
      (byte & 0x100000 ? '1' : '0'),  \
      (byte & 0x80000 ? '1' : '0'),   \
      (byte & 0x40000 ? '1' : '0'),   \
      (byte & 0x20000 ? '1' : '0'),   \
      (byte & 0x10000 ? '1' : '0'),   \
      (byte & 0x8000 ? '1' : '0'),    \
      (byte & 0x4000 ? '1' : '0'),    \
      (byte & 0x2000 ? '1' : '0'),    \
      (byte & 0x1000 ? '1' : '0'),    \
      (byte & 0x0800 ? '1' : '0'),    \
      (byte & 0x0400 ? '1' : '0'),    \
      (byte & 0x0200 ? '1' : '0'),    \
      (byte & 0x0100 ? '1' : '0'),    \
      (byte & 0x0080 ? '1' : '0'),    \
      (byte & 0x0040 ? '1' : '0'),    \
      (byte & 0x0020 ? '1' : '0'),    \
      (byte & 0x0010 ? '1' : '0'),    \
      (byte & 0x0008 ? '1' : '0'),    \
      (byte & 0x0004 ? '1' : '0'),    \
      (byte & 0x0002 ? '1' : '0'),    \
      (byte & 0x0001 ? '1' : '0')

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define charToInt(chr) ((int)chr - 48)

#define Aligned_CleanDCache_by_Addr(buff, size) (SCB_CleanDCache_by_Addr((uint32_t *)(((uint32_t)buff) & ~(uint32_t)0x1F), (size) + 32))
#define Aligned_InvalidateDCache_by_Addr(buff, size) (SCB_InvalidateDCache_by_Addr((uint32_t *)(((uint32_t)buff) & ~(uint32_t)0x1F), (size) + 32))
#define Aligned_CleanInvalidateDCache_by_Addr(buff, size) (SCB_CleanInvalidateDCache_by_Addr((uint32_t *)(((uint32_t)buff) & ~(uint32_t)0x1F), (size) + 32))

#define isnanf __ARM_isnanf
#define isinff __ARM_isinff

#define F_PI 3.141592653589793238463f
#define F_2PI (3.141592653589793238463f * 2.0f)
#define SQRT2 1.414213562373095f
#define DEG2RAD(x) ((float32_t)(x)*F_PI / 180.f)
#define RAD2DEG(x) ((float32_t)(x)*180.f / F_PI)
#define ARRLENTH(x) (sizeof(x) / sizeof((x)[0]))
#define MINI_DELAY                                       \
  for (uint_fast16_t wait_i = 0; wait_i < 100; wait_i++) \
    __asm("nop");

#define DMA_MAX_BLOCK 65000

// Example of __DATE__ string: "Jul 27 2012"
//                              01234567890
#define BUILD_YEAR_CH0 (__DATE__[7])
#define BUILD_YEAR_CH1 (__DATE__[8])
#define BUILD_YEAR_CH2 (__DATE__[9])
#define BUILD_YEAR_CH3 (__DATE__[10])
#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')
#define BUILD_MONTH_CH0 \
  ((BUILD_MONTH_IS_OCT || BUILD_MONTH_IS_NOV || BUILD_MONTH_IS_DEC) ? '1' : '0')
#define BUILD_MONTH_CH1                                       \
  (                                                           \
      (BUILD_MONTH_IS_JAN) ? '1' : (BUILD_MONTH_IS_FEB) ? '2' \
                               : (BUILD_MONTH_IS_MAR)   ? '3' \
                               : (BUILD_MONTH_IS_APR)   ? '4' \
                               : (BUILD_MONTH_IS_MAY)   ? '5' \
                               : (BUILD_MONTH_IS_JUN)   ? '6' \
                               : (BUILD_MONTH_IS_JUL)   ? '7' \
                               : (BUILD_MONTH_IS_AUG)   ? '8' \
                               : (BUILD_MONTH_IS_SEP)   ? '9' \
                               : (BUILD_MONTH_IS_OCT)   ? '0' \
                               : (BUILD_MONTH_IS_NOV)   ? '1' \
                               : (BUILD_MONTH_IS_DEC)   ? '2' \
                                                        : /* error default */ '?')
#define BUILD_DAY_CH0 (__DATE__[4] == ' ' ? '0' : __DATE__[4])
#define BUILD_DAY_CH1 (__DATE__[5])

// Example of __TIME__ string: "21:06:19"
//                              01234567
#define BUILD_HOUR_CH0 (__TIME__[0])
#define BUILD_HOUR_CH1 (__TIME__[1])
#define BUILD_MIN_CH0 (__TIME__[3])
#define BUILD_MIN_CH1 (__TIME__[4])
#define BUILD_SEC_CH0 (__TIME__[6])
#define BUILD_SEC_CH1 (__TIME__[7])

#define ELEM_SWAP_F32(a, b)     \
  {                             \
    register float32_t t = (a); \
    (a) = (b);                  \
    (b) = t;                    \
  }

typedef struct
{
  float32_t Load; /*!< CPU load percentage */
  uint32_t WCNT;  /*!< Number of working cycles in one period. Meant for private use */
  uint32_t SCNT;  /*!< Number of sleeping cycles in one period. Meant for private use */
  uint32_t SINC;
} CPULOAD_t;

extern CPULOAD_t CPU_LOAD;
volatile extern bool SPI_busy;
volatile extern bool SPI_process;
volatile extern bool SPI_TXRX_ready;

extern void CPULOAD_Init(void);
extern void CPULOAD_GoToSleepMode(void);
extern void CPULOAD_WakeUp(void);
extern void CPULOAD_Calc(void);
extern uint32_t getRXPhraseFromFrequency(int32_t freq, uint8_t rx_num);
extern uint32_t getTXPhraseFromFrequency(float64_t freq);
extern void addSymbols(char *dest, char *str, uint_fast8_t length, char *symbol, bool toEnd);
extern void print_chr_LCDOnly(char chr);
extern void print_flush(void);
extern void print_hex(uint8_t data, bool _inline);
extern void print_bin8(uint8_t data, bool _inline);
extern void print_bin16(uint16_t data, bool _inline);
extern void print_bin26(uint32_t data, bool _inline);
// extern void delay_us(uint32_t us);
extern float32_t log10f_fast(float32_t X);
extern void readFromCircleBuffer32(uint32_t *source, uint32_t *dest, uint32_t index, uint32_t length, uint32_t words_to_read);
extern void readHalfFromCircleUSBBuffer24Bit(uint8_t *source, int32_t *dest, uint32_t index, uint32_t length);
extern void dma_memcpy32(void *dest, void *src, uint32_t size);
extern void dma_memcpy(void *dest, void *src, uint32_t size);
extern void dma_memset32(void *dest, uint32_t val, uint32_t size);
extern void memset16(void *dest, uint16_t val, uint32_t size);
extern void dma_memset(void *dest, uint8_t val, uint32_t size);
extern void SLEEPING_MDMA_PollForTransfer(MDMA_HandleTypeDef *hmdma);
extern float32_t db2rateV(float32_t i);
extern float32_t db2rateP(float32_t i);
extern float32_t rate2dbV(float32_t i);
extern float32_t rate2dbP(float32_t i);
extern float32_t volume2rate(float32_t i);
extern void shiftTextLeft(char *string, uint_fast16_t shiftLength);
extern float32_t getMaxTXAmplitudeOnFreq(uint32_t freq);
extern float32_t generateSin(float32_t amplitude, float32_t *index, uint32_t samplerate, uint32_t freq);
extern int32_t convertToSPIBigEndian(int32_t in);
extern uint8_t rev8(uint8_t data);
extern bool SPI_Transmit(uint8_t *out_data, uint8_t *in_data, uint16_t count, GPIO_TypeDef *CS_PORT, uint16_t CS_PIN, bool hold_cs, uint32_t prescaler, bool dma);
extern float32_t quick_median_select(float32_t *arr, int n);
extern uint8_t getInputType(void);
extern unsigned int sd_crc16_byte(unsigned int crcval, unsigned int byte);
extern unsigned int sd_crc7_byte(unsigned int crcval, unsigned int byte);
extern void sd_crc_generate_table(void);
extern void arm_biquad_cascade_df2T_f32_single(const arm_biquad_cascade_df2T_instance_f32 *S, const float32_t *pSrc, float32_t *pDst, uint32_t blockSize);
extern void arm_biquad_cascade_df2T_f32_IQ(const arm_biquad_cascade_df2T_instance_f32 *I, const arm_biquad_cascade_df2T_instance_f32 *Q, const float32_t *pSrc_I, const float32_t *pSrc_Q, float32_t *pDst_I, float32_t *pDst_Q, uint32_t blockSize);
extern char cleanASCIIgarbage(char chr);
extern bool textStartsWith(const char *a, const char *b);
extern void *alloc_to_wtf(uint32_t size, bool reset);

#endif
