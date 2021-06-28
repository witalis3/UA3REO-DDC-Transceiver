#ifndef LOCATOR_H
#define LOCATOR_H

#include "stm32h7xx.h"
#include "main.h"
#include "stdbool.h"
#include "stdint.h"

//Public variabled
extern bool SYSMENU_locator_info_opened;

//Public methods
extern void LOCINFO_Start(void);				  //launch
extern void LOCINFO_Stop(void);				  //stop session
extern void LOCINFO_Draw(void);			  //drawing
extern void LOCINFO_EncRotate(int8_t direction); //events per encoder tick

#endif
