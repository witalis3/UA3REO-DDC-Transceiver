#ifndef __HARDWARE_H
#define __HARDWARE_H

#include "main.h"
#include "arm_math_types.h"
#include <stdbool.h>

#define HRDW_HAS_DMA2D true
#define HRDW_AUDIO_CODEC_I2S hi2s3

// SPI Speed
#define SPI_FRONT_UNIT_PRESCALER SPI_BAUDRATEPRESCALER_4
#define SPI_SD_PRESCALER SPI_BAUDRATEPRESCALER_2
#define SPI_EEPROM_PRESCALER SPI_BAUDRATEPRESCALER_4

//buffers
#define CODEC_AUDIO_BUFFER_SIZE (AUDIO_BUFFER_SIZE * 2) // the size of the circular buffer is 2 times larger than the FPGA buffer, we work in the first half, then on the other
#define CODEC_AUDIO_BUFFER_HALF_SIZE AUDIO_BUFFER_SIZE  // half buffer
#define AUDIO_BUFFER_SIZE (192 * 2)								 // the size of the buffer for working with sound 48kHz
#define AUDIO_BUFFER_HALF_SIZE (AUDIO_BUFFER_SIZE / 2)			 // buffer size for working with sound 48kHz
#define FPGA_TX_IQ_BUFFER_SIZE AUDIO_BUFFER_SIZE				 // size of TX data buffer for FPGA
#define FPGA_TX_IQ_BUFFER_HALF_SIZE (FPGA_TX_IQ_BUFFER_SIZE / 2) // half the size of the TX data buffer for FPGA
#define FPGA_RX_IQ_BUFFER_SIZE FPGA_TX_IQ_BUFFER_SIZE			 // size of the RX data buffer from the PGA
#define FPGA_RX_IQ_BUFFER_HALF_SIZE (FPGA_RX_IQ_BUFFER_SIZE / 2) // half the size of the RX data buffer from the PGA

extern void HRDW_Init(void);
extern float32_t HRDW_getCPUTemperature(void);
extern float32_t HRDW_getCPUVref(void);
extern uint32_t HRDW_getAudioCodecTX_DMAIndex(void);
extern uint32_t HRDW_getAudioCodecRX_DMAIndex(void);
extern bool HRDW_FrontUnit_SPI(uint8_t *out_data, uint8_t *in_data, uint16_t count, bool hold_cs);
extern bool HRDW_FrontUnit2_SPI(uint8_t *out_data, uint8_t *in_data, uint16_t count, bool hold_cs);
extern bool HRDW_FrontUnit3_SPI(uint8_t *out_data, uint8_t *in_data, uint16_t count, bool hold_cs);
extern bool HRDW_EEPROM_SPI(uint8_t *out_data, uint8_t *in_data, uint16_t count, bool hold_cs);
extern bool HRDW_SD_SPI(uint8_t *out_data, uint8_t *in_data, uint16_t count, bool hold_cs);

#endif
