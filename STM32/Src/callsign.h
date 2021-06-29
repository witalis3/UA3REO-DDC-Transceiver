#ifndef CALLSIGN_H
#define CALLSIGN_H

#include "stm32h7xx.h"
#include "main.h"
#include "stdbool.h"
#include "stdint.h"

//Public variabled
extern bool SYSMENU_callsign_info_opened;

//Public methods
extern void CALSIGN_INFO_Start(void);				  //launch
extern void CALSIGN_INFO_Stop(void);				  //stop session
extern void CALSIGN_INFO_Draw(void);			  //drawing
extern void CALSIGN_INFO_EncRotate(int8_t direction); //events per encoder tick

#endif
