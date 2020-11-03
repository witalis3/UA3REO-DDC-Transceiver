#include "settings.h"

//Header files
#include "lcd_driver.h"
#include "main.h"
#include "fonts.h"
#include "functions.h"

static bool _cp437 = false;
static uint16_t text_cursor_y = 0;
static uint16_t text_cursor_x = 0;
static bool wrap = false;

uint16_t LCDDriver_GetCurrentXOffset(void)
{
	return text_cursor_x;
}

//Text printing functions
ITCM void LCDDriver_drawChar(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size)
{
	uint8_t line;
	if ((x >= LCD_WIDTH) ||			// Clip right
		(y >= LCD_HEIGHT) ||		// Clip bottom
		((x + 6 * size - 1) < 0) || // Clip left
		((y + 8 * size - 1) < 0))	// Clip top
		return;

	if (!_cp437 && (c >= 176))
		c++;																   // Handle 'classic' charset behavior
	LCDDriver_SetCursorAreaPosition(x, y, x + 6 * size - 1, y + 8 * size - 1); //char area

	for (int8_t j = 0; j < 8; j++)
	{											//y line out
		for (int8_t s_y = 0; s_y < size; s_y++) //y size scale
			for (int8_t i = 0; i < 6; i++)
			{ //x line out
				{
					if (i == 5)
						line = 0x0;
					else
						line = pgm_read_byte(rastr_font + (c * 5) + i); //read font
					line >>= j;
					for (int8_t s_x = 0; s_x < size; s_x++) //x size scale
					{
						if (line & 0x1)
							LCDDriver_SendData(color); //font pixel
						else
							LCDDriver_SendData(bg); //background pixel
					}
				}
			}
	}
}

ITCM void LCDDriver_printText(char text[], uint16_t x, uint16_t y, uint16_t color, uint16_t bg, uint8_t size)
{
	uint16_t offset;
	offset = size * 6;
	for (uint16_t i = 0; i < 40 && text[i] != NULL; i++)
	{
		LCDDriver_drawChar(x + (offset * i), y, text[i], color, bg, size);
		text_cursor_x = x + (offset * (i + 1));
	}
}

ITCM void LCDDriver_drawCharFont(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bg, GFXfont *gfxFont)
{
	c -= (uint8_t)pgm_read_byte(&gfxFont->first);
	GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
	uint8_t *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);

	uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
	uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height), xa = pgm_read_byte(&glyph->xAdvance);
	int8_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
	int8_t yo = (int8_t)pgm_read_byte(&glyph->yOffset);
	uint8_t xx, yy, bits = 0, bit = 0;
	int16_t ys1 = y + yo;
	int16_t ys2 = y + yo + h - 1;
	if (ys1 < 0)
		ys1 = 0;
	if (ys2 < 0)
		ys2 = 0;
	LCDDriver_SetCursorAreaPosition(x, (uint16_t)ys1, x + xa - 1, (uint16_t)ys2); //char area
	
	for (yy = 0; yy < h; yy++)
	{
		for (xx = 0; xx < xa; xx++)
		{
			if (xx < xo || xx >= (xo + w))
			{
				LCDDriver_SendData(bg); //background pixel
				continue;
			}
			if (!(bit++ & 7))
			{
				bits = pgm_read_byte(&bitmap[bo++]);
			}
			if (bits & 0x80)
			{
				LCDDriver_SendData(color); //font pixel
			}
			else
			{
				LCDDriver_SendData(bg); //background pixel
			}
			bits <<= 1;
		}
	}
}

ITCM void LCDDriver_printTextFont(char text[], uint16_t x, uint16_t y, uint16_t color, uint16_t bg, GFXfont *gfxFont)
{
	uint8_t c = 0;
	text_cursor_x = x;
	text_cursor_y = y;
	for (uint16_t i = 0; i < 40 && text[i] != NULL; i++)
	{
		c = text[i];
		if (c == '\n')
		{
			text_cursor_x = 0;
			text_cursor_y += (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
		}
		else if (c != '\r')
		{
			uint8_t first = pgm_read_byte(&gfxFont->first);
			uint8_t last = pgm_read_byte(&gfxFont->last);
			if ((c >= first) && (c <= last))
			{
				GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c - first]);
				uint8_t w = pgm_read_byte(&glyph->width);
				uint8_t h = pgm_read_byte(&glyph->height);
				if ((w > 0) && (h > 0))
				{
					int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
					if (wrap && ((text_cursor_x + (xo + w)) > LCD_WIDTH))
					{
						text_cursor_x = 0;
						text_cursor_y += (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
					}
					LCDDriver_drawCharFont(text_cursor_x, text_cursor_y, c, color, bg, gfxFont);
				}
				text_cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance);
			}
		}
	}
}

/**************************************************************************/
/*!
	@brief    Helper to determine size of a character with current font/size.
	   Broke this out as it's used by both the PROGMEM- and RAM-resident getTextBounds() functions.
	@param    c     The ascii character in question
	@param    x     Pointer to x location of character
	@param    y     Pointer to y location of character
	@param    minx  Minimum clipping value for X
	@param    miny  Minimum clipping value for Y
	@param    maxx  Maximum clipping value for X
	@param    maxy  Maximum clipping value for Y
*/
/**************************************************************************/
ITCM static void LCDDriver_charBounds(char c, uint16_t *x, uint16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy, GFXfont *gfxFont)
{
	if (c == '\n')
	{			// Newline?
		*x = 0; // Reset x to zero, advance y by one line
		*y += (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
	}
	else if (c != '\r')
	{ // Not a carriage return; is normal char
		uint8_t first = pgm_read_byte(&gfxFont->first),
				last = pgm_read_byte(&gfxFont->last);
		if ((c >= first) && (c <= last))
		{ // Char present in this font?
			GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(
				&gfxFont->glyph))[c - first]);
			uint8_t gw = pgm_read_byte(&glyph->width),
					gh = pgm_read_byte(&glyph->height),
					xa = pgm_read_byte(&glyph->xAdvance);
			int8_t xo = (int8_t)pgm_read_byte(&glyph->xOffset),
				   yo = (int8_t)pgm_read_byte(&glyph->yOffset);
			if (wrap && ((*x + (((int16_t)xo + gw))) > LCD_WIDTH))
			{
				*x = 0; // Reset x to zero, advance y by one line
				*y += (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
			}
			int16_t x1 = *x + xo,
					y1 = *y + yo,
					x2 = x1 + gw - 1,
					y2 = y1 + gh - 1;
			if (x1 < *minx)
				*minx = x1;
			if (y1 < *miny)
				*miny = y1;
			if (x2 > *maxx)
				*maxx = x2;
			if (y2 > *maxy)
				*maxy = y2;
			*x += xa;
		}
	}
}

/**************************************************************************/
/*!
	@brief    Helper to determine size of a string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
	@param    str     The ascii string to measure
	@param    x       The current cursor X
	@param    y       The current cursor Y
	@param    x1      The boundary X coordinate, set by function
	@param    y1      The boundary Y coordinate, set by function
	@param    w      The boundary width, set by function
	@param    h      The boundary height, set by function
*/
/**************************************************************************/
ITCM void LCDDriver_getTextBounds(char text[], uint16_t x, uint16_t y, uint16_t *x1, uint16_t *y1, uint16_t *w, uint16_t *h, GFXfont *gfxFont)
{
	uint8_t c; // Current character

	*x1 = x;
	*y1 = y;
	*w = *h = 0;

	int16_t minx = LCD_WIDTH, miny = LCD_HEIGHT, maxx = 0, maxy = 0;

	for (uint16_t i = 0; i < 40 && text[i] != NULL; i++)
	{
		c = text[i];
		LCDDriver_charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy, gfxFont);
	}

	if (maxx >= minx)
	{
		*x1 = (uint16_t)minx;
		*w = (uint16_t)(maxx - minx + 1);
	}
	if (maxy >= miny)
	{
		*y1 = (uint16_t)miny;
		*h = (uint16_t)(maxy - miny + 1);
	}
}

//Image print (RGB 565, 2 bytes per pixel)
ITCM void LCDDriver_printImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *data)
{
	uint32_t n = w * h * 2;
	LCDDriver_SetCursorAreaPosition(x, y, w + x - 1, h + y - 1);
	for (uint32_t i = 0; i < n; i += 2)
	{
		LCDDriver_SendData((uint16_t)((data[i] << 8) | data[i + 1]));
	}
}

ITCM void LCDDriver_printImage_RLECompressed(uint16_t x, uint16_t y, const tIMAGE *image)
{
	uint32_t pixels = image->width * image->height;
	uint32_t i = 0;
	uint32_t decoded = 0;

	LCDDriver_SetCursorAreaPosition(x, y, image->width + x - 1, image->height + y - 1);
	while (true)
	{
		if ((int16_t)image->data[i] < 0) // no repeats
		{
			uint16_t count = (-(int16_t)image->data[i]);
			i++;
			for (uint16_t p = 0; p < count; p++)
			{
				LCDDriver_SendData(image->data[i]);
				decoded++;
				i++;
				if (pixels <= decoded)
					return;
			}
		}
		else //repeats
		{
			uint16_t count = ((int16_t)image->data[i]);
			i++;
			for (uint16_t p = 0; p < count; p++)
			{
				LCDDriver_SendData(image->data[i]);
				decoded++;
				if (pixels <= decoded)
					return;
			}
			i++;
		}
	}
}

ITCM inline uint16_t rgb888torgb565(uint_fast8_t r, uint_fast8_t g, uint_fast8_t b)
{
	return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xFF) >> 3));
}
