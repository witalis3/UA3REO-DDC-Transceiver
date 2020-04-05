#ifndef SPEC_ANALYZER_H
#define SPEC_ANALYZER_H

#include "stm32h7xx.h"
#include "main.h"
#include "stdbool.h"

#define SPEC_Resolution 1000 //1khz
#define SPEC_StepDelay 1
#define SPEC_VParts 6 //вертикальных подписей

extern void SPEC_Start(void);
extern void SPEC_Draw(void);
extern void SPEC_EncRotate(int8_t direction);

#endif
