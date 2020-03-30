#ifndef AUTO_NOTCH_h
#define AUTO_NOTCH_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "audio_processor.h"

#define AUTO_NOTCH_BLOCK_SIZE (FPGA_AUDIO_BUFFER_HALF_SIZE / 3)
#define AUTO_NOTCH_TAPS AUTO_NOTCH_BLOCK_SIZE
#define AUTO_NOTCH_REFERENCE_SIZE (AUTO_NOTCH_BLOCK_SIZE * 2)
#define AUTO_NOTCH_STEP 0.0001f

extern void InitAutoNotchReduction(void);
extern void processAutoNotchReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id);

#endif
