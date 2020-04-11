#ifndef SPEC_ANALYZER_H
#define SPEC_ANALYZER_H

#include "stm32h7xx.h"
#include "main.h"
#include "stdbool.h"

#define SPEC_Resolution 1000 //разрешение, 1khz
#define SPEC_StepDelay 1     //задержка при сканирвоании, мсек
#define SPEC_VParts 6        //вертикальных подписей

//Public methods
extern void SPEC_Start(void);                 //запуск анализатора
extern void SPEC_Draw(void);                  //отрисовка анализатора
extern void SPEC_EncRotate(int8_t direction); //события анализатора на энкодер

#endif
