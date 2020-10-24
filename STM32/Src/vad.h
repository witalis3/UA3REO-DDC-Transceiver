#ifndef VAD_h
#define VAD_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "audio_processor.h"

#define VAD_BLOCK_SIZE 64						 // block size for processing in VAD
#define VAD_MAGNIFY 8								 // FFT Zooming
#define VAD_FFT_SIZE 256						 // FFT size for processing in VAD
#define VAD_FFT_SIZE_HALF (VAD_FFT_SIZE / 2)
#define VAD_FFT_SAMPLES (VAD_BLOCK_SIZE * 4) // number of samples for analysis
#define VAD_ZOOMED_SAMPLES (VAD_BLOCK_SIZE / VAD_MAGNIFY)

// Public methods
extern void InitVAD(void);										   // initialize VAD
extern void processVAD(float32_t *buffer); // run VAD for the data block

#endif
