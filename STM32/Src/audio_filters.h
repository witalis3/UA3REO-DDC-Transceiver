#ifndef AUDIO_FILTERS_h
#define AUDIO_FILTERS_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "fpga.h"
#include "functions.h"

#define IIR_FILTERS_COUNT 41													  // Total Filters In The Collection
#define IQ_HILBERT_TAPS 201														  // Hilbert filter order
#define IIR_MAX_STAGES 15														  // Maximum order of IIR filters
#define NOTCH_STAGES 1															  // order of manual Notch filter
#define EQ_STAGES 1																  // order of the biquad of the equalizer filter
#define BIQUAD_COEFF_IN_STAGE 5													  // coefficients in manual Notch filter order
#define FIR_RX1_HILBERT_STATE_SIZE (IQ_HILBERT_TAPS + AUDIO_BUFFER_HALF_SIZE - 1) // size of state buffers
#define FIR_RX2_HILBERT_STATE_SIZE (IQ_HILBERT_TAPS + AUDIO_BUFFER_HALF_SIZE - 1)
#define FIR_TX_HILBERT_STATE_SIZE (IQ_HILBERT_TAPS + AUDIO_BUFFER_HALF_SIZE - 1)
#define IIR_RX1_LPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX2_LPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX1_GAUSS_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX2_GAUSS_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_TX_LPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX1_HPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX2_HPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_TX_HPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX1_HPF_SQL_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX2_HPF_SQL_STATE_SIZE (IIR_MAX_STAGES * 2)

#define MAX_HPF_WIDTH_CW 100
#define MAX_HPF_WIDTH_SSB 500
#define MAX_LPF_WIDTH_CW 2100
#define MAX_LPF_WIDTH_SSB 3400
#define MAX_LPF_WIDTH_AM 10000
#define MAX_LPF_WIDTH_NFM 20000

typedef enum // BiQuad filter type for automatic calculation
{
	BIQUAD_onepolelp,
	BIQUAD_onepolehp,
	BIQUAD_lowpass,
	BIQUAD_highpass,
	BIQUAD_bandpass,
	BIQUAD_notch,
	BIQUAD_peak,
	BIQUAD_lowShelf,
	BIQUAD_highShelf
} BIQUAD_TYPE;

typedef enum // type of filter in the collection
{
	IIR_BIQUAD_HPF,
	IIR_BIQUAD_LPF,
	IIR_BIQUAD_LPF_GAUSS
} IIR_BIQUAD_FILTER_TYPE;

typedef enum // states of DC correctors for each user
{
	DC_FILTER_RX1_I,
	DC_FILTER_RX1_Q,
	DC_FILTER_RX2_I,
	DC_FILTER_RX2_Q,
	DC_FILTER_TX_I,
	DC_FILTER_TX_Q,
	DC_FILTER_FFT_I,
	DC_FILTER_FFT_Q,
} DC_FILTER_STATE;

typedef struct // keep old sample values for DC filter. Multiple states for different consumers
{
	float32_t x_prev;
	float32_t y_prev;
} DC_filter_state_type;

typedef struct // filter in collection
{
	const IIR_BIQUAD_FILTER_TYPE type;
	const float32_t *coeffs; // Coefficients converted to ARMA in reverse order by MATLAB
	const uint8_t stages;
	const uint16_t width;
} IIR_BIQUAD_FILTER;

// Public variables
extern arm_fir_instance_f32 FIR_RX1_Hilbert_I; // filter instances
extern arm_fir_instance_f32 FIR_RX1_Hilbert_Q;
extern arm_fir_instance_f32 FIR_RX2_Hilbert_I;
extern arm_fir_instance_f32 FIR_RX2_Hilbert_Q;
extern arm_fir_instance_f32 FIR_TX_Hilbert_I;
extern arm_fir_instance_f32 FIR_TX_Hilbert_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_GAUSS;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_GAUSS;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_LPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_LPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_HPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_HPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_HPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_HPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_HPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_HPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_Squelch_HPF;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_Squelch_HPF;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_RX1_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_RX2_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_FFT_I_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_FFT_Q_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_LOW_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_MID_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_RX_HIG_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_LOW_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_MID_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 EQ_MIC_HIG_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 AGC_RX1_KW_HSHELF_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 AGC_RX1_KW_HPASS_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 AGC_RX2_KW_HSHELF_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 AGC_RX2_KW_HPASS_FILTER;
extern arm_fir_decimate_instance_f32 DECIMATE_RX1_AUDIO_I;
extern arm_fir_decimate_instance_f32 DECIMATE_RX1_AUDIO_Q;
extern arm_fir_decimate_instance_f32 DECIMATE_RX2_AUDIO_I;
extern arm_fir_decimate_instance_f32 DECIMATE_RX2_AUDIO_Q;
extern volatile bool NeedReinitNotch;		 // need to reinitialize the manual Notch filter
extern volatile bool NeedReinitAudioFilters; // need to reinitialize the Audio filters

//Public methods
extern void InitAudioFilters(void);													   // initialize audio filters
extern void ReinitAudioFilters(void);												   // reinitialize audio filters
extern void InitNotchFilter(void);													   // initialize the manual Notch filter
extern void dc_filter(float32_t *Buffer, int16_t blockSize, DC_FILTER_STATE stateNum); // start DC corrector

#endif
