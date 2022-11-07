#include "noise_blanker.h"

// https://github.com/df8oe/UHSDR/wiki/Noise-blanker

// Private variables
// static NB_Instance NB_RX1 = {0};
#if HRDW_HAS_DUAL_RX
// SRAM static NB_Instance NB_RX2 = {0};
#endif

// start NB for the data block
void processNoiseBlanking(float32_t *buffer, AUDIO_PROC_RX_NUM rx_id) {
	/*NB_Instance *instance = &NB_RX1;
#if HRDW_HAS_DUAL_RX
	if (rx_id == AUDIO_RX2)
	  instance = &NB_RX2;
#endif*/

#define AUDIO_RX_NB_DELAY_BUFFER_ITEMS 120
#define AUDIO_RX_NB_DELAY_BUFFER_SIZE (AUDIO_RX_NB_DELAY_BUFFER_ITEMS * 2)
#define nb_sig_filt 0.005f
#define nb_agc_filt (1.0f - nb_sig_filt)

	static float32_t delay_buf[AUDIO_RX_NB_DELAY_BUFFER_SIZE];
	static uint16_t delbuf_inptr = 0, delbuf_outptr = 2;
	static uint8_t nb_delay = 0;
	static float32_t nb_agc = 0;
	static float32_t last_normal_value = 0;
	static float32_t muting_avg = 1.0f;

	// bool has_blank = false;
	for (uint64_t i = 0; i < NB_BLOCK_SIZE; i++) // Noise blanker function
	{
		float32_t sig = fabsf(buffer[i]); // get signal amplitude.  We need only look at one of the two audio channels since they will be the same.
		delay_buf[delbuf_inptr++] = buffer[i]; // copy first byte into delay buffer

		nb_agc = (nb_agc_filt * nb_agc) + (nb_sig_filt * sig); // IIR-filtered "AGC" of current overall signal level

		if (sig > (nb_agc * TRX.NOISE_BLANKER_THRESHOLD)) // did a pulse exceed the threshold? // && (nb_delay == 0)
		{
			nb_delay = AUDIO_RX_NB_DELAY_BUFFER_ITEMS; // yes - set the blanking duration counter
		}

		if (!nb_delay) // blank counter not active
		{
			buffer[i] = delay_buf[delbuf_outptr++] * muting_avg; // pass through delayed audio, unchanged
			last_normal_value = buffer[i];

			if (muting_avg < 1.0f)
				muting_avg *= 1.1f;
			if (muting_avg > 1.0f)
				muting_avg = 1.0f;
		} else // It is within the blanking pulse period
		{
			if (muting_avg > 0.0000001f)
				muting_avg *= 0.9f;
			// has_blank = true;
			buffer[i] = last_normal_value * muting_avg; // set the audio buffer to "mute" during the blanking period
			nb_delay--; // count down the number of samples that we are to blank
		}

		// RINGBUFFER
		delbuf_outptr %= AUDIO_RX_NB_DELAY_BUFFER_SIZE;
		delbuf_inptr %= AUDIO_RX_NB_DELAY_BUFFER_SIZE;
	}
	// if(has_blank) print("b");

	/*
	dma_memcpy(&instance->NR_InputBuffer[instance->NR_InputBuffer_index * NB_BLOCK_SIZE], buffer, NB_BLOCK_SIZE * 4);
	instance->NR_InputBuffer_index++;
	if (instance->NR_InputBuffer_index == (NB_FIR_SIZE / NB_BLOCK_SIZE)) // input buffer ready
	{
	  instance->NR_InputBuffer_index = 0;
	  instance->NR_OutputBuffer_index = 0;

	  arm_fir_instance_f32 LPC;
	  float32_t lpcs[NB_order + 1] = {0};         // we reserve one more than "order" because of a leading "1"
	  float32_t reverse_lpcs[NB_order + 1] = {0}; // this takes the reversed order lpc coefficients
	  float32_t sigma2 = 0.0f;                    // taking the variance of the inpo
	  float32_t lpc_power = 0.0f;
	  float32_t impulse_threshold = 0.0f;
	  uint16_t search_pos = 0;
	  uint16_t impulse_count = 0;

	  float32_t R[NB_order + 1] = {0}; // takes the autocorrelation results
	  float32_t k = 0.0f;
	  float32_t alfa = 0.0f;

	  float32_t any[NB_order + 1] = {0}; // some internal buffers for the levinson durben algorithm

	  float32_t Rfw[NB_impulse_length + NB_order] = {0}; // takes the forward predicted audio restauration
	  float32_t Rbw[NB_impulse_length + NB_order] = {0}; // takes the backward predicted audio restauration
	  float32_t Wfw[NB_impulse_length] = {0};
	  float32_t Wbw[NB_impulse_length] = {0}; // taking linear windows for the combination of fwd and bwd

	  float32_t s = 0.0f;

	  // working_buffer //we need 128 + 26 floats to work on -//necessary to watch for impulses as close to the frame boundaries as possible
	  dma_memcpy(&instance->NR_Working_buffer[2 * NB_PL + 2 * NB_order], &instance->NR_InputBuffer,
	             NB_FIR_SIZE * sizeof(float32_t)); // copy incomming samples to the end of our working bufer

	  //  start of test timing zone
	  for (uint16_t i = 0; i < NB_impulse_length; i++) // generating 2 Windows for the combination of the 2 predictors
	  {                                                // will be a constant window later!
	    Wbw[i] = 1.0 * i / (NB_impulse_length - 1);
	    Wfw[NB_impulse_length - i - 1] = Wbw[i];
	  }

	  // calculate the autocorrelation of insamp (moving by max. of #order# samples)
	  for (uint16_t i = 0; i < (NB_order + 1); i++)
	    arm_dot_prod_f32(&instance->NR_Working_buffer[NB_order + NB_PL + 0], &instance->NR_Working_buffer[NB_order + NB_PL + i], NB_FIR_SIZE - i,
	                     &R[i]); // R is carrying the crosscorrelations
	  // end of autocorrelation

	  // alternative levinson durben algorithm to calculate the lpc coefficients from the crosscorrelation
	  R[0] = R[0] * (1.0f + 1.0e-9f);
	  lpcs[0] = 1.0f; // set lpc 0 to 1

	  for (uint16_t i = 1; i < NB_order + 1; i++)
	    lpcs[i] = 0.0f; // fill rest of array with zeros - could be done by memfill

	  alfa = R[0];

	  for (uint16_t m = 1; m <= NB_order; m++) {
	    s = 0.0f;
	    for (uint16_t u = 1; u < m; u++)
	      s = s + lpcs[u] * R[m - u];

	    k = -(R[m] + s) / alfa;

	    for (uint16_t v = 1; v < m; v++)
	      any[v] = lpcs[v] + k * lpcs[m - v];

	    for (uint16_t w = 1; w < m; w++)
	      lpcs[w] = any[w];

	    lpcs[m] = k;
	    alfa = alfa * (1.0f - k * k);
	  }
	  // end of levinson durben algorithm

	  for (uint16_t o = 0; o < NB_order + 1; o++) // store the reverse order coefficients separately
	    reverse_lpcs[NB_order - o] = lpcs[o];     // for the matched impulse filter

	  arm_fir_init_f32(&LPC, NB_order + 1, &reverse_lpcs[0], &instance->firStateF32[0],
	                   NB_FIR_SIZE); // we are using the same function as used in freedv
	  arm_fir_f32(&LPC, &instance->NR_Working_buffer[NB_order + NB_PL], instance->tempsamp,
	              NB_FIR_SIZE); // do the inverse filtering to eliminate voice and enhance the impulses
	  arm_fir_init_f32(&LPC, NB_order + 1, &lpcs[0], &instance->firStateF32[0], NB_FIR_SIZE); // we are using the same function as used in freedv
	  arm_fir_f32(&LPC, instance->tempsamp, instance->tempsamp,
	              NB_FIR_SIZE);                              // do a matched filtering to detect an impulse in our now voiceless signal
	  arm_var_f32(instance->tempsamp, NB_FIR_SIZE, &sigma2); // calculate sigma2 of the original signal ? or tempsignal
	  arm_power_f32(lpcs, NB_order, &lpc_power);             // calculate the sum of the squares (the "power") of the lpc's

	  float32_t square;
	  arm_sqrt_f32((sigma2 * lpc_power), &square);
	  impulse_threshold = 3.0f * square; // set a detection level (3 is not really a final setting)

	  search_pos = NB_order + NB_PL; // lower boundary problem has been solved! - so here we start from 1 or 0?
	  impulse_count = 0;

	  float32_t max_impulse = 0;
	  do { // going through the filtered samples to find an impulse larger than the threshold
	    if ((instance->tempsamp[search_pos] > impulse_threshold) || (instance->tempsamp[search_pos] < (-impulse_threshold))) {
	      instance->impulse_positions[impulse_count] = search_pos - NB_order; // save the impulse positions and correct it by the filter delay
	      impulse_count++;
	      search_pos += NB_PL; //  set search_pos a bit away, cause we are already repairing this area later
	                           //  and the next impulse should not be that close
	    }
	    search_pos++;

	    if (max_impulse < instance->tempsamp[search_pos])
	      max_impulse = instance->tempsamp[search_pos];
	  } while ((search_pos < NB_FIR_SIZE) && (impulse_count < NB_max_inpulse_count));

	  // if(impulse_count>0) println("CNT: ", impulse_count, " THRES: ", impulse_threshold, " MAX: ", max_impulse);

	  // from here: reconstruction of the impulse-distorted audio part:

	  // first we form the forward and backward prediction transfer functions from the lpcs
	  // that is easy, as they are just the negated coefficients  without the leading "1"
	  // we can do this in place of the lpcs, as they are not used here anymore and being recalculated in the next frame!
	  arm_negate_f32(&lpcs[1], &lpcs[1], NB_order);
	  arm_negate_f32(&reverse_lpcs[0], &reverse_lpcs[0], NB_order);

	  for (uint16_t j = 0; j < impulse_count; j++) {
	    for (uint16_t f = 0; f < NB_order; f++) // we have to copy some samples from the original signal as
	    {                                       // basis for the reconstructions - could be done by memcopy
	      Rfw[f] =
	          instance->NR_Working_buffer[instance->impulse_positions[j] + f]; // take the sample from this frame as we are away from the boundary
	      Rbw[NB_impulse_length + f] = instance->NR_Working_buffer[NB_order + NB_PL + instance->impulse_positions[j] + NB_PL + f + 1];
	    } // bis hier alles ok

	    for (uint16_t i = 0; i < NB_impulse_length; i++) // now we calculate the forward and backward predictions
	    {
	      arm_dot_prod_f32(&reverse_lpcs[0], &Rfw[i], NB_order, &Rfw[i + NB_order]);
	      arm_dot_prod_f32(&lpcs[1], &Rbw[NB_impulse_length - i], NB_order, &Rbw[NB_impulse_length - i - 1]);
	    }

	    // do the windowing, or better: weighing
	    arm_mult_f32(&Wfw[0], &Rfw[NB_order], &Rfw[NB_order], NB_impulse_length);
	    arm_mult_f32(&Wbw[0], &Rbw[0], &Rbw[0], NB_impulse_length);

	    // finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
	    arm_add_f32(&Rfw[NB_order], &Rbw[0], &instance->NR_Working_buffer[NB_order + instance->impulse_positions[j]], NB_impulse_length);
	  }
	  dma_memcpy(instance->NR_OutputBuffer, &instance->NR_Working_buffer[NB_order + NB_PL],
	             NB_FIR_SIZE * sizeof(float32_t)); // copy the samples of the current frame back to the insamp-buffer for output
	  dma_memcpy(instance->NR_Working_buffer, &instance->NR_Working_buffer[NB_FIR_SIZE], (2 * NB_order + 2 * NB_PL) * sizeof(float32_t)); // copy
	}
	// NaNs fix
	bool nans = false;
	for (uint32_t i = (instance->NR_OutputBuffer_index * NB_BLOCK_SIZE); i < (instance->NR_OutputBuffer_index * NB_BLOCK_SIZE + NB_BLOCK_SIZE);
	     i++)
	  if (!nans && isnanf(instance->NR_OutputBuffer[i]))
	    nans = true;

	if (!nans && instance->NR_OutputBuffer_index < (NB_FIR_SIZE / NB_BLOCK_SIZE)) {
	  dma_memcpy(buffer, &instance->NR_OutputBuffer[instance->NR_OutputBuffer_index * NB_BLOCK_SIZE], NB_BLOCK_SIZE * 4);
	}
	instance->NR_OutputBuffer_index++;
	*/
}
