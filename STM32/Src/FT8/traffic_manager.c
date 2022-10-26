#include <stdint.h>
#include "traffic_manager.h"
#include "decode_ft8.h"
#include "gen_ft8.h"

#include "FT8_main.h"
#include "FT8_GUI.h"

#include "trx_manager.h"
#include "sd.h"

#include "lcd_driver.h" //For debug
#include "lcd.h"		//For debug

//extern RTC_HandleTypeDef hrtc; // used for the "date" and "time" acquisition

uint16_t cursor_freq;  // the AF frequency wich will be tansmited now (roughly from 0 to 3kHz)
uint32_t FT8_BND_Freq; // frequency for the FT8 on the current Band
int xmit_flag, ft8_xmit_counter;

char Station_Call[7]; // six character call sign + /0 for example  => "DB5AT"
char Locator[7];	  // four character locator  + /0	for example = "JN48"

#define FT8_TONE_SPACING 6.25

//extern uint16_t cursor_freq;  // the AF frequency wich will be tansmited now (roughly from 0 to 3kHz)
//extern uint32_t FT8_BND_Freq; // frequency for the FT8 on the current Band
extern uint16_t cursor_line;

int Beacon_State; //
extern int num_decoded_msg;

uint32_t F_Offset;

char QSODate[9];	// string containing the potential QSO date
char QSOOnTime[7];	// potential QSO Start time
char QSOOffTime[7]; // potential QSO Stop time

void transmit_sequence(void)
{
	Set_Data_Colection(0); // Disable the data colection
	//	FT8_ColectDataFlg = false;  	//Disable the Data colection
	//	FT8_DatBlockNum = 0; 					// Reset the data buffer

	set_Xmit_Freq(FT8_BND_Freq, cursor_freq); // Set band frequency and the frequency in the FT8 (cursor freq.)
	TRX_Tune = true;

	//      si5351.set_freq(F_Long, SI5351_CLK0);
	//      si4735.setVolume(35);
	//      si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set for max power if desired
	//      si5351.output_enable(SI5351_CLK0, 1);
	//      pinMode(PTT_Pin, OUTPUT);
	//      digitalWrite(PTT_Pin, HIGH);
}

void receive_sequence(void)
{
	Set_Data_Colection(0); // Disable the data colection (it will be enabled by next 15s marker)
	xmit_flag = 0;		   // disable if transmit was activeted

	set_Xmit_Freq(FT8_BND_Freq, 0); // Set band frequency and the frequency in the FT8 (cursor freq.)
	TRX_Tune = false;

	//       si5351.output_enable(SI5351_CLK0, 0);
	//       pinMode(PTT_Pin, OUTPUT);
	//       digitalWrite(PTT_Pin, LOW);
	//       si4735.setVolume(50);
	//    clear_FT8_message();
}

void tune_On_sequence(void)
{
	Set_Data_Colection(0);					  // Disable the data colection
	set_Xmit_Freq(FT8_BND_Freq, cursor_freq); // Set band frequency and the frequency in the FT8 (cursor freq.)
	TRX_Tune = true;

	//      si5351.set_freq(F_Long, SI5351_CLK0);
	//      si4735.setVolume(35);
	//      si5351.output_enable(SI5351_CLK0, 1);
	//      pinMode(PTT_Pin, OUTPUT);
	//      digitalWrite(PTT_Pin, HIGH);
}

void tune_Off_sequence(void)
{
	Set_Data_Colection(0);			// Disable the data colection (it will be enabled by next 15s marker)
	set_Xmit_Freq(FT8_BND_Freq, 0); // Set band frequency and the frequency in the FT8 (cursor freq.)
	TRX_Tune = false;

	//       si5351.output_enable(SI5351_CLK0, 0);
	//       pinMode(PTT_Pin, OUTPUT);
	//       digitalWrite(PTT_Pin, LOW);
	//       si4735.setVolume(50);
}

void set_Xmit_Freq(uint32_t BandFreq, uint16_t Freq)
{
	uint32_t F_Long;

	//     F_Long = (uint64_t) ((currentFrequency * 1000 + cursor_freq + offset_freq) * 100);
	F_Long = BandFreq * 1000 + Freq; // BandFreq is in kHz and add the needed offset
	TRX_setFrequency(F_Long, CurrentVFO);

	//      si5351.set_freq(F_Long, SI5351_CLK0);
}

void set_FT8_Tone(char ft8_tone)
{
	//char ctmp[20] = {0}; // Debug
	uint32_t F_FT8;

	//  F_FT8 =  F_Long + ft8_tone * FT8_TONE_SPACING;

	F_FT8 = ft8_tone * FT8_TONE_SPACING;
	set_Xmit_Freq(FT8_BND_Freq, cursor_freq + F_FT8);

	//          si5351.set_freq(F_FT8, SI5351_CLK0);
}

void setup_to_transmit_on_next_DSP_Flag(void)
{
	ft8_xmit_counter = 0;
	transmit_sequence();
	//	set_Xmit_Freq(FT8_BND_Freq,cursor_freq);				//Set band frequency and the frequency in the FT8 (cursor freq.)
	xmit_flag = 1;
}

void service_CQ(void)
{
#define MaxAttempt_Tries 3 // The Attempt tries to send the same mesage (if reached => jump to "CQ" call)
	int receive_index;
	static uint8_t Attempt_Count = 0; // Count the attempts to send the same message

	char ctmp[20] = {0}; // Debug

	receive_index = Check_Calling_Stations(num_decoded_msg); // index of the station that called us (last if we got more calls)

	if (receive_index == -1) // if we didn't recieved mesage calling us
	{
		if (Beacon_State > 1) // if conversation was initiated
		{
			if (Beacon_State == 5) // we just started the "CQ Answer" call
				Attempt_Count = 0; // set the Attempt_Count (the call is already done once - initialised by pressing the button in "FT8_GUI.c")

			Attempt_Count++;
			if (Attempt_Count < MaxAttempt_Tries)
			{
				if (Beacon_State == 2)
					Beacon_State = 20; // Repeat the "report signal" call
				else if (Beacon_State == 3)
					Beacon_State = 30; // Repeat the "73" call
				else if (Beacon_State == 5)
					Beacon_State = 50; // Repeat the "CQ Answer" call
			}
			else // We tried enough without answer => go back to "CQ" call
			{
				FT8_Clear_TargetCall(); // Clear the place on the display for the "TargetCall"
				Beacon_State = 1;		// Set Call - "CQ"
				Attempt_Count = 0;
			}
		}
		else				  // The conversation was not initiated => then we just call CQ
			Beacon_State = 1; // Set Call - "CQ"
	}						  // if(receive_index == -1) - we didn't recieved mesage calling us

	else if (receive_index >= 0) // we did recieve a message calling us
	{
		if (Beacon_State == 1) // if previous state was "call CQ" -> then we got an answer -> report the signal level
		{
			GetQSOTime(1);	   // Record the QSO Start Time
			Beacon_State = 2;  // Set call - report signal
			Attempt_Count = 0; // zero the attempt counter
		}
		else if ((Beacon_State == 2) || (Beacon_State == 20)) // if previous state was "report signal" ->
		{
			receive_index = FindPartnerIDX(num_decoded_msg); // find the index of the station that we were talking till now
			if (receive_index >= 0)
			{
				Attempt_Count = 0;							  // zero the attempt counter
				if (CheckRecievedRaportRSL(receive_index, 0)) // check if the oposite side answered corespondingly
					Beacon_State = 3;						  // Set call - "RR73"
			}
			else // The answer we got it was not from the station we were talking till now
				Beacon_State = 20;
		}
		else if ((Beacon_State == 3) || (Beacon_State == 30)) // if previous state was call - "RR73"
		{
			receive_index = FindPartnerIDX(num_decoded_msg); // find the index of the station that we were talking till now
			if (receive_index >= 0)
			{
				Attempt_Count = 0;					   // zero the attempt counter
				if (CheckRecieved73(receive_index, 0)) // check if the oposite side answered corespondingly
				{
					Beacon_State = 1; // if yes then we are done and can go further (call next "CQ")
					LogQSO();
					FT8_Clear_TargetCall(); // Clear the place on the display for the "TargetCall"
				}
			}
			else // The answer we got it was not from the station we were talking till now
				Beacon_State = 30;
		}
		// end the CALL initiated by us
		//------------------------------------------------------
		// The CALL initiated by oposite side (we answer their CQ)
		else if ((Beacon_State == 5) || (Beacon_State == 50)) // if previous state was "answer CQ"
		{
			receive_index = FindPartnerIDX(num_decoded_msg); // find the index of the station that we were talking till now
			if (receive_index >= 0)
			{
				Attempt_Count = 0;							  // zero the attempt counter
				if (CheckRecievedRaportRSL(receive_index, 1)) // check if the oposite side answered corespondingly
				{
					Beacon_State = 6; // Send RSL
					GetQSOTime(1);	  // Record the QSO Start Time
				}
			}
			else				   // The answer we got it was not from the station we were talking till now
				Beacon_State = 50; // Try again
		}
		else if (Beacon_State == 6) // if previous state was send RSL
		{
			receive_index = FindPartnerIDX(num_decoded_msg); // find the index of the station that we were talking till now
			if (receive_index >= 0)
			{
				Attempt_Count = 0;					   // zero the attempt counter
				if (CheckRecieved73(receive_index, 1)) // check if the oposite side answered corespondingly
				{
					Beacon_State = 7; // Set call - "73"
					LogQSO();
				}
			}
			else				  // The answer we got it was not from the station we were talking till now
				Beacon_State = 6; // Try again
		}
	}

	//!
	//! Need to check all loops if everything makes sence
	//!

	// Debug
	sprintf(ctmp, "Beacon_State: %d ", Beacon_State);
	LCDDriver_printText(ctmp, 241, 260, COLOR_GREEN, COLOR_BLACK, 2);

	switch (Beacon_State)
	{
	// CALL initiated by us
	case 1:				// Send CQ
		set_message(0); // send CQ
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 2:								  // send RSL
		SetNew_TargetCall(receive_index); // put in to the buffers the "Target_Call" and the "Target_Grid" (the index of the message who answered us)
		FT8_Print_TargetCall();
		set_message(2); // send RSL
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 3:				// send 73
		set_message(3); // send 73
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 20:			// repat send RSL
		set_message(2); // send RSL
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 30:			// repeat send 73
		set_message(3); // send 73
		setup_to_transmit_on_next_DSP_Flag();
		break;
		// end CALL initiated by us

		//------------------------------------------

		// The CALL initiated by oposite side

	case 6: // send a raport to a "CQ answer"
		// SetNew_TargetCall(receive_index);
		set_message(4); // send a raport to a "CQ answer"
		setup_to_transmit_on_next_DSP_Flag();
		break;

	case 7:				// send a "73" to a "CQ answer"
		set_message(5); // send a 73 to a "CQ answer"
		setup_to_transmit_on_next_DSP_Flag();
		Beacon_State = 8; // Disable the CQ after the mesage is transmited - see in the "FT8_main.c"
		break;

	case 50:			// repat the send "answer CQ"
		set_message(1); // send answer a "CQ"
		setup_to_transmit_on_next_DSP_Flag();
		break;
	}
}

void GetQSODate(void)
{
	RTC_DateTypeDef sDate = {0};
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	sprintf(QSODate, "20%02d%02d%02d", sDate.Year, sDate.Month, sDate.Date);
}

// "QSO_Start" flag showig to take the QSO "Start" or "Stop" time
void GetQSOTime(uint8_t QSO_Start)
{
	uint32_t Time = RTC->TR;
	int8_t Hours = ((Time >> 20) & 0x03) * 10 + ((Time >> 16) & 0x0f) - WIFI.Timezone; // corect the time to be in UTC
	uint8_t Minutes = ((Time >> 12) & 0x07) * 10 + ((Time >> 8) & 0x0f);
	uint8_t Seconds = ((Time >> 4) & 0x07) * 10 + ((Time >> 0) & 0x0f);

	if (Hours < 0) // due to the time correction
		Hours = 24 + Hours;
	if (Seconds == 60) // Fix the seconds
		Seconds = 0;

	if (QSO_Start == 1)
		sprintf(QSOOnTime, "%02d%02d%02d", Hours, Minutes, Seconds);
	else
		sprintf(QSOOffTime, "%02d%02d%02d", Hours, Minutes, Seconds);

	/*
		RTC_TimeTypeDef sTime = {0};
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

		if(QSO_Start)
			sprintf(QSOOnTime, "%02d%02d%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
		else
			sprintf(QSOOffTime, "%02d%02d%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
	*/
}

void LogQSO(void)
{
	char StrToLog[260];
	uint8_t Len;  //=strlen(ctmp);
	int CR = 0xD; // CR -  ascii code
				  // ctmp[Len+3] = 0;
	char cBND[4]; // for the strng containing the current band

	if (SD_Present)
	{
		switch (FT8_BND_Freq)
		{
		case FT8_Freq_80M:
			strcpy(cBND, "80m");
			break;
		case FT8_Freq_40M:
			strcpy(cBND, "40m");
			break;
		case FT8_Freq_30M:
			strcpy(cBND, "30m");
			break;
		case FT8_Freq_20M:
			strcpy(cBND, "20m");
			break;
		case FT8_Freq_17M:
			strcpy(cBND, "17m");
			break;
		case FT8_Freq_15M:
			strcpy(cBND, "15m");
			break;
		case FT8_Freq_12M:
			strcpy(cBND, "12m");
			break;
		case FT8_Freq_10M:
			strcpy(cBND, "10m");
			break;
		case FT8_Freq_6M:
			strcpy(cBND, "6m");
			break;
		case FT8_Freq_2M:
			strcpy(cBND, "2m");
			break;
		}

		float QSO_Freq = ((float)FT8_BND_Freq + (float)cursor_freq / 1000) / 1000; // Calculate the QSO Frequency in MHz (for example 7.075500)

		GetQSODate();  // Get the date to be able to put it in the Log later
		GetQSOTime(0); // End Time

		// example message
		//<call:5>M0JJF <gridsquare:4>IO91 <mode:3>FT8 <rst_sent:3>-21 <rst_rcvd:3>-18 <qso_date:8>20210403 <time_on:6>090500 <qso_date_off:8>20210403 <time_off:6>090821 <band:3>40m <freq:8>7.075500 <station_callsign:5>DB5AT <my_gridsquare:6>JN48ov <eor>

		char RapRcv_RSL_filtered[5] = {0};
		if(RapRcv_RSL[0] == 'R')
			strcpy(RapRcv_RSL_filtered, RapRcv_RSL + 1);
		else
			strcpy(RapRcv_RSL_filtered, RapRcv_RSL);

		sprintf(StrToLog, " <call:%d>%s <gridsquare:4>%s <mode:3>FT8 <rst_sent:3>%3i <rst_rcvd:%d>%s <qso_date:8>%s <time_on:6>%s <qso_date_off:8>%s <time_off:6>%s <band:3>%s <freq:8>%1.6f <station_callsign:5>%s <my_gridsquare:6>%s <eor>", strlen(Target_Call), Target_Call, Target_Grid, Target_RSL, strlen(RapRcv_RSL_filtered), RapRcv_RSL_filtered, QSODate, QSOOnTime, QSODate, QSOOffTime, cBND, QSO_Freq, Station_Call, Locator);
		StrToLog[0] = CR;

		Len = strlen(StrToLog);

		strcpy((char *)SD_workbuffer_A, "FT8_QSO_Log.txt"); // File name
		strcpy((char *)SD_workbuffer_B, (char *)StrToLog);	// Data to write
		SDCOMM_WRITE_TO_FILE_partsize = Len;

		SD_doCommand(SDCOMM_WRITE_TO_FILE, false);
	}
}