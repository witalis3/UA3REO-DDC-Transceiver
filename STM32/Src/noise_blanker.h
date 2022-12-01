#ifndef NOISE_BLANKER_h
#define NOISE_BLANKER_h

#include "audio_processor.h"
#include "hardware.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define NB_DELAY_BUFFER_ITEMS 120
#define NB_DELAY_BUFFER_SIZE (NB_DELAY_BUFFER_ITEMS * 2)
#define NB_SIG_FILT 0.005f
#define NB_AGC_FILT (1.0f - NB_SIG_FILT)
#define NB_BLOCK_SIZE (AUDIO_BUFFER_HALF_SIZE / 3) // size of the NB filter processing block
// #define NB_impulse_length 7                        // has to be odd !!!! 7/3 should be enough // 7
// #define NB_PL ((NB_impulse_length - 1) / 2)        // has to be (impulse_length-1) / 2 !!!!
// #define NB_order 10                                // lpc's order // 10
// #define NB_FIR_SIZE 64                             // filter buffer size
// #define NB_max_inpulse_count 5                     // maximum impulses in the block for suppression

typedef struct {
	float32_t *delay_buf;
	uint16_t delbuf_inptr;
	uint16_t delbuf_outptr;
	uint8_t nb_delay;
	float32_t nb_agc;
	float32_t last_normal_value;
	float32_t muting_avg;
	/*
	float32_t NR_InputBuffer[NB_FIR_SIZE];
	uint16_t NR_InputBuffer_index;
	float32_t NR_OutputBuffer[NB_FIR_SIZE];
	uint16_t NR_OutputBuffer_index;
	float32_t NR_Working_buffer[NB_FIR_SIZE + 2 * NB_order + 2 * NB_PL];
	float32_t firStateF32[NB_FIR_SIZE + NB_order];
	float32_t tempsamp[NB_FIR_SIZE];
	uint16_t impulse_positions[NB_max_inpulse_count]; // maximum of impulses per frame
	*/
} NB_Instance;

// Public methods
extern void processNoiseBlanking(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id); // start NB for the data block

#endif
