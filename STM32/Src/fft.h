#ifndef FFT_h
#define FFT_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <math.h>
#include "functions.h"
#include "wm8731.h"
#include "screen_layout.h"

#define FFT_SIZE 1024                                            // specify the size of the calculated FFT
#define FFT_DOUBLE_SIZE_BUFFER (FFT_SIZE * 2)                    // Buffer size for FFT calculation
#define FFT_MIN 4.0f                                             // MIN threshold of FFT signal
#define FFT_TARGET 6.0f                                          // average threshold of the FFT signal
#define FFT_MAX 8.0f                                             // MAX FFT signal threshold
#define FFT_STEP_COEFF 10.0f                                     // step coefficient for auto-calibration of the FFT signal (more - slower)
#define FFT_HZ_IN_PIXEL (float32_t)((float32_t)IQ_SAMPLERATE / (float32_t)LAY_FFT_PRINT_SIZE)     // hertz per pixel
#define FFT_TX_HZ_IN_PIXEL (float32_t)((float32_t)TRX_SAMPLERATE / (float32_t)LAY_FFT_PRINT_SIZE) // hertz per pixel
#define FFT_BW_BRIGHTNESS 5																		 // pixel brightness on bw bar

// Public variables
extern volatile uint32_t FFT_buff_index;
extern bool NeedFFTInputBuffer;
extern bool FFT_need_fft;
extern bool FFT_buffer_ready;
extern float32_t FFTInput_I[FFT_SIZE];
extern float32_t FFTInput_Q[FFT_SIZE];

// Public methods
extern void FFT_Init(void);              // FFT initialization
extern void FFT_Reset(void);             // reset FFT
extern void FFT_doFFT(void);             // FFT calculation
extern void FFT_printFFT(void);          // FFT output
extern void FFT_printWaterfallDMA(void); // waterfall output

#endif
