#ifndef AGC_H
#define AGC_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include "audio_processor.h"

//Public methods
extern void DoAGC(float32_t *agcbuffer, uint_fast16_t blockSize, AUDIO_PROC_RX_NUM rx_id); //запуск AGC на блок данных
extern void ResetAGC(void);

#endif
