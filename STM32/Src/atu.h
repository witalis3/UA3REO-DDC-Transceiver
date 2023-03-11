#ifndef ATU_H
#define ATU_H

#include "hardware.h"

extern bool ATU_TunePowerStabilized;

extern void ATU_Process(void);
extern void ATU_Invalidate(void);
extern void ATU_SetCompleted(void);

#endif
