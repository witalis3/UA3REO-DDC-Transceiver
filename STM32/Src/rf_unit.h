#ifndef RF_UNIT_h
#define RF_UNIT_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>

extern void PERIPH_RF_UNIT_UpdateState(bool clean);
extern void PERIPH_ProcessSWRMeter(void);

#endif
