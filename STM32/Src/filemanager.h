#ifndef FILEMANAGER_h
#define FILEMANAGER_h

#include "stm32h7xx_hal.h"
#include <stdbool.h>

extern void FILEMANAGER_Draw(bool redraw);
extern void FILEMANAGER_EventRotate(int8_t direction);
extern void FILEMANAGER_Closing(void);
extern void FILEMANAGER_EventSecondaryRotate(int8_t direction);

#endif
