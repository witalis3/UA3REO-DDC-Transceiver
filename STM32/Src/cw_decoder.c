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

//Public variables
volatile uint16_t CW_Decoder_WPM = 0;						//декодированная скорость, WPM
char CW_Decoder_Text[CWDECODER_STRLEN + 1] = {0}; //декодирвоанная строка

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
const static arm_cfft_instance_f32 *CWDECODER_FFT_Inst = &arm_cfft_sR_f32_len128;
static float32_t InputBuffer[CWDECODER_SAMPLES] = {0};
static float32_t FFTBufferCharge[CWDECODER_FFTSIZE] = {0}; //накопительный буфер FFT
static float32_t FFTBuffer[CWDECODER_FFTSIZE_DOUBLE] = {0}; //совмещённый буфер FFT
static float32_t window_multipliers[CWDECODER_FFTSIZE] = {0};
static arm_fir_decimate_instance_f32 CWDEC_DECIMATE;
static float32_t CWDEC_decimState[CWDECODER_SAMPLES + 4 - 1];

//Коэффициенты для дециматора
static const arm_fir_decimate_instance_f32 CW_DEC_FirDecimate =
{
	// 48ksps, 1.5kHz lowpass
	.numTaps = 4,
	.pCoeffs = (float32_t *)(const float32_t[]){0.199820836596682871f, 0.272777397353925699f, 0.272777397353925699f, 0.199820836596682871f},
	.pState = NULL,
};
//Prototypes
static void CWDecoder_Decode(void);			//декодирование из морзе в символы
static void CWDecoder_PrintChar(char *str); //вывод символа в результирующую строку

//инициализация CW декодера
void CWDecoder_Init(void)
{
	arm_fir_decimate_init_f32(&CWDEC_DECIMATE, CW_DEC_FirDecimate.numTaps, CWDECODER_MAGNIFY, CW_DEC_FirDecimate.pCoeffs, CWDEC_decimState, CWDECODER_SAMPLES);
	//windowing
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
	{
		//Окно Blackman-Harris
		window_multipliers[i] = 0.35875f - 0.48829f * arm_cos_f32(2.0f * PI * i / ((float32_t)CWDECODER_FFTSIZE - 1.0f)) + 0.14128f * arm_cos_f32(4.0f * PI * i / ((float32_t)CWDECODER_FFTSIZE - 1.0f)) - 0.01168f * arm_cos_f32(6.0f * PI * i / ((float32_t)CWDECODER_FFTSIZE - 1.0f));
	}
}

//запуск CW декодера для блока данных
void CWDecoder_Process(float32_t *bufferIn)
{
	//копируем входящие данные для проследующей работы
	memcpy(InputBuffer, bufferIn, sizeof(InputBuffer));
	//Дециматор
	arm_fir_decimate_f32(&CWDEC_DECIMATE, InputBuffer, InputBuffer, CWDECODER_SAMPLES);
	//Смещаем старые данные в  буфере, чтобы собрать необходимый размер
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
	{
		if (i < (CWDECODER_FFTSIZE - CWDECODER_ZOOMED_SAMPLES))
			FFTBufferCharge[i] = FFTBufferCharge[(i + CWDECODER_ZOOMED_SAMPLES)];
		else //Добавляем новые данные в буфер FFT для расчёта
			FFTBufferCharge[i] = InputBuffer[i - (CWDECODER_FFTSIZE - CWDECODER_ZOOMED_SAMPLES)];
	}
	
	//Окно для FFT
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++)
	{
		FFTBuffer[i * 2] = window_multipliers[i] * FFTBufferCharge[i];
		FFTBuffer[i * 2 + 1] = 0.0f;
	}
	
	//Делаем FFT
	arm_cfft_f32(CWDECODER_FFT_Inst, FFTBuffer, 0, 1);
	arm_cmplx_mag_f32(FFTBuffer, FFTBuffer, CWDECODER_FFTSIZE);
	
	//Ищем максимум магнитуды для определения источника сигнала
	float32_t maxValue = 0;
	uint32_t maxIndex = 0;
	arm_max_f32(FFTBuffer, CWDECODER_FFTSIZE_HALF, &maxValue, &maxIndex);
	
	//Ищем среднее для определения шумового порога
	float32_t meanValue = 0;
	arm_mean_f32(FFTBuffer, CWDECODER_FFTSIZE_HALF, &meanValue);
	
	if(signal_freq_index == -1)
	{
		if(maxValue > meanValue * CWDECODER_NOISEGATE) //сигнал найден
		{
			/*sendToDebug_uint32(maxIndex, true); sendToDebug_str(" "); sendToDebug_float32(maxValue, true); sendToDebug_str(" "); sendToDebug_float32(meanValue, false); */
			signal_freq_index = maxIndex;
			sendToDebug_uint32(signal_freq_index, false);
		}
	}
	
	if(signal_freq_index != -1 && FFTBuffer[signal_freq_index] > meanValue * CWDECODER_NOISEGATE) //сигнал всё ещё активен
	{
		//sendToDebug_str("s");
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
