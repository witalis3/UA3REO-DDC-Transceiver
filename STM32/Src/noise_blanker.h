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
#define NB_SIGNAL_SMOOTH TRX.NB1_SIGNAL_SMOOTH           // reaction filter
#define NB_EDGES_SMOOTH TRX.NB1_EDGES_SMOOTH             // edges smooth filter
#define NB_DELAY_BUFFER_ITEMS TRX.NB1_DELAY_BUFFER_ITEMS // delay zero samples
// NB2
#define NB_c1 TRX.NB2_Avg    // averaging coefficients
#define NB_c2 (1.0f - NB_c1) // averaging coefficients
#define NB_c3 0.999f         // averaging coefficients
#define NB_c4 (1.0f - NB_c3) // averaging coefficients

typedef struct {
	// NB1 with muting
	float32_t delay_buf[NB_DELAY_BUFFER_SIZE];
	int32_t delbuf_inptr;
	int32_t delbuf_outptr;
	float32_t nb_agc;
	uint32_t nb_delay;
	float32_t edge_strength;
	// NB2 with averaging
	float32_t d_avgsig;
	float32_t d_avgmag_nb2;
// NB3 with LPC prdiction
#define NB_impulse_length TRX.NB3_impulse_length    // has to be odd !
#define NB_MAX_impulse_length 31                    // has to be odd !
#define NB_MAX_PL ((NB_MAX_impulse_length - 1) / 2) // has to be (impulse_length-1) / 2
#define NB_PL ((TRX.NB3_impulse_length - 1) / 2)    // has to be (impulse_length-1) / 2
#define NB_order TRX.NB3_order                      // 10                         // lpc's order
#define NB_MAX_order 64                             // 10                         // lpc's order
#define NB_FIR_SIZE 128                             // 64                      // filter buffer size
#define NB_MAX_inpulse_count 50                     // maximum impulses in the block for suppression
	uint16_t NR_InputBuffer_index;
	uint16_t NR_OutputBuffer_index;
	float32_t NR_InputBuffer[NB_FIR_SIZE];
	float32_t NR_OutputBuffer[NB_FIR_SIZE];
	float32_t NR_Working_buffer[NB_FIR_SIZE + 2 * NB_MAX_order + 2 * NB_MAX_PL];
} NB_Instance;

// Public methods
extern void NB_Init(void);
extern void processNoiseBlanking(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id); // start NB for the data block

#endif
