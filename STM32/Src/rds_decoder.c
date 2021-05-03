#include "rds_decoder.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "arm_math.h"
#include "settings.h"
#include "functions.h"
#include "lcd.h"
#include "fpga.h"
#include "audio_filters.h"
#include "arm_const_structs.h"
#include "decoder.h"

char RDS_Decoder_Text[RDS_DECODER_STRLEN + 1] = {0}; // decoded string

//signal
static float32_t RDS_Signal_Filter_Coeffs[BIQUAD_COEFF_IN_STAGE * RDS_FILTER_STAGES] = {0};
static float32_t RDS_Signal_Filter_State[2 * RDS_FILTER_STAGES] = {0};
static arm_biquad_cascade_df2T_instance_f32 RDS_Signal_Filter;
//lpf
static float32_t RDS_LPF_Filter_Coeffs[BIQUAD_COEFF_IN_STAGE * RDS_FILTER_STAGES] = {0};
static float32_t RDS_LPF_Filter_I_State[2 * RDS_FILTER_STAGES] = {0};
static float32_t RDS_LPF_Filter_Q_State[2 * RDS_FILTER_STAGES] = {0};
static arm_biquad_cascade_df2T_instance_f32 RDS_LPF_I_Filter;
static arm_biquad_cascade_df2T_instance_f32 RDS_LPF_Q_Filter;

static float32_t RDS_buff_I[DECODER_PACKET_SIZE] = {0};
static float32_t RDS_buff_Q[DECODER_PACKET_SIZE] = {0};

static void GoertzelAccum(float32_t *bufferIn, uint16_t size, float32_t freq, float32_t samplerate);
static float32_t GoertzelGetPwr(float32_t freq, float32_t samplerate);
static void testFFT(float32_t *bufferIn);

static uint32_t RDS_decoder_samplerate = 0;
void RDSDecoder_Init(void)
{
	RDS_decoder_samplerate = TRX_GetRXSampleRate;
	//no rds in signal
	if(RDS_decoder_samplerate < 192000)
		return;
	
	//RDS signal filter
	iir_filter_t *filter = biquad_create(RDS_FILTER_STAGES);
	biquad_init_bandpass(filter, RDS_decoder_samplerate, RDS_FREQ - 2000, RDS_FREQ + 2000);
	fill_biquad_coeffs(filter, RDS_Signal_Filter_Coeffs, RDS_FILTER_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RDS_Signal_Filter, RDS_FILTER_STAGES, RDS_Signal_Filter_Coeffs, RDS_Signal_Filter_State);
	
	//RDS LPF Filter
	filter = biquad_create(RDS_FILTER_STAGES);
	biquad_init_lowpass(filter, RDS_decoder_samplerate, 2240);
	fill_biquad_coeffs(filter, RDS_LPF_Filter_Coeffs, RDS_FILTER_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RDS_LPF_I_Filter, RDS_FILTER_STAGES, RDS_LPF_Filter_Coeffs, RDS_LPF_Filter_I_State);
	arm_biquad_cascade_df2T_init_f32(&RDS_LPF_Q_Filter, RDS_FILTER_STAGES, RDS_LPF_Filter_Coeffs, RDS_LPF_Filter_Q_State);
}

void RDSDecoder_Process(float32_t *bufferIn)
{
	//reinit?
	if(RDS_decoder_samplerate != TRX_GetRXSampleRate)
		RDSDecoder_Init();
	//no rds in signal
	if(RDS_decoder_samplerate < 192000)
		return;
	//filter RDS signal
	arm_biquad_cascade_df2T_f32_rolled(&RDS_Signal_Filter, bufferIn, bufferIn, DECODER_PACKET_SIZE);
	//move signal to low freq
	static float32_t RDS_gen_index = 0;
	for (uint_fast16_t i = 0; i < DECODER_PACKET_SIZE; i++)
	{
		float32_t sin = arm_sin_f32(RDS_gen_index * (2.0f * F_PI));
		float32_t cos = arm_cos_f32(RDS_gen_index * (2.0f * F_PI));
		RDS_gen_index += ((float32_t)RDS_FREQ / (float32_t)RDS_decoder_samplerate);
		while (RDS_gen_index >= 1.0f)
			RDS_gen_index -= 1.0f;
	
		RDS_buff_I[i] = bufferIn[i] * sin;
		RDS_buff_Q[i] = bufferIn[i] * cos;
	}
	//filter mirror
	arm_biquad_cascade_df2T_f32_rolled(&RDS_LPF_I_Filter, RDS_buff_I, RDS_buff_I, DECODER_PACKET_SIZE);
	arm_biquad_cascade_df2T_f32_rolled(&RDS_LPF_Q_Filter, RDS_buff_Q, RDS_buff_Q, DECODER_PACKET_SIZE);
	//decimate
	arm_fir_decimate_f32(&DECIMATE_FIR_RX1_AUDIO_I, in_i, out_i, size);
	arm_fir_decimate_f32(&DECIMATE_FIR_RX1_AUDIO_Q, in_q, out_q, size);
	//test
	testFFT(bufferIn);
	
	/*static uint32_t rds_samples = 0;
	GoertzelAccum(bufferIn, DECODER_PACKET_SIZE, RDS_LOW_FREQ, RDS_decoder_samplerate);
	rds_samples += DECODER_PACKET_SIZE;
	if(rds_samples >= RDS_decoder_samplerate / 1187.5f) //1187.5hz rds speed
	{
		rds_samples = 0;
		FPGA_samples++;
		float32_t rds_power = GoertzelGetPwr(RDS_LOW_FREQ, RDS_decoder_samplerate);
		//println(rds_power);
		static float32_t min_pwr = 1.0f;
		static float32_t max_pwr = 0.0f;
		if(fabsf(max_pwr - min_pwr) > 0.0001f)
		{
			max_pwr *= 0.999f;
			min_pwr *= 1.001f;
		}
		if(rds_power > max_pwr)
			max_pwr = rds_power;
		if(rds_power < min_pwr && rds_power > 0.00001f)
			min_pwr = rds_power;
		float32_t avg_pwr = (min_pwr + max_pwr) / 2.0f;
		
		//println("PWR: ", rds_power, " MIN: ", min_pwr, " MAX: ", max_pwr, " AVG: ", avg_pwr);
		if(rds_power > avg_pwr)
			print("1");
		if(rds_power < avg_pwr)
			print("0");
	}*/
}

static void testFFT(float32_t *bufferIn)
{
	static float32_t *FFTInput_I_current = (float32_t *)&FFTInput_I_A[0];
	static float32_t *FFTInput_Q_current = (float32_t *)&FFTInput_Q_A[0];
	
	for(uint32_t i = 0 ; i < DECODER_PACKET_SIZE ; i++)
	{
		FFTInput_I_current[FFT_buff_index] = RDS_buff_I[i];
		FFTInput_Q_current[FFT_buff_index] = RDS_buff_Q[i];
		
		FFT_buff_index++;
		if (FFT_buff_index == FFT_HALF_SIZE)
		{
			FFT_buff_index = 0;
			if (FFT_new_buffer_ready)
			{
				//println("fft overrun");
			}
			else
			{
				FFT_new_buffer_ready = true;
				FFT_buff_current = !FFT_buff_current;	
			}
			if (TRX_RX1_IQ_swap)
			{
				if(FFT_buff_current)
				{
					FFTInput_I_current = (float32_t *)&FFTInput_Q_A[0];
					FFTInput_Q_current = (float32_t *)&FFTInput_I_A[0];
				}
				else
				{
					FFTInput_I_current = (float32_t *)&FFTInput_Q_B[0];
					FFTInput_Q_current = (float32_t *)&FFTInput_I_B[0];
				}
			}
			else
			{
				if(FFT_buff_current)
				{
					FFTInput_I_current = (float32_t *)&FFTInput_I_A[0];
					FFTInput_Q_current = (float32_t *)&FFTInput_Q_A[0];
				}
				else
				{
					FFTInput_I_current = (float32_t *)&FFTInput_I_B[0];
					FFTInput_Q_current = (float32_t *)&FFTInput_Q_B[0];
				}
			}
		}
	}
}

static float32_t goertzel_n1;
static float32_t goertzel_n2;
static void GoertzelAccum(float32_t *bufferIn, uint16_t size, float32_t freq, float32_t samplerate)
{
  float32_t *x;
  float32_t  v0;
  uint32_t  i;

// 1. Compute for each sample: 
// vk(n) = (2*cos(2*PI*f0/fs)) * vk(n-1) - vk(n-2) + x(n)
  x = bufferIn;
  for (i = 0; i < size; i++) {
    v0 = (2.0f * cosf(2.0f * F_PI * freq / samplerate)) * goertzel_n1 - goertzel_n2 + *x;
    x++;
    goertzel_n2 = goertzel_n1;
    goertzel_n1 = v0;
  }
}

static float32_t GoertzelGetPwr(float32_t freq, float32_t samplerate)
{
  float32_t  pwr;
  float32_t p1, p2, p01;

// 2. Compute once every N samples:
// |X(k)|2 = vk(N)2 + vk(N-1)2 - (2*cos(2*PI*f0/fs)) * vk(N) * vk(N-1))
  p1  = goertzel_n1 * goertzel_n1;
  p2  = goertzel_n2 * goertzel_n2;
  p01 = goertzel_n1 * goertzel_n2;
  pwr = p1 + p2 - (2.0f * cosf(2.0f * F_PI * freq / samplerate)) * p01;
  pwr = sqrtf(pwr);
	
	// reset accum
	goertzel_n1  = 0;
  goertzel_n2  = 0;
	
  return pwr;
}
