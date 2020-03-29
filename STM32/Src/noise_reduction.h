#ifndef NOISE_REDUCTION_h
#define NOISE_REDUCTION_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "audio_processor.h"

#define NOISE_REDUCTION_BLOCK_SIZE 64
#define NOISE_REDUCTION_TAPS 16
#define NOISE_REDUCTION_REFERENCE_SIZE (NOISE_REDUCTION_BLOCK_SIZE * 2)
#define NOISE_REDUCTION_STEP 0.000001f

extern void InitNoiseReduction(void);
extern void processNoiseReduction(float32_t *bufferIn, float32_t *bufferOut, AUDIO_PROC_RX_NUM rx_id);

#endif
