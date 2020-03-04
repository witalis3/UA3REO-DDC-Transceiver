#ifndef AUDIO_FILTERS_h
#define AUDIO_FILTERS_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "arm_math.h"
#include "fpga.h"

#define IIR_FILTERS_COUNT 35
#define IQ_RX_HILBERT_TAPS 89
#define IQ_TX_HILBERT_TAPS 201
#define IIR_MAX_STAGES 41
#define NOTCH_STAGES 1
#define NOTCH_COEFF_IN_STAGE 5

#define FIR_RX_HILBERT_STATE_SIZE (IQ_RX_HILBERT_TAPS + FPGA_AUDIO_BUFFER_HALF_SIZE - 1)
#define FIR_TX_HILBERT_STATE_SIZE (IQ_TX_HILBERT_TAPS + FPGA_AUDIO_BUFFER_HALF_SIZE - 1)
#define IIR_LPF_Taps_STATE_SIZE (IIR_MAX_STAGES + FPGA_AUDIO_BUFFER_HALF_SIZE)
#define IIR_HPF_Taps_STATE_SIZE (IIR_MAX_STAGES + FPGA_AUDIO_BUFFER_HALF_SIZE)
#define IIR_HPF_SQL_STATE_SIZE (IIR_MAX_STAGES + FPGA_AUDIO_BUFFER_HALF_SIZE)

typedef enum
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

typedef enum
{
	IIR_LATTICE_HPF,
	IIR_LATTICE_LPF
} IIR_LATTICE_FILTER_TYPE;

//Coefficients converted to ARMA in reverse order by MATLAB
typedef struct
{
	const uint16_t width;
	const IIR_LATTICE_FILTER_TYPE type;
	const uint32_t stages;
	const float32_t* coeffsK_lattice;
	const float32_t* coeffsV_ladder;
} IIR_LATTICE_FILTER;

extern arm_fir_instance_f32 FIR_RX_Hilbert_I;
extern arm_fir_instance_f32 FIR_RX_Hilbert_Q;
extern arm_fir_instance_f32 FIR_TX_Hilbert_I;
extern arm_fir_instance_f32 FIR_TX_Hilbert_Q;
extern arm_iir_lattice_instance_f32 IIR_LPF_I;
extern arm_iir_lattice_instance_f32 IIR_LPF_Q;
extern arm_iir_lattice_instance_f32 IIR_HPF_I;
extern arm_iir_lattice_instance_f32 IIR_HPF_Q;
extern arm_iir_lattice_instance_f32 IIR_Squelch_HPF;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_FILTER_FFT_I;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_FILTER_FFT_Q;
extern volatile bool NeedReinitNotch;

extern void InitAudioFilters(void);
extern void ReinitAudioFilters(void);
extern void InitNotchFilter(void);
extern void dc_filter(float32_t *Buffer, int16_t blockSize, uint8_t stateNum);

//сохранение старых значений семплов для DC фильтра. Несколько состояний для разных потребителей
typedef struct
{
	float32_t x_prev;
	float32_t y_prev;
} dc_filter_state_type;

#endif
