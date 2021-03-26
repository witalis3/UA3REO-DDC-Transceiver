#include "filemanager.h"
#include "lcd.h"
#include "sd.h"
#include "system_menu.h"

static bool first_start = true;
static uint16_t current_index = 0;

SRAM char FILEMANAGER_CurrentPath[128] = "";
SRAM char FILEMANAGER_LISTING[FILEMANAGER_LISTING_MAX_FILES][FILEMANAGER_LISTING_MAX_FILELEN + 1] = {""};
uint16_t FILEMANAGER_files_startindex = 0;
uint16_t FILEMANAGER_files_count = 0;

static void FILEMANAGER_Refresh(void);

void FILEMANAGER_Draw(bool redraw)
{
	if (first_start)
	{
		first_start = false;
		FILEMANAGER_files_startindex = 0;
		current_index = 0;
		strcpy(FILEMANAGER_CurrentPath, "");
		FILEMANAGER_Refresh();
		return;
	}
	if (redraw)
	{
		LCDDriver_Fill(BG_COLOR);
		uint16_t cur_y = 5;
		LCDDriver_printText("SD CARD FILE MANAGER", 5, cur_y, COLOR_GREEN, BG_COLOR, 2);
		cur_y += 24;
		if (strlen(FILEMANAGER_CurrentPath) == 0)
			LCDDriver_printText("/", 5, cur_y, FG_COLOR, BG_COLOR, 2);
		else
			LCDDriver_printText(FILEMANAGER_CurrentPath, 5, cur_y, FG_COLOR, BG_COLOR, 2);
		cur_y += 24;
		if (FILEMANAGER_files_startindex == 0)
			LCDDriver_printText("..", 5, cur_y, FG_COLOR, BG_COLOR, 2);
		cur_y += LAYOUT->SYSMENU_ITEM_HEIGHT;

		for (uint16_t file_id = 0; file_id < FILEMANAGER_LISTING_ITEMS_ON_PAGE; file_id++)
		{
			LCDDriver_printText(FILEMANAGER_LISTING[file_id], 5, cur_y, FG_COLOR, BG_COLOR, 2);
			cur_y += LAYOUT->SYSMENU_ITEM_HEIGHT;
		}

		LCD_UpdateQuery.SystemMenuRedraw = false;
	}

	LCDDriver_drawFastHLine(0, 5 + 24 + 24 + LAYOUT->SYSMENU_ITEM_HEIGHT + (current_index * LAYOUT->SYSMENU_ITEM_HEIGHT) - 1, LAYOUT->SYSMENU_W, FG_COLOR);

	LCD_UpdateQuery.SystemMenu = false;
}

void FILEMANAGER_EventRotate(int8_t direction)
{
	if (current_index == 0)
	{
		if (strcmp(FILEMANAGER_CurrentPath, "") == 0) // root
		{
			SYSMENU_eventCloseSystemMenu();
		}
		else //inner folder
		{
			char *istr = strrchr(FILEMANAGER_CurrentPath, '/');
			*istr = 0;
			FILEMANAGER_files_startindex = 0;
			current_index = 0;
			FILEMANAGER_Refresh();
		}
	}
	else
	{
		char *istr = strstr(FILEMANAGER_LISTING[current_index - 1], "[DIR] ");
		if (istr != NULL && ((strlen(istr + 6) + 1) < sizeof(FILEMANAGER_CurrentPath))) //is directory
		{
			strcat(FILEMANAGER_CurrentPath, "/");
			strcat(FILEMANAGER_CurrentPath, istr + 6);
			FILEMANAGER_files_startindex = 0;
			current_index = 0;
			FILEMANAGER_Refresh();
		}
	}
}

void FILEMANAGER_EventSecondaryRotate(int8_t direction)
{
	LCDDriver_drawFastHLine(0, 5 + 24 + 24 + LAYOUT->SYSMENU_ITEM_HEIGHT + (current_index * LAYOUT->SYSMENU_ITEM_HEIGHT) - 1, LAYOUT->SYSMENU_W, BG_COLOR);
	if (direction > 0 || current_index > 0)
		current_index += direction;

	int16_t real_file_index = FILEMANAGER_files_startindex + current_index - 1;

	//limit
	if (real_file_index >= FILEMANAGER_files_count)
		current_index--;

	//list down
	if (current_index > FILEMANAGER_LISTING_ITEMS_ON_PAGE && real_file_index < FILEMANAGER_files_count)
	{
		FILEMANAGER_files_startindex += FILEMANAGER_LISTING_ITEMS_ON_PAGE;
		current_index = 1;
		FILEMANAGER_Refresh();
	}

	//list up
	if (FILEMANAGER_files_startindex > 0 && current_index == 0)
	{
		FILEMANAGER_files_startindex -= FILEMANAGER_LISTING_ITEMS_ON_PAGE;
		current_index = FILEMANAGER_LISTING_ITEMS_ON_PAGE;
		FILEMANAGER_Refresh();
	}

	LCD_UpdateQuery.SystemMenu = true;
}

void FILEMANAGER_Closing(void)
{
	first_start = true;
}

static void FILEMANAGER_Refresh(void)
{
	if (!SD_doCommand(SDCOMM_LIST_DIRECTORY, false))
		SYSMENU_eventCloseSystemMenu();

	LCD_UpdateQuery.SystemMenuRedraw = true;
}