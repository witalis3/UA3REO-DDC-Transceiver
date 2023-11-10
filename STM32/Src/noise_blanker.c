#include "noise_blanker.h"

// https://github.com/df8oe/UHSDR/wiki/Noise-blanker

// Private variables
static NB_Instance NB_RX1 = {};

#if HRDW_HAS_DUAL_RX
SRAM4 static NB_Instance NB_RX2 = {};
#endif

static uint16_t NB_impulse_positions[NB_max_inpulse_count]; // maximum of impulses per frame
static float32_t NB_firStateF32[NB_FIR_SIZE + NB_order];
static float32_t NB_tempsamp[NB_FIR_SIZE];
static float32_t NB_Wfw[NB_impulse_length] = {0};
static float32_t NB_Wbw[NB_impulse_length] = {0}; // taking linear windows for the combination of fwd and bwd
static float32_t NB_lpcs[NB_order + 1] = {0};     // we reserve one more than "order" because of a leading "1"
static float32_t NB_R[NB_order + 1] = {0};        // takes the autocorrelation results
static float32_t NB_any[NB_order + 1] = {0};      // some internal buffers for the levinson durben algorithm

void NB_Init(void) {
	//  start of test timing zone
	for (uint16_t i = 0; i < NB_impulse_length; i++) // generating 2 Windows for the combination of the 2 predictors
	{                                                // will be a constant window later!
		NB_Wbw[i] = 1.0 * i / (NB_impulse_length - 1);
		NB_Wfw[NB_impulse_length - i - 1] = NB_Wbw[i];
	}
}

// start NB for the data block
void processNoiseBlanking(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id) {
	NB_Instance *instance = &NB_RX1;
#if HRDW_HAS_DUAL_RX
	if (rx_id == AUDIO_RX2) {
		instance = &NB_RX2;
	}
#endif

	dma_memcpy(&instance->NR_InputBuffer[instance->NR_InputBuffer_index * NB_BLOCK_SIZE], buffer, NB_BLOCK_SIZE * 4);
	instance->NR_InputBuffer_index++;
	if (instance->NR_InputBuffer_index == (NB_FIR_SIZE / NB_BLOCK_SIZE)) // input buffer ready
	{
		instance->NR_InputBuffer_index = 0;
		instance->NR_OutputBuffer_index = 0;

		arm_fir_instance_f32 LPC;
		float32_t NB_Rfw[NB_impulse_length + NB_order] = {0}; // takes the forward predicted audio restauration
		float32_t NB_Rbw[NB_impulse_length + NB_order] = {0}; // takes the backward predicted audio restauration
		float32_t NB_reverse_lpcs[NB_order + 1] = {0};        // this takes the reversed order lpc coefficients
		float32_t sigma2 = 0.0f;                              // taking the variance of the inpo
		float32_t lpc_power = 0.0f;

		// working_buffer //we need 128 + 26 floats to work on -//necessary to watch for impulses as close to the frame boundaries as possible
		// copy incomming samples to the end of our working bufer
		dma_memcpy(&instance->NR_Working_buffer[2 * NB_PL + 2 * NB_order], instance->NR_InputBuffer, NB_FIR_SIZE * sizeof(float32_t));

		// calculate the autocorrelation of insamp (moving by max. of #order# samples)
		for (uint16_t i = 0; i < (NB_order + 1); i++) {
			// R is carrying the crosscorrelations
			arm_dot_prod_f32(&instance->NR_Working_buffer[NB_order + NB_PL + 0], &instance->NR_Working_buffer[NB_order + NB_PL + i], NB_FIR_SIZE - i, &NB_R[i]);
		}
		// end of autocorrelation

		// alternative levinson durben algorithm to calculate the lpc coefficients from the crosscorrelation
		NB_R[0] = NB_R[0] * (1.0f + 1.0e-9f);
		NB_lpcs[0] = 1.0f; // set lpc 0 to 1

		for (uint16_t i = 1; i < NB_order + 1; i++) {
			NB_lpcs[i] = 0.0f; // fill rest of array with zeros - could be done by memfill
		}

		float32_t alfa = NB_R[0];
		float32_t k = 0.0f;
		float32_t s = 0.0f;

		for (uint16_t m = 1; m <= NB_order; m++) {
			s = 0.0f;
			for (uint16_t u = 1; u < m; u++) {
				s = s + NB_lpcs[u] * NB_R[m - u];
			}

			k = -(NB_R[m] + s) / alfa;

			for (uint16_t v = 1; v < m; v++) {
				NB_any[v] = NB_lpcs[v] + k * NB_lpcs[m - v];
			}

			for (uint16_t w = 1; w < m; w++) {
				NB_lpcs[w] = NB_any[w];
			}

			NB_lpcs[m] = k;
			alfa = alfa * (1.0f - k * k);
		}
		// end of levinson durben algorithm

		for (uint16_t f = 0; f < NB_order + 1; f++) { // store the reverse order coefficients separately
			NB_reverse_lpcs[NB_order - f] = NB_lpcs[f]; // for the matched impulse filter
		}

		// do the inverse filtering to eliminate voice and enhance the impulses
		arm_fir_init_f32(&LPC, NB_order + 1, NB_reverse_lpcs, NB_firStateF32, NB_FIR_SIZE);
		arm_fir_f32(&LPC, &instance->NR_Working_buffer[NB_order + NB_PL], NB_tempsamp, NB_FIR_SIZE);

		// do a matched filtering to detect an impulse in our now voiceless signal
		arm_fir_init_f32(&LPC, NB_order + 1, NB_lpcs, NB_firStateF32, NB_FIR_SIZE);
		arm_fir_f32(&LPC, NB_tempsamp, NB_tempsamp, NB_FIR_SIZE);

		arm_var_f32(NB_tempsamp, NB_FIR_SIZE, &sigma2); // calculate sigma2 of the tempsignal
		// arm_var_f32(&instance->NR_Working_buffer[NB_order + NB_PL], NB_FIR_SIZE, &sigma2); // calculate sigma2 of the original signal
		arm_power_f32(NB_lpcs, NB_order, &lpc_power);                                      // calculate the sum of the squares (the "power") of the lpc's

		float32_t square;
		arm_sqrt_f32((sigma2 * lpc_power), &square);
		float32_t impulse_threshold = ((float32_t)TRX.NOISE_BLANKER_THRESHOLD / 10.0f) * square; // set a detection level (3 is not really a final setting)

		uint16_t search_pos = NB_order + NB_PL; // lower boundary problem has been solved! - so here we start from 1 or 0?
		uint16_t impulse_count = 0;

		float32_t max_impulse = 0;
		do { // going through the filtered samples to find an impulse larger than the threshold
			if (max_impulse < NB_tempsamp[search_pos]) {
				max_impulse = NB_tempsamp[search_pos];
			}

			if ((NB_tempsamp[search_pos] > impulse_threshold) || (NB_tempsamp[search_pos] < (-impulse_threshold))) {
				NB_impulse_positions[impulse_count] = search_pos - NB_order; // save the impulse positions and correct it by the filter delay
				impulse_count++;
				search_pos += NB_PL; //  set search_pos a bit away, cause we are already repairing this area later
				                     //  and the next impulse should not be that close
			} else {
				search_pos++;
			}
		} while ((search_pos < NB_FIR_SIZE) && (impulse_count < NB_max_inpulse_count));

		// if (impulse_count>0) println("CNT: ", impulse_count, " THRES: ", (double)impulse_threshold, " MAX: ", (double)max_impulse, " DIV: ", (double)(max_impulse / impulse_threshold));
		// if (impulse_count>0) println("MAX: ", (double)max_impulse, " SQ: ", (double)square, " SIG: ", (double)sigma2, " POW: ", (double)lpc_power);

		// from here: reconstruction of the impulse-distorted audio part:
		// first we form the forward and backward prediction transfer functions from the lpcs
		// that is easy, as they are just the negated coefficients  without the leading "1"
		// we can do this in place of the lpcs, as they are not used here anymore and being recalculated in the next frame!
		arm_negate_f32(&NB_lpcs[1], &NB_lpcs[1], NB_order);
		arm_negate_f32(NB_reverse_lpcs, NB_reverse_lpcs, NB_order);

		for (uint16_t j = 0; j < impulse_count; j++) {
			// copy original data + out bound
			dma_memcpy(NB_Rfw, &instance->NR_Working_buffer[NB_order + NB_PL + NB_impulse_positions[j]], (NB_order + NB_impulse_length) * sizeof(float32_t));
			dma_memcpy(NB_Rbw, &instance->NR_Working_buffer[NB_order + NB_PL + NB_impulse_positions[j] + NB_order], (NB_order + NB_impulse_length) * sizeof(float32_t));
			
			for (uint16_t i = 0; i < NB_impulse_length; i++) // now we calculate the forward and backward predictions
			{
				arm_dot_prod_f32(NB_reverse_lpcs, &NB_Rfw[i], NB_order, &NB_Rfw[NB_order + i]);
				arm_dot_prod_f32(&NB_lpcs[1], &NB_Rbw[NB_impulse_length - i], NB_order, &NB_Rbw[NB_impulse_length - i - 1]);
			}

			// do the windowing, or better: weighing
			arm_mult_f32(NB_Wfw, &NB_Rfw[NB_order], &NB_Rfw[NB_order], NB_impulse_length);
			arm_mult_f32(NB_Wbw, NB_Rbw, NB_Rbw, NB_impulse_length);

			// finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
			arm_add_f32(&NB_Rfw[NB_order], NB_Rbw, &instance->NR_Working_buffer[NB_order + NB_impulse_positions[j]], NB_impulse_length);
		}

		// test voiceless signal
		// for(uint32_t i = 0 ; i < NB_FIR_SIZE; i ++) {
		//	instance->NR_Working_buffer[NB_order + NB_PL + i] = NB_tempsamp[i];
		//}

		// copy the samples of the current frame back to the insamp-buffer for output
		dma_memcpy(instance->NR_OutputBuffer, &instance->NR_Working_buffer[NB_order + NB_PL], NB_FIR_SIZE * sizeof(float32_t));
		dma_memcpy(instance->NR_Working_buffer, &instance->NR_Working_buffer[NB_FIR_SIZE], (2 * NB_order + 2 * NB_PL) * sizeof(float32_t)); // copy
	}

	// NaNs fix
	bool nans = false;
	for (uint32_t i = (instance->NR_OutputBuffer_index * NB_BLOCK_SIZE); i < (instance->NR_OutputBuffer_index * NB_BLOCK_SIZE + NB_BLOCK_SIZE); i++) {
		if (!nans && isnanf(instance->NR_OutputBuffer[i])) {
			nans = true;
		}
	}

	if (nans) {
		dma_memset(instance->NR_Working_buffer, 0x00, sizeof(instance->NR_Working_buffer));
	}

	if (!nans && instance->NR_OutputBuffer_index < (NB_FIR_SIZE / NB_BLOCK_SIZE)) {
		dma_memcpy(buffer, &instance->NR_OutputBuffer[instance->NR_OutputBuffer_index * NB_BLOCK_SIZE], NB_BLOCK_SIZE * 4);
	}
	instance->NR_OutputBuffer_index++;
}
