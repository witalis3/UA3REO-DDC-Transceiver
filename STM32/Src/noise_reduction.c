#include "noise_reduction.h"
#include "stm32h7xx_hal.h"
#include "arm_const_structs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include "arm_math.h"
#include "settings.h"
#include "functions.h"

IRAM2 static NR_Instance NR_RX1 = {
	.NR_InputBuffer = {0},
	.NR_InputBuffer_index = 0,
	.NR_OutputBuffer = {0},
	.NR_OutputBuffer_index = 0,
	.NR_Prev_Buffer = {0},
	.FFT_Buffer = {0},
	.FFT_Inst = NOISE_REDUCTION_FFT_INSTANCE,
	.FFT_COMPLEX_MAG = {0},
	.FFT_AVERAGE_MAG = {0},
	.FFT_MINIMUM_MAG = {0},
	.NR_GAIN = {0},
	.LAST_IFFT_RESULT = {0},
};
IRAM2 static NR_Instance NR_RX2 = {
	.NR_InputBuffer = {0},
	.NR_InputBuffer_index = 0,
	.NR_OutputBuffer = {0},
	.NR_OutputBuffer_index = 0,
	.NR_Prev_Buffer = {0},
	.FFT_Buffer = {0},
	.FFT_Inst = NOISE_REDUCTION_FFT_INSTANCE,
	.FFT_COMPLEX_MAG = {0},
	.FFT_AVERAGE_MAG = {0},
	.FFT_MINIMUM_MAG = {0},
	.NR_GAIN = {0},
	.LAST_IFFT_RESULT = {0},
};

static float32_t von_Hann[NOISE_REDUCTION_FFT_SIZE] = {0};

void InitNoiseReduction(void)
{
	for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
		von_Hann[idx] = sqrtf(0.5f * (1.0f - arm_cos_f32((2.0f * PI * idx) / (float32_t)NOISE_REDUCTION_FFT_SIZE)));
}

void processNoiseReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id)
{
	NR_Instance* instance = &NR_RX1;
	if(rx_id==AUDIO_RX2)
		instance = &NR_RX2;
	//fill input buffer
	memcpy(&instance->NR_InputBuffer[instance->NR_InputBuffer_index * NOISE_REDUCTION_BLOCK_SIZE], buffer, NOISE_REDUCTION_BLOCK_SIZE * 4);
	instance->NR_InputBuffer_index++;
	if(instance->NR_InputBuffer_index == (NOISE_REDUCTION_FFT_SIZE / NOISE_REDUCTION_BLOCK_SIZE)) //input buffer ready
	{
		instance->NR_InputBuffer_index = 0;
		instance->NR_OutputBuffer_index = 0;
		//overlap is 50%
		for(uint8_t loop = 0; loop < 2 ; loop++)
		{
			//prepare fft / fill buffers
			//first half - prev data
			for(uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
					instance->FFT_Buffer[idx * 2] = instance->NR_Prev_Buffer[idx]; // real
					instance->FFT_Buffer[idx * 2 + 1] = 0.0f; // imaginary
			}
			//save last half to prev data
			for(uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
					instance->NR_Prev_Buffer[idx] = instance->NR_InputBuffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF + idx];
			}
			//last half - last data
			for(uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
					instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE + idx * 2] = instance->NR_InputBuffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF + idx]; // real
					instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE + idx * 2 + 1] = 0.0f; // imaginary
			}
			//windowing
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
					instance->FFT_Buffer[idx * 2] *= von_Hann[idx];
			//do fft
			arm_cfft_f32(instance->FFT_Inst, instance->FFT_Buffer, 0, 1);
			//get magnitude
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF ; idx++)
				instance->FFT_COMPLEX_MAG[idx]=sqrtf(instance->FFT_Buffer[idx * 2] * instance->FFT_Buffer[idx * 2] + instance->FFT_Buffer[idx * 2 + 1] * instance->FFT_Buffer[idx * 2 + 1]);
			//average magnitude
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF ; idx++)
				instance->FFT_AVERAGE_MAG[idx] = (instance->FFT_AVERAGE_MAG[idx] + instance->FFT_COMPLEX_MAG[idx]) / (float32_t)TRX.DNR_AVERAGE;
			//minimum magnitude
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF ; idx++)
				if(instance->FFT_MINIMUM_MAG[idx] > instance->FFT_COMPLEX_MAG[idx])
					instance->FFT_MINIMUM_MAG[idx] = instance->FFT_COMPLEX_MAG[idx];
				else
					instance->FFT_MINIMUM_MAG[idx] += instance->FFT_COMPLEX_MAG[idx] / (float32_t)TRX.DNR_MINIMAL;
			//calculate signal-noise-ratio
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF ; idx++)
			{
				float32_t snr = instance->FFT_COMPLEX_MAG[idx] / instance->FFT_MINIMUM_MAG[idx];
				float32_t lambda = 0.0f;
				if (snr > (float32_t)TRX.DNR_SNR_THRESHOLD)
					lambda = instance->FFT_MINIMUM_MAG[idx];
				else 
					lambda = instance->FFT_AVERAGE_MAG[idx];	
				//gain calc
				float32_t gain = 0.0f;
				if(instance->FFT_COMPLEX_MAG[idx] > 0.0f)
					gain = 1.0f - (lambda / instance->FFT_COMPLEX_MAG[idx]);
				//time smoothing (exponential averaging) of gain weights
				instance->NR_GAIN[idx] = NOISE_REDUCTION_ALPHA * instance->NR_GAIN[idx] + (1 - NOISE_REDUCTION_ALPHA) * gain;
				//frequency smoothing of gain weights
				if(idx > 0)
					instance->NR_GAIN[idx] = NOISE_REDUCTION_BETA * instance->NR_GAIN[idx - 1] + (1 - 2 * NOISE_REDUCTION_BETA) * instance->NR_GAIN[idx] + NOISE_REDUCTION_BETA * instance->NR_GAIN[idx + 1];
			}
			//apply gain weighting
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
				instance->FFT_Buffer[idx * 2] *= instance->NR_GAIN[idx];
				instance->FFT_Buffer[idx * 2 + 1] *= instance->NR_GAIN[idx];
				//symmetry
				instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE * 2 - idx * 2 - 2] *= instance->NR_GAIN[idx];
				instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE * 2 - idx * 2 - 1] *= instance->NR_GAIN[idx];
			}
			//do inverse fft
			arm_cfft_f32(instance->FFT_Inst, instance->FFT_Buffer, 1, 1);
			//windowing
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
					instance->FFT_Buffer[idx * 2] *= von_Hann[idx];
			//return data (do overlap-add: take real part of first half of current iFFT result and add to 2nd half of last frames´ iFFT result)
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				instance->NR_OutputBuffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF + idx] = instance->FFT_Buffer[idx * 2] + instance->LAST_IFFT_RESULT[idx];
			//save 2nd half of ifft result
			for(uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
					instance->LAST_IFFT_RESULT[idx] = instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE_HALF * 2 + idx * 2];
		}
	}
	if(instance->NR_OutputBuffer_index < (NOISE_REDUCTION_FFT_SIZE / NOISE_REDUCTION_BLOCK_SIZE))
		memcpy(buffer, &instance->NR_OutputBuffer[instance->NR_OutputBuffer_index * NOISE_REDUCTION_BLOCK_SIZE], NOISE_REDUCTION_BLOCK_SIZE * 4);
	instance->NR_OutputBuffer_index++;
}
