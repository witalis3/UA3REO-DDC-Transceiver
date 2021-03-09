#include "filemanager.h"
#include "lcd.h"

void FILEMANAGER_Draw(bool redraw)
{
	if(redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("SD CARD FILE MANAGER", 5, 5, COLOR_GREEN, BG_COLOR, 2);
	}
}

void FILEMANAGER_EventRotate(int8_t direction)
{
	
}

void FILEMANAGER_EventSecondaryRotate(int8_t direction)
{
	
}

void FILEMANAGER_Closing(void)
{
	
}
