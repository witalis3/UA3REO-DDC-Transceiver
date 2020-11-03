#ifndef LCD_Layout_h
#define LCD_Layout_h

#include "lcd_driver.h"

//LCD dimensions defines
#if (defined(LAY_480x320))
	#include "screen_layout_480x320.h"
#endif
#if (defined(LAY_800x480))
	#include "screen_layout_800x480.h"
#endif

#endif
