#include "filemanager.h"
#include "lcd.h"
#include "sd.h"
#include "system_menu.h"

static bool first_start = true;
static bool current_index = 0;

SRAM char FILEMANAGER_CurrentPath[128] = "/";
SRAM char FILEMANAGER_LISTING[FILEMANAGER_LISTING_MAX_FILES][FILEMANAGER_LISTING_MAX_FILELEN] = {""};

static void FILEMANAGER_Refresh(void);

void FILEMANAGER_Draw(bool redraw)
{
	if(first_start)
	{
		first_start = false;
		FILEMANAGER_Refresh();
		return;
	}
	if(redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		uint16_t cur_y = 5;
		LCDDriver_printText("SD CARD FILE MANAGER", 5, cur_y, COLOR_GREEN, BG_COLOR, 2);
		cur_y += 24;
		LCDDriver_printText(FILEMANAGER_CurrentPath, 5, cur_y, FG_COLOR, BG_COLOR, 2);
		cur_y += 24;
		LCDDriver_printText("..", 5, cur_y, FG_COLOR, BG_COLOR, 2);
		cur_y += LAYOUT->SYSMENU_ITEM_HEIGHT;
		
		for(uint16_t file_id = 0; file_id < FILEMANAGER_LISTING_ITEMS_ON_PAGE; file_id++)
		{
			LCDDriver_printText(FILEMANAGER_LISTING[file_id], 5, cur_y, FG_COLOR, BG_COLOR, 2);
			cur_y += LAYOUT->SYSMENU_ITEM_HEIGHT;
		}
		
		LCD_UpdateQuery.SystemMenuRedraw = false;
	}
	
	LCDDriver_drawFastHLine(0, 5 + 24 + 24 + LAYOUT->SYSMENU_ITEM_HEIGHT + (current_index * LAYOUT->SYSMENU_ITEM_HEIGHT), LAYOUT->SYSMENU_W, FG_COLOR);
	
	LCD_UpdateQuery.SystemMenu = false;
}

void FILEMANAGER_EventRotate(int8_t direction)
{
	
}

void FILEMANAGER_EventSecondaryRotate(int8_t direction)
{
	LCDDriver_drawFastHLine(0, 5 + 24 + 24 + LAYOUT->SYSMENU_ITEM_HEIGHT + (current_index * LAYOUT->SYSMENU_ITEM_HEIGHT), LAYOUT->SYSMENU_W, BG_COLOR);
	if(direction > 0 || current_index > 0)
		current_index += direction;
	if(current_index > FILEMANAGER_LISTING_ITEMS_ON_PAGE)
		current_index = FILEMANAGER_LISTING_ITEMS_ON_PAGE;
	println(current_index);
	LCD_UpdateQuery.SystemMenu = true;
}

void FILEMANAGER_Closing(void)
{
	first_start = true;
}

static void FILEMANAGER_Refresh(void)
{
	current_index = 0;
	if(!SD_doCommand(SDCOMM_LIST_DIRECTORY, false))
		SYSMENU_eventCloseSystemMenu();
	
	LCD_UpdateQuery.SystemMenuRedraw = true;
}