#ifndef CW_DECODER_h
#define CW_DECODER_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "functions.h"

#define CWDECODER_NBTIME 6  // ms noise blanker
#define CWDECODER_STRLEN 30 //длинна декодированной строки
#define CWDECODER_MAGNIFY 16 //во сколько приближаем FFT
#define CWDECODER_FFTSIZE 256 //размер FFT для анализа
#define CWDECODER_FFT_SAMPLES (DECODER_PACKET_SIZE * 4) //количество семплов для анализа
#define CWDECODER_FFTSIZE_HALF (CWDECODER_FFTSIZE / 2) //половина размера FFT
#define CWDECODER_NOISE_THRES 0.2f //Шумовой порог
#define CWDECODER_MAX_THRES 0.7f //Ниже этой силы сигнала считаем что он не активен
#define CWDECODER_MAX_SLIDE 0.999f //коэффициент снижения максимального порога FFT
#define CWDECODER_ZOOMED_SAMPLES (DECODER_PACKET_SIZE / CWDECODER_MAGNIFY)
#define CWDECODER_SPEC_PART (CWDECODER_FFTSIZE_HALF * 3 / 4) //ищем сигнал только в первой части сигнала (полоса CW узкая)
#define CWDECODER_WPM_UP_SPEED 0.90f //коэффициент увеличения скорости при автоподстройке
#define CWDECODER_ERROR_DIFF 0.5f //коэффициент погрешности при определении точки и тире
#define CWDECODER_ERROR_SPACE_DIFF 0.6f //коэффициент погрешности при определении символа и пробела

//Public variables
//extern float32_t CWDEC_FFTBuffer_Export[CWDECODER_FFTSIZE];
extern volatile uint16_t CW_Decoder_WPM;
extern char CW_Decoder_Text[CWDECODER_STRLEN + 1];

//Public methods
extern void CWDecoder_Init(void);                   //инициализация CW декодера
extern void CWDecoder_Process(float32_t *bufferIn); //запуск CW декодера для блока данных

#endif
