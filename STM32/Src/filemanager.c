#include "filemanager.h"
#include "lcd.h"
#include "sd.h"
#include "system_menu.h"

static bool first_start = true;

static bool FILEMANAGER_Refresh(void);

void FILEMANAGER_Draw(bool redraw)
{
	if(first_start)
	{
		first_start = false;
		if(!FILEMANAGER_Refresh())
			return;
	}
	if(redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		LCDDriver_printText("SD CARD FILE MANAGER", 5, 5, COLOR_GREEN, BG_COLOR, 2);
	}
	
	LCD_UpdateQuery.SystemMenu = false;
	if (redraw)
		LCD_UpdateQuery.SystemMenuRedraw = false;
}

void FILEMANAGER_EventRotate(int8_t direction)
{
	
}

void FILEMANAGER_EventSecondaryRotate(int8_t direction)
{
	
}

void FILEMANAGER_Closing(void)
{
	first_start = true;
}

static bool FILEMANAGER_Refresh(void)
{
	if(!SD_doCommand(SDCOMM_LIST_DIRECTORY, false))
	{
		SYSMENU_eventCloseSystemMenu();
		return false;
	}
	return true;
}