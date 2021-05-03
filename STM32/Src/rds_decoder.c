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
//decimator
static const float32_t DECIMATE_FIR_Coeffs[4] = {-0.05698952454792, 0.5574889164132, 0.5574889164132, -0.05698952454792};
arm_fir_decimate_instance_f32 DECIMATE_FIR_I =
	{
		.M = RDS_DECIMATOR,
		.numTaps = 4,
		.pCoeffs = DECIMATE_FIR_Coeffs,
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}
	};
arm_fir_decimate_instance_f32 DECIMATE_FIR_Q =
	{
		.M = RDS_DECIMATOR,
		.numTaps = 4,
		.pCoeffs = DECIMATE_FIR_Coeffs,
		.pState = (float32_t[FPGA_RX_IQ_BUFFER_HALF_SIZE + 4 - 1]){0}
	};
	
static float32_t RDS_buff_I[DECODER_PACKET_SIZE] = {0};
static float32_t RDS_buff_Q[DECODER_PACKET_SIZE] = {0};
static float32_t RDS_gen_step = 0.0f;
static uint32_t RDS_decoder_samplerate = 0;

static void testFFT(float32_t *bufferIn);
static uint16_t BuildSyndrome(uint32_t raw);
static uint32_t ApplyFEC(uint32_t *block, uint32_t _syndrome);

void RDSDecoder_Init(void)
{
	RDS_decoder_samplerate = TRX_GetRXSampleRate;
	//no rds in signal
	if(RDS_decoder_samplerate < 192000)
		return;
	
	//RDS signal filter
	iir_filter_t *filter = biquad_create(RDS_FILTER_STAGES);
	biquad_init_bandpass(filter, RDS_decoder_samplerate, RDS_FREQ - RDS_FILTER_WIDTH, RDS_FREQ + RDS_FILTER_WIDTH);
	fill_biquad_coeffs(filter, RDS_Signal_Filter_Coeffs, RDS_FILTER_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RDS_Signal_Filter, RDS_FILTER_STAGES, RDS_Signal_Filter_Coeffs, RDS_Signal_Filter_State);
	
	//RDS LPF Filter
	filter = biquad_create(RDS_FILTER_STAGES);
	biquad_init_lowpass(filter, RDS_decoder_samplerate, RDS_FILTER_WIDTH);
	fill_biquad_coeffs(filter, RDS_LPF_Filter_Coeffs, RDS_FILTER_STAGES);
	arm_biquad_cascade_df2T_init_f32(&RDS_LPF_I_Filter, RDS_FILTER_STAGES, RDS_LPF_Filter_Coeffs, RDS_LPF_Filter_I_State);
	arm_biquad_cascade_df2T_init_f32(&RDS_LPF_Q_Filter, RDS_FILTER_STAGES, RDS_LPF_Filter_Coeffs, RDS_LPF_Filter_Q_State);
	
	//RDS NCO
	RDS_gen_step = ((float32_t)RDS_FREQ / (float32_t)RDS_decoder_samplerate);
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
		RDS_gen_index += RDS_gen_step;
		while (RDS_gen_index >= 1.0f)
			RDS_gen_index -= 1.0f;
	
		RDS_buff_I[i] = bufferIn[i] * sin;
		RDS_buff_Q[i] = bufferIn[i] * cos;
	}
	//filter mirror
	arm_biquad_cascade_df2T_f32_rolled(&RDS_LPF_I_Filter, RDS_buff_I, RDS_buff_I, DECODER_PACKET_SIZE);
	arm_biquad_cascade_df2T_f32_rolled(&RDS_LPF_Q_Filter, RDS_buff_Q, RDS_buff_Q, DECODER_PACKET_SIZE);
	//decimate
	arm_fir_decimate_f32(&DECIMATE_FIR_I, RDS_buff_I, RDS_buff_I, DECODER_PACKET_SIZE);
	arm_fir_decimate_f32(&DECIMATE_FIR_Q, RDS_buff_Q, RDS_buff_Q, DECODER_PACKET_SIZE);
	//test
	testFFT(bufferIn);
	//get bits data
	static uint32_t block1 = 0;
	static uint32_t block2 = 0;
	static uint32_t block3 = 0;
	static uint32_t block4 = 0;
	static bool angle_state_prev = false;
	static bool prev_bit = false;
	static uint8_t bit_sample_counter = 0;
	for(uint32_t i = 0 ; i < (DECODER_PACKET_SIZE / RDS_DECIMATOR) ; i++)
	{
		float32_t angle = atan2f(RDS_buff_Q[i], RDS_buff_I[i]);
		if(angle > -0.8f && angle < 0.8f) //phase gone away, do some pll!
		{
			//RDS_gen_step += angle * 0.000001f;
			//println(RDS_gen_step, " ", angle);
			continue;
		}
		bool angle_state = (angle > 0.0f) ? true : false;
		if((!angle_state_prev && angle_state) || (angle_state_prev && !angle_state))
		{
			if(bit_sample_counter >= 4) //anti-noise
			{
				//shift data
				block4 <<= 1;
				block4 |= (block3 >> 25) & 0x1;
				block3 <<= 1;
				block3 |= (block2 >> 25) & 0x1;
				block2 <<= 1;
				block2 |= (block1 >> 25) & 0x1;
				block1 <<= 1;
				//do diff
				if(angle_state_prev && !prev_bit)
					block1 |= 1;
				if(!angle_state_prev && prev_bit)
					block1 |= 1;
				angle_state_prev = prev_bit;
				//wait block A
				#define MaxCorrectableBits 5
				bool gotA = false;
				uint16_t _syndrome = BuildSyndrome(block1);
				_syndrome ^= 0x3d8;
				gotA = _syndrome == 0 ? true : false;
				if(gotA)
				{
					print("A");
					
					//wait block B
					bool gotB = false;
					_syndrome = BuildSyndrome(block2);
					_syndrome ^= 0x3d4;
					gotB = _syndrome == 0 ? true : false;
					if(!gotB && ApplyFEC(&block2, _syndrome) <= MaxCorrectableBits)
						gotB = true;
					if(gotB)
					{
						print("B");
						
						//wait block C
						bool gotC = false;
						_syndrome = BuildSyndrome(block3);
						_syndrome ^= (uint16_t)((block2 & 0x800) == 0 ? 0x25c : 0x3cc);
						gotC = _syndrome == 0 ? true : false;
						if(!gotC && ApplyFEC(&block3, _syndrome) <= MaxCorrectableBits)
							gotC = true;
						if(gotC)
						{
							print("C");
							
							//wait block D
							bool gotD = false;
							_syndrome = BuildSyndrome(block4);
							_syndrome ^= 0x258;
							gotD = _syndrome == 0 ? true : false;
							if(!gotD && ApplyFEC(&block4, _syndrome) <= MaxCorrectableBits)
								gotD = true;
							if(gotD)
							{
								print("D");
							}
						}
					}
				}
				
				/*print_bin26(block1, true);
				print(" ");
				print_bin26(block2, true);
				print(" ");
				print_bin26(block3, true);
				print(" ");
				print_bin26(block4, false);*/
				//
				bit_sample_counter = 1;
			}
		}
		else
		{
			bit_sample_counter++;
		}
		angle_state_prev = angle_state;
		//println(angle);
	}
	
}

static uint32_t ApplyFEC(uint32_t *block, uint32_t _syndrome)
{
		const uint16_t poly = 0x5b9;
		const int errorMask = (1 << 5);

		uint32_t correction = (uint32_t)(1 << 25);
		uint8_t correctedBitsCount = 0;
		
		for (uint8_t i = 0; i < 16; i++)
		{
				uint32_t st = ((_syndrome & 0x200) == 0x200);
				uint32_t bitError = (_syndrome & errorMask) == 0;
				*block ^= (st && bitError) ? correction : 0;
				_syndrome <<= 1;
				_syndrome ^= ((st && !bitError) ? poly : (uint16_t)0);
				correctedBitsCount += (st && bitError) ? 1 : 0;
				correction >>= 1;
		}
		_syndrome &= 0x3ff;
		return correctedBitsCount;
}

static uint16_t BuildSyndrome(uint32_t raw)
{
		uint16_t Parity[] = 
		{
				0x2DC,
				0x16E,
				0x0B7,
				0x287,
				0x39F,
				0x313,
				0x355,
				0x376,
				0x1BB,
				0x201,
				0x3DC,
				0x1EE,
				0x0F7,
				0x2A7,
				0x38F,
				0x31B
		};

		uint32_t block = raw & 0x3ffffff;
		uint16_t syndrome = (uint16_t)(block >> 16);
		for (uint8_t i = 0; i < 16; i++)
		{
				syndrome ^= ((block & 0x8000) == 0x8000) ? Parity[i] : (uint16_t) 0;
				block <<= 1;
		}

		return syndrome;
}

static void testFFT(float32_t *bufferIn)
{
	static float32_t *FFTInput_I_current = (float32_t *)&FFTInput_I_A[0];
	static float32_t *FFTInput_Q_current = (float32_t *)&FFTInput_Q_A[0];
	
	for(uint32_t i = 0 ; i < (DECODER_PACKET_SIZE / RDS_DECIMATOR) ; i++)
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
