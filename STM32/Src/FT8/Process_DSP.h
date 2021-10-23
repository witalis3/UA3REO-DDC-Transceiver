#ifndef PROCESS_DSP_H_
#define PROCESS_DSP_H_

#include "arm_math.h"
#include <stdbool.h>

#define FFT_SIZE_FT8  2048		//2048
#define num_que_blocks 40			//8
#define block_size 24				//128
#define input_gulp_size 960 //1024

#define ft8_buffer 400  //400 arbitrary for 3 kc
#define ft8_min_bin 48
#define FFT_Resolution 6.25
#define ft8_min_freq FFT_Resolution * ft8_min_bin

#define ft8_msg_samples 86 //92


extern int CQ_Flag;

void init_DSP(void);
float ft_blackman_i(int i, int N);

void process_FT8_FFT(void);
void update_offset_waterfall(int offset);

void Service_FT8(void);
void Set_Data_Colection(bool enable);



#endif /* PROCESS_DSP_H_ */




