#ifndef NOISE_REDUCTION_h
#define NOISE_REDUCTION_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "audio_processor.h"

#define NOISE_REDUCTION_BLOCK_SIZE 64						 // block size for processing in DNR
#define SPEEXALLOCSIZE (15584 / 64 * NOISE_REDUCTION_BLOCK_SIZE * 2) //speex buffer for 2 rx

// Public methods
extern void InitNoiseReduction(void);															// initialize DNR
extern void processNoiseReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id); // run DNR for the data block

#endif
