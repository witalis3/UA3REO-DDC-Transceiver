#ifndef FT8_MAIN_H_
#define FT8_MAIN_H_

#include "stdint.h"
#include <stdbool.h>
#include "Process_DSP.h"


//extern q15_t AudioBuffer_for_FT8[input_gulp_size];
extern q15_t dsp_buffer[] __attribute__ ((aligned (4)));
extern q15_t dsp_output[] __attribute__ ((aligned (4)));
extern q15_t  input_gulp[input_gulp_size] __attribute__ ((aligned (4)));

extern uint8_t export_fft_power[ft8_msg_samples*ft8_buffer*4] ;

extern int ft8_flag, FT_8_counter, ft8_marker, decode_flag, WF_counter;

extern uint16_t FT8_DatBlockNum;
extern bool FT8_DecodeActiveFlg;			//Flag indicating if the FT8 decode is activated
extern bool FT8_ColectDataFlg;				//Flag indicating the FT8 data colection
//extern bool FT8_Bussy;

extern int DSP_Flag;

void InitFT8_Decoder(void);
void MenagerFT8(void);

// For FT8 GUI 
void FT8_EncRotate(int8_t direction);
void FT8_Enc2Rotate(int8_t direction);
void FT8_Enc2Click(void);

#endif /* FT_MAIN_H_ */