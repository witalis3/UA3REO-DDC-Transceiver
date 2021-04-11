#ifndef RF_UNIT_h
#define RF_UNIT_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>

extern bool ATU_TunePowerStabilized;

extern void RF_UNIT_UpdateState(bool clean);
extern void RF_UNIT_ProcessSensors(void);
extern void RF_UNIT_MeasureVoltage(void); //Tisho
extern void RF_UNIT_ATU_Invalidate(void);
	
#endif
