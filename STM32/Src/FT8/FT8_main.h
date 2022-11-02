#ifndef FT8_MAIN_H_
#define FT8_MAIN_H_

#include "Process_DSP.h"
#include "stdint.h"
#include <stdbool.h>

// extern q15_t AudioBuffer_for_FT8[input_gulp_size];

extern int ft8_flag, FT_8_counter, ft8_marker, decode_flag, WF_counter;

extern uint16_t FT8_DatBlockNum;
extern bool FT8_DecodeActiveFlg; // Flag indicating if the FT8 decode is activated
extern bool FT8_ColectDataFlg;   // Flag indicating the FT8 data colection
// extern bool FT8_Bussy;

extern int DSP_Flag;

void InitFT8_Decoder(void);
void MenagerFT8(void);

// For FT8 GUI
void FT8_EncRotate(int8_t direction);
void FT8_Enc2Rotate(int8_t direction);
void FT8_Enc2Click(void);

#endif /* FT_MAIN_H_ */