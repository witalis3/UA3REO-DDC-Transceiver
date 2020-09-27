#ifndef NOISE_REDUCTION_h
#define NOISE_REDUCTION_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "audio_processor.h"

#define NOISE_REDUCTION_BLOCK_SIZE 64						 // block size for processing in DNR
#define NOISE_REDUCTION_FFT_SIZE 256						 // FFT size for processing in DNR
#define NOISE_REDUCTION_FFT_INSTANCE &arm_cfft_sR_f32_len256 // FFT instance
#define NOISE_REDUCTION_FFT_SIZE_HALF (NOISE_REDUCTION_FFT_SIZE / 2)
#define NOISE_REDUCTION_ALPHA 0.98f // time averaging constant
#define NOISE_REDUCTION_BETA 0.25f	// frequency averaging constant

typedef struct
{
	float32_t NR_InputBuffer[NOISE_REDUCTION_FFT_SIZE];
	uint16_t NR_InputBuffer_index;
	float32_t NR_OutputBuffer[NOISE_REDUCTION_FFT_SIZE];
	uint16_t NR_OutputBuffer_index;
	float32_t NR_Prev_Buffer[NOISE_REDUCTION_FFT_SIZE_HALF];

	float32_t FFT_Buffer[NOISE_REDUCTION_FFT_SIZE * 2];
	const arm_cfft_instance_f32 *FFT_Inst;
	float32_t FFT_COMPLEX_MAG[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t FFT_AVERAGE_MAG[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t FFT_MINIMUM_MAG[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t NR_GAIN[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t LAST_IFFT_RESULT[NOISE_REDUCTION_FFT_SIZE_HALF];
} NR_Instance;

// Public methods
extern void InitNoiseReduction(void);										   // initialize DNR
extern void processNoiseReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id); // run DNR for the data block

#endif
