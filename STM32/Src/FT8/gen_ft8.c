#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "encode.h"
#include "pack.h"

#include "gen_ft8.h"

#include <stdio.h>

#include "arm_math.h"
#include "decode_ft8.h"
#include "locator_ft8.h"
#include <string.h>

#include "FT8_GUI.h"
#include "lcd.h"        //For debug
#include "lcd_driver.h" //For debug

static char message[20];
extern char Station_Call[];
extern char Locator[];

// int message_state;

void set_message(uint16_t index) {

	//	char ctmp[20] = {0};		//Debug

	// char big_gulp[60];
	uint8_t packed[K_BYTES];
	char blank[] = "                   ";
	char seventy_three[] = "RR73";
	char seventy_three2[] = "73";
	// char Reply_State[20];

	strcpy(message, blank);
	//		message_state = 0;

	switch (index) {

	case 0:
		sprintf(message, "%s %s %s", "CQ", Station_Call, Locator);
		break;

	case 1:
		sprintf(message, "%s %s %s", Target_Call, Station_Call, Locator); // Answer a "CQ"
		break;

	case 2:
		sprintf(message, "%s %s %3i", Target_Call, Station_Call, Target_RSL);
		// case 2:  sprintf(message,"%s %s", Target_Call,Station_Call);
		break;

	case 3:
		sprintf(message, "%s %s %3s", Target_Call, Station_Call, seventy_three);
		break;

	case 4:
		sprintf(message, "%s %s R%3i", Target_Call, Station_Call,
		        Target_RSL); // Send a raport for the - Answer "CQ" case (diffrence from "2" is the "R")
		break;

	case 5:
		sprintf(message, "%s %s %s", Target_Call, Station_Call,
		        seventy_three2); // Send a "73" for the - Answer "CQ" case (diffrence from "3" is the missing "RR")
		break;
	}

	// Debug
	FT8_Print_TX_Mess(message);

	pack77_1(message, packed);
	genft8(packed, tones);
}
