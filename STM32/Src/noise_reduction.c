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
	.NR_Buffer = {0},
	.NR_Prev_Buffer = {0},
	.FFT_Buffer = {0},
	.FFT_Inst = &arm_cfft_sR_f32_len64,
	.FFT_COMPLEX_MAG = {0},
	.FFT_AVERAGE_MAG = {0},
	.FFT_MINIMUM_MAG = {0},
	.NR_GAIN = {0},
	.LAST_IFFT_RESULT = {0},
};
IRAM2 static NR_Instance NR_RX2 = {
	.NR_Buffer = {0},
	.NR_Prev_Buffer = {0},
	.FFT_Buffer = {0},
	.FFT_Inst = &arm_cfft_sR_f32_len64,
	.FFT_COMPLEX_MAG = {0},
	.FFT_AVERAGE_MAG = {0},
	.FFT_MINIMUM_MAG = {0},
	.NR_GAIN = {0},
	.LAST_IFFT_RESULT = {0},
};

static float32_t von_Hann[NOISE_REDUCTION_FFT_SIZE] = {0};
static const float32_t decimationFirCoeff[4] = (const float32_t[]){475.1179397144384210E-6f, 0.503905202786044337f, 0.503905202786044337f, 475.1179397144384210E-6f};

void InitNoiseReduction(void)
{
	for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
		von_Hann[idx] = sqrtf(0.5f * (1.0f - arm_cos_f32((2.0f * PI * idx) / (float32_t)NOISE_REDUCTION_FFT_SIZE)));
	
	//decimation by 4
	arm_fir_decimate_init_f32(&NR_RX1.DECIMATE_FIR, 4, 4, decimationFirCoeff, NR_RX1.DECIMATE_FIR_State, NOISE_REDUCTION_BLOCK_SIZE);
	arm_fir_decimate_init_f32(&NR_RX2.DECIMATE_FIR, 4, 4, decimationFirCoeff, NR_RX2.DECIMATE_FIR_State, NOISE_REDUCTION_BLOCK_SIZE);
}

void processNoiseReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id)
{
	NR_Instance* instance = &NR_RX1;
	if(rx_id==AUDIO_RX2)
		instance = &NR_RX2;
	//overlap is 50%
	for(uint8_t loop = 0; loop < 2 ; loop++)
	{
		//fill buffer
		memcpy(&instance->NR_Buffer[0], &instance->NR_Prev_Buffer[0], NOISE_REDUCTION_FFT_SIZE_HALF * 4);
		memcpy(&instance->NR_Prev_Buffer[0], &buffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF], NOISE_REDUCTION_FFT_SIZE_HALF * 4);
		memcpy(&instance->NR_Buffer[NOISE_REDUCTION_FFT_SIZE_HALF], &buffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF], NOISE_REDUCTION_FFT_SIZE_HALF * 4);
		//prepare fft
		for(uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
		{
				instance->FFT_Buffer[idx * 2] = instance->NR_Buffer[idx]; // real
				instance->FFT_Buffer[idx * 2 + 1] = 0.0; // imaginary
		}
		//windowing
		for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
				instance->FFT_Buffer[idx * 2] *= von_Hann[idx];
		//do fft
		arm_cfft_f32(instance->FFT_Inst, instance->FFT_Buffer, 0, 1);
		//get magnitude
		arm_cmplx_mag_f32(instance->FFT_Buffer, instance->FFT_COMPLEX_MAG, NOISE_REDUCTION_FFT_SIZE_HALF);
		//average magnitude
		for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF ; idx++)
			instance->FFT_AVERAGE_MAG[idx] = (instance->FFT_AVERAGE_MAG[idx] + instance->FFT_COMPLEX_MAG[idx]) / NOISE_REDUCTION_AVERAGE;
		//minimum magnitude
		for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF ; idx++)
			if(instance->FFT_MINIMUM_MAG[idx] > instance->FFT_COMPLEX_MAG[idx])
				instance->FFT_MINIMUM_MAG[idx] = instance->FFT_COMPLEX_MAG[idx];
			else
				instance->FFT_MINIMUM_MAG[idx] += instance->FFT_COMPLEX_MAG[idx] / NOISE_REDUCTION_MINIMAL_MAG_LAST;
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
			if(idx==0)
				instance->NR_GAIN[idx] = (1 - 2 * NOISE_REDUCTION_BETA) * instance->NR_GAIN[idx] + NOISE_REDUCTION_BETA * instance->NR_GAIN[idx + 1];
			else if(idx == (NOISE_REDUCTION_BLOCK_SIZE - 1))
				instance->NR_GAIN[idx] = NOISE_REDUCTION_BETA * instance->NR_GAIN[idx - 1] + (1 - 2 * NOISE_REDUCTION_BETA) * instance->NR_GAIN[idx];
			else
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
				buffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF + idx] = instance->FFT_Buffer[idx * 2] + instance->LAST_IFFT_RESULT[idx];
		//save 2nd half of ifft result
		for(uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				instance->LAST_IFFT_RESULT[idx] = instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE_HALF * 2 + idx * 2];
	}
}
