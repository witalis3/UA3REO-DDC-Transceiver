#ifndef CW_DECODER_h
#define CW_DECODER_h

#include "functions.h"
#include "hardware.h"
#include "lcd.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef STM32F407xx

#if (defined(LAY_800x480))
#define CWDECODER_STRLEN (57) // length of decoded string (7 on start - status)
#elif (defined(LAY_480x320))
#define CWDECODER_STRLEN (30) // length of decoded string (7 on start - status)
#elif (defined(LAY_320x240))
#define CWDECODER_STRLEN (26 - 7) // length of decoded string (7 on start - status)
#elif (defined(LAY_160x128))
#define CWDECODER_STRLEN (26 - 7) // length of decoded string (7 on start - status)
#endif

#define CWDECODER_NBTIME 2                              // ms noise blanker
#define CWDECODER_MAGNIFY 16                            // what time we approximate FFT
#define CWDECODER_FFTSIZE 256                           // FFT size for analysis
#define CWDECODER_FFT_SAMPLES (DECODER_PACKET_SIZE * 4) // number of samples for analysis
#define CWDECODER_FFTSIZE_HALF (CWDECODER_FFTSIZE / 2)  // half the size of the FFT
// #define CWDECODER_MEAN_THRES 10.0f                      // Below this signal strength, we consider that it is noise
#define CWDECODER_MAX_THRES 0.5f  // Below this signal strength, we consider that it is not active
#define CWDECODER_MAX_SLIDE 0.99f // reduction factor for the maximum FFT threshold
#define CWDECODER_ZOOMED_SAMPLES (DECODER_PACKET_SIZE / CWDECODER_MAGNIFY)
#define CWDECODER_SPEC_PART (CWDECODER_FFTSIZE_HALF / 2) // search for a signal only in the first part of the signal (CW bandwidth is narrow)
#define CWDECODER_WPM_UP_SPEED 0.50f                     // speed increase factor during auto-tuning
#define CWDECODER_ERROR_DIFF 0.5f                        // error factor when determining the point and dash
#define CWDECODER_ERROR_SPACE_DIFF 0.6f                  // error factor when determining the character and space
#define CWDECODER_MAX_CODE_SIZE 10                       // maximum character code size
#define CWDECODER_MAX_WPM 30                             // maximum WPM
#define CWDECODER_DEBUG false                            // Show Debug

// Public variables
extern float32_t CWDEC_FFTBuffer_Export[CWDECODER_FFTSIZE];
extern volatile uint16_t CW_Decoder_WPM;
extern char CW_Decoder_Text[CWDECODER_STRLEN + 1];

// Public methods
extern void CWDecoder_Init(void);                   // initialize the CW decoder
extern void CWDecoder_Process(float32_t *bufferIn); // start CW decoder for the data block

#endif
#endif
