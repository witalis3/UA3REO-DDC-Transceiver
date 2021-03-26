#include "noise_reduction.h"
#include "arm_const_structs.h"
#include "settings.h"

// useful info https://github.com/df8oe/UHSDR/wiki/Noise-reduction

//Private variables
SRAM static NR_Instance NR_RX1 = {0};
SRAM static NR_Instance NR_RX2 = {0};
static float32_t von_Hann[NOISE_REDUCTION_FFT_SIZE] = {0}; // coefficients for the window function

// initialize DNR
void InitNoiseReduction(void)
{
	dma_memset(&NR_RX1, 0, sizeof(NR_RX1));
	dma_memset(&NR_RX2, 0, sizeof(NR_RX1));
	NR_RX1.FFT_Inst = NOISE_REDUCTION_FFT_INSTANCE;
	NR_RX2.FFT_Inst = NOISE_REDUCTION_FFT_INSTANCE;

	for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
		von_Hann[idx] = sqrtf(0.5f * (1.0f - arm_cos_f32((2.0f * F_PI * idx) / (float32_t)NOISE_REDUCTION_FFT_SIZE)));

	for (uint16_t bindx = 0; bindx < NOISE_REDUCTION_FFT_SIZE_HALF; bindx++)
	{
		NR_RX1.NR_Prev_Buffer[bindx] = 0.0;
		NR_RX1.NR_GAIN[bindx] = 1.0;
		NR_RX1.NR_GAIN_old[bindx] = 1.0;

		NR_RX2.NR_Prev_Buffer[bindx] = 0.0;
		NR_RX2.NR_GAIN[bindx] = 1.0;
		NR_RX2.NR_GAIN_old[bindx] = 1.0;
	}
}

// run DNR for the data block
void processNoiseReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id, uint8_t nr_type)
{
	NR_Instance *instance = &NR_RX1;
	if (rx_id == AUDIO_RX2)
		instance = &NR_RX2;

#define snr_prio_min 0.001 //range should be down to -30dB min
#define alpha 0.94

	//fill input buffer
	if (instance->NR_InputBuffer_index >= (NOISE_REDUCTION_FFT_SIZE / NOISE_REDUCTION_BLOCK_SIZE))
		instance->NR_InputBuffer_index = 0;
	dma_memcpy(&instance->NR_InputBuffer[instance->NR_InputBuffer_index * NOISE_REDUCTION_BLOCK_SIZE], buffer, NOISE_REDUCTION_BLOCK_SIZE * 4);
	instance->NR_InputBuffer_index++;

	if (instance->NR_InputBuffer_index == (NOISE_REDUCTION_FFT_SIZE / NOISE_REDUCTION_BLOCK_SIZE)) //input buffer ready
	{
		instance->NR_InputBuffer_index = 0;
		instance->NR_OutputBuffer_index = 0;

		//overlap is 50%
		for (uint8_t loop = 0; loop < 2; loop++)
		{
			//prepare fft / fill buffers
			//first half - prev data
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
				instance->FFT_Buffer[idx * 2] = instance->NR_Prev_Buffer[idx]; // real
				instance->FFT_Buffer[idx * 2 + 1] = 0.0f;					   // imaginary
			}
			//save last half to prev data
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
				instance->NR_Prev_Buffer[idx] = instance->NR_InputBuffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF + idx];
			}
			//last half - last data
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
				instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE + idx * 2] = instance->NR_InputBuffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF + idx]; // real
				instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE + idx * 2 + 1] = 0.0f;															 // imaginary
			}
			//windowing
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
				instance->FFT_Buffer[idx * 2] *= von_Hann[idx];
			//do fft
			arm_cfft_f32(instance->FFT_Inst, instance->FFT_Buffer, 0, 1);
			//get magnitude
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				instance->FFT_COMPLEX_MAG[idx] = sqrtf(instance->FFT_Buffer[idx * 2] * instance->FFT_Buffer[idx * 2] + instance->FFT_Buffer[idx * 2 + 1] * instance->FFT_Buffer[idx * 2 + 1]);
			//average magnitude
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				instance->FFT_AVERAGE_MAG[idx] = instance->FFT_COMPLEX_MAG[idx] * (1.0f - (float32_t)TRX.DNR_AVERAGE / 500.0f) + instance->FFT_AVERAGE_MAG[idx] * ((float32_t)TRX.DNR_AVERAGE / 500.0f);
			//minimum magnitude
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				if (instance->FFT_MINIMUM_MAG[idx] > instance->FFT_COMPLEX_MAG[idx])
					instance->FFT_MINIMUM_MAG[idx] = instance->FFT_COMPLEX_MAG[idx];
				else
					instance->FFT_MINIMUM_MAG[idx] = instance->FFT_COMPLEX_MAG[idx] * (1.0f - (float32_t)TRX.DNR_MINIMAL / 100.0f) + instance->FFT_MINIMUM_MAG[idx] * ((float32_t)TRX.DNR_MINIMAL / 100.0f);

			if (nr_type == 1)
			{
				//calculate signal-noise-ratio
				float32_t threshold = ((float32_t)TRX.DNR_SNR_THRESHOLD + 10.0f) / 10.0f;
				for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				{
					float32_t snr = instance->FFT_COMPLEX_MAG[idx] / instance->FFT_MINIMUM_MAG[idx];
					float32_t lambda = 0.0f;
					if (snr > threshold)
					{
						lambda = instance->FFT_MINIMUM_MAG[idx];
					}
					else
					{
						lambda = instance->FFT_AVERAGE_MAG[idx];
					}
					//gain calc
					float32_t gain = 0.0f;
					if (instance->FFT_COMPLEX_MAG[idx] > 0.0f)
						gain = 1.0f - (lambda / instance->FFT_COMPLEX_MAG[idx]);
					//delete noise
					if (snr < threshold)
						gain = 0.0f;
					//else gain = 1.0f;
					//time smoothing (exponential averaging) of gain weights
					instance->NR_GAIN[idx] = NOISE_REDUCTION_ALPHA * instance->NR_GAIN[idx] + (1.0f - NOISE_REDUCTION_ALPHA) * gain;
					//frequency smoothing of gain weights
					if (idx > 0 && (idx < NOISE_REDUCTION_FFT_SIZE_HALF - 1))
						instance->NR_GAIN[idx] = NOISE_REDUCTION_BETA * instance->NR_GAIN[idx - 1] + (1.0f - 2 * NOISE_REDUCTION_BETA) * instance->NR_GAIN[idx] + NOISE_REDUCTION_BETA * instance->NR_GAIN[idx + 1];
				}
			}

			if (nr_type == 2)
			{
				//new noise estimate MMSE based
				for (int bindx = 0; bindx < NOISE_REDUCTION_FFT_SIZE_HALF; bindx++) // 1. Step of NR - calculate the SNR's
				{
					float32_t xt_coeff = 0.5;																				   //????
					instance->SNR_post[bindx] = fmax(fmin(instance->FFT_COMPLEX_MAG[bindx] / xt_coeff, 1000.0), snr_prio_min); // limited to +30 /-15 dB, might be still too much of reduction, let's try it?
					instance->SNR_prio[bindx] = fmax(alpha * instance->NR_GAIN_old[bindx] + (1.0 - alpha) * fmax(instance->SNR_post[bindx] - 1.0, 0.0), 0.0);
				}

				//calculate v = SNRprio(n, bin[i]) / (SNRprio(n, bin[i]) + 1) * SNRpost(n, bin[i]) (eq. 12 of Schmitt et al. 2002, eq. 9 of Romanin et al. 2009)  and calculate the HK's
				for (int bindx = 0; bindx < NOISE_REDUCTION_FFT_SIZE_HALF; bindx++)
				{
					float32_t v = instance->SNR_prio[bindx] / (instance->SNR_prio[bindx] + 1.0) * instance->SNR_post[bindx];
					instance->NR_GAIN[bindx] = fmax(1.0 / instance->SNR_post[bindx] * sqrtf((0.7212 * v + v * v)), 0.001); //limit HK's to 0.001
					instance->NR_GAIN_old[bindx] = instance->SNR_post[bindx] * instance->NR_GAIN[bindx] * instance->NR_GAIN[bindx];
				}
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
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				instance->LAST_IFFT_RESULT[idx] = instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE_HALF * 2 + idx * 2];
		}
	}
	if (instance->NR_OutputBuffer_index < (NOISE_REDUCTION_FFT_SIZE / NOISE_REDUCTION_BLOCK_SIZE))
		dma_memcpy(buffer, &instance->NR_OutputBuffer[instance->NR_OutputBuffer_index * NOISE_REDUCTION_BLOCK_SIZE], NOISE_REDUCTION_BLOCK_SIZE * 4);
	instance->NR_OutputBuffer_index++;
}
