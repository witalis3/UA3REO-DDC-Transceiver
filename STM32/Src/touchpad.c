#include "touchpad.h"
#include "main.h"
#include "lcd.h"

#if (defined(HAS_TOUCHPAD))

static bool touched = false;
static bool hold_touch_handled = false;
static bool hold_swipe_handled = false;
static uint32_t touch_startime = 0;
static uint32_t touch_lasttime = 0;
static uint16_t touch_start_x = 0;
static uint16_t touch_start_y = 0;
static uint16_t touch_end_x = 0;
static uint16_t touch_end_y = 0;

void TOUCHPAD_Init(void)
{
#if (defined(TOUCHPAD_GT911))
	//read touchpad info
	GT911_ReadStatus();
	GT911_ReadFirmwareVersion();
	GT911_Init();

	//calibrate
	uint8_t send = 3;
	GT911_WR_Reg(0x8040, (uint8_t *)&send, 1); //Reference capacitance update (Internal test);
#endif
}

void TOUCHPAD_ProcessInterrupt(void)
{
#if (defined(TOUCHPAD_GT911))
	GT911_Scan();

	if (touched && !hold_swipe_handled && (touch_lasttime < (HAL_GetTick() - TOUCHPAD_TIMEOUT)) && ((touch_lasttime - touch_startime) <= TOUCHPAD_CLICK_TIMEOUT) && ((touch_lasttime - touch_startime) >= TOUCHPAD_CLICK_THRESHOLD))
	{
		LCD_processTouch(touch_end_x, touch_end_y);
		touched = false;
	}
	else if (touched && !hold_touch_handled && !hold_swipe_handled && (touch_lasttime > (HAL_GetTick() - TOUCHPAD_TIMEOUT)) && ((touch_lasttime - touch_startime) >= TOUCHPAD_HOLD_TIMEOUT))
	{
		hold_touch_handled = true;
		LCD_processHoldTouch(touch_end_x, touch_end_y);
	}
	else if (touched && (touch_lasttime > (HAL_GetTick() - TOUCHPAD_TIMEOUT)))
	{
		if (hold_swipe_handled || abs(touch_end_x - touch_start_x) > TOUCHPAD_SWIPE_THRESHOLD_PX || abs(touch_end_y - touch_start_y) > TOUCHPAD_SWIPE_THRESHOLD_PX)
			if (LCD_processSwipeTouch(touch_start_x, touch_start_y, (touch_end_x - touch_start_x), (touch_end_y - touch_start_y)))
			{
				touch_start_x = touch_end_x;
				touch_start_y = touch_end_y;
				hold_swipe_handled = true;
			}
	}
	else if (touched && (touch_lasttime < (HAL_GetTick() - TOUCHPAD_TIMEOUT)))
	{
		touched = false;
		hold_touch_handled = false;
		hold_swipe_handled = false;
	}
#endif
}

void TOUCHPAD_reserveInterrupt(void)
{
#if (defined(TOUCHPAD_GT911))
	GT911.Touch = 1;
#endif
}

void TOUCHPAD_processTouch(uint16_t x, uint16_t y)
{
	touch_end_x = x;
	touch_end_y = y;
	touch_lasttime = HAL_GetTick();

	if (!touched)
	{
		touched = true;
		touch_startime = touch_lasttime;
		touch_start_x = x;
		touch_start_y = y;
	}
}

#endif
