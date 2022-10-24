#include "noise_reduction.h"
#include "arm_const_structs.h"
#include "settings.h"
#include "audio_processor.h"
#include "audio_filters.h"
#include "trx_manager.h"
#include "vad.h"

// useful info https://github.com/df8oe/UHSDR/wiki/Noise-reduction

// Private variables
SRAM_ON_H743 static NR_Instance NR_RX1 = {0};
#if HRDW_HAS_DUAL_RX
SRAM static NR_Instance NR_RX2 = {0};
#endif

static float32_t NR_von_Hann[NOISE_REDUCTION_FFT_SIZE] = {0}; // coefficients for the window function

// initialize DNR
void InitNoiseReduction(void)
{
	dma_memset(&NR_RX1, 0, sizeof(NR_RX1));
	NR_RX1.FFT_Inst = NOISE_REDUCTION_FFT_INSTANCE;
	
	#if HRDW_HAS_DUAL_RX
	dma_memset(&NR_RX2, 0, sizeof(NR_RX2));
	NR_RX2.FFT_Inst = NOISE_REDUCTION_FFT_INSTANCE;
	#endif
	
	for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++) {
		arm_sqrt_f32(0.5f * (1.0f - arm_cos_f32((2.0f * F_PI * idx) / (float32_t)NOISE_REDUCTION_FFT_SIZE)), &NR_von_Hann[idx]);
	}

	for (uint16_t bindx = 0; bindx < NOISE_REDUCTION_FFT_SIZE_HALF; bindx++)
	{
		NR_RX1.Prev_Buffer[bindx] = 0.0;
		NR_RX1.GAIN[bindx] = 1.0;
		NR_RX1.GAIN_old[bindx] = 1.0;

		#if HRDW_HAS_DUAL_RX
		NR_RX2.Prev_Buffer[bindx] = 0.0;
		NR_RX2.GAIN[bindx] = 1.0;
		NR_RX2.GAIN_old[bindx] = 1.0;
		#endif
	}
}

// run DNR for the data block
void processNoiseReduction(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id, uint8_t nr_type, uint_fast8_t mode, bool do_agc)
{
	NR_Instance *instance = &NR_RX1;
	#if HRDW_HAS_DUAL_RX
	if (rx_id == AUDIO_RX2)
		instance = &NR_RX2;
	#endif

#define snr_prio_min 0.001f // range should be down to -30dB min
#define alpha 0.94f

	// fill input buffer
	if (instance->InputBuffer_index >= (NOISE_REDUCTION_FFT_SIZE / NOISE_REDUCTION_BLOCK_SIZE))
		instance->InputBuffer_index = 0;
	dma_memcpy(&instance->InputBuffer[instance->InputBuffer_index * NOISE_REDUCTION_BLOCK_SIZE], buffer, NOISE_REDUCTION_BLOCK_SIZE * 4);
	instance->InputBuffer_index++;

	if (instance->InputBuffer_index == (NOISE_REDUCTION_FFT_SIZE / NOISE_REDUCTION_BLOCK_SIZE)) // input buffer ready
	{
		instance->InputBuffer_index = 0;
		instance->OutputBuffer_index = 0;

		// overlap is 50%
		for (uint8_t loop = 0; loop < 2; loop++)
		{
			// prepare fft / fill buffers
			// first half - prev data
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
				instance->FFT_Buffer[idx * 2] = instance->Prev_Buffer[idx]; // real
				instance->FFT_Buffer[idx * 2 + 1] = 0.0f;					   // imaginary
			}
			// save last half to prev data
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
				instance->Prev_Buffer[idx] = instance->InputBuffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF + idx];
			}
			// last half - last data
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
			{
				instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE + idx * 2] = instance->InputBuffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF + idx]; // real
				instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE + idx * 2 + 1] = 0.0f;															 // imaginary
			}
			// windowing
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
				instance->FFT_Buffer[idx * 2] *= NR_von_Hann[idx];
			// do fft
			arm_cfft_f32(instance->FFT_Inst, instance->FFT_Buffer, 0, 1);
			// get magnitude
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++) {
				arm_sqrt_f32(instance->FFT_Buffer[idx * 2] * instance->FFT_Buffer[idx * 2] + instance->FFT_Buffer[idx * 2 + 1] * instance->FFT_Buffer[idx * 2 + 1], &instance->FFT_COMPLEX_MAG[idx]);
			}
			// average magnitude
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				instance->FFT_AVERAGE_MAG[idx] = instance->FFT_COMPLEX_MAG[idx] * (1.0f - (float32_t)TRX.DNR_AVERAGE / 500.0f) + instance->FFT_AVERAGE_MAG[idx] * ((float32_t)TRX.DNR_AVERAGE / 500.0f);
			// minimum magnitude
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				if (instance->FFT_MINIMUM_MAG[idx] > instance->FFT_COMPLEX_MAG[idx])
					instance->FFT_MINIMUM_MAG[idx] = instance->FFT_COMPLEX_MAG[idx];
				else
					instance->FFT_MINIMUM_MAG[idx] = instance->FFT_COMPLEX_MAG[idx] * (1.0f - (float32_t)TRX.DNR_MINIMAL / 100.0f) + instance->FFT_MINIMUM_MAG[idx] * ((float32_t)TRX.DNR_MINIMAL / 100.0f);

			if (nr_type == 1)
			{
				// calculate signal-noise-ratio
				float32_t threshold = ((float32_t)TRX.DNR1_SNR_THRESHOLD + 10.0f) / 10.0f;
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
					// gain calc
					float32_t gain = 0.0f;
					if (instance->FFT_COMPLEX_MAG[idx] > 0.0f)
						gain = 1.0f - (lambda / instance->FFT_COMPLEX_MAG[idx]);
					// delete noise
					if (snr < threshold)
						gain = 0.0f;
					// else gain = 1.0f;
					// time smoothing (exponential averaging) of gain weights
					instance->GAIN[idx] = NOISE_REDUCTION_ALPHA * instance->GAIN[idx] + (1.0f - NOISE_REDUCTION_ALPHA) * gain;
					// frequency smoothing of gain weights
					if (idx > 0 && (idx < NOISE_REDUCTION_FFT_SIZE_HALF - 1))
						instance->GAIN[idx] = NOISE_REDUCTION_BETA * instance->GAIN[idx - 1] + (1.0f - 2 * NOISE_REDUCTION_BETA) * instance->GAIN[idx] + NOISE_REDUCTION_BETA * instance->GAIN[idx + 1];
				}
			}

			if (nr_type == 2)
			{
				// new noise estimate MMSE based
				static float32_t xt_coeff = 0.5f;
				for (int bindx = 0; bindx < NOISE_REDUCTION_FFT_SIZE_HALF; bindx++) // 1. Step of NR - calculate the SNR's
				{
					instance->SNR_post[bindx] = fmax(fmin(instance->FFT_COMPLEX_MAG[bindx] / xt_coeff, 1000.0), snr_prio_min); // limited to +30 /-15 dB, might be still too much of reduction, let's try it?
					instance->SNR_prio[bindx] = fmax(alpha * instance->GAIN_old[bindx] + (1.0 - alpha) * fmax(instance->SNR_post[bindx] - 1.0, 0.0), 0.0);
				}

				// some automatic
				if (instance->SNR_post[10] > (((float32_t)TRX.DNR2_SNR_THRESHOLD / 100.0f) * 10.0f) && xt_coeff < 50.0f)
					xt_coeff += 0.01f;
				else if (instance->SNR_post[10] > ((float32_t)TRX.DNR2_SNR_THRESHOLD / 100.0f) && xt_coeff < 50.0f)
					xt_coeff += 0.001f;

				if (instance->SNR_post[10] < (((float32_t)TRX.DNR2_SNR_THRESHOLD / 100.0f) * 0.1f) && xt_coeff > 0.1f)
					xt_coeff -= 0.01f;
				else if (instance->SNR_post[10] < ((float32_t)TRX.DNR2_SNR_THRESHOLD / 100.0f) && xt_coeff > 0.1f)
					xt_coeff -= 0.001f;
				// println(instance->FFT_COMPLEX_MAG[10], " ",instance->SNR_post[10], " ", xt_coeff);

				// calculate v = SNRprio(n, bin[i]) / (SNRprio(n, bin[i]) + 1) * SNRpost(n, bin[i]) (eq. 12 of Schmitt et al. 2002, eq. 9 of Romanin et al. 2009)  and calculate the HK's
				for (int bindx = 0; bindx < NOISE_REDUCTION_FFT_SIZE_HALF; bindx++)
				{
					float32_t prio = instance->SNR_prio[bindx];
					float32_t post = instance->SNR_post[bindx];
					float32_t v = prio / (prio + 1.0f) * post;
					#ifdef STM32F407xx
					v = fast_sqrt(0.7212f * v + v * v);
					#else
					arm_sqrt_f32((0.7212f * v + v * v), &v);
					#endif
					instance->GAIN[bindx] = fmax(1.0f / post * v, 0.001f); // limit HK's to 0.001
					instance->GAIN_old[bindx] = post * instance->GAIN[bindx] * instance->GAIN[bindx];
				}
			}
			
			if(do_agc) //do spectral AGC
			{
				float32_t RX_AGC_STEPSIZE_UP = 0.0f;
				float32_t RX_AGC_STEPSIZE_DOWN = 0.0f;
				if (mode == TRX_MODE_CW)
				{
					RX_AGC_STEPSIZE_UP = 200.0f / (float32_t)TRX.RX_AGC_CW_speed;
					RX_AGC_STEPSIZE_DOWN = 20.0f / (float32_t)TRX.RX_AGC_CW_speed;
				}
				else
				{
					RX_AGC_STEPSIZE_UP = 1000.0f / (float32_t)TRX.RX_AGC_SSB_speed;
					RX_AGC_STEPSIZE_DOWN = 20.0f / (float32_t)TRX.RX_AGC_SSB_speed;
				}
				
				float32_t AGC_RX_magnitude = 0;
				arm_rms_f32(instance->InputBuffer, NOISE_REDUCTION_FFT_SIZE_HALF, &AGC_RX_magnitude);
				
				if (AGC_RX_magnitude == 0.0f)
					AGC_RX_magnitude = 0.001f;
				
				float32_t full_scale_rate = AGC_RX_magnitude / FLOAT_FULL_SCALE_POW;
				float32_t AGC_RX_dbFS = rate2dbV(full_scale_rate);
				if (nr_type != 0)
					AGC_RX_dbFS -= 12.0f; //DNR compensation
				
				float32_t gain_target = (float32_t)TRX.AGC_GAIN_TARGET;
				if (mode == TRX_MODE_CW)
					gain_target += CW_ADD_GAIN_AF;
				float32_t diff = (gain_target - (AGC_RX_dbFS + instance->need_gain_db));

				// hold time limiter
				#define AGC_HOLDTIME_LIMITER_DB 10.0f
				#define AGC_HOLDTIME_STEP 5
				if (fabsf(diff) < AGC_HOLDTIME_LIMITER_DB && instance->hold_time < TRX.RX_AGC_Hold)
				{
					instance->hold_time += AGC_HOLDTIME_STEP;
				}
				
				// move
				if (diff > 0)
				{
					if ((HAL_GetTick() - instance->last_agc_peak_time) > instance->hold_time)
					{
						instance->need_gain_db += diff / RX_AGC_STEPSIZE_UP;

						if (diff > AGC_HOLDTIME_LIMITER_DB && instance->hold_time > 0)
							instance->hold_time -= AGC_HOLDTIME_STEP;
					}
				}
				else
				{
					instance->need_gain_db += diff / RX_AGC_STEPSIZE_DOWN;
					instance->last_agc_peak_time = HAL_GetTick();
				}
				
				// gain limiter
				if (instance->need_gain_db > (float32_t)TRX.RX_AGC_Max_gain)
					instance->need_gain_db = (float32_t)TRX.RX_AGC_Max_gain;
				if ((AGC_RX_dbFS + instance->need_gain_db) > (gain_target + AGC_CLIPPING))
				{
					println("AGC overload ", diff, " ", instance->need_gain_db - (gain_target - AGC_RX_dbFS));
					instance->need_gain_db = gain_target - AGC_RX_dbFS;
					//instance->need_gain_db -= 20.0f;
				}
								
				//appy gain
				float32_t rateV = db2rateV(instance->need_gain_db);
				
				// Muting if need
				bool VAD_Muting = VAD_RX1_Muting;
				#if HRDW_HAS_DUAL_RX
				if (rx_id == AUDIO_RX2)
					VAD_Muting = VAD_RX2_Muting;
				#endif
				if (CODEC_Muting || VAD_Muting)
				{
					rateV = db2rateV(-200.0f);
				}
								
				//println("[SpectraAGC] Min: ", minValue, " AGC_RX_dbFS: ", AGC_RX_dbFS, " Gain: ", instance->need_gain_db);
				
				// apply AGC
				for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
				{
					instance->FFT_Buffer[idx] *= rateV;
				}
			}
			
			if (nr_type != 0)
			{
				// smooth gain
				for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				{
					#define smooth_gain_alpha 0.9f
					#define smooth_gain_beta (1.0f - smooth_gain_alpha)
					if ((idx - 1) >= 0) {
						instance->GAIN[idx - 1] = instance->GAIN[idx - 1] * smooth_gain_alpha + instance->GAIN[idx] * smooth_gain_beta;
					}
					if ((idx + 1) < NOISE_REDUCTION_FFT_SIZE_HALF) {
						instance->GAIN[idx + 1] = instance->GAIN[idx + 1] * smooth_gain_alpha + instance->GAIN[idx] * smooth_gain_beta;
					}
				}
				
				// apply gain weighting
				for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++)
				{
					// NR
					instance->FFT_Buffer[idx * 2] *= instance->GAIN[idx];
					instance->FFT_Buffer[idx * 2 + 1] *= instance->GAIN[idx];
					// symmetry
					instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE * 2 - idx * 2 - 2] *= instance->GAIN[idx];
					instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE * 2 - idx * 2 - 1] *= instance->GAIN[idx];
				}
			}
			
			// do inverse fft
			arm_cfft_f32(instance->FFT_Inst, instance->FFT_Buffer, 1, 1);
			// windowing
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE; idx++)
			{
				instance->FFT_Buffer[idx * 2] *= NR_von_Hann[idx];
				instance->FFT_Buffer[idx * 2 + 1] *= NR_von_Hann[idx];
			}
			// return data (do overlap-add: take real part of first half of current iFFT result and add to 2nd half of last frames´ iFFT result)
			for (uint16_t idx = 0; idx < NOISE_REDUCTION_FFT_SIZE_HALF; idx++) {
				instance->OutputBuffer[loop * NOISE_REDUCTION_FFT_SIZE_HALF + idx] = instance->FFT_Buffer[idx * 2] + instance->LAST_IFFT_RESULT[idx];
				// save 2nd half of ifft result
				instance->LAST_IFFT_RESULT[idx] = instance->FFT_Buffer[NOISE_REDUCTION_FFT_SIZE_HALF * 2 + idx * 2];
			}
		}
	}
	if (instance->OutputBuffer_index < (NOISE_REDUCTION_FFT_SIZE / NOISE_REDUCTION_BLOCK_SIZE))
		dma_memcpy(buffer, &instance->OutputBuffer[instance->OutputBuffer_index * NOISE_REDUCTION_BLOCK_SIZE], NOISE_REDUCTION_BLOCK_SIZE * 4);
	instance->OutputBuffer_index++;
}
