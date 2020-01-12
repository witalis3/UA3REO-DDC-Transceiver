#ifndef SPEC_ANALYZER_H
#define SPEC_ANALYZER_H

#include "stm32h7xx.h"
#include "main.h"
#include "stdbool.h"

#define SPEC_Resolution 10000 //10khz
#define SPEC_StepDelay 30

extern void SPEC_Start(void);
extern void SPEC_Draw(void);

#endif
