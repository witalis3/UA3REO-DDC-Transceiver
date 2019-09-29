#ifndef Encoder_h
#define Encoder_h

#include "stm32f4xx_hal.h"

extern void ENCODERS_Init(void);
extern void ENCODER_checkRotate(void);
extern void ENCODER2_checkRotate(void);
extern void ENCODER2_checkSwitch(void);

extern RTC_HandleTypeDef hrtc;

#endif
