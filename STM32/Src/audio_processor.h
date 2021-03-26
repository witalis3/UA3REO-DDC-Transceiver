#ifndef AUDIO_PROCESSOR_h
#define AUDIO_PROCESSOR_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "functions.h"
#include "settings.h"

#define AUDIO_BUFFER_SIZE (192 * 2)								 // the size of the buffer for working with sound 48kHz
#define AUDIO_BUFFER_HALF_SIZE (AUDIO_BUFFER_SIZE / 2)			 // buffer size for working with sound 48kHz
#define FPGA_TX_IQ_BUFFER_SIZE AUDIO_BUFFER_SIZE				 // size of TX data buffer for FPGA
#define FPGA_TX_IQ_BUFFER_HALF_SIZE (FPGA_TX_IQ_BUFFER_SIZE / 2) // half the size of the TX data buffer for FPGA
#define FPGA_RX_IQ_BUFFER_SIZE FPGA_TX_IQ_BUFFER_SIZE			 // size of the RX data buffer from the PGA
#define FPGA_RX_IQ_BUFFER_HALF_SIZE (FPGA_RX_IQ_BUFFER_SIZE / 2) // half the size of the RX data buffer from the PGA

#define FM_TX_HPF_ALPHA 0.95f			  // For FM modulator: "Alpha" (high-pass) factor to pre-emphasis
#define FM_SQUELCH_HYSTERESIS 0.3f		  // Hysteresis for FM squelch
#define FM_SQUELCH_PROC_DECIMATION 10	  // Number of times we go through the FM demod algorithm before we do a squelch calculation
#define FM_RX_SQL_SMOOTHING 0.05f		  // Smoothing factor for IIR squelch noise averaging
#define AUDIO_RX_NB_DELAY_BUFFER_ITEMS 32 // NoiseBlanker buffer size
#define AUDIO_RX_NB_DELAY_BUFFER_SIZE (AUDIO_RX_NB_DELAY_BUFFER_ITEMS * 2)
#define AUDIO_MAX_REVERBER_TAPS 10

typedef enum // receiver number
{
	AUDIO_RX1,
	AUDIO_RX2
} AUDIO_PROC_RX_NUM;

// Public variables
extern volatile uint32_t AUDIOPROC_samples;								// audio samples processed in the processor
extern volatile bool Processor_NeedRXBuffer;							// codec needs data from processor for RX
extern volatile bool Processor_NeedTXBuffer;							// codec needs data from processor for TX
extern float32_t APROC_Audio_Buffer_RX1_Q[FPGA_RX_IQ_BUFFER_HALF_SIZE]; // copy of the working part of the FPGA buffers for processing
extern float32_t APROC_Audio_Buffer_RX1_I[FPGA_RX_IQ_BUFFER_HALF_SIZE];
extern float32_t APROC_Audio_Buffer_RX2_Q[FPGA_RX_IQ_BUFFER_HALF_SIZE];
extern float32_t APROC_Audio_Buffer_RX2_I[FPGA_RX_IQ_BUFFER_HALF_SIZE];
extern float32_t APROC_Audio_Buffer_TX_Q[FPGA_TX_IQ_BUFFER_HALF_SIZE];
extern float32_t APROC_Audio_Buffer_TX_I[FPGA_TX_IQ_BUFFER_HALF_SIZE];
extern volatile float32_t Processor_TX_MAX_amplitude_OUT; // TX uplift after ALC
extern volatile float32_t Processor_RX_Power_value;		  // RX signal magnitude
extern bool NeedReinitReverber;
extern bool APROC_IFGain_Overflow;
extern bool DFM_RX1_Squelched;
extern bool DFM_RX2_Squelched;

// Public methods
extern void processRxAudio(void);	  // start audio processor for RX
extern void preProcessRxAudio(void);  // start audio pre-processor for RX
extern void processTxAudio(void);	  // start audio processor for TX
extern void initAudioProcessor(void); // initialize audio processor

#endif
