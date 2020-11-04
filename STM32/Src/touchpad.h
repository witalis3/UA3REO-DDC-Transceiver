#ifndef TOUCHPAD_h
#define TOUCHPAD_h

#include "settings.h"

#if (defined(TOUCHPAD_GT911))
	#include "touchpad_GT911.h"
#endif

void TOUCHPAD_Init(void);
void TOUCHPAD_ProcessInterrupt(void);
void TOUCHPAD_reserveInterrupt(void);

#endif
