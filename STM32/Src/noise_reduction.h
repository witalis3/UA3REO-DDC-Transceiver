#ifndef NOISE_REDUCTION_h
#define NOISE_REDUCTION_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "audio_processor.h"

#define NOISE_REDUCTION_BLOCK_SIZE 64
#define NOISE_REDUCTION_FFT_SIZE NOISE_REDUCTION_BLOCK_SIZE
#define NOISE_REDUCTION_FFT_SIZE_HALF (NOISE_REDUCTION_FFT_SIZE/2)

#define NOISE_REDUCTION_AVERAGE 3.0f //number of frames to average magnitudes
#define NOISE_REDUCTION_MINIMAL_MAG_LAST 18.0f //number of averaged magnitudes values for minimum search
#define NOISE_REDUCTION_ALPHA 0.98f //time averaging constant
#define NOISE_REDUCTION_BETA 0.25f //frequency averaging constant

typedef struct
{
	float32_t NR_Buffer[NOISE_REDUCTION_FFT_SIZE];
	float32_t NR_Prev_Buffer[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t FFT_Buffer[NOISE_REDUCTION_FFT_SIZE * 2];
	const arm_cfft_instance_f32* FFT_Inst;
	float32_t FFT_COMPLEX_MAG[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t FFT_AVERAGE_MAG[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t FFT_MINIMUM_MAG[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t NR_GAIN[NOISE_REDUCTION_FFT_SIZE_HALF];
	float32_t LAST_IFFT_RESULT[NOISE_REDUCTION_FFT_SIZE_HALF];
	arm_fir_decimate_instance_f32 DECIMATE_FIR;
	float32_t DECIMATE_FIR_State[NOISE_REDUCTION_BLOCK_SIZE + 4 - 1];
} NR_Instance;

extern void InitNoiseReduction(void);
extern void processNoiseReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id);

#endif
