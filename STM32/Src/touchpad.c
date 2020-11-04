#include "touchpad.h"
#include "main.h"

#if (defined(HAS_TOUCHPAD))

void TOUCHPAD_Init(void)
{
	#if (defined(TOUCHPAD_GT911))
		//read touchpad info
		GT911_ReadStatus();
		GT911_ReadFirmwareVersion();
		
		//calibrate
		uint8_t send = 3;
		GT911_WR_Reg(0x8040, (uint8_t *)&send, 1); //Reference capacitance update (Internal test);
	#endif
}

void TOUCHPAD_ProcessInterrupt(void)
{
	#if (defined(TOUCHPAD_GT911))
		GT911_Scan();
	#endif
}

void TOUCHPAD_reserveInterrupt(void)
{
	#if (defined(TOUCHPAD_GT911))
		GT911.Touch = 1;
	#endif
}

#endif
