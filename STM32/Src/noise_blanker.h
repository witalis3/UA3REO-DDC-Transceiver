#ifndef NOISE_BLANKER_h
#define NOISE_BLANKER_h

#include "audio_processor.h"
#include "hardware.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define NB_BLOCK_SIZE (AUDIO_BUFFER_HALF_SIZE / 3) // size of the NB filter processing block

// NB1
#define NB_DELAY_STAGE 2 // buffer blocks count
#define NB_DELAY_BUFFER_SIZE (NB_BLOCK_SIZE * NB_DELAY_STAGE)
#define NB_SIGNAL_SMOOTH 0.5f    // reaction filter
#define NB_EDGES_SMOOTH 0.7f     // edges smooth filter
#define NB_DELAY_BUFFER_ITEMS 16 // delay zero samples
// NB2
#define NB_c1 0.999f         // averaging coefficients
#define NB_c2 (1.0f - NB_c1) // averaging coefficients
#define NB_c3 0.999f         // averaging coefficients
#define NB_c4 (1.0f - NB_c3) // averaging coefficients

typedef struct {
	// NB1
	float32_t delay_buf[NB_DELAY_BUFFER_SIZE];
	int32_t delbuf_inptr;
	int32_t delbuf_outptr;
	float32_t nb_agc;
	uint32_t nb_delay;
	float32_t edge_strength;
	// NB2
	float32_t d_avgsig;
	float32_t d_avgmag_nb2;
} NB_Instance;

#if false
// NB with LPC prdiction
#define NB_impulse_length 11                // has to be odd !
#define NB_PL ((NB_impulse_length - 1) / 2) // has to be (impulse_length-1) / 2
#define NB_order 4                          // 10                         // lpc's order
#define NB_FIR_SIZE 128                     // 64                      // filter buffer size
#define NB_max_inpulse_count 10             // maximum impulses in the block for suppression

typedef struct {
	uint16_t NR_InputBuffer_index;
	uint16_t NR_OutputBuffer_index;
	float32_t NR_InputBuffer[NB_FIR_SIZE];
	float32_t NR_OutputBuffer[NB_FIR_SIZE];
	float32_t NR_Working_buffer[NB_FIR_SIZE + 2 * NB_order + 2 * NB_PL];
} NB_Instance;
#endif

// Public methods
extern void NB_Init(void);
extern void processNoiseBlanking(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id); // start NB for the data block

#endif
