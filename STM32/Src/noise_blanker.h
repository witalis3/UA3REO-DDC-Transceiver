#ifndef NOISE_BLANKER_h
#define NOISE_BLANKER_h

#include "audio_processor.h"
#include "hardware.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define NB_BLOCK_SIZE (AUDIO_BUFFER_HALF_SIZE / 3) // size of the NB filter processing block

#define NB_impulse_length 11                // has to be odd !
#define NB_PL ((NB_impulse_length - 1) / 2) // has to be (impulse_length-1) / 2
#define NB_order 4 //10                         // lpc's order
#define NB_FIR_SIZE 128 //64                      // filter buffer size
#define NB_max_inpulse_count 10             // maximum impulses in the block for suppression

typedef struct {
	uint16_t NR_InputBuffer_index;
	uint16_t NR_OutputBuffer_index;
	float32_t NR_InputBuffer[NB_FIR_SIZE];
	float32_t NR_OutputBuffer[NB_FIR_SIZE];
	float32_t NR_Working_buffer[NB_FIR_SIZE + 2 * NB_order + 2 * NB_PL];
} NB_Instance;

// Public methods
extern void NB_Init(void);
extern void processNoiseBlanking(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id); // start NB for the data block

#endif
