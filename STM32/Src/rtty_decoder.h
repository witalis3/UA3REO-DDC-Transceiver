#ifndef RTTY_h
#define RTTY_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "functions.h"
#include "lcd.h"

#if (defined(LAY_800x480))
#define RTTY_DECODER_STRLEN 57 // length of decoded string
#else
#define RTTY_DECODER_STRLEN 30 // length of decoded string
#endif

// Public variables
extern char RTTY_Decoder_Text[RTTY_DECODER_STRLEN + 1];

// Public methods
extern void RTTYDecoder_Init(void);                   // initialize the CW decoder
extern void RTTYDecoder_Process(float32_t *bufferIn); // start CW decoder for the data block

#endif
