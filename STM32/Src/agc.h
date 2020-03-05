#ifndef AGC_H
#define AGC_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include "arm_math.h"
#include "audio_processor.h"

extern void DoAGC(float32_t *agcbuffer, uint_fast16_t blockSize, AUDIO_PROC_RX_NUM rx_id);
extern void InitAGC(void);

#endif
