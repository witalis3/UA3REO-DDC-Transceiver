#include "settings.h"
#if (defined(LCD_ILI9481) || defined(LCD_HX8357B))

//Header files
#include "lcd_driver.h"
#include "main.h"
#include "fonts.h"
#include "functions.h"

uint32_t LCD_FSMC_COMM_ADDR = 0;
uint32_t LCD_FSMC_DATA_ADDR = 0;

//***** Functions prototypes *****//

//Write command to LCD
ITCM inline void LCDDriver_SendCommand(uint16_t com)
{
	*(__IO uint16_t *)((uint32_t)(LCD_FSMC_COMM_ADDR)) = com;
}

//Write data to LCD
ITCM inline void LCDDriver_SendData(uint16_t data)
{
	*(__IO uint16_t *)((uint32_t)(LCD_FSMC_DATA_ADDR)) = data;
}

//Write pair command-data
ITCM inline void LCDDriver_writeReg(uint16_t reg, uint16_t val) {
  LCDDriver_SendCommand(reg);
  LCDDriver_SendData(val);
}

//Read command from LCD
ITCM inline uint16_t LCDDriver_ReadStatus(void)
{
	return *(__IO uint16_t *)((uint32_t)(LCD_FSMC_COMM_ADDR));
}
//Read data from LCD
ITCM inline uint16_t LCDDriver_ReadData(void)
{
	return *(__IO uint16_t *)((uint32_t)(LCD_FSMC_DATA_ADDR));
}


//Read Register
ITCM inline uint16_t LCDDriver_readReg(uint16_t reg)
{
  LCDDriver_SendCommand(reg);
  return LCDDriver_ReadData();
}

//Initialise function
void LCDDriver_Init(void)
{
	//init remap
#if FMC_REMAP
	if (hsram1.Init.NSBank == FMC_NORSRAM_BANK1)
		LCD_FSMC_COMM_ADDR = 0xC0000000;
	if (hsram1.Init.NSBank == FMC_NORSRAM_BANK2)
		LCD_FSMC_COMM_ADDR = 0xCA000000;
	if (hsram1.Init.NSBank == FMC_NORSRAM_BANK3)
		LCD_FSMC_COMM_ADDR = 0xCB000000;
	if (hsram1.Init.NSBank == FMC_NORSRAM_BANK4)
		LCD_FSMC_COMM_ADDR = 0xCC000000;
#else
	if (hsram1.Init.NSBank == FMC_NORSRAM_BANK1)
		LCD_FSMC_COMM_ADDR = 0x60000000;
	if (hsram1.Init.NSBank == FMC_NORSRAM_BANK2)
		LCD_FSMC_COMM_ADDR = 0x6A000000;
	if (hsram1.Init.NSBank == FMC_NORSRAM_BANK3)
		LCD_FSMC_COMM_ADDR = 0x6B000000;
	if (hsram1.Init.NSBank == FMC_NORSRAM_BANK4)
		LCD_FSMC_COMM_ADDR = 0x6C000000;
#endif
	LCD_FSMC_DATA_ADDR = LCD_FSMC_COMM_ADDR + (1 << (FSMC_REGISTER_SELECT + 1));
	
#if (defined(LCD_RA8875))
	//swap addr
	uint32_t temp_addr = LCD_FSMC_COMM_ADDR;
	LCD_FSMC_COMM_ADDR = LCD_FSMC_DATA_ADDR;
	LCD_FSMC_DATA_ADDR = temp_addr;
#endif
	
#if (defined(LCD_ILI9481) || defined(LCD_HX8357B))
	#define ILI9481_COMM_DELAY 20
	
	LCDDriver_SendCommand(LCD_COMMAND_SOFT_RESET); //0x01
	HAL_Delay(ILI9481_COMM_DELAY);

	LCDDriver_SendCommand(LCD_COMMAND_EXIT_SLEEP_MODE); //0x11
	HAL_Delay(ILI9481_COMM_DELAY);

	LCDDriver_SendCommand(LCD_COMMAND_NORMAL_MODE_ON); //0x13
	
	LCDDriver_SendCommand(LCD_COMMAND_POWER_SETTING); //(0xD0);
	LCDDriver_SendData(0x07);
	LCDDriver_SendData(0x42);
	LCDDriver_SendData(0x18);
	
	LCDDriver_SendCommand(LCD_COMMAND_VCOM); //(0xD1);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x07);
	LCDDriver_SendData(0x10);
	
	LCDDriver_SendCommand(LCD_COMMAND_NORMAL_PWR_WR); //(0xD2);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0x02);
	HAL_Delay(ILI9481_COMM_DELAY);

#if defined(LCD_HX8357B)	
	LCDDriver_SendCommand(LCD_COMMAND_PANEL_DRV_CTL); //(0xC0);
	LCDDriver_SendData(0x10);
	LCDDriver_SendData(0x3B);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x02);
	LCDDriver_SendData(0x11);
	HAL_Delay(ILI9481_COMM_DELAY);
#endif
	
	LCDDriver_SendCommand(LCD_COMMAND_FR_SET); //(0xC5);
	LCDDriver_SendData(0x03);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_GAMMAWR); //(0xC8);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x32);
	LCDDriver_SendData(0x36);
	LCDDriver_SendData(0x45);
	LCDDriver_SendData(0x06);
	LCDDriver_SendData(0x16);
	LCDDriver_SendData(0x37);
	LCDDriver_SendData(0x75);
	LCDDriver_SendData(0x77);
	LCDDriver_SendData(0x54);
	LCDDriver_SendData(0x0C);
	LCDDriver_SendData(0x00);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_MADCTL); //(0x36);
	LCDDriver_SendData(LCD_PARAM_MADCTL_BGR);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_PIXEL_FORMAT); //(0x3A);
	LCDDriver_SendData(0x55);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_COLUMN_ADDR); //(0x2A);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0x3F);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_PAGE_ADDR); //(0x2B);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x00);
	LCDDriver_SendData(0x01);
	LCDDriver_SendData(0xDF);
	HAL_Delay(ILI9481_COMM_DELAY);
	
#if defined(LCD_HX8357B)	
	LCDDriver_SendCommand(LCD_COMMAND_COLOR_INVERSION_ON); //(0x21);
	HAL_Delay(ILI9481_COMM_DELAY);
#endif

	LCDDriver_SendCommand(LCD_COMMAND_IDLE_OFF);		   //(0x38);
	HAL_Delay(ILI9481_COMM_DELAY);
	
	LCDDriver_SendCommand(LCD_COMMAND_DISPLAY_ON);		   //(0x29);
	HAL_Delay(ILI9481_COMM_DELAY);
#endif

}

//Set screen rotation
void LCDDriver_setRotation(uint8_t rotate)
{
	LCDDriver_SendCommand(LCD_COMMAND_MADCTL);
	switch (rotate)
	{
	case 1: // Portrait
		LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MX);
		break;
	case 2: // Landscape (Portrait + 90)
		LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MV);
		break;
	case 3: // Inverter portrait
		LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MY);
		break;
	case 4: // Inverted landscape
		LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MV | LCD_PARAM_MADCTL_MX | LCD_PARAM_MADCTL_MY);
		//LCDDriver_SendData(LCD_PARAM_MADCTL_BGR | LCD_PARAM_MADCTL_MV | LCD_PARAM_MADCTL_SS | LCD_PARAM_MADCTL_GS);
		break;
	}
	HAL_Delay(120);
}

//Set cursor position
ITCM inline void LCDDriver_SetCursorAreaPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCDDriver_SendCommand(LCD_COMMAND_COLUMN_ADDR);
	LCDDriver_SendData(x1 >> 8);
	LCDDriver_SendData(x1 & 0xFF);
	LCDDriver_SendData(x2 >> 8);
	LCDDriver_SendData(x2 & 0xFF);
	LCDDriver_SendCommand(LCD_COMMAND_PAGE_ADDR);
	LCDDriver_SendData(y1 >> 8);
	LCDDriver_SendData(y1 & 0xFF);
	LCDDriver_SendData(y2 >> 8);
	LCDDriver_SendData(y2 & 0xFF);
	LCDDriver_SendCommand(LCD_COMMAND_GRAM);
}

ITCM static inline void LCDDriver_SetCursorPosition(uint16_t x, uint16_t y)
{
	LCDDriver_SendCommand(LCD_COMMAND_COLUMN_ADDR);
	LCDDriver_SendData(x >> 8); //-V760
	LCDDriver_SendData(x & 0xFF);
	LCDDriver_SendData(x >> 8);
	LCDDriver_SendData(x & 0xFF);
	LCDDriver_SendCommand(LCD_COMMAND_PAGE_ADDR);
	LCDDriver_SendData(y >> 8); //-V760
	LCDDriver_SendData(y & 0xFF);
	LCDDriver_SendData(y >> 8);
	LCDDriver_SendData(y & 0xFF);
	LCDDriver_SendCommand(LCD_COMMAND_GRAM);
}

//Write data to a single pixel
ITCM void LCDDriver_drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	LCDDriver_SetCursorPosition(x, y);
	LCDDriver_SendData(color);
}

//Fill the entire screen with a background color
ITCM void LCDDriver_Fill(uint16_t color)
{
	LCDDriver_Fill_RectXY(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

//Rectangle drawing functions
ITCM void LCDDriver_Fill_RectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	if (x1 > (LCD_WIDTH - 1))
		x1 = LCD_WIDTH - 1;
	if (y1 > (LCD_HEIGHT - 1))
		y1 = LCD_HEIGHT - 1;
	uint32_t n = ((x1 + 1) - x0) * ((y1 + 1) - y0);
	if (n > LCD_PIXEL_COUNT)
		n = LCD_PIXEL_COUNT;
	LCDDriver_SetCursorAreaPosition(x0, y0, x1, y1);

	static IRAM2 uint16_t fillxy_color;
	fillxy_color = color;
	if (n > 50)
	{
		SCB_CleanDCache_by_Addr((uint32_t *)&fillxy_color, sizeof(fillxy_color));
		const uint32_t part_size = 32000;
		uint32_t estamated = n;
		while (estamated > 0)
		{
			if (estamated >= part_size)
			{
				HAL_MDMA_Start(&hmdma_mdma_channel43_sw_0, (uint32_t)&fillxy_color, LCD_FSMC_DATA_ADDR, part_size * 2, 1);
				HAL_MDMA_PollForTransfer(&hmdma_mdma_channel43_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
				estamated -= part_size;
			}
			else
			{
				HAL_MDMA_Start(&hmdma_mdma_channel43_sw_0, (uint32_t)&fillxy_color, LCD_FSMC_DATA_ADDR, estamated * 2, 1);
				HAL_MDMA_PollForTransfer(&hmdma_mdma_channel43_sw_0, HAL_MDMA_FULL_TRANSFER, HAL_MAX_DELAY);
				estamated = 0;
			}
		}
	}
	else
	{
		while (n--)
		{
			LCDDriver_SendData(color);
		}
	}
}

ITCM void LCDDriver_Fill_RectWH(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	LCDDriver_Fill_RectXY(x, y, x + w, y + h, color);
}

//Line drawing functions
ITCM void LCDDriver_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep)
	{
		uswap(x0, y0)
			uswap(x1, y1)
	}

	if (x0 > x1)
	{
		uswap(x0, x1)
			uswap(y0, y1)
	}

	int16_t dx, dy;
	dx = (int16_t)(x1 - x0);
	dy = (int16_t)abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1)
	{
		ystep = 1;
	}
	else
	{
		ystep = -1;
	}

	for (; x0 <= x1; x0++)
	{
		if (steep)
		{
			LCDDriver_drawPixel(y0, x0, color);
		}
		else
		{
			LCDDriver_drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}
}

ITCM void LCDDriver_drawFastHLine(uint16_t x, uint16_t y, int16_t w, uint16_t color)
{
	int16_t x2 = x + w;
	if (x2 < 0)
		x2 = 0;
	if (x2 > (LCD_WIDTH - 1))
		x2 = LCD_WIDTH - 1;

	if (x2 < x)
		LCDDriver_Fill_RectXY((uint16_t)x2, y, x, y, color);
	else
		LCDDriver_Fill_RectXY(x, y, (uint16_t)x2, y, color);
}

ITCM void LCDDriver_drawFastVLine(uint16_t x, uint16_t y, int16_t h, uint16_t color)
{
	int16_t y2 = y + h - 1;
	if (y2 < 0)
		y2 = 0;
	if (y2 > (LCD_HEIGHT - 1))
		y2 = LCD_HEIGHT - 1;

	if (y2 < y)
		LCDDriver_Fill_RectXY(x, (uint16_t)y2, x, y, color);
	else
		LCDDriver_Fill_RectXY(x, y, x, (uint16_t)y2, color);
}

ITCM void LCDDriver_drawRectXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	LCDDriver_drawFastHLine(x0, y0, (int16_t)(x1 - x0), color);
	LCDDriver_drawFastHLine(x0, y1, (int16_t)(x1 - x0), color);
	LCDDriver_drawFastVLine(x0, y0, (int16_t)(y1 - y0), color);
	LCDDriver_drawFastVLine(x1, y0, (int16_t)(y1 - y0), color);
}

#endif
