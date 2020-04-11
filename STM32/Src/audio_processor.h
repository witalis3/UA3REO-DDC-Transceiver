#ifndef AUDIO_PROCESSOR_h
#define AUDIO_PROCESSOR_h

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "functions.h"

#define FPGA_AUDIO_BUFFER_SIZE (192 * 2) //размер буффера данных FPGA
#define FPGA_AUDIO_BUFFER_HALF_SIZE FPGA_AUDIO_BUFFER_SIZE / 2 //половина буфера
#define FM_RX_LPF_ALPHA 0.05f         // For NFM demodulator:  "Alpha" (low-pass) factor to result in -6dB "knee" at approx. 270 Hz 0.05f
#define FM_RX_HPF_ALPHA 0.96f         // For NFM demodulator:  "Alpha" (high-pass) factor to result in -6dB "knee" at approx. 180 Hz 0.96f
#define FM_TX_HPF_ALPHA 0.95f         // For FM modulator:  "Alpha" (high-pass) factor to pre-emphasis
#define FM_SQUELCH_HYSTERESIS 0.3f    // Hysteresis for FM squelch
#define FM_SQUELCH_PROC_DECIMATION 50 // Number of times we go through the FM demod algorithm before we do a squelch calculation
#define FM_RX_SQL_SMOOTHING 0.005f    // Smoothing factor for IIR squelch noise averaging
#define AUDIO_RX_NB_DELAY_BUFFER_ITEMS 32 //NoiseBlanker buffer size
#define AUDIO_RX_NB_DELAY_BUFFER_SIZE (AUDIO_RX_NB_DELAY_BUFFER_ITEMS * 2)

typedef enum //номер приёмника
{
	AUDIO_RX1,
	AUDIO_RX2
} AUDIO_PROC_RX_NUM;

//Public variables
extern volatile uint32_t AUDIOPROC_samples; //аудиосемплов обработано в процессоре
extern int32_t Processor_AudioBuffer_A[FPGA_AUDIO_BUFFER_SIZE]; //буффер A аудио-процессора
extern int32_t Processor_AudioBuffer_B[FPGA_AUDIO_BUFFER_SIZE]; //буффер B аудио-процессора
extern volatile uint_fast8_t Processor_AudioBuffer_ReadyBuffer; //какой буффер сейчас используется, A или B
extern volatile bool Processor_NeedRXBuffer; //кодеку нужны данные с процессора для RX
extern volatile bool Processor_NeedTXBuffer; //кодеку нужны данные с процессора для TX
extern float32_t FPGA_Audio_Buffer_RX1_Q_tmp[FPGA_AUDIO_BUFFER_HALF_SIZE]; //копия рабочей части буфферов FPGA для обработки
extern float32_t FPGA_Audio_Buffer_RX1_I_tmp[FPGA_AUDIO_BUFFER_HALF_SIZE];
extern float32_t FPGA_Audio_Buffer_RX2_Q_tmp[FPGA_AUDIO_BUFFER_HALF_SIZE];
extern float32_t FPGA_Audio_Buffer_RX2_I_tmp[FPGA_AUDIO_BUFFER_HALF_SIZE];
extern float32_t FPGA_Audio_Buffer_TX_Q_tmp[FPGA_AUDIO_BUFFER_HALF_SIZE];
extern float32_t FPGA_Audio_Buffer_TX_I_tmp[FPGA_AUDIO_BUFFER_HALF_SIZE];
extern volatile float32_t Processor_TX_MAX_amplitude_OUT; //аплитуда TX после ALC
extern volatile float32_t Processor_RX_Power_value; //магнитуда RX сигнала
extern volatile float32_t Processor_selected_RFpower_amplitude; //целевая амплитуда TX сигнала

//Public methods
extern void processRxAudio(void); //запуск аудио-процессора для RX
extern void processTxAudio(void); //запуск аудио-процессора для TX
extern void initAudioProcessor(void); //инициализация аудио-процессора


#endif
