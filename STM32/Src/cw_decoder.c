#include "cw_decoder.h"
#include "arm_const_structs.h"
#include "arm_math.h"
#include "audio_filters.h"
#include "decoder.h"
#include "fpga.h"
#include "functions.h"
#include "hardware.h"
#include "lcd.h"
#include "settings.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Public variables
volatile uint16_t CW_Decoder_WPM = 0;             // decoded speed, WPM
char CW_Decoder_Text[CWDECODER_STRLEN + 1] = {0}; // decoded string

// Private variables
static bool realstate = false;           // current signal state
static bool realstatebefore = false;     // previous signal state
static bool filteredstate = false;       // signal state filtered from interference
static bool filteredstatebefore = false; // previous signal state filtered from interference
static bool stop = false;                // word accepted, no more signal, stop
static uint32_t laststarttime = 0;       // time of the last decoder pass
static uint32_t starttimehigh = 0;       // signal start time
static uint32_t highduration = 0;        // measured signal duration
static uint32_t startttimelow = 0;       // start time of no signal
static uint32_t lowduration = 0;         // measured duration of no signal
static float32_t dash_time = 0;          // signal duration dash
static float32_t dot_time = 0;           // point signal duration
static float32_t char_time = 0;          // pause between characters
static float32_t word_time = 0;          // pause between words
static bool last_space = false;          // the last character was a space
static char code[CWDECODER_MAX_CODE_SIZE] = {0};
static arm_cfft_instance_f32 CWDECODER_FFT_Inst;
SRAM static float32_t CWDEC_FFTBuffer[CWDECODER_FFTSIZE * 2] = {0};               // FFT buffer
SRAM_ON_H743 static float32_t CWDEC_FFTBufferCharge[CWDECODER_FFTSIZE * 2] = {0}; // cumulative buffer
// SRAM float32_t CWDEC_FFTBuffer_Export[CWDECODER_FFTSIZE] = {0};
SRAM_ON_H743 static float32_t CWDEC_window_multipliers[CWDECODER_FFT_SAMPLES] = {0};
// Decimator
SRAM_ON_H743 static float32_t CWDEC_InputBuffer[DECODER_PACKET_SIZE] = {0};
static arm_fir_decimate_instance_f32 CWDEC_DECIMATE;
static float32_t CWDEC_decimState[DECODER_PACKET_SIZE + 4 - 1];
static const arm_fir_decimate_instance_f32 CW_DEC_FirDecimate = {
    // 48ksps, 1.5kHz lowpass
    .numTaps = 4,
    .pCoeffs = (float32_t *)(const float32_t[]){0.199820836596682871f, 0.272777397353925699f, 0.272777397353925699f, 0.199820836596682871f},
    .pState = NULL,
};

// Prototypes
static void CWDecoder_Decode(void);         // decode from morse to symbols
static void CWDecoder_Recognise(void);      // recognize the character
static void CWDecoder_PrintChar(char *str); // output the character to the resulting string

// initialize the CW decoder
void CWDecoder_Init(void) {
	// initialize CFFT
	arm_cfft_init_f32(&CWDECODER_FFT_Inst, CWDECODER_FFTSIZE);
	// decimator
	arm_fir_decimate_init_f32(&CWDEC_DECIMATE, CW_DEC_FirDecimate.numTaps, CWDECODER_MAGNIFY, CW_DEC_FirDecimate.pCoeffs, CWDEC_decimState,
	                          DECODER_PACKET_SIZE);
	// Blackman-Harris window function
	for (uint_fast16_t i = 0; i < CWDECODER_FFT_SAMPLES; i++)
		CWDEC_window_multipliers[i] = 0.35875f - 0.48829f * arm_cos_f32(2.0f * F_PI * (float32_t)i / ((float32_t)CWDECODER_FFT_SAMPLES - 1.0f)) +
		                              0.14128f * arm_cos_f32(4.0f * F_PI * (float32_t)i / ((float32_t)CWDECODER_FFT_SAMPLES - 1.0f)) -
		                              0.01168f * arm_cos_f32(6.0f * F_PI * (float32_t)i / ((float32_t)CWDECODER_FFT_SAMPLES - 1.0f));
}

// start CW decoder for the data block
void CWDecoder_Process(float32_t *bufferIn) {
	// clear the old FFT buffer
	dma_memset(CWDEC_FFTBuffer, 0x00, sizeof(CWDEC_FFTBuffer));
	// copy the incoming data for the next work
	dma_memcpy(CWDEC_InputBuffer, bufferIn, sizeof(CWDEC_InputBuffer));
	// Decimator
	arm_fir_decimate_f32(&CWDEC_DECIMATE, CWDEC_InputBuffer, CWDEC_InputBuffer, DECODER_PACKET_SIZE);
	// Fill the unnecessary part of the buffer with zeros
	for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE; i++) {
		if (i < CWDECODER_FFT_SAMPLES) {
			if (i < (CWDECODER_FFT_SAMPLES - CWDECODER_ZOOMED_SAMPLES)) // offset old data
				CWDEC_FFTBufferCharge[i] = CWDEC_FFTBufferCharge[(i + CWDECODER_ZOOMED_SAMPLES)];
			else // Add new data to the FFT buffer for calculation
				CWDEC_FFTBufferCharge[i] = CWDEC_InputBuffer[i - (CWDECODER_FFT_SAMPLES - CWDECODER_ZOOMED_SAMPLES)];

			CWDEC_FFTBuffer[i * 2] = CWDEC_window_multipliers[i] * CWDEC_FFTBufferCharge[i]; // + Window function for FFT
			CWDEC_FFTBuffer[i * 2 + 1] = 0.0f;
		} else {
			CWDEC_FFTBuffer[i * 2] = 0.0f;
			CWDEC_FFTBuffer[i * 2 + 1] = 0.0f;
		}
	}

	// Do FFT
	arm_cfft_f32(&CWDECODER_FFT_Inst, CWDEC_FFTBuffer, 0, 0);
	arm_cmplx_mag_f32(CWDEC_FFTBuffer, CWDEC_FFTBuffer, CWDECODER_FFTSIZE);

	for (uint16_t i = 0; i < CWDECODER_FFTSIZE; i++)
		if (isinff(CWDEC_FFTBuffer[i]))
			return;

	// Debug CWDecoder
	/*for (uint_fast16_t i = 0; i < CWDECODER_FFTSIZE_HALF; i ++)
	{
	  CWDEC_FFTBuffer_Export[i] = CWDEC_FFTBuffer[i];
	  CWDEC_FFTBuffer_Export[i + CWDECODER_FFTSIZE_HALF] = CWDEC_FFTBuffer[i];
	}*/

	// Looking for the maximum and minimum magnitude to determine the signal source
	float32_t maxValue = 0;
	uint32_t maxIndex = 0;
	float32_t meanValue = 0;
	arm_max_f32(&CWDEC_FFTBuffer[1], (CWDECODER_SPEC_PART - 1), &maxValue, &maxIndex);
	arm_mean_f32(&CWDEC_FFTBuffer[1], (CWDECODER_SPEC_PART - 1), &meanValue);
	maxIndex++;

	// Sliding top bar
	static float32_t maxValueAvg = 0;
	// if(isinff(maxValueAvg)) maxValueAvg = 0;
	maxValueAvg = maxValueAvg * CWDECODER_MAX_SLIDE + maxValue * (1.0f - CWDECODER_MAX_SLIDE);
	if (maxValueAvg < maxValue)
		maxValueAvg = maxValue;

	// Normalize the frequency response to one
	if (maxValueAvg > 0.0f)
		arm_scale_f32(&CWDEC_FFTBuffer[1], 1.0f / maxValueAvg, &CWDEC_FFTBuffer[1], (CWDECODER_SPEC_PART - 1));

	// println(maxValue, " ", maxValueAvg, " ", CWDEC_FFTBuffer[maxIndex], " ", maxIndex);

	if (CWDEC_FFTBuffer[maxIndex] > CWDECODER_MAX_THRES && (maxValue > meanValue * (float32_t)TRX.CW_Decoder_Threshold)) // signal is active
	{
		// print("s");
		// prinln(maxValue / meanValue);
		realstate = true;
	} else // signal is not active
	{
		if (realstate) {
			// print("-");
		}
		realstate = false;
	}

	// here we clean up the state with a noise blanker
	if (realstate != realstatebefore) {
		// sendToDebug_newline();
		laststarttime = HAL_GetTick();
	}
	if ((HAL_GetTick() - laststarttime) > CWDECODER_NBTIME) {
		if (realstate != filteredstate) {
			filteredstate = realstate;
		}
	}
	// if(filteredstate) sendToDebug_uint8(filteredstate,true);

	// Then we do want to have some durations on high and low
	if (filteredstate != filteredstatebefore) {
		stop = false;

		if (filteredstate == true) {
			// print("s");
			starttimehigh = HAL_GetTick();
			lowduration = (HAL_GetTick() - startttimelow);
		}

		if (filteredstate == false) {
			// print("-");
			startttimelow = HAL_GetTick();
			highduration = HAL_GetTick() - starttimehigh;
		}

		CWDecoder_Recognise();
	}

	// write if no more letters
	if (!filteredstate && ((HAL_GetTick() - startttimelow) > (word_time * (2.0f - CWDECODER_ERROR_SPACE_DIFF))) && stop == false) {
		CWDecoder_Decode();
		if (!last_space) {
			CWDecoder_PrintChar(" ");
			code[0] = '\0';
			last_space = true;
		}
		if (CWDECODER_DEBUG)
			print("s");
		stop = true;
	}

	// the end of main loop clean up
	realstatebefore = realstate;
	filteredstatebefore = filteredstate;
}

static void CWDecoder_Recognise(void) {
	if (filteredstate == false) {
		if (dash_time < highduration) {
			dash_time = dash_time * 0.5f + highduration * 0.5f; // averaging
			dot_time = dash_time / 3.0f;
			char_time = dash_time;
			word_time = dot_time * 7.0f;
		}

		if (highduration > (dot_time * CWDECODER_ERROR_DIFF) && highduration < (dot_time * (2.0f - CWDECODER_ERROR_DIFF))) {
			dot_time = dot_time * 0.7f + highduration * 0.3f; // averaging
			dash_time = dot_time * 3.0f;
			char_time = dash_time;
			word_time = dot_time * 7.0f;
			strcat(code, ".");
			if (CWDECODER_DEBUG)
				print(".");
		} else if (highduration >= (dash_time * CWDECODER_ERROR_DIFF)) {
			dash_time = dash_time * 0.7f + highduration * 0.3f; // averaging
			dot_time = dash_time / 3.0f;
			char_time = dash_time;
			word_time = dot_time * 7.0f;
			strcat(code, "-");
			if (CWDECODER_DEBUG)
				print("-");
		} else {
			dash_time *= CWDECODER_WPM_UP_SPEED;
			// if(CWDECODER_DEBUG)
			// sendToDebug_strln("e");
		}
		CW_Decoder_WPM = (uint16_t)((float32_t)CW_Decoder_WPM * 0.7f + (1200.0f / (float32_t)dot_time) * 0.3f); //// the most precise we can do ;o)
		if (CW_Decoder_WPM > CWDECODER_MAX_WPM)
			CW_Decoder_WPM = CWDECODER_MAX_WPM;
	}
	if (filteredstate == true) {
		float32_t lacktime = 1.0f;
		if (CW_Decoder_WPM > 30) //  when high speeds we have to have a little more pause before new letter or new word
			lacktime = 1.2f;
		if (CW_Decoder_WPM > 35)
			lacktime = 1.5f;

		if (lowduration > (char_time * CWDECODER_ERROR_SPACE_DIFF * lacktime) &&
		    lowduration < (char_time * (2.0f - CWDECODER_ERROR_SPACE_DIFF) * lacktime)) // char space
		{
			CWDecoder_Decode();
			last_space = false;
			if (CWDECODER_DEBUG)
				print("c");
		} else if (lowduration > (word_time * CWDECODER_ERROR_SPACE_DIFF * lacktime)) // word space
		{
			CWDecoder_Decode();
			if (!last_space) {
				CWDecoder_PrintChar(" ");
				last_space = true;
			}
			if (CWDECODER_DEBUG)
				println("w");
		} else {
			// if(CWDECODER_DEBUG)
			// sendToDebug_strln("e");
		}
	}
	if (strlen(code) >= (CWDECODER_MAX_CODE_SIZE - 1))
		code[0] = '\0';
}

// decode from morse to symbols
static void CWDecoder_Decode(void) {
	if (code[0] == '\0')
		return;

	if (strcmp(code, ".-") == 0) // А
		CWDecoder_PrintChar("A");
	else if (strcmp(code, "-...") == 0) // Б
		CWDecoder_PrintChar("B");
	else if (strcmp(code, "-.-.") == 0) // Ц
		CWDecoder_PrintChar("C");
	else if (strcmp(code, "-..") == 0) // Д
		CWDecoder_PrintChar("D");
	else if (strcmp(code, ".") == 0) // Е
		CWDecoder_PrintChar("E");
	else if (strcmp(code, "..-.") == 0) // Ф
		CWDecoder_PrintChar("F");
	else if (strcmp(code, "--.") == 0) // Г
		CWDecoder_PrintChar("G");
	else if (strcmp(code, "....") == 0) // Х
		CWDecoder_PrintChar("H");
	else if (strcmp(code, "..") == 0) // И
		CWDecoder_PrintChar("I");
	else if (strcmp(code, ".---") == 0) // Й
		CWDecoder_PrintChar("J");
	else if (strcmp(code, "-.-") == 0) // К
		CWDecoder_PrintChar("K");
	else if (strcmp(code, ".-..") == 0) // Л
		CWDecoder_PrintChar("L");
	else if (strcmp(code, "--") == 0) // М
		CWDecoder_PrintChar("M");
	else if (strcmp(code, "-.") == 0) // Н
		CWDecoder_PrintChar("N");
	else if (strcmp(code, "---") == 0) // О
		CWDecoder_PrintChar("O");
	else if (strcmp(code, ".--.") == 0) // П
		CWDecoder_PrintChar("P");
	else if (strcmp(code, "--.-") == 0) // Щ
		CWDecoder_PrintChar("Q");
	else if (strcmp(code, ".-.") == 0) // Р
		CWDecoder_PrintChar("R");
	else if (strcmp(code, "...") == 0) // С
		CWDecoder_PrintChar("S");
	else if (strcmp(code, "-") == 0) // Т
		CWDecoder_PrintChar("T");
	else if (strcmp(code, "..-") == 0) // У
		CWDecoder_PrintChar("U");
	else if (strcmp(code, "...-") == 0) // Ж
		CWDecoder_PrintChar("V");
	else if (strcmp(code, ".--") == 0) // В
		CWDecoder_PrintChar("W");
	else if (strcmp(code, "-..-") == 0) // Ъ,Ь
		CWDecoder_PrintChar("X");
	else if (strcmp(code, "-.--") == 0) // Ы
		CWDecoder_PrintChar("Y");
	else if (strcmp(code, "--..") == 0) // З
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

	else if (strcmp(code, "---.") == 0) // Ч
	{
		CWDecoder_PrintChar("C");
		CWDecoder_PrintChar("H");
	} else if (strcmp(code, "----") == 0) // Ш
	{
		CWDecoder_PrintChar("S");
		CWDecoder_PrintChar("H");
	} else if (strcmp(code, "...--...") == 0) // Э
	{
		CWDecoder_PrintChar("E");
	} else if (strcmp(code, "..--") == 0) // Ю
	{
		CWDecoder_PrintChar("Y");
		CWDecoder_PrintChar("U");
	} else if (strcmp(code, ".-.-") == 0) // Я
	{
		CWDecoder_PrintChar("Y");
		CWDecoder_PrintChar("A");
	} else if (strcmp(code, ".--.-") == 0) {
		CWDecoder_PrintChar("*");
	} else {
		CWDecoder_PrintChar("*");
		dash_time *= CWDECODER_WPM_UP_SPEED;
	}

	code[0] = '\0';
}

// output the character to the resulting string
static void CWDecoder_PrintChar(char *str) {
	// sendToDebug_str(str);
	if (strlen(CW_Decoder_Text) >= CWDECODER_STRLEN)
		shiftTextLeft(CW_Decoder_Text, 1);
	strcat(CW_Decoder_Text, str);
	LCD_UpdateQuery.TextBar = true;
}
