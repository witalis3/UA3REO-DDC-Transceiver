#ifndef WM8731_h
#define WM8731_h

#include <stdio.h>
#include "audio_processor.h"
#include "functions.h"
#include "hardware.h"

#define I2C_ADDRESS_WM8731 0x34                         // audio codec address

// Public variables
extern int32_t CODEC_Audio_Buffer_RX[CODEC_AUDIO_BUFFER_SIZE];
extern int32_t CODEC_Audio_Buffer_TX[CODEC_AUDIO_BUFFER_SIZE];
extern bool WM8731_DMA_state;       // what part of the buffer are we working with, true - complete; false - half
extern bool WM8731_Buffer_underrun; // lack of data in the buffer from the audio processor
extern uint32_t WM8731_DMA_samples; // count the number of samples transmitted to the audio codec
extern bool WM8731_Beeping;         // Beeping flag
extern bool WM8731_Muting;          // Muting flag
extern bool WM8731_test_result;

// Public methods
extern void WM8731_Init(void);              // I2C audio codec initialization
extern void WM8731_start_i2s_and_dma(void); // I2S bus start
extern void WM8731_CleanBuffer(void);       // clear the audio codec and USB audio buffer
// extern void WM8731_TX_mode(void);           //switch to TX mode (mute the speaker, etc.)
// extern void WM8731_RX_mode(void);           //switching to RX mode (mute the microphone, etc.)
extern void WM8731_TXRX_mode(void);     // switch to mixed mode RX-TX (for LOOP)
extern void WM8731_Mute(void);          // mute audio out
extern void WM8731_UnMute(void);        // disable audio mute
extern void WM8731_Mute_AF_AMP(void);   // mute audio out (AF AMP only)
extern void WM8731_UnMute_AF_AMP(void); // disable audio mute (AF AMP only)
extern void WM8731_Beep(void);          // beep on key press
#endif
