#include "rtty_decoder.h"
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

char RTTY_Decoder_Text[RTTY_DECODER_STRLEN + 1] = {0}; // decoded string

void RTTYDecoder_Init(void)
{
	sprintf(RTTY_Decoder_Text, " RTTY: -");
	addSymbols(RTTY_Decoder_Text, RTTY_Decoder_Text, RTTY_DECODER_STRLEN, " ", true);
	LCD_UpdateQuery.TextBar = true;
}

void RTTYDecoder_Process(float32_t *bufferIn)
{
	
}

/*
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
*/
