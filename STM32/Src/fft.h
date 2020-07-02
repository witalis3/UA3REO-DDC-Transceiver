#ifndef FFT_h
#define FFT_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <math.h>
#include "functions.h"
#include "wm8731.h"
#include "screen_layout.h"

#define FFT_SIZE 1024                                         //указываем размер расчитываемого FFT
#define FFT_DOUBLE_SIZE_BUFFER (FFT_SIZE * 2)                 //Размер буффера для расчёта FFT
#define FFT_MIN 4.0f                                       //MIN порог сигнала FFT
#define FFT_TARGET 6.0f                                       //средний порог сигнала FFT
#define FFT_MAX 8.0f                                       //MAX порог сигнала FFT
#define FFT_STEP_COEFF 10.0f                                  //коэффициент шага автокалибровки сигнала FFT (больше - медленней)
#define FFT_HZ_IN_PIXEL (IQ_SAMPLERATE / LAY_FFT_PRINT_SIZE) //герц в одном пикселе

//Public variables
extern volatile uint32_t FFT_buff_index;
extern bool NeedFFTInputBuffer;
extern bool FFT_need_fft;
extern bool FFT_buffer_ready;
extern float32_t FFTInput_I[FFT_SIZE];
extern float32_t FFTInput_Q[FFT_SIZE];

//Public methods
extern void FFT_Init(void);                        //инициализация FFT
extern void FFT_Reset(void);                       //сброс FFT
extern void FFT_doFFT(void);                       //расчёт FFT
extern void FFT_printFFT(void);                    //вывод FFT
extern void FFT_printWaterfallDMA(void);           //вывод водопада

#endif
