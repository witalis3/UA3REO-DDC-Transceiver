#ifndef WIFI_H
#define WIFI_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include "settings.h"

#define WIFI_ANSWER_BUFFER_SIZE (512 * 8)
#define WIFI_LINE_BUFFER_SIZE WIFI_ANSWER_BUFFER_SIZE
#define WIFI_HTML_RESP_BUFFER_SIZE (512 * 4)
#define WIFI_RLE_BUFFER_SIZE (512)
#define WIFI_COMMAND_DELAY 10
#define WIFI_COMMAND_TIMEOUT 5000
#define WIFI_FOUNDED_AP_MAXCOUNT 16

typedef enum
{
	WIFI_UNDEFINED,
	WIFI_NOTFOUND,
	WIFI_REINIT,
	WIFI_INITED,
	WIFI_CONFIGURED ,
	WIFI_CONNECTING,
	WIFI_READY,
	WIFI_PROCESS_COMMAND,
	WIFI_TIMEOUT,
	WIFI_FAIL,
	WIFI_SLEEP,
} WiFiState;

typedef enum
{
	WIFI_COMM_NONE,
	WIFI_COMM_LISTAP,
	WIFI_COMM_GETSNTP,
	WIFI_COMM_GETIP,
	WIFI_COMM_GETSTATUS,
	WIFI_COMM_DEEPSLEEP,
	WIFI_COMM_CREATESERVER,
	WIFI_COMM_SENDTCPDATA,
	WIFI_COMM_UPDATEFW,
	WIFI_COMM_TCP_CONNECT,
	WIFI_COMM_TCP_GET_RESPONSE,
	WIFI_COMM_SW_RESTART,
} WiFiProcessingCommand;

extern RTC_HandleTypeDef hrtc;

extern bool WIFI_connected;
extern bool WIFI_CAT_server_started;
extern volatile WiFiState WIFI_State;
extern volatile char WIFI_FoundedAP[WIFI_FOUNDED_AP_MAXCOUNT][MAX_WIFIPASS_LENGTH];
extern bool WIFI_IP_Gotted;
extern char WIFI_IP[15];
extern char WIFI_AP[MAX_WIFIPASS_LENGTH];

extern void WIFI_Init(void);
extern void WIFI_Process(void);
extern bool WIFI_ListAP(void (*callback)(void));
extern bool WIFI_GetSNTPTime(void (*callback)(void));
extern bool WIFI_GetIP(void (*callback)(void));
//extern void WIFI_GetStatus(void);
extern void WIFI_GoSleep(void);
extern bool WIFI_StartCATServer(void (*callback)(void));
extern bool WIFI_UpdateFW(void (*callback)(void));
extern bool WIFI_SendCatAnswer(char *data, uint32_t link_id, void (*callback)(void));
extern bool WIFI_getHTTPpage(char *host, char *url, void (*callback)(void), bool https);
extern void WIFI_getRDA(void);
extern void WIFI_getDXCluster(void);
extern void WIFI_getPropagination(void);
extern bool WIFI_SW_Restart(void (*callback)(void));

#endif
