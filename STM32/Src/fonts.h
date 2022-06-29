#ifndef FONTS_H_
#define FONTS_H_

#include <stdbool.h>
#include "hardware.h"
#include "lcd_driver.h"
#include <stdlib.h>
#include <stdio.h>

#define RASTR_FONT_W 6
#define RASTR_FONT_H 8
#define RASTR_FONT_4x6_W 4
#define RASTR_FONT_4x6_H 6

extern const GFXfont FreeSans7pt7b;
extern const GFXfont FreeSans9pt7b;
extern const GFXfont FreeSans12pt7b;
extern const GFXfont FreeSans18pt7b;
extern const GFXfont FreeSans24pt7b;
extern const GFXfont FreeSans32pt7b;
extern const GFXfont FreeSans36pt7b;
extern const GFXfont Quito32pt7b;
extern const GFXfont Quito36pt7b;
extern const GFXfont DS_DIGIT18pt7b;
extern const GFXfont DS_DIGIT32pt7b;
extern const GFXfont DS_DIGIT36pt7b;
extern const unsigned char rastr_font[];
extern const unsigned char rastr_font_4x6[];

#endif
