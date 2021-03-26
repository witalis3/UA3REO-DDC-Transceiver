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
bool FILEMANAGER_dialog_opened = false;
uint16_t FILEMANAGER_dialog_button_index = 0;

static void FILEMANAGER_Refresh(void);
static void FILEMANAGER_OpenDialog(void);
static void FILEMANAGER_DialogAction(void);

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

	if(FILEMANAGER_dialog_opened)
		FILEMANAGER_OpenDialog();
	
	LCD_UpdateQuery.SystemMenu = false;
}

void FILEMANAGER_EventRotate(int8_t direction)
{
	if(FILEMANAGER_dialog_opened)
	{
		FILEMANAGER_DialogAction();
		return;
	}
	
	if (current_index == 0) //go up
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
		else //is file
		{
			FILEMANAGER_dialog_button_index = 0;
			FILEMANAGER_OpenDialog();
		}
	}
}

void FILEMANAGER_EventSecondaryRotate(int8_t direction)
{
	if(FILEMANAGER_dialog_opened)
	{
		if(FILEMANAGER_dialog_button_index > 0 || direction > 0)
			FILEMANAGER_dialog_button_index += direction;
		FILEMANAGER_OpenDialog();
		return;
	}
	
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

static void FILEMANAGER_OpenDialog(void)
{
	FILEMANAGER_dialog_opened = true;
	uint8_t max_buttons_index = 1;
	if(FILEMANAGER_dialog_button_index > max_buttons_index)
		FILEMANAGER_dialog_button_index = max_buttons_index;
	
	#define margin 30
	//frame
	LCDDriver_Fill_RectXY(margin, margin, LCD_WIDTH - margin, LCD_HEIGHT - margin, BG_COLOR);
	LCDDriver_drawRectXY(margin, margin, LCD_WIDTH - margin, LCD_HEIGHT - margin, FG_COLOR);
	//buttons
	uint16_t button_y = margin * 2;
	uint16_t button_x = margin * 2;
	uint16_t button_w = LCD_WIDTH - margin * 2 - button_x;
	uint16_t button_h = margin;
	uint16_t bounds_x, bounds_y, bounds_w, bounds_h;
	//back
	LCDDriver_Fill_RectXY(button_x, button_y, LCD_WIDTH - margin * 2, button_y + button_h, (FILEMANAGER_dialog_button_index == 0) ? FG_COLOR : BG_COLOR);
	LCDDriver_drawRectXY(button_x, button_y, LCD_WIDTH - margin * 2, button_y + button_h, (FILEMANAGER_dialog_button_index == 0) ? BG_COLOR : FG_COLOR);
	LCDDriver_getTextBounds("Cancel", button_x, button_y, &bounds_x, &bounds_y, &bounds_w, &bounds_h, &FreeSans9pt7b);
	LCDDriver_printTextFont("Cancel", button_x + button_w / 2 - bounds_w / 2, button_y + button_h / 2 + bounds_h / 2, (FILEMANAGER_dialog_button_index == 0) ? BG_COLOR : FG_COLOR, (FILEMANAGER_dialog_button_index == 0) ? FG_COLOR : BG_COLOR, &FreeSans9pt7b);
	button_y += button_h + margin;
	//delete
	LCDDriver_Fill_RectXY(button_x, button_y, LCD_WIDTH - margin * 2, button_y + button_h, (FILEMANAGER_dialog_button_index == 1) ? FG_COLOR : BG_COLOR);
	LCDDriver_drawRectXY(button_x, button_y, LCD_WIDTH - margin * 2, button_y + button_h, (FILEMANAGER_dialog_button_index == 1) ? BG_COLOR : FG_COLOR);
	LCDDriver_getTextBounds("Delete", button_x, button_y, &bounds_x, &bounds_y, &bounds_w, &bounds_h, &FreeSans9pt7b);
	LCDDriver_printTextFont("Delete", button_x + button_w / 2 - bounds_w / 2, button_y + button_h / 2 + bounds_h / 2, (FILEMANAGER_dialog_button_index == 1) ? BG_COLOR : FG_COLOR, (FILEMANAGER_dialog_button_index == 1) ? FG_COLOR : BG_COLOR, &FreeSans9pt7b);
	button_y += button_h + margin;
}

static void FILEMANAGER_DialogAction(void)
{
	if(FILEMANAGER_dialog_button_index == 0) //back
	{
		FILEMANAGER_dialog_opened = false;
		FILEMANAGER_Refresh();
		return;
	}
	if(FILEMANAGER_dialog_button_index == 1) //delete
	{
		if(!SD_CommandInProcess)
		{
			dma_memset(SD_workbuffer_A, 0, sizeof(SD_workbuffer_A));
			if(strlen(FILEMANAGER_CurrentPath) > 0)
			{
				strcat((char*)SD_workbuffer_A, FILEMANAGER_CurrentPath);
				strcat((char*)SD_workbuffer_A, "/");
			}
			strcat((char*)SD_workbuffer_A, FILEMANAGER_LISTING[current_index - 1]);
			FILEMANAGER_dialog_opened = false;
			current_index--;
			SD_doCommand(SDCOMM_DELETE_FILE, false);
		}
		return;
	}
}
