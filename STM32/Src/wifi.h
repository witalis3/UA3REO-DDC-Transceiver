#ifndef WIFI_H
#define WIFI_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define WIFI_ANSWER_BUFFER_SIZE (512*8)
#define WIFI_COMMAND_DELAY 10
#define WIFI_COMMAND_TIMEOUT 5000
#define WIFI_FOUNDED_AP_MAXCOUNT 10
#define WIFI_DEBUG (true || TRX.Debug_Console) //WIFI debug output to console

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
	WIFI_COMM_GETSNTP = 0x02U,
	WIFI_COMM_GETIP = 0x03U,
	WIFI_COMM_GETSTATUS = 0x04U,
	WIFI_COMM_DEEPSLEEP = 0x05U,
	WIFI_COMM_CREATESERVER = 0x06U,
	WIFI_COMM_SENDTCPDATA = 0x07U,
	WIFI_COMM_UPDATEFW = 0x08U,
	WIFI_COMM_TCP_CONNECT = 0x09U,
	WIFI_COMM_TCP_GET_RESPONSE = 0x10U,
} WiFiProcessingCommand;

extern RTC_HandleTypeDef hrtc;

extern bool WIFI_connected;
extern bool WIFI_CAT_server_started;
extern volatile uint8_t WIFI_InitStateIndex;
extern volatile WiFiState WIFI_State;
extern volatile char WIFI_FoundedAP[10][32];
extern bool WIFI_IP_Gotted;
extern char WIFI_IP[15];

extern void WIFI_Init(void);
extern void WIFI_Process(void);
extern void WIFI_ListAP(void *callback);
extern bool WIFI_GetSNTPTime(void *callback);
extern bool WIFI_GetIP(void *callback);
//extern void WIFI_GetStatus(void);
extern void WIFI_GoSleep(void);
extern bool WIFI_StartCATServer(void *callback);
extern bool WIFI_UpdateFW(void *callback);
extern bool WIFI_SendCatAnswer(char *data, uint32_t link_id, void *callback);
extern bool WIFI_getHTTPpage(char* host, char* url, void *callback, bool https);
extern void WIFI_getRDA(void);
extern void WIFI_getPropagination(void);

#endif
