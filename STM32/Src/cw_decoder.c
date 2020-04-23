#include "cw_decoder.h"
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

//Public variables
volatile uint16_t CW_Decoder_WPM = 0;						//декодированная скорость, WPM
char CW_Decoder_Text[CWDECODER_STRLEN + 1] = {0}; //декодированная строка

//Private variables
static bool realstate = false;
static bool realstatebefore = false;
static bool filteredstate = false;
static bool filteredstatebefore = false;
static bool stop = false;
static uint32_t laststarttime = 0;
static uint32_t starttimehigh = 0;
static uint32_t highduration = 0;
static uint32_t startttimelow = 0;
static uint32_t lowduration = 0;
static uint32_t hightimesavg = 0;
static int32_t signal_freq_index = -1;
static uint32_t signal_freq_index_lasttime = 0;
static char code[20] = {0};
static arm_rfft_fast_instance_f32 CWDECODER_FFT_Inst;
static float32_t FFTBuffer[CWDECODER_FFTSIZE] = {0}; //буфер FFT
static float32_t FFTBufferCharge[CWDECODER_FFTSIZE] = {0}; //накопительный буфер FFT
static float32_t window_multipliers[DECODER_PACKET_SIZE] = {0};

//Prototypes
static void CWDecoder_Decode(void);			//декодирование из морзе в символы
static void CWDecoder_PrintChar(char *str); //вывод символа в результирующую строку

//инициализация CW декодера
void CWDecoder_Init(void)
{
	arm_rfft_fast_init_f32(&CWDECODER_FFT_Inst, CWDECODER_FFTSIZE);
	//Окно Hann
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
		window_multipliers[i] = sqrtf(0.5f * (1.0f - arm_cos_f32((2.0f * PI * i) / (float32_t)CWDECODER_FFTSIZE)));
}

//запуск CW декодера для блока данных
void CWDecoder_Process(float32_t *bufferIn)
{
	//Смещаем старые данные в  буфере, чтобы собрать необходимый размер
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
	{
		if (i < (CWDECODER_FFTSIZE - DECODER_PACKET_SIZE))
			FFTBufferCharge[i] = FFTBufferCharge[(i + DECODER_PACKET_SIZE)];
		else //Добавляем новые данные в буфер FFT для расчёта
			FFTBufferCharge[i] = bufferIn[i - (CWDECODER_FFTSIZE - DECODER_PACKET_SIZE)];
	}
	
	//Окно для FFT
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
		FFTBuffer[i] = window_multipliers[i] * FFTBufferCharge[i];
	
	//Ищем и вычитаем DC составляющую сигнала
	/*float32_t dcValue = 0;
	arm_mean_f32(FFTBuffer, CWDECODER_FFTSIZE, &dcValue);
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
		FFTBuffer[i] = FFTBuffer[i] - dcValue;*/
	
	//Делаем FFT
	arm_rfft_fast_f32(&CWDECODER_FFT_Inst, FFTBuffer, FFTBuffer, 0);
	arm_abs_f32(FFTBuffer, FFTBuffer, CWDECODER_FFTSIZE);
	arm_mult_f32(FFTBuffer, FFTBuffer, FFTBuffer, CWDECODER_FFTSIZE);
	
	//Ищем максимум магнитуды для определения источника сигнала
	float32_t maxValue = 0;
	uint32_t maxIndex = 0;
	arm_max_f32(FFTBuffer, CWDECODER_SPEC_PART, &maxValue, &maxIndex);
	
	//Ищем среднее для определения шумового порога
	float32_t meanValue = 0;
	arm_mean_f32(FFTBuffer, CWDECODER_SPEC_PART, &meanValue);
	
	/*static uint32_t dbg_start = 0;
	if((HAL_GetTick() - dbg_start) > 1000)
	{
		for(uint16_t i=0;i<64;i+=2)
		{
			sendToDebug_uint16(i, true);
			sendToDebug_str(": ");
			sendToDebug_float32(FFTBuffer[i], false);
			//sendToDebug_flush();
		}
		sendToDebug_uint32(maxIndex, false);
		sendToDebug_newline();
		dbg_start = HAL_GetTick();
	}*/
	
	//sendToDebug_uint32(maxIndex, true); sendToDebug_str(" "); sendToDebug_float32(maxValue, true); sendToDebug_str(" "); sendToDebug_float32(meanValue, false);
	
	if(signal_freq_index == -1)
	{
		if(maxValue > meanValue * CWDECODER_NOISEGATE) //сигнал найден
		{
			/*sendToDebug_uint32(maxIndex, true); sendToDebug_str(" "); sendToDebug_float32(maxValue, true); sendToDebug_str(" "); sendToDebug_float32(meanValue, false); */
			signal_freq_index = maxIndex;
			sendToDebug_uint32(signal_freq_index, false);
		}
	}
	
	if(signal_freq_index != -1 && FFTBuffer[signal_freq_index] > (meanValue * CWDECODER_NOISEGATE)) //сигнал всё ещё активен
	{
		//sendToDebug_float32(FFTBuffer[signal_freq_index], true); sendToDebug_str(" "); sendToDebug_float32((meanValue * CWDECODER_NOISEGATE), false);
		sendToDebug_str("s");
		realstate = true;
		signal_freq_index_lasttime = HAL_GetTick();
	}
	else
	{
		realstate = false;
	}
	
	if(signal_freq_index != -1 && (HAL_GetTick() - signal_freq_index_lasttime) > CWDECODER_AFC_LATENCY) //сигнал потерян, ищем новый
	{
		realstate = false;
		signal_freq_index = -1;
		sendToDebug_uint32(signal_freq_index, false);
	}

	
	// here we clean up the state with a noise blanker
	if (realstate != realstatebefore)
	{
		laststarttime = HAL_GetTick();
	}
	if ((HAL_GetTick() - laststarttime) > CWDECODER_NBTIME)
	{
		if (realstate != filteredstate)
		{
			filteredstate = realstate;
		}
	}
	//sendToDebug_uint8(filteredstate,true);

	// Then we do want to have some durations on high and low
	if (filteredstate != filteredstatebefore)
	{
		if (filteredstate == true)
		{
			starttimehigh = HAL_GetTick();
			lowduration = (HAL_GetTick() - startttimelow);
		}

		if (filteredstate == false)
		{
			startttimelow = HAL_GetTick();
			highduration = (HAL_GetTick() - starttimehigh);
			if (highduration < (2 * hightimesavg) || hightimesavg == 0)
			{
				hightimesavg = (highduration + hightimesavg + hightimesavg) / 3; // now we know avg dit time ( rolling 3 avg)
			}
			if (highduration > (5 * hightimesavg))
			{
				hightimesavg = highduration + hightimesavg; // if speed decrease fast ..
			}
		}
	}

	// now we will check which kind of baud we have - dit or dah
	// and what kind of pause we do have 1 - 3 or 7 pause
	// we think that hightimeavg = 1 bit
	if (filteredstate != filteredstatebefore)
	{
		stop = false;
		if (filteredstate == false)
		{																				  //// we did end a HIGH
			if (highduration < (hightimesavg * 2) && highduration > (hightimesavg * 0.6f)) /// 0.6 filter out false dits
			{
				strcat(code, ".");
				sendToDebug_str(".");
			}
			if (highduration > (hightimesavg * 2) && highduration < (hightimesavg * 6.0f))
			{
				strcat(code, "-");
				sendToDebug_str("-");
				CW_Decoder_WPM = (CW_Decoder_WPM + (1200 / ((highduration) / 3))) / 2; //// the most precise we can do ;o)
			}
		}

		if (filteredstate == true) //// we did end a LOW
		{
			float32_t lacktime = 1.0f;
			if (CW_Decoder_WPM > 25)
				lacktime = 1.0f; ///  when high speeds we have to have a little more pause before new letter or new word
			if (CW_Decoder_WPM > 30)
				lacktime = 1.2f;
			if (CW_Decoder_WPM > 35)
				lacktime = 1.5f;

			if (lowduration > (hightimesavg * (2.0f * lacktime)) && lowduration < hightimesavg * (5.0f * lacktime)) // letter space
			{
				CWDecoder_Decode();
				code[0] = '\0';
				sendToDebug_str(" ");
			}
			if (lowduration >= hightimesavg * (5.0f * lacktime)) // word space
			{
				CWDecoder_Decode();
				code[0] = '\0';
				CWDecoder_PrintChar(" ");
				sendToDebug_newline();
			}
		}
	}

	// write if no more letters
	if ((HAL_GetTick() - startttimelow) > (highduration * 6.0f) && stop == false)
	{
		CWDecoder_Decode();
		code[0] = '\0';
		stop = true;
	}

	// the end of main loop clean up
	realstatebefore = realstate;
	filteredstatebefore = filteredstate;
}

//декодирование из морзе в символы
static void CWDecoder_Decode(void)
{
	if (strcmp(code, ".-") == 0)
		CWDecoder_PrintChar("A");
	else if (strcmp(code, "-...") == 0)
		CWDecoder_PrintChar("B");
	else if (strcmp(code, "-.-.") == 0)
		CWDecoder_PrintChar("C");
	else if (strcmp(code, "-..") == 0)
		CWDecoder_PrintChar("D");
	else if (strcmp(code, ".") == 0)
		CWDecoder_PrintChar("E");
	else if (strcmp(code, "..-.") == 0)
		CWDecoder_PrintChar("F");
	else if (strcmp(code, "--.") == 0)
		CWDecoder_PrintChar("G");
	else if (strcmp(code, "....") == 0)
		CWDecoder_PrintChar("H");
	else if (strcmp(code, "..") == 0)
		CWDecoder_PrintChar("I");
	else if (strcmp(code, ".---") == 0)
		CWDecoder_PrintChar("J");
	else if (strcmp(code, "-.-") == 0)
		CWDecoder_PrintChar("K");
	else if (strcmp(code, ".-..") == 0)
		CWDecoder_PrintChar("L");
	else if (strcmp(code, "--") == 0)
		CWDecoder_PrintChar("M");
	else if (strcmp(code, "-.") == 0)
		CWDecoder_PrintChar("N");
	else if (strcmp(code, "---") == 0)
		CWDecoder_PrintChar("O");
	else if (strcmp(code, ".--.") == 0)
		CWDecoder_PrintChar("P");
	else if (strcmp(code, "--.-") == 0)
		CWDecoder_PrintChar("Q");
	else if (strcmp(code, ".-.") == 0)
		CWDecoder_PrintChar("R");
	else if (strcmp(code, "...") == 0)
		CWDecoder_PrintChar("S");
	else if (strcmp(code, "-") == 0)
		CWDecoder_PrintChar("T");
	else if (strcmp(code, "..-") == 0)
		CWDecoder_PrintChar("U");
	else if (strcmp(code, "...-") == 0)
		CWDecoder_PrintChar("V");
	else if (strcmp(code, ".--") == 0)
		CWDecoder_PrintChar("W");
	else if (strcmp(code, "-..-") == 0)
		CWDecoder_PrintChar("X");
	else if (strcmp(code, "-.--") == 0)
		CWDecoder_PrintChar("Y");
	else if (strcmp(code, "--..") == 0)
		CWDecoder_PrintChar("Z");

	else if (strcmp(code, ".----") == 0)
		CWDecoder_PrintChar("1");
	else if (strcmp(code, "..---") == 0)
		CWDecoder_PrintChar("2");
	else if (strcmp(code, "...--") == 0)
		CWDecoder_PrintChar("3");
	else if (strcmp(code, "....-") == 0)
		CWDecoder_PrintChar("4");
	else if (strcmp(code, ".....") == 0)
		CWDecoder_PrintChar("5");
	else if (strcmp(code, "-....") == 0)
		CWDecoder_PrintChar("6");
	else if (strcmp(code, "--...") == 0)
		CWDecoder_PrintChar("7");
	else if (strcmp(code, "---..") == 0)
		CWDecoder_PrintChar("8");
	else if (strcmp(code, "----.") == 0)
		CWDecoder_PrintChar("9");
	else if (strcmp(code, "-----") == 0)
		CWDecoder_PrintChar("0");

	else if (strcmp(code, "..--..") == 0)
		CWDecoder_PrintChar("?");
	else if (strcmp(code, ".-.-.-") == 0)
		CWDecoder_PrintChar(".");
	else if (strcmp(code, "--..--") == 0)
		CWDecoder_PrintChar(",");
	else if (strcmp(code, "-.-.--") == 0)
		CWDecoder_PrintChar("!");
	else if (strcmp(code, ".--.-.") == 0)
		CWDecoder_PrintChar("@");
	else if (strcmp(code, "---...") == 0)
		CWDecoder_PrintChar(":");
	else if (strcmp(code, "-....-") == 0)
		CWDecoder_PrintChar("-");
	else if (strcmp(code, "-..-.") == 0)
		CWDecoder_PrintChar("/");

	else if (strcmp(code, "-.--.") == 0)
		CWDecoder_PrintChar("(");
	else if (strcmp(code, "-.--.-") == 0)
		CWDecoder_PrintChar(")");
	else if (strcmp(code, ".-...") == 0)
		CWDecoder_PrintChar("_");
	else if (strcmp(code, "...-..-") == 0)
		CWDecoder_PrintChar("$");
	else if (strcmp(code, "...-.-") == 0)
		CWDecoder_PrintChar(">");
	else if (strcmp(code, ".-.-.") == 0)
		CWDecoder_PrintChar("<");
	else if (strcmp(code, "...-.") == 0)
		CWDecoder_PrintChar("~");
	//////////////////
	// The specials //
	//////////////////
	//else if (strcmp(code,".-.-") == 0) CWDecoder_PrintChar(""); ascii(3)
	//else if (strcmp(code,"---.") == 0) CWDecoder_PrintChar(""); ascii(4)
	//else if (strcmp(code,".--.-") == 0) CWDecoder_PrintChar(""); ascii(6)
}

//вывод символа в результирующую строку
static void CWDecoder_PrintChar(char *str)
{
	if (strlen(CW_Decoder_Text) >= CWDECODER_STRLEN)
		shiftTextLeft(CW_Decoder_Text, 1);
	strcat(CW_Decoder_Text, str);
	LCD_UpdateQuery.TextBar = true;
}
