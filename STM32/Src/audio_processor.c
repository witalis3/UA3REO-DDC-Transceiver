#include "audio_processor.h"
#include "wm8731.h"
#include "audio_filters.h"
#include "agc.h"
#include "settings.h"
#include "usbd_audio_if.h"
#include "noise_reduction.h"
#include "noise_blanker.h"
#include "auto_notch.h"
#include "decoder.h"
#include "vad.h"
#include "trx_manager.h"

// Public variables
volatile uint32_t AUDIOPROC_samples = 0;								  // audio samples processed in the processor
volatile uint_fast8_t Processor_AudioBuffer_ReadyBuffer = 0;			  // which buffer is currently in use, A or B
volatile bool Processor_NeedRXBuffer = false;							  // codec needs data from processor for RX
volatile bool Processor_NeedTXBuffer = false;							  // codec needs data from processor for TX
float32_t FPGA_Audio_Buffer_RX1_Q_tmp[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0}; // copy of the working part of the FPGA buffers for processing
float32_t FPGA_Audio_Buffer_RX1_I_tmp[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0};
float32_t FPGA_Audio_Buffer_RX2_Q_tmp[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0};
float32_t FPGA_Audio_Buffer_RX2_I_tmp[FPGA_RX_IQ_BUFFER_HALF_SIZE] = {0};
float32_t FPGA_Audio_Buffer_TX_Q_tmp[FPGA_TX_IQ_BUFFER_HALF_SIZE] = {0};
float32_t FPGA_Audio_Buffer_TX_I_tmp[FPGA_TX_IQ_BUFFER_HALF_SIZE] = {0};
volatile float32_t Processor_RX_Power_value;					// RX signal magnitude
volatile float32_t Processor_selected_RFpower_amplitude = 0.0f; // target TX signal amplitude
volatile float32_t Processor_TX_MAX_amplitude_OUT;				// TX uplift after ALC
bool NeedReinitReverber = false;

// Private variables
static int32_t Processor_AudioBuffer_A[AUDIO_BUFFER_SIZE] = {0};								 // buffer A of the audio processor
static int32_t Processor_AudioBuffer_B[AUDIO_BUFFER_SIZE] = {0};								 // buffer B of the audio processor
static uint32_t two_signal_gen_position = 0;													 // signal position in a two-signal generator
static float32_t ALC_need_gain = 1.0f;															 // current gain of ALC and audio compressor
static float32_t ALC_need_gain_target = 1.0f;													 // Target Gain Of ALC And Audio Compressor
static float32_t DFM_RX1_i_prev = 0, DFM_RX1_q_prev = 0, DFM_RX2_i_prev = 0, DFM_RX2_q_prev = 0; // used in FM detection and low / high pass processing
static uint_fast8_t DFM_RX1_fm_sql_count = 0, DFM_RX2_fm_sql_count = 0;							 // used for squelch processing and debouncing tone detection, respectively
static float32_t DFM_RX1_fm_sql_avg = 0.0f;														 // average SQL in FM
static float32_t DFM_RX2_fm_sql_avg = 0.0f;
static bool DFM_RX1_Squelched = false, DFM_RX2_Squelched = false;
static float32_t current_if_gain = 0.0f;
static float32_t volume_gain = 0.0f;
IRAM2 static float32_t Processor_Reverber_Buffer[AUDIO_BUFFER_HALF_SIZE * AUDIO_MAX_REVERBER_TAPS] = {0};
static float32_t deemph_a = 0.0f; //deemphasis coeff

// Prototypes
static void doRX_HILBERT(AUDIO_PROC_RX_NUM rx_id, uint16_t size);				 // Hilbert filter for phase shift of signals
static void doRX_LPF_IQ(AUDIO_PROC_RX_NUM rx_id, uint16_t size);				 // Low-pass filter for I and Q
static void doRX_LPF_I(AUDIO_PROC_RX_NUM rx_id, uint16_t size);					 // LPF filter for I
static void doRX_GAUSS_I(AUDIO_PROC_RX_NUM rx_id, uint16_t size);				 // Gauss filter for I
static void doRX_HPF_I(AUDIO_PROC_RX_NUM rx_id, uint16_t size);					 // HPF filter for I
static void doRX_DNR(AUDIO_PROC_RX_NUM rx_id, uint16_t size);					 // Digital Noise Reduction
static void doRX_AGC(AUDIO_PROC_RX_NUM rx_id, uint16_t size, uint_fast8_t mode); // automatic gain control
static void doRX_NOTCH(AUDIO_PROC_RX_NUM rx_id, uint16_t size);					 // notch filter
static void doRX_NoiseBlanker(AUDIO_PROC_RX_NUM rx_id, uint16_t size);			 // impulse noise suppressor
static void doRX_SMETER(AUDIO_PROC_RX_NUM rx_id, uint16_t size);				 // s-meter
static void doRX_COPYCHANNEL(AUDIO_PROC_RX_NUM rx_id, uint16_t size);			 // copy I to Q channel
static void DemodulateFM(AUDIO_PROC_RX_NUM rx_id, uint16_t size);				 // FM demodulator
static void ModulateFM(uint16_t size);											 // FM modulator
static void doRX_EQ(uint16_t size);												 // receiver equalizer
static void doMIC_EQ(uint16_t size);											 // microphone equalizer
static void doVAD(uint16_t size);												 // voice activity detector
static void doRX_IFGain(AUDIO_PROC_RX_NUM rx_id, uint16_t size);				 //IF gain

// initialize audio processor
void initAudioProcessor(void)
{
	deemph_a = roundf(1.0f / ((1.0f - expf(-1.0f / ((float32_t)IQ_SAMPLERATE * 75e-6)))));
	InitAudioFilters();
	DECODER_Init();
	NeedReinitReverber = true;
}

// start audio processor for RX
void processRxAudio(void)
{
	if (!Processor_NeedRXBuffer)
		return;

	VFO *current_vfo = CurrentVFO();
	VFO *secondary_vfo = SecondaryVFO();

	AUDIOPROC_samples++;
	uint_fast16_t FPGA_Audio_Buffer_Index_tmp = FPGA_Audio_RXBuffer_Index;
	if (FPGA_Audio_Buffer_Index_tmp == 0)
		FPGA_Audio_Buffer_Index_tmp = FPGA_RX_IQ_BUFFER_SIZE;
	else
		FPGA_Audio_Buffer_Index_tmp--;

	// copy buffer from FPGA
	readFromCircleBuffer32((uint32_t *)&FPGA_Audio_Buffer_RX1_Q[0], (uint32_t *)&FPGA_Audio_Buffer_RX1_Q_tmp[0], FPGA_Audio_Buffer_Index_tmp, FPGA_RX_IQ_BUFFER_SIZE, FPGA_RX_IQ_BUFFER_HALF_SIZE);
	readFromCircleBuffer32((uint32_t *)&FPGA_Audio_Buffer_RX1_I[0], (uint32_t *)&FPGA_Audio_Buffer_RX1_I_tmp[0], FPGA_Audio_Buffer_Index_tmp, FPGA_RX_IQ_BUFFER_SIZE, FPGA_RX_IQ_BUFFER_HALF_SIZE);
	if (TRX.Dual_RX)
	{
		readFromCircleBuffer32((uint32_t *)&FPGA_Audio_Buffer_RX2_Q[0], (uint32_t *)&FPGA_Audio_Buffer_RX2_Q_tmp[0], FPGA_Audio_Buffer_Index_tmp, FPGA_RX_IQ_BUFFER_SIZE, FPGA_RX_IQ_BUFFER_HALF_SIZE);
		readFromCircleBuffer32((uint32_t *)&FPGA_Audio_Buffer_RX2_I[0], (uint32_t *)&FPGA_Audio_Buffer_RX2_I_tmp[0], FPGA_Audio_Buffer_Index_tmp, FPGA_RX_IQ_BUFFER_SIZE, FPGA_RX_IQ_BUFFER_HALF_SIZE);
	}

	//Decimate
	uint16_t decimated_block_size_rx1 = FPGA_RX_IQ_BUFFER_HALF_SIZE;
	uint16_t decimated_block_size_rx2 = FPGA_RX_IQ_BUFFER_HALF_SIZE;
	if (current_vfo->Mode != TRX_MODE_NFM && current_vfo->Mode != TRX_MODE_WFM)
	{
		arm_fir_decimate_f32(&DECIMATE_RX1_AUDIO_I, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		arm_fir_decimate_f32(&DECIMATE_RX1_AUDIO_Q, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, decimated_block_size_rx1);
		decimated_block_size_rx1 = AUDIO_BUFFER_HALF_SIZE;
	}
	if (TRX.Dual_RX && secondary_vfo->Mode != TRX_MODE_NFM && secondary_vfo->Mode != TRX_MODE_WFM)
	{
		arm_fir_decimate_f32(&DECIMATE_RX2_AUDIO_I, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2);
		arm_fir_decimate_f32(&DECIMATE_RX2_AUDIO_Q, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, decimated_block_size_rx2);
		decimated_block_size_rx2 = AUDIO_BUFFER_HALF_SIZE;
	}

	//Process DC corrector filter
	if (current_vfo->Mode != TRX_MODE_AM && current_vfo->Mode != TRX_MODE_NFM && current_vfo->Mode != TRX_MODE_WFM)
	{
		dc_filter(FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1, DC_FILTER_RX1_I);
		dc_filter(FPGA_Audio_Buffer_RX1_Q_tmp, decimated_block_size_rx1, DC_FILTER_RX1_Q);
	}
	if (TRX.Dual_RX && secondary_vfo->Mode != TRX_MODE_AM && secondary_vfo->Mode != TRX_MODE_NFM && secondary_vfo->Mode != TRX_MODE_WFM)
	{
		dc_filter(FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2, DC_FILTER_RX2_I);
		dc_filter(FPGA_Audio_Buffer_RX2_Q_tmp, decimated_block_size_rx2, DC_FILTER_RX2_Q);
	}

	//IQ Phase corrector https://github.com/df8oe/UHSDR/wiki/IQ---correction-and-mirror-frequencies
	float32_t teta1_new = 0;
	float32_t teta3_new = 0;
	static float32_t teta1 = 0;
	static float32_t teta3 = 0;
	for (uint16_t i = 0; i < decimated_block_size_rx1; i++)
	{
		teta1_new += FPGA_Audio_Buffer_RX1_Q_tmp[i] * (FPGA_Audio_Buffer_RX1_I_tmp[i] < 0.0f ? -1.0f : 1.0f);
		teta3_new += FPGA_Audio_Buffer_RX1_Q_tmp[i] * (FPGA_Audio_Buffer_RX1_Q_tmp[i] < 0.0f ? -1.0f : 1.0f);
	}
	teta1_new = teta1_new / (float32_t)decimated_block_size_rx1;
	teta3_new = teta3_new / (float32_t)decimated_block_size_rx1;
	teta1 = 0.003f * teta1_new + 0.997f * teta1;
	teta3 = 0.003f * teta3_new + 0.997f * teta3;
	if (teta3 > 0.0f)
		TRX_IQ_phase_error = asinf(teta1 / teta3);

	VAD_Muting = false;
	if (current_vfo->Mode != TRX_MODE_IQ)
		doRX_IFGain(AUDIO_RX1, decimated_block_size_rx1);

	switch (current_vfo->Mode) // first receiver
	{
	case TRX_MODE_LSB:
	case TRX_MODE_CW_L:
		doRX_HILBERT(AUDIO_RX1, decimated_block_size_rx1);
		arm_sub_f32(FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1); // difference of I and Q - LSB
		doRX_HPF_I(AUDIO_RX1, decimated_block_size_rx1);
		doRX_LPF_I(AUDIO_RX1, decimated_block_size_rx1);
		doRX_GAUSS_I(AUDIO_RX1, decimated_block_size_rx1);
		doRX_NOTCH(AUDIO_RX1, decimated_block_size_rx1);
		doRX_NoiseBlanker(AUDIO_RX1, decimated_block_size_rx1);
		doRX_SMETER(AUDIO_RX1, decimated_block_size_rx1);
		DECODER_PutSamples(FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		doRX_DNR(AUDIO_RX1, decimated_block_size_rx1);
		doVAD(decimated_block_size_rx1);
		doRX_AGC(AUDIO_RX1, decimated_block_size_rx1, current_vfo->Mode);
		doRX_COPYCHANNEL(AUDIO_RX1, decimated_block_size_rx1);
		break;
	case TRX_MODE_DIGI_L:
		doRX_HILBERT(AUDIO_RX1, decimated_block_size_rx1);
		arm_sub_f32(FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1); // difference of I and Q - LSB
		doRX_LPF_I(AUDIO_RX1, decimated_block_size_rx1);
		doRX_SMETER(AUDIO_RX1, decimated_block_size_rx1);
		DECODER_PutSamples(FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		doRX_AGC(AUDIO_RX1, decimated_block_size_rx1, current_vfo->Mode);
		doRX_COPYCHANNEL(AUDIO_RX1, decimated_block_size_rx1);
		break;
	case TRX_MODE_USB:
	case TRX_MODE_CW_U:
		doRX_HILBERT(AUDIO_RX1, decimated_block_size_rx1);
		arm_add_f32(FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1); // sum of I and Q - USB
		doRX_HPF_I(AUDIO_RX1, decimated_block_size_rx1);
		doRX_LPF_I(AUDIO_RX1, decimated_block_size_rx1);
		doRX_GAUSS_I(AUDIO_RX1, decimated_block_size_rx1);
		doRX_NOTCH(AUDIO_RX1, decimated_block_size_rx1);
		doRX_NoiseBlanker(AUDIO_RX1, decimated_block_size_rx1);
		doRX_SMETER(AUDIO_RX1, decimated_block_size_rx1);
		DECODER_PutSamples(FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		doRX_DNR(AUDIO_RX1, decimated_block_size_rx1);
		doVAD(decimated_block_size_rx1);
		doRX_AGC(AUDIO_RX1, decimated_block_size_rx1, current_vfo->Mode);
		doRX_COPYCHANNEL(AUDIO_RX1, decimated_block_size_rx1);
		break;
	case TRX_MODE_DIGI_U:
		doRX_HILBERT(AUDIO_RX1, decimated_block_size_rx1);
		arm_add_f32(FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1); // sum of I and Q - USB
		doRX_LPF_I(AUDIO_RX1, decimated_block_size_rx1);
		doRX_SMETER(AUDIO_RX1, decimated_block_size_rx1);
		DECODER_PutSamples(FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		doRX_AGC(AUDIO_RX1, decimated_block_size_rx1, current_vfo->Mode);
		doRX_COPYCHANNEL(AUDIO_RX1, decimated_block_size_rx1);
		break;
	case TRX_MODE_AM:
		doRX_LPF_IQ(AUDIO_RX1, decimated_block_size_rx1);
		arm_mult_f32(FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		arm_mult_f32(FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, decimated_block_size_rx1);
		arm_add_f32(FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		//arm_vsqrt_f32(FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_AUDIO_BUFFER_HALF_SIZE);
		for (uint_fast16_t i = 0; i < decimated_block_size_rx1; i++)
			arm_sqrt_f32(FPGA_Audio_Buffer_RX1_I_tmp[i], &FPGA_Audio_Buffer_RX1_I_tmp[i]);
		arm_scale_f32(FPGA_Audio_Buffer_RX1_I_tmp, 0.5f, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		dc_filter(FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1, DC_FILTER_RX1_I);
		doRX_NOTCH(AUDIO_RX1, decimated_block_size_rx1);
		doRX_NoiseBlanker(AUDIO_RX1, decimated_block_size_rx1);
		doRX_SMETER(AUDIO_RX1, decimated_block_size_rx1);
		doRX_DNR(AUDIO_RX1, decimated_block_size_rx1);
		doVAD(decimated_block_size_rx1);
		doRX_AGC(AUDIO_RX1, decimated_block_size_rx1, current_vfo->Mode);
		doRX_COPYCHANNEL(AUDIO_RX1, decimated_block_size_rx1);
		break;
	case TRX_MODE_NFM:
		doRX_LPF_IQ(AUDIO_RX1, decimated_block_size_rx1);
	case TRX_MODE_WFM:
		doRX_SMETER(AUDIO_RX1, decimated_block_size_rx1);
		DemodulateFM(AUDIO_RX1, decimated_block_size_rx1);
		//now decimate output
		arm_fir_decimate_f32(&DECIMATE_RX1_AUDIO_I, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		arm_fir_decimate_f32(&DECIMATE_RX1_AUDIO_Q, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, decimated_block_size_rx1);
		decimated_block_size_rx1 = AUDIO_BUFFER_HALF_SIZE;
		//end decimate
		doRX_DNR(AUDIO_RX1, decimated_block_size_rx1);
		doRX_AGC(AUDIO_RX1, decimated_block_size_rx1, current_vfo->Mode);
		doRX_COPYCHANNEL(AUDIO_RX1, decimated_block_size_rx1);
		break;
	case TRX_MODE_IQ:
	default:
		doRX_SMETER(AUDIO_RX1, decimated_block_size_rx1);
		break;
	}

	if (TRX.Dual_RX)
	{
		if (secondary_vfo->Mode != TRX_MODE_IQ)
			doRX_IFGain(AUDIO_RX2, decimated_block_size_rx2);

		switch (secondary_vfo->Mode) // second receiver
		{
		case TRX_MODE_LSB:
		case TRX_MODE_CW_L:
			doRX_HILBERT(AUDIO_RX2, decimated_block_size_rx2);
			arm_sub_f32(FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2); // difference of I and Q - LSB
			doRX_HPF_I(AUDIO_RX2, decimated_block_size_rx2);
			doRX_LPF_I(AUDIO_RX2, decimated_block_size_rx2);
			doRX_GAUSS_I(AUDIO_RX2, decimated_block_size_rx2);
			doRX_NOTCH(AUDIO_RX2, decimated_block_size_rx2);
			doRX_NoiseBlanker(AUDIO_RX2, decimated_block_size_rx2);
			doRX_DNR(AUDIO_RX2, decimated_block_size_rx2);
			doRX_AGC(AUDIO_RX2, decimated_block_size_rx2, secondary_vfo->Mode);
			break;
		case TRX_MODE_DIGI_L:
			doRX_HILBERT(AUDIO_RX2, decimated_block_size_rx2);
			arm_sub_f32(FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2); // difference of I and Q - LSB
			doRX_LPF_I(AUDIO_RX2, decimated_block_size_rx2);
			doRX_AGC(AUDIO_RX2, decimated_block_size_rx2, secondary_vfo->Mode);
			break;
		case TRX_MODE_USB:
		case TRX_MODE_CW_U:
			doRX_HILBERT(AUDIO_RX2, decimated_block_size_rx2);
			arm_add_f32(FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2); // sum of I and Q - USB
			doRX_HPF_I(AUDIO_RX2, decimated_block_size_rx2);
			doRX_LPF_I(AUDIO_RX2, decimated_block_size_rx2);
			doRX_GAUSS_I(AUDIO_RX2, decimated_block_size_rx2);
			doRX_NOTCH(AUDIO_RX2, decimated_block_size_rx2);
			doRX_NoiseBlanker(AUDIO_RX2, decimated_block_size_rx2);
			doRX_DNR(AUDIO_RX2, decimated_block_size_rx2);
			doRX_AGC(AUDIO_RX2, decimated_block_size_rx2, secondary_vfo->Mode);
			break;
		case TRX_MODE_DIGI_U:
			doRX_HILBERT(AUDIO_RX2, decimated_block_size_rx2);
			arm_add_f32(FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2); // sum of I and Q - USB
			doRX_LPF_I(AUDIO_RX2, decimated_block_size_rx2);
			doRX_AGC(AUDIO_RX2, decimated_block_size_rx2, secondary_vfo->Mode);
			break;
		case TRX_MODE_AM:
			doRX_LPF_IQ(AUDIO_RX2, decimated_block_size_rx2);
			arm_mult_f32(FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2);
			arm_mult_f32(FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, decimated_block_size_rx2);
			arm_add_f32(FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2);
			//arm_vsqrt_f32(FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp,FPGA_AUDIO_BUFFER_HALF_SIZE);
			for (uint_fast16_t i = 0; i < decimated_block_size_rx2; i++)
				arm_sqrt_f32(FPGA_Audio_Buffer_RX2_I_tmp[i], &FPGA_Audio_Buffer_RX2_I_tmp[i]);
			arm_scale_f32(FPGA_Audio_Buffer_RX2_I_tmp, 0.5f, FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2);
			dc_filter(FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2, DC_FILTER_RX2_I);
			doRX_NOTCH(AUDIO_RX2, decimated_block_size_rx2);
			doRX_NoiseBlanker(AUDIO_RX2, decimated_block_size_rx2);
			doRX_DNR(AUDIO_RX2, decimated_block_size_rx2);
			doRX_AGC(AUDIO_RX2, decimated_block_size_rx2, secondary_vfo->Mode);
			break;
		case TRX_MODE_NFM:
			doRX_LPF_IQ(AUDIO_RX2, decimated_block_size_rx2);
		case TRX_MODE_WFM:
			DemodulateFM(AUDIO_RX2, decimated_block_size_rx2);
			//now decimate output
			arm_fir_decimate_f32(&DECIMATE_RX2_AUDIO_I, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, decimated_block_size_rx2);
			arm_fir_decimate_f32(&DECIMATE_RX2_AUDIO_Q, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, decimated_block_size_rx2);
			decimated_block_size_rx2 = AUDIO_BUFFER_HALF_SIZE;
			//end decimate
			doRX_DNR(AUDIO_RX2, decimated_block_size_rx2);
			doRX_AGC(AUDIO_RX2, decimated_block_size_rx2, secondary_vfo->Mode);
			break;
		case TRX_MODE_IQ:
		default:
			break;
		}
	}

	//Prepare data to DMA
	int32_t *Processor_AudioBuffer_current = Processor_AudioBuffer_A;
	if (Processor_AudioBuffer_ReadyBuffer == 0)
		Processor_AudioBuffer_current = Processor_AudioBuffer_B;

	// addition of signals in double receive mode
	if (TRX.Dual_RX && TRX.Dual_RX_Type == VFO_A_PLUS_B)
	{
		arm_add_f32(FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
		arm_scale_f32(FPGA_Audio_Buffer_RX1_I_tmp, 0.5f, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);
	}

	// receiver equalizer
	if (current_vfo->Mode != TRX_MODE_DIGI_L && current_vfo->Mode != TRX_MODE_DIGI_U && current_vfo->Mode != TRX_MODE_IQ)
		doRX_EQ(decimated_block_size_rx1);

	// muting
	if (TRX_Mute)
		arm_scale_f32(FPGA_Audio_Buffer_RX1_I_tmp, 0.0f, FPGA_Audio_Buffer_RX1_I_tmp, decimated_block_size_rx1);

	// create buffers for transmission to the codec
	for (uint_fast16_t i = 0; i < decimated_block_size_rx1; i++)
	{
		if (!TRX.Dual_RX)
		{
			arm_float_to_q31(&FPGA_Audio_Buffer_RX1_I_tmp[i], &Processor_AudioBuffer_current[i * 2], 1); //left channel
			if (current_vfo->Mode == TRX_MODE_IQ)
				arm_float_to_q31(&FPGA_Audio_Buffer_RX1_Q_tmp[i], &Processor_AudioBuffer_current[i * 2 + 1], 1); //right channel
			else
				arm_float_to_q31(&FPGA_Audio_Buffer_RX1_I_tmp[i], &Processor_AudioBuffer_current[i * 2 + 1], 1); //right channel
		}
		else if (TRX.Dual_RX_Type == VFO_A_AND_B)
		{
			if (!TRX.current_vfo)
			{
				arm_float_to_q31(&FPGA_Audio_Buffer_RX1_I_tmp[i], &Processor_AudioBuffer_current[i * 2], 1);	 //left channel
				arm_float_to_q31(&FPGA_Audio_Buffer_RX2_I_tmp[i], &Processor_AudioBuffer_current[i * 2 + 1], 1); //right channel
			}
			else
			{
				arm_float_to_q31(&FPGA_Audio_Buffer_RX2_I_tmp[i], &Processor_AudioBuffer_current[i * 2], 1);	 //left channel
				arm_float_to_q31(&FPGA_Audio_Buffer_RX1_I_tmp[i], &Processor_AudioBuffer_current[i * 2 + 1], 1); //right channel
			}
		}
		else if (TRX.Dual_RX_Type == VFO_A_PLUS_B)
		{
			arm_float_to_q31(&FPGA_Audio_Buffer_RX1_I_tmp[i], &Processor_AudioBuffer_current[i * 2], 1); //left channel
			Processor_AudioBuffer_current[i * 2 + 1] = Processor_AudioBuffer_current[i * 2];			 //right channel
		}
	}
	if (Processor_AudioBuffer_ReadyBuffer == 0)
		Processor_AudioBuffer_ReadyBuffer = 1;
	else
		Processor_AudioBuffer_ReadyBuffer = 0;

	//Send to USB Audio
	if (USB_AUDIO_need_rx_buffer && TRX_Inited)
	{
		uint8_t *USB_AUDIO_rx_buffer_current = USB_AUDIO_rx_buffer_b;
		if (!USB_AUDIO_current_rx_buffer)
			USB_AUDIO_rx_buffer_current = USB_AUDIO_rx_buffer_a;

		//drop LSB 32b->24b
		for (uint_fast16_t i = 0; i < (USB_AUDIO_RX_BUFFER_SIZE / BYTES_IN_SAMPLE_AUDIO_OUT_PACKET); i++)
		{
			USB_AUDIO_rx_buffer_current[i * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET + 0] = (Processor_AudioBuffer_current[i] >> 8) & 0xFF;
			USB_AUDIO_rx_buffer_current[i * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET + 1] = (Processor_AudioBuffer_current[i] >> 16) & 0xFF;
			USB_AUDIO_rx_buffer_current[i * BYTES_IN_SAMPLE_AUDIO_OUT_PACKET + 2] = (Processor_AudioBuffer_current[i] >> 24) & 0xFF;
		}
		USB_AUDIO_need_rx_buffer = false;
	}

	//OUT Volume
	float32_t volume_gain_new = volume2rate((float32_t)TRX_Volume / 1023.0f);
	volume_gain = 0.9f * volume_gain + 0.1f * volume_gain_new;
	for (uint_fast16_t i = 0; i < AUDIO_BUFFER_SIZE; i++)
	{
		Processor_AudioBuffer_current[i] = (int32_t)((float32_t)Processor_AudioBuffer_current[i] * volume_gain);
		Processor_AudioBuffer_current[i] = convertToSPIBigEndian(Processor_AudioBuffer_current[i]); //for 32bit audio
	}

	//Beep signal
	if (WM8731_Beeping)
	{
		float32_t signal = 0;
		int32_t out = 0;
		float32_t amplitude = volume2rate((float32_t)TRX_Volume / 1023.0f) * 0.1f;
		for (uint32_t pos = 0; pos < AUDIO_BUFFER_HALF_SIZE; pos++)
		{
			signal = generateSin(amplitude, pos, TRX_SAMPLERATE, 1500);
			arm_float_to_q31(&signal, &out, 1);
			Processor_AudioBuffer_current[pos * 2] = convertToSPIBigEndian(out);				 //left channel
			Processor_AudioBuffer_current[pos * 2 + 1] = Processor_AudioBuffer_current[pos * 2]; //right channel
		}
	}

	//Mute codec
	if (WM8731_Muting)
	{
		for (uint32_t pos = 0; pos < AUDIO_BUFFER_HALF_SIZE; pos++)
		{
			Processor_AudioBuffer_current[pos * 2] = 0;		//left channel
			Processor_AudioBuffer_current[pos * 2 + 1] = 0; //right channel
		}
	}

	//Send to Codec DMA
	if (TRX_Inited)
	{
		Aligned_CleanDCache_by_Addr((uint32_t *)&Processor_AudioBuffer_current[0], sizeof(Processor_AudioBuffer_A));
		if (WM8731_DMA_state) //complete
		{
			HAL_MDMA_Start_IT(&hmdma_mdma_channel41_sw_0, (uint32_t)&Processor_AudioBuffer_current[0], (uint32_t)&CODEC_Audio_Buffer_RX[AUDIO_BUFFER_SIZE], CODEC_AUDIO_BUFFER_HALF_SIZE * 4, 1); //*2 -> left_right
			HAL_MDMA_PollForTransfer(&hmdma_mdma_channel41_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
		else //half
		{
			HAL_MDMA_Start_IT(&hmdma_mdma_channel42_sw_0, (uint32_t)&Processor_AudioBuffer_current[0], (uint32_t)&CODEC_Audio_Buffer_RX[0], CODEC_AUDIO_BUFFER_HALF_SIZE * 4, 1); //*2 -> left_right
			HAL_MDMA_PollForTransfer(&hmdma_mdma_channel42_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
	}

	Processor_NeedRXBuffer = false;
}

// start audio processor for TX
void processTxAudio(void)
{
	if (!Processor_NeedTXBuffer)
		return;
	VFO *current_vfo = CurrentVFO();
	AUDIOPROC_samples++;
	uint_fast8_t mode = current_vfo->Mode;

	// get the amplitude for the selected power and range
	Processor_selected_RFpower_amplitude = log10f_fast(((float32_t)TRX.RF_Power * 0.9f + 10.0f) / 10.0f) * TRX_MAX_TX_Amplitude;
	if (Processor_selected_RFpower_amplitude < 0.0f)
		Processor_selected_RFpower_amplitude = 0.0f;

	// zero beats
	if ((TRX_Tune && !TRX.TWO_SIGNAL_TUNE) || (TRX_Tune && (mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U)))
		Processor_selected_RFpower_amplitude = Processor_selected_RFpower_amplitude * 1.0f;

	if (TRX.InputType_USB) //USB AUDIO
	{
		uint32_t buffer_index = USB_AUDIO_GetTXBufferIndex_FS() / BYTES_IN_SAMPLE_AUDIO_OUT_PACKET; //buffer 8bit, data 24 bit
		if ((buffer_index % BYTES_IN_SAMPLE_AUDIO_OUT_PACKET) == 1)
			buffer_index -= (buffer_index % BYTES_IN_SAMPLE_AUDIO_OUT_PACKET);
		readHalfFromCircleUSBBuffer24Bit(&USB_AUDIO_tx_buffer[0], &Processor_AudioBuffer_A[0], buffer_index, (USB_AUDIO_TX_BUFFER_SIZE / BYTES_IN_SAMPLE_AUDIO_OUT_PACKET));
	}
	else //AUDIO CODEC AUDIO
	{
		uint32_t dma_index = CODEC_AUDIO_BUFFER_SIZE - (uint16_t)__HAL_DMA_GET_COUNTER(hi2s3.hdmarx);
		if ((dma_index % 2) == 1)
			dma_index--;
		readFromCircleBuffer32((uint32_t *)&CODEC_Audio_Buffer_TX[0], (uint32_t *)&Processor_AudioBuffer_A[0], dma_index, CODEC_AUDIO_BUFFER_SIZE, AUDIO_BUFFER_SIZE);
	}

	//One-signal zero-tune generator
	if (TRX_Tune && !TRX.TWO_SIGNAL_TUNE)
	{
		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			FPGA_Audio_Buffer_TX_I_tmp[i] = (Processor_selected_RFpower_amplitude / 100.0f * TUNE_POWER);
			FPGA_Audio_Buffer_TX_Q_tmp[i] = 0.0f;
		}
	}

	//Two-signal tune generator
	if (TRX_Tune && TRX.TWO_SIGNAL_TUNE)
	{
		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			float32_t point = generateSin((Processor_selected_RFpower_amplitude / 100.0f * TUNE_POWER) / 2.0f, two_signal_gen_position, TRX_SAMPLERATE, 1000);
			point += generateSin((Processor_selected_RFpower_amplitude / 100.0f * TUNE_POWER) / 2.0f, two_signal_gen_position, TRX_SAMPLERATE, 2000);
			two_signal_gen_position++;
			if (two_signal_gen_position >= TRX_SAMPLERATE)
				two_signal_gen_position = 0;
			FPGA_Audio_Buffer_TX_I_tmp[i] = point;
			FPGA_Audio_Buffer_TX_Q_tmp[i] = point;
		}
		//hilbert fir
		// + 45 deg to Q data
		arm_fir_f32(&FIR_TX_Hilbert_Q, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
		// - 45 deg to I data
		arm_fir_f32(&FIR_TX_Hilbert_I, FPGA_Audio_Buffer_TX_Q_tmp, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
	}

	//FM tone generator
	if (TRX_Tune && (mode == TRX_MODE_NFM || mode == TRX_MODE_WFM))
	{
		static uint32_t tone_counter = 100;
		tone_counter++;
		if (tone_counter >= 400)
			tone_counter = 0;
		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			float32_t point = 0.0f;
			if (tone_counter > 300)
				point = generateSin(1.0f, two_signal_gen_position, TRX_SAMPLERATE, 3500);
			else if (tone_counter > 200)
				point = generateSin(1.0f, two_signal_gen_position, TRX_SAMPLERATE, 2000);
			else if (tone_counter > 100)
				point = generateSin(1.0f, two_signal_gen_position, TRX_SAMPLERATE, 1000);

			two_signal_gen_position++;
			if (two_signal_gen_position >= TRX_SAMPLERATE)
				two_signal_gen_position = 0;

			FPGA_Audio_Buffer_TX_I_tmp[i] = point;
			FPGA_Audio_Buffer_TX_Q_tmp[i] = point;
		}
		ModulateFM(AUDIO_BUFFER_HALF_SIZE);
	}

	if (!TRX_Tune)
	{
		//Copy and convert buffer
		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			FPGA_Audio_Buffer_TX_I_tmp[i] = (float32_t)Processor_AudioBuffer_A[i * 2] / 2147483648.0f;
			FPGA_Audio_Buffer_TX_Q_tmp[i] = (float32_t)Processor_AudioBuffer_A[i * 2 + 1] / 2147483648.0f;
		}

		if (TRX.InputType_MIC)
		{
			//Mic Gain
			arm_scale_f32(FPGA_Audio_Buffer_TX_I_tmp, TRX.MIC_GAIN, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			arm_scale_f32(FPGA_Audio_Buffer_TX_Q_tmp, TRX.MIC_GAIN, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
			//Mic Equalizer
			if (mode != TRX_MODE_DIGI_L && mode != TRX_MODE_DIGI_U && mode != TRX_MODE_IQ)
				doMIC_EQ(AUDIO_BUFFER_HALF_SIZE);
		}
		//USB Gain (24bit)
		if (TRX.InputType_USB)
		{
			arm_scale_f32(FPGA_Audio_Buffer_TX_I_tmp, 10.0f, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			arm_scale_f32(FPGA_Audio_Buffer_TX_Q_tmp, 10.0f, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
		}

		//Process DC corrector filter
		dc_filter(FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE, DC_FILTER_TX_I);
		dc_filter(FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE, DC_FILTER_TX_Q);
	}

	if (mode != TRX_MODE_IQ && !TRX_Tune)
	{
		//IIR HPF
		if (current_vfo->HPF_Filter_Width > 0)
			arm_biquad_cascade_df2T_f32(&IIR_TX_HPF_I, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
		//IIR LPF
		if (current_vfo->LPF_Filter_Width > 0)
			arm_biquad_cascade_df2T_f32(&IIR_TX_LPF_I, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
		memcpy(&FPGA_Audio_Buffer_TX_Q_tmp[0], &FPGA_Audio_Buffer_TX_I_tmp[0], AUDIO_BUFFER_HALF_SIZE * 4); //double left and right channel

		switch (mode)
		{
		case TRX_MODE_CW_L:
		case TRX_MODE_CW_U:
			if (!TRX_key_serial && !TRX_ptt_hard && !TRX_key_dot_hard && !TRX_key_dash_hard)
				Processor_selected_RFpower_amplitude = 0;
			float32_t cw_signal = TRX_GenerateCWSignal(Processor_selected_RFpower_amplitude);
			for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
			{
				FPGA_Audio_Buffer_TX_I_tmp[i] = cw_signal;
				FPGA_Audio_Buffer_TX_Q_tmp[i] = 0.0f;
			}
			break;
		case TRX_MODE_USB:
		case TRX_MODE_DIGI_U:
			//hilbert fir
			// + 45 deg to Q data
			arm_fir_f32(&FIR_TX_Hilbert_Q, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			// - 45 deg to I data
			arm_fir_f32(&FIR_TX_Hilbert_I, FPGA_Audio_Buffer_TX_Q_tmp, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
			break;
		case TRX_MODE_LSB:
		case TRX_MODE_DIGI_L:
			//hilbert fir
			// + 45 deg to I data
			arm_fir_f32(&FIR_TX_Hilbert_I, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			// - 45 deg to Q data
			arm_fir_f32(&FIR_TX_Hilbert_Q, FPGA_Audio_Buffer_TX_Q_tmp, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
			break;
		case TRX_MODE_AM:
			// + 45 deg to I data
			arm_fir_f32(&FIR_TX_Hilbert_I, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
			// - 45 deg to Q data
			arm_fir_f32(&FIR_TX_Hilbert_Q, FPGA_Audio_Buffer_TX_Q_tmp, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);
			for (size_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
			{
				float32_t i_am = ((FPGA_Audio_Buffer_TX_I_tmp[i] - FPGA_Audio_Buffer_TX_Q_tmp[i]) + (Processor_selected_RFpower_amplitude)) / 2.0f;
				float32_t q_am = ((FPGA_Audio_Buffer_TX_Q_tmp[i] - FPGA_Audio_Buffer_TX_I_tmp[i]) - (Processor_selected_RFpower_amplitude)) / 2.0f;
				FPGA_Audio_Buffer_TX_I_tmp[i] = i_am;
				FPGA_Audio_Buffer_TX_Q_tmp[i] = q_am;
			}
			break;
		case TRX_MODE_NFM:
		case TRX_MODE_WFM:
			ModulateFM(AUDIO_BUFFER_HALF_SIZE);
			break;
		case TRX_MODE_LOOPBACK:
			DECODER_PutSamples(FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE); //отправляем данные в цифровой декодер
			break;
		default:
			break;
		}
	}

	//// RF PowerControl (Audio Level Control) Compressor
	// looking for a maximum in amplitude
	float32_t ampl_max_i = 0.0f;
	float32_t ampl_max_q = 0.0f;
	float32_t ampl_min_i = 0.0f;
	float32_t ampl_min_q = 0.0f;
	uint32_t tmp_index;
	arm_max_f32(FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE, &ampl_max_i, &tmp_index);
	arm_max_f32(FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE, &ampl_max_q, &tmp_index);
	arm_min_f32(FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE, &ampl_min_i, &tmp_index);
	arm_min_f32(FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE, &ampl_min_q, &tmp_index);
	float32_t Processor_TX_MAX_amplitude_IN = ampl_max_i;
	if (ampl_max_q > Processor_TX_MAX_amplitude_IN)
		Processor_TX_MAX_amplitude_IN = ampl_max_q;
	if ((-ampl_min_i) > Processor_TX_MAX_amplitude_IN)
		Processor_TX_MAX_amplitude_IN = -ampl_min_i;
	if ((-ampl_min_q) > Processor_TX_MAX_amplitude_IN)
		Processor_TX_MAX_amplitude_IN = -ampl_min_q;

	// calculate the target gain
	if (Processor_TX_MAX_amplitude_IN > 0.0f)
	{
		ALC_need_gain_target = (Processor_selected_RFpower_amplitude * 0.99f) / Processor_TX_MAX_amplitude_IN;
		// move the gain one step
		if (fabsf(ALC_need_gain_target - ALC_need_gain) > 0.00001f) // hysteresis
		{
			if (ALC_need_gain_target > ALC_need_gain)
			{
				if (mode == TRX_MODE_DIGI_L || mode == TRX_MODE_DIGI_U) // FAST AGC
					ALC_need_gain = (ALC_need_gain * (1.0f - (float32_t)TRX.TX_AGC_speed / 30.0f)) + (ALC_need_gain_target * ((float32_t)TRX.TX_AGC_speed / 30.0f));
				else // SLOW AGC
					ALC_need_gain = (ALC_need_gain * (1.0f - (float32_t)TRX.TX_AGC_speed / 1000.0f)) + (ALC_need_gain_target * ((float32_t)TRX.TX_AGC_speed / 1000.0f));
			}
		}
		//just in case
		if (ALC_need_gain < 0.0f)
			ALC_need_gain = 0.0f;
		// overload (clipping), sharply reduce the gain
		if ((ALC_need_gain * Processor_TX_MAX_amplitude_IN) > (Processor_selected_RFpower_amplitude * 1.0f))
		{
			ALC_need_gain = ALC_need_gain_target;
			// sendToDebug_str ("MIC_CLIP");
		}
		if (ALC_need_gain > TX_AGC_MAXGAIN)
			ALC_need_gain = TX_AGC_MAXGAIN;
		// noise threshold
		if (Processor_TX_MAX_amplitude_IN < TX_AGC_NOISEGATE)
			ALC_need_gain = 0.0f;
	}
	// disable gain for some types of mod
	if ((ALC_need_gain > 1.0f) && (mode == TRX_MODE_LOOPBACK))
		ALC_need_gain = 1.0f;
	if (mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U || mode == TRX_MODE_NFM || mode == TRX_MODE_WFM)
		ALC_need_gain = 1.0f;
	if (TRX_Tune)
		ALC_need_gain = 1.0f;

	// apply gain
	arm_scale_f32(FPGA_Audio_Buffer_TX_I_tmp, ALC_need_gain, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);
	arm_scale_f32(FPGA_Audio_Buffer_TX_Q_tmp, ALC_need_gain, FPGA_Audio_Buffer_TX_Q_tmp, AUDIO_BUFFER_HALF_SIZE);

	Processor_TX_MAX_amplitude_OUT = Processor_TX_MAX_amplitude_IN * ALC_need_gain;
	if (Processor_selected_RFpower_amplitude > 0.0f)
		TRX_ALC = Processor_TX_MAX_amplitude_OUT / Processor_selected_RFpower_amplitude;
	else
		TRX_ALC = 0.0f;
	//RF PowerControl (Audio Level Control) Compressor END

	//Send TX data to FFT
	for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
	{
		if (NeedFFTInputBuffer)
		{
			FFTInput_I[FFT_buff_index] = FPGA_Audio_Buffer_TX_I_tmp[i];
			FFTInput_Q[FFT_buff_index] = FPGA_Audio_Buffer_TX_Q_tmp[i];
			FFT_buff_index++;
			if (FFT_buff_index >= FFT_SIZE)
			{
				FFT_buff_index = 0;
				NeedFFTInputBuffer = false;
				FFT_buffer_ready = true;
			}
		}
	}

	//Loopback mode
	if (mode == TRX_MODE_LOOPBACK && !TRX_Tune)
	{
		//OUT Volume
		float32_t volume_gain_tx = volume2rate((float32_t)TRX_Volume / 1023.0f);
		arm_scale_f32(FPGA_Audio_Buffer_TX_I_tmp, volume_gain_tx, FPGA_Audio_Buffer_TX_I_tmp, AUDIO_BUFFER_HALF_SIZE);

		for (uint_fast16_t i = 0; i < AUDIO_BUFFER_HALF_SIZE; i++)
		{
			arm_float_to_q31(&FPGA_Audio_Buffer_TX_I_tmp[i], &Processor_AudioBuffer_A[i * 2], 1);
			Processor_AudioBuffer_A[i * 2] = convertToSPIBigEndian(Processor_AudioBuffer_A[i * 2]); //left channel
			Processor_AudioBuffer_A[i * 2 + 1] = Processor_AudioBuffer_A[i * 2];					//right channel
		}

		Aligned_CleanDCache_by_Addr((uint32_t *)&Processor_AudioBuffer_A[0], sizeof(Processor_AudioBuffer_A));
		if (WM8731_DMA_state) //compleate
		{
			HAL_MDMA_Start(&hmdma_mdma_channel41_sw_0, (uint32_t)&Processor_AudioBuffer_A[0], (uint32_t)&CODEC_Audio_Buffer_RX[AUDIO_BUFFER_SIZE], AUDIO_BUFFER_SIZE * 4, 1);
			HAL_MDMA_PollForTransfer(&hmdma_mdma_channel41_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
		else //half
		{
			HAL_MDMA_Start(&hmdma_mdma_channel42_sw_0, (uint32_t)&Processor_AudioBuffer_A[0], (uint32_t)&CODEC_Audio_Buffer_RX[0], AUDIO_BUFFER_SIZE * 4, 1);
			HAL_MDMA_PollForTransfer(&hmdma_mdma_channel42_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
	}
	else
	{
		//CW SelfHear
		if (TRX.CW_SelfHear && (TRX_key_serial || TRX_key_dot_hard || TRX_key_dash_hard) && (mode == TRX_MODE_CW_L || mode == TRX_MODE_CW_U) && !TRX_Tune)
		{
			if (Processor_TX_MAX_amplitude_IN > 0)
			{
				float32_t volume_gain_tx = volume2rate((float32_t)TRX_Volume / 1023.0f);
				float32_t amplitude = (db2rateV(TRX.AGC_GAIN_TARGET) * volume_gain_tx * CODEC_BITS_FULL_SCALE / 2.0f);
				for (uint_fast16_t i = 0; i < AUDIO_BUFFER_SIZE; i++)
				{
					CODEC_Audio_Buffer_RX[i * 2] = convertToSPIBigEndian((int32_t)(amplitude * arm_sin_f32(((float32_t)i / (float32_t)TRX_SAMPLERATE) * PI * 2.0f * (float32_t)TRX.CW_GENERATOR_SHIFT_HZ)));
					CODEC_Audio_Buffer_RX[i * 2 + 1] = CODEC_Audio_Buffer_RX[i * 2];
				}
				Aligned_CleanDCache_by_Addr((uint32_t *)&CODEC_Audio_Buffer_RX[0], sizeof(CODEC_Audio_Buffer_RX));
			}
			else
			{
				memset(CODEC_Audio_Buffer_RX, 0x00, sizeof CODEC_Audio_Buffer_RX);
				Aligned_CleanDCache_by_Addr((uint32_t *)&CODEC_Audio_Buffer_RX[0], sizeof(CODEC_Audio_Buffer_RX));
			}
		}
		//
		Aligned_CleanDCache_by_Addr((uint32_t *)&FPGA_Audio_Buffer_TX_I_tmp[0], sizeof(FPGA_Audio_Buffer_TX_I_tmp));
		Aligned_CleanDCache_by_Addr((uint32_t *)&FPGA_Audio_Buffer_TX_Q_tmp[0], sizeof(FPGA_Audio_Buffer_TX_Q_tmp));
		if (FPGA_Audio_Buffer_State) //Send to FPGA DMA
		{
			HAL_MDMA_Start(&hmdma_mdma_channel41_sw_0, (uint32_t)&FPGA_Audio_Buffer_TX_I_tmp[0], (uint32_t)&FPGA_Audio_SendBuffer_I[AUDIO_BUFFER_HALF_SIZE], AUDIO_BUFFER_HALF_SIZE * 4, 1);
			HAL_MDMA_PollForTransfer(&hmdma_mdma_channel41_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
			HAL_MDMA_Start(&hmdma_mdma_channel41_sw_0, (uint32_t)&FPGA_Audio_Buffer_TX_Q_tmp[0], (uint32_t)&FPGA_Audio_SendBuffer_Q[AUDIO_BUFFER_HALF_SIZE], AUDIO_BUFFER_HALF_SIZE * 4, 1);
			HAL_MDMA_PollForTransfer(&hmdma_mdma_channel41_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
		else
		{
			HAL_MDMA_Start(&hmdma_mdma_channel42_sw_0, (uint32_t)&FPGA_Audio_Buffer_TX_I_tmp[0], (uint32_t)&FPGA_Audio_SendBuffer_I[0], AUDIO_BUFFER_HALF_SIZE * 4, 1);
			HAL_MDMA_PollForTransfer(&hmdma_mdma_channel42_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
			HAL_MDMA_Start(&hmdma_mdma_channel42_sw_0, (uint32_t)&FPGA_Audio_Buffer_TX_Q_tmp[0], (uint32_t)&FPGA_Audio_SendBuffer_Q[0], AUDIO_BUFFER_HALF_SIZE * 4, 1);
			HAL_MDMA_PollForTransfer(&hmdma_mdma_channel42_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
		}
		Aligned_InvalidateDCache_by_Addr((uint32_t *)&FPGA_Audio_SendBuffer_I[0], sizeof(FPGA_Audio_SendBuffer_I));
		Aligned_InvalidateDCache_by_Addr((uint32_t *)&FPGA_Audio_SendBuffer_Q[0], sizeof(FPGA_Audio_SendBuffer_Q));
	}

	Processor_NeedTXBuffer = false;
	Processor_NeedRXBuffer = false;
	USB_AUDIO_need_rx_buffer = false;
}

// Hilbert filter for phase shift of signals
static void doRX_HILBERT(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	if (rx_id == AUDIO_RX1)
	{
		arm_fir_f32(&FIR_RX1_Hilbert_I, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, size);
		arm_fir_f32(&FIR_RX1_Hilbert_Q, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, size);
	}
	else
	{
		arm_fir_f32(&FIR_RX2_Hilbert_I, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, size);
		arm_fir_f32(&FIR_RX2_Hilbert_Q, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, size);
	}
}

// Low-pass filter for I and Q
static void doRX_LPF_IQ(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	if (rx_id == AUDIO_RX1)
	{
		if (CurrentVFO()->LPF_Filter_Width > 0)
		{
			arm_biquad_cascade_df2T_f32(&IIR_RX1_LPF_I, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, size);
			arm_biquad_cascade_df2T_f32(&IIR_RX1_LPF_Q, FPGA_Audio_Buffer_RX1_Q_tmp, FPGA_Audio_Buffer_RX1_Q_tmp, size);
		}
	}
	else
	{
		if (SecondaryVFO()->LPF_Filter_Width > 0)
		{
			arm_biquad_cascade_df2T_f32(&IIR_RX2_LPF_I, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, size);
			arm_biquad_cascade_df2T_f32(&IIR_RX2_LPF_Q, FPGA_Audio_Buffer_RX2_Q_tmp, FPGA_Audio_Buffer_RX2_Q_tmp, size);
		}
	}
}

// LPF filter for I
static void doRX_LPF_I(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	if (rx_id == AUDIO_RX1)
	{
		if (CurrentVFO()->LPF_Filter_Width > 0)
		{
			arm_biquad_cascade_df2T_f32(&IIR_RX1_LPF_I, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, size);
		}
	}
	else
	{
		if (SecondaryVFO()->LPF_Filter_Width > 0)
		{
			arm_biquad_cascade_df2T_f32(&IIR_RX2_LPF_I, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, size);
		}
	}
}

// Gauss filter for I
static void doRX_GAUSS_I(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	if (!TRX.CW_GaussFilter)
		return;
	if (rx_id == AUDIO_RX1)
	{
		if (CurrentVFO()->Mode == TRX_MODE_CW_L || CurrentVFO()->Mode == TRX_MODE_CW_U)
		{
			arm_biquad_cascade_df2T_f32(&IIR_RX1_GAUSS, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, size);
		}
	}
	else
	{
		if (SecondaryVFO()->Mode == TRX_MODE_CW_L || SecondaryVFO()->Mode == TRX_MODE_CW_U)
		{
			arm_biquad_cascade_df2T_f32(&IIR_RX2_GAUSS, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, size);
		}
	}
}

// HPF filter for I
static void doRX_HPF_I(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	if (rx_id == AUDIO_RX1)
	{
		if (CurrentVFO()->HPF_Filter_Width > 0)
		{
			arm_biquad_cascade_df2T_f32(&IIR_RX1_HPF_I, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, size);
		}
	}
	else
	{
		if (SecondaryVFO()->HPF_Filter_Width > 0)
		{
			arm_biquad_cascade_df2T_f32(&IIR_RX2_HPF_I, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, size);
		}
	}
}

// notch filter
static void doRX_NOTCH(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	if (rx_id == AUDIO_RX1)
	{
		if (CurrentVFO()->ManualNotchFilter) // manual filter
			arm_biquad_cascade_df2T_f32(&NOTCH_RX1_FILTER, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, size);
		else if (CurrentVFO()->AutoNotchFilter) // automatic filter
		{
			for (uint32_t block = 0; block < (size / AUTO_NOTCH_BLOCK_SIZE); block++)
				processAutoNotchReduction(FPGA_Audio_Buffer_RX1_I_tmp + (block * AUTO_NOTCH_BLOCK_SIZE), rx_id);
		}
	}
	else
	{
		if (SecondaryVFO()->ManualNotchFilter)
			arm_biquad_cascade_df2T_f32(&NOTCH_RX2_FILTER, FPGA_Audio_Buffer_RX2_I_tmp, FPGA_Audio_Buffer_RX2_I_tmp, size);
		else if (SecondaryVFO()->AutoNotchFilter)
		{
			for (uint32_t block = 0; block < (size / AUTO_NOTCH_BLOCK_SIZE); block++)
				processAutoNotchReduction(FPGA_Audio_Buffer_RX2_I_tmp + (block * AUTO_NOTCH_BLOCK_SIZE), rx_id);
		}
	}
}

// RX Equalizer
static void doRX_EQ(uint16_t size)
{
	if (TRX.RX_EQ_LOW != 0)
		arm_biquad_cascade_df2T_f32(&EQ_RX_LOW_FILTER, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, size);
	if (TRX.RX_EQ_MID != 0)
		arm_biquad_cascade_df2T_f32(&EQ_RX_MID_FILTER, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, size);
	if (TRX.RX_EQ_HIG != 0)
		arm_biquad_cascade_df2T_f32(&EQ_RX_HIG_FILTER, FPGA_Audio_Buffer_RX1_I_tmp, FPGA_Audio_Buffer_RX1_I_tmp, size);
}

// Equalizer microphone
static void doMIC_EQ(uint16_t size)
{
	if (TRX.MIC_EQ_LOW != 0)
		arm_biquad_cascade_df2T_f32(&EQ_MIC_LOW_FILTER, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, size);
	if (TRX.MIC_EQ_MID != 0)
		arm_biquad_cascade_df2T_f32(&EQ_MIC_MID_FILTER, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, size);
	if (TRX.MIC_EQ_HIG != 0)
		arm_biquad_cascade_df2T_f32(&EQ_MIC_HIG_FILTER, FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, size);

	//Reverber
	if (TRX.MIC_REVERBER > 0)
	{
		//reset
		static uint32_t reverber_position = 0;
		if (NeedReinitReverber)
		{
			reverber_position = 0;
			memset(Processor_Reverber_Buffer, 0x00, sizeof(Processor_Reverber_Buffer));
			NeedReinitReverber = false;
		}

		//save
		memcpy(&Processor_Reverber_Buffer[reverber_position * size], FPGA_Audio_Buffer_TX_I_tmp, sizeof(float32_t) * size);

		//move
		reverber_position++;
		if (reverber_position > TRX.MIC_REVERBER)
			reverber_position = 0;

		//apply
		arm_add_f32(&Processor_Reverber_Buffer[reverber_position * size], FPGA_Audio_Buffer_TX_I_tmp, FPGA_Audio_Buffer_TX_I_tmp, size);
		arm_scale_f32(FPGA_Audio_Buffer_TX_I_tmp, 0.5f, FPGA_Audio_Buffer_TX_I_tmp, size);
	}
}

// Digital Noise Reduction
static void doRX_DNR(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	if (rx_id == AUDIO_RX1)
	{
		if (CurrentVFO()->DNR > 0)
		{
			for (uint32_t block = 0; block < (size / NOISE_REDUCTION_BLOCK_SIZE); block++)
				processNoiseReduction(FPGA_Audio_Buffer_RX1_I_tmp + (block * NOISE_REDUCTION_BLOCK_SIZE), rx_id);
		}
	}
	else
	{
		if (SecondaryVFO()->DNR > 0)
		{
			for (uint32_t block = 0; block < (size / NOISE_REDUCTION_BLOCK_SIZE); block++)
				processNoiseReduction(FPGA_Audio_Buffer_RX2_I_tmp + (block * NOISE_REDUCTION_BLOCK_SIZE), rx_id);
		}
	}
}

// automatic gain control
static void doRX_AGC(AUDIO_PROC_RX_NUM rx_id, uint16_t size, uint_fast8_t mode)
{
	if (rx_id == AUDIO_RX1)
		DoRxAGC(FPGA_Audio_Buffer_RX1_I_tmp, size, rx_id, mode);
	else
		DoRxAGC(FPGA_Audio_Buffer_RX2_I_tmp, size, rx_id, mode);
}

// impulse noise suppressor
static void doRX_NoiseBlanker(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	if (!TRX.NOISE_BLANKER)
		return;
	if (rx_id == AUDIO_RX1)
	{
		for (uint32_t block = 0; block < (size / NB_BLOCK_SIZE); block++)
			processNoiseBlanking(FPGA_Audio_Buffer_RX1_I_tmp + (block * NB_BLOCK_SIZE), rx_id);
	}
	else
	{
		for (uint32_t block = 0; block < (size / NB_BLOCK_SIZE); block++)
			processNoiseBlanking(FPGA_Audio_Buffer_RX2_I_tmp + (block * NB_BLOCK_SIZE), rx_id);
	}
}

// s-meter
static void doRX_SMETER(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	if (Processor_RX_Power_value != 0)
		return;
	// Prepare data to calculate s-meter
	static float32_t i = 0;
	if (rx_id == AUDIO_RX1)
	{
		arm_rms_f32(FPGA_Audio_Buffer_RX1_I_tmp, size, &i);
		if (current_if_gain != 0.0f)
			i *= 1.0f / current_if_gain;
	}
	else
	{
		arm_rms_f32(FPGA_Audio_Buffer_RX2_I_tmp, size, &i);
	}
	Processor_RX_Power_value = i;
}

// copy I to Q channel
static void doRX_COPYCHANNEL(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	// Double channel I-> Q
	if (rx_id == AUDIO_RX1)
	{
		Aligned_CleanDCache_by_Addr((uint32_t *)&FPGA_Audio_Buffer_RX1_I_tmp[0], size * 4);
		dma_memcpy32((uint32_t *)&FPGA_Audio_Buffer_RX1_Q_tmp[0], (uint32_t *)&FPGA_Audio_Buffer_RX1_I_tmp[0], size);
		Aligned_InvalidateDCache_by_Addr((uint32_t *)&FPGA_Audio_Buffer_RX1_Q_tmp[0], size * 4);
	}
	else
	{
		Aligned_CleanDCache_by_Addr((uint32_t *)&FPGA_Audio_Buffer_RX2_I_tmp[0], size * 4);
		dma_memcpy32((uint32_t *)&FPGA_Audio_Buffer_RX2_Q_tmp[0], (uint32_t *)&FPGA_Audio_Buffer_RX2_I_tmp[0], size);
		Aligned_InvalidateDCache_by_Addr((uint32_t *)&FPGA_Audio_Buffer_RX2_Q_tmp[0], size * 4);
	}
}

// FM demodulator
static void DemodulateFM(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	float32_t *i_prev = &DFM_RX1_i_prev;
	float32_t *q_prev = &DFM_RX1_q_prev;
	uint_fast8_t *fm_sql_count = &DFM_RX1_fm_sql_count;
	float32_t *FPGA_Audio_Buffer_I_tmp = &FPGA_Audio_Buffer_RX1_I_tmp[0];
	float32_t *FPGA_Audio_Buffer_Q_tmp = &FPGA_Audio_Buffer_RX1_Q_tmp[0];
	float32_t *fm_sql_avg = &DFM_RX1_fm_sql_avg;
	arm_biquad_cascade_df2T_instance_f32 *iir_filter_inst = &IIR_RX1_Squelch_HPF;
	bool *squelched = &DFM_RX1_Squelched;

	float32_t angle, x, y, b;
	static float32_t squelch_buf[FPGA_RX_IQ_BUFFER_HALF_SIZE];

	if (rx_id == AUDIO_RX2)
	{
		i_prev = &DFM_RX2_i_prev;
		q_prev = &DFM_RX2_q_prev;
		fm_sql_count = &DFM_RX2_fm_sql_count;
		fm_sql_avg = &DFM_RX2_fm_sql_avg;
		iir_filter_inst = &IIR_RX2_Squelch_HPF;
		FPGA_Audio_Buffer_I_tmp = &FPGA_Audio_Buffer_RX2_I_tmp[0];
		FPGA_Audio_Buffer_Q_tmp = &FPGA_Audio_Buffer_RX2_Q_tmp[0];
		squelched = &DFM_RX2_Squelched;
	}

	for (uint_fast16_t i = 0; i < size; i++)
	{
		// first, calculate "x" and "y" for the arctan2, comparing the vectors of present data with previous data
		y = (FPGA_Audio_Buffer_Q_tmp[i] * *i_prev) - (FPGA_Audio_Buffer_I_tmp[i] * *q_prev);
		x = (FPGA_Audio_Buffer_I_tmp[i] * *i_prev) + (FPGA_Audio_Buffer_Q_tmp[i] * *q_prev);
		angle = atan2f(y, x);

		// we now have our audio in "angle"
		squelch_buf[i] = angle; // save audio in "d" buffer for squelch noise filtering/detection - done later

		*q_prev = FPGA_Audio_Buffer_Q_tmp[i]; // save "previous" value of each channel to allow detection of the change of angle in next go-around
		*i_prev = FPGA_Audio_Buffer_I_tmp[i];

		if ((!*squelched) || (!TRX.FM_SQL_threshold)) // high-pass audio only if we are un-squelched (to save processor time)
		{
			if (CurrentVFO()->Mode == TRX_MODE_WFM)
			{
				FPGA_Audio_Buffer_I_tmp[i] = (float32_t)(angle / F_PI) * 0.1f; //second way
				//fm de emphasis
				static float32_t avg = 0.0f;
				float32_t d = FPGA_Audio_Buffer_I_tmp[i] - avg;
				if (d > 0)
				{
					avg += (d + deemph_a / 2) / deemph_a;
				}
				else
				{
					avg += (d - deemph_a / 2) / deemph_a;
				}
				FPGA_Audio_Buffer_I_tmp[i] = avg;
			}
			else
			{
				FPGA_Audio_Buffer_I_tmp[i] = (float32_t)(angle / F_PI) * 0.1f; //second way
			}
		}
		else if (*squelched)				// were we squelched or tone NOT detected?
			FPGA_Audio_Buffer_I_tmp[i] = 0; // do not filter receive audio - fill buffer with zeroes to mute it
	}

	// *** Squelch Processing ***
	arm_biquad_cascade_df2T_f32(iir_filter_inst, squelch_buf, squelch_buf, size);									   // Do IIR high-pass filter on audio so we may detect squelch noise energy
	*fm_sql_avg = ((1.0f - FM_RX_SQL_SMOOTHING) * *fm_sql_avg) + (FM_RX_SQL_SMOOTHING * sqrtf(fabsf(squelch_buf[0]))); // IIR filter squelch energy magnitude:  We need look at only one representative sample

	*fm_sql_count = *fm_sql_count + 1; // bump count that controls how often the squelch threshold is checked
	if (*fm_sql_count >= FM_SQUELCH_PROC_DECIMATION)
		*fm_sql_count = 0; // enforce the count limit

	// Determine if the (averaged) energy in "ads.fm_sql_avg" is above or below the squelch threshold
	if (*fm_sql_count == 0) // do the squelch threshold calculation much less often than we are called to process this audio
	{
		if (*fm_sql_avg > 0.7f) // limit maximum noise value in averaging to keep it from going out into the weeds under no-signal conditions (higher = noisier)
			*fm_sql_avg = 0.7f;
		b = *fm_sql_avg * 10.0f; // scale noise amplitude to range of squelch setting

		// Now evaluate noise power with respect to squelch setting
		if (!TRX.FM_SQL_threshold) // is squelch set to zero?
			*squelched = false;	   // yes, the we are un-squelched
		else if (*squelched)	   // are we squelched?
		{
			if (b <= (float)((10 - TRX.FM_SQL_threshold) - FM_SQUELCH_HYSTERESIS)) // yes - is average above threshold plus hysteresis?
				*squelched = false;												   //  yes, open the squelch
		}
		else // is the squelch open (e.g. passing audio)?
		{
			if ((10.0f - TRX.FM_SQL_threshold) > FM_SQUELCH_HYSTERESIS) // is setting higher than hysteresis?
			{
				if (b > (float)((10 - TRX.FM_SQL_threshold) + FM_SQUELCH_HYSTERESIS)) // yes - is average below threshold minus hysteresis?
					*squelched = true;												  // yes, close the squelch
			}
			else // setting is lower than hysteresis so we can't use it!
			{
				if (b > (10.0f - (float)TRX.FM_SQL_threshold)) // yes - is average below threshold?
					*squelched = true;						   // yes, close the squelch
			}
		}
	}
}

// FM modulator
static void ModulateFM(uint16_t size)
{
	static float32_t modulation = (float32_t)TRX_SAMPLERATE;
	static float32_t hpf_prev_a = 0;
	static float32_t hpf_prev_b = 0;
	static float32_t sin_data = 0;
	static float32_t fm_mod_accum = 0;
	static float32_t modulation_index = 15000.0f;
	if (CurrentVFO()->LPF_Filter_Width == 5000)
		modulation_index = 4000.0f;
	if (CurrentVFO()->LPF_Filter_Width == 6000)
		modulation_index = 6000.0f;
	if (CurrentVFO()->LPF_Filter_Width == 7000)
		modulation_index = 8000.0f;
	if (CurrentVFO()->LPF_Filter_Width == 8000)
		modulation_index = 11000.0f;
	if (CurrentVFO()->LPF_Filter_Width == 9000)
		modulation_index = 13000.0f;
	if (CurrentVFO()->LPF_Filter_Width == 10000)
		modulation_index = 15000.0f;
	if (CurrentVFO()->LPF_Filter_Width == 15000)
		modulation_index = 30000.0f;
	if (CurrentVFO()->LPF_Filter_Width == 20000)
		modulation_index = 40000.0f;
	if (CurrentVFO()->LPF_Filter_Width == 0)
		modulation_index = 45000.0f;

	// Do differentiating high-pass filter to provide 6dB/octave pre-emphasis - which also removes any DC component!
	float32_t ampl = (Processor_selected_RFpower_amplitude / 100.0f * TUNE_POWER);
	for (uint_fast16_t i = 0; i < size; i++)
	{
		hpf_prev_b = FM_TX_HPF_ALPHA * (hpf_prev_b + FPGA_Audio_Buffer_TX_I_tmp[i] - hpf_prev_a); // do differentiation
		hpf_prev_a = FPGA_Audio_Buffer_TX_I_tmp[i];												  // save "[n-1] samples for next iteration
		fm_mod_accum = (1.0f - 0.999f) * fm_mod_accum + 0.999f * hpf_prev_b;					  // save differentiated data in audio buffer // change frequency using scaled audio
		while (fm_mod_accum > modulation)
			fm_mod_accum -= modulation; // limit range
		while (fm_mod_accum < -modulation)
			fm_mod_accum += modulation; // limit range
		sin_data = ((fm_mod_accum * modulation_index) / modulation) * PI;
		FPGA_Audio_Buffer_TX_I_tmp[i] = ampl * arm_sin_f32(sin_data);
		FPGA_Audio_Buffer_TX_Q_tmp[i] = ampl * arm_cos_f32(sin_data);
	}
}

// voice activity detector
static void doVAD(uint16_t size)
{
	for (uint32_t block = 0; block < (size / VAD_BLOCK_SIZE); block++)
		processVAD(FPGA_Audio_Buffer_RX1_I_tmp + (block * VAD_BLOCK_SIZE));
}

// Apply IF Gain IF Gain
static void doRX_IFGain(AUDIO_PROC_RX_NUM rx_id, uint16_t size)
{
	float32_t if_gain = db2rateV(TRX.IF_Gain);
	float32_t minVal = 0;
	float32_t maxVal = 0;
	uint32_t index = 0;

	if (rx_id == AUDIO_RX1)
	{
		//overflow protect
		arm_min_f32(FPGA_Audio_Buffer_RX1_I_tmp, AUTO_NOTCH_BLOCK_SIZE, &minVal, &index);
		arm_max_no_idx_f32(FPGA_Audio_Buffer_RX1_I_tmp, AUTO_NOTCH_BLOCK_SIZE, &maxVal);
		while ((minVal * if_gain) < -1.0f || (maxVal * if_gain) > 1.0f)
			if_gain -= 0.1f;
		arm_min_f32(FPGA_Audio_Buffer_RX1_Q_tmp, AUTO_NOTCH_BLOCK_SIZE, &minVal, &index);
		arm_max_no_idx_f32(FPGA_Audio_Buffer_RX1_Q_tmp, AUTO_NOTCH_BLOCK_SIZE, &maxVal);
		while ((minVal * if_gain) < -1.0f || (maxVal * if_gain) > 1.0f)
			if_gain -= 0.1f;

		//apply gain
		arm_scale_f32(FPGA_Audio_Buffer_RX1_I_tmp, if_gain, FPGA_Audio_Buffer_RX1_I_tmp, size);
		arm_scale_f32(FPGA_Audio_Buffer_RX1_Q_tmp, if_gain, FPGA_Audio_Buffer_RX1_Q_tmp, size);
		current_if_gain = if_gain;
	}
	else
	{
		//overflow protect RX2
		arm_min_f32(FPGA_Audio_Buffer_RX2_I_tmp, AUTO_NOTCH_BLOCK_SIZE, &minVal, &index);
		arm_max_no_idx_f32(FPGA_Audio_Buffer_RX2_I_tmp, AUTO_NOTCH_BLOCK_SIZE, &maxVal);
		while ((minVal * if_gain) < -1.0f || (maxVal * if_gain) > 1.0f)
			if_gain -= 0.1f;
		arm_min_f32(FPGA_Audio_Buffer_RX2_Q_tmp, AUTO_NOTCH_BLOCK_SIZE, &minVal, &index);
		arm_max_no_idx_f32(FPGA_Audio_Buffer_RX2_Q_tmp, AUTO_NOTCH_BLOCK_SIZE, &maxVal);
		while ((minVal * if_gain) < -1.0f || (maxVal * if_gain) > 1.0f)
			if_gain -= 0.1f;

		//apply gain
		arm_scale_f32(FPGA_Audio_Buffer_RX2_I_tmp, if_gain, FPGA_Audio_Buffer_RX2_I_tmp, size);
		arm_scale_f32(FPGA_Audio_Buffer_RX2_Q_tmp, if_gain, FPGA_Audio_Buffer_RX2_Q_tmp, size);
	}
}
