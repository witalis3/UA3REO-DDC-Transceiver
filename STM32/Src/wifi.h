#ifndef WIFI_H
#define WIFI_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define WIFI_ANSWER_BUFFER_SIZE 512
#define WIFI_COMMAND_DELAY 10
#define WIFI_COMMAND_TIMEOUT 5000
#define WIFI_FOUNDED_AP_MAXCOUNT 10
#define WIFI_DEBUG false //вывод отладки WIFI в консоль

typedef enum
{
	WIFI_UNDEFINED = 0x00U,
	WIFI_NOTFOUND = 0x01U,
	WIFI_INITED = 0x02U,
	WIFI_CONFIGURED = 0x03U,
	WIFI_CONNECTING = 0x04U,
	WIFI_READY = 0x05U,
	WIFI_PROCESS_COMMAND = 0x06U,
	WIFI_TIMEOUT = 0x07U,
	WIFI_FAIL = 0x08U,
	WIFI_SLEEP = 0x09U,
} WiFiState;

typedef enum
{
	WIFI_COMM_NONE = 0x00U,
	WIFI_COMM_LISTAP = 0x01U,
	WIFI_COMM_GETSNMP = 0x02U,
	WIFI_COMM_GETIP = 0x03U,
	WIFI_COMM_GETSTATUS = 0x04U,
	WIFI_COMM_DEEPSLEEP = 0x05U,
} WiFiProcessingCommand;

extern RTC_HandleTypeDef hrtc;

extern bool WIFI_connected;
extern volatile uint8_t WIFI_InitStateIndex;
extern volatile WiFiState WIFI_State;
extern volatile char WIFI_FoundedAP[10][32];
extern bool WIFI_IP_Gotted;
extern char WIFI_IP[15];

extern void WIFI_Init(void);
extern void WIFI_Process(void);
extern void WIFI_ListAP(void *callback);
extern bool WIFI_GetSNMPTime(void *callback);
extern bool WIFI_GetIP(void *callback);
//extern void WIFI_GetStatus(void);
extern void WIFI_GoSleep(void);

#endif
