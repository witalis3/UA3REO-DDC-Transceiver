#ifndef AUDIO_FILTERS_h
#define AUDIO_FILTERS_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "fpga.h"
#include "functions.h"

#define IIR_FILTERS_COUNT 35														   //Всего фильтров в коллекции
#define IQ_HILBERT_TAPS 201															   //Порядок фильтра гильберта
#define IIR_MAX_STAGES 15															   //Максимальный порядок IIR фильтров
#define NOTCH_STAGES 1																   //порядок ручного Notch фильтра
#define NOTCH_COEFF_IN_STAGE 5														   //коэффициентов в порядке ручного Notch фильтра
#define FIR_RX1_HILBERT_STATE_SIZE (IQ_HILBERT_TAPS + FPGA_AUDIO_BUFFER_HALF_SIZE - 1) //размер буфферов состояний
#define FIR_RX2_HILBERT_STATE_SIZE (IQ_HILBERT_TAPS + FPGA_AUDIO_BUFFER_HALF_SIZE - 1)
#define FIR_TX_HILBERT_STATE_SIZE (IQ_HILBERT_TAPS + FPGA_AUDIO_BUFFER_HALF_SIZE - 1)
#define IIR_RX1_LPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX2_LPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_TX_LPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX1_HPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX2_HPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_TX_HPF_Taps_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX1_HPF_SQL_STATE_SIZE (IIR_MAX_STAGES * 2)
#define IIR_RX2_HPF_SQL_STATE_SIZE (IIR_MAX_STAGES * 2)

typedef enum //тип BiQuad фильтра для автоматического расчёта
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

typedef enum //тип фильтра в коллекции
{
	IIR_BIQUAD_HPF,
	IIR_BIQUAD_LPF
} IIR_BIQUAD_FILTER_TYPE;

typedef enum //состояния DC-корректоров для каждого пользователя
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

typedef struct //сохранение старых значений семплов для DC фильтра. Несколько состояний для разных потребителей
{
	float32_t x_prev;
	float32_t y_prev;
} DC_filter_state_type;

typedef struct //фильтр в коллекции
{
	const uint16_t width;
	const IIR_BIQUAD_FILTER_TYPE type;
	const uint8_t stages;
	const float32_t *coeffs; //Coefficients converted to ARMA in reverse order by MATLAB
} IIR_BIQUAD_FILTER;

//Public variables
extern arm_fir_instance_f32 FIR_RX1_Hilbert_I; //инстансы фильтров
extern arm_fir_instance_f32 FIR_RX1_Hilbert_Q;
extern arm_fir_instance_f32 FIR_RX2_Hilbert_I;
extern arm_fir_instance_f32 FIR_RX2_Hilbert_Q;
extern arm_fir_instance_f32 FIR_TX_Hilbert_I;
extern arm_fir_instance_f32 FIR_TX_Hilbert_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_LPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_LPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_LPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_LPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_HPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX1_HPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_HPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_RX2_HPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_HPF_I;
extern arm_biquad_cascade_df2T_instance_f32 IIR_TX_HPF_Q;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_RX1_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_RX2_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_FFT_I_FILTER;
extern arm_biquad_cascade_df2T_instance_f32 NOTCH_FFT_Q_FILTER;
extern volatile bool NeedReinitNotch; //необходимо переинициализировать ручной Notch-фильтр
//Public methods
extern void InitAudioFilters(void);											   //инифиализация аудио-фильтров
extern void ReinitAudioFilters(void);										   //переинициализация аудио-фильтров
extern void InitNotchFilter(void);											   //инициализация ручного Notch-фильтра
extern void dc_filter(float32_t *Buffer, int16_t blockSize, uint8_t stateNum); //запуск DC-корректора

#endif
