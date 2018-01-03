#ifndef XLINK_TCP_TYPE_H_
#define XLINK_TCP_TYPE_H_
#ifdef  __cplusplus
extern "C" {
#endif
#include "xlink_message.h"
#include "xlink_type.h"

#define TCP_KEEPALIVETIME 60

typedef enum _tcp_eMessageType {
	TCP_TYPE_ACTIVATE = 0,
	TCP_TYPE_LOGIN = 1,
	TCP_TYPE_CONNECT = 2,
	TCP_TYPE_SET = 3,
	TCP_TYPE_SYNC = 4,
	TCP_TYPE_SETPWD = 5,
	TCP_TYPE_COLLECT = 6,
	TCP_TYPE_PIPE = 7,
	TCP_TYPE_PIPE_2 = 8,
	TCP_TYPE_SUBSCRIBE = 9,
	TCP_TYPE_PROBE = 10,
	TCP_TYPE_EVENT = 12,
	TCP_TYPE_PING = 13,
	TCP_TYPE_DISCONNECT = 14,
//TCP_TYPE_KEY,

} XLINK_TCP_EMESSAGETYPE;

typedef enum XLINK_TCP_WORK_TYPE_T {
	TCP_NOT_CONNECT,//
	TCP_SEND_HTTPCONNECT, //
	TCP_WAIT_HTTP_RESULT, //
	TCP_ACTIVATION, //
	TCP_WAIT_ACTIVE_RESULT,//
	TCP_CONNECT, //
	TCP_WAIT_CONNECT_RESULT,//
	//TCP_NEED_SEND_UPGRADE, //
	TCP_PING, /*ping*/
	TCP_WAIT_PING_RESULT,//

} XLINK_TCP_WORK_TYPE;

//typedef enum XLINK_TCP_WORK_STATUS_T {
//	TCP_STATUS_START, //
//	TCP_STATUS_ING, //
//	TCP_STATUS_END, //
//} XLINK_TCP_WORK_STATUS;

//typedef struct XLINK_TCP_WORK {
//	//XLINK_TCP_WORK_STATUS work_sta;
//	XLINK_TCP_WORK_TYPE work_type;
//} XLINK_TCP_WORK;

typedef struct XLINK_TCP_CLIENT {
	//XLINK_TCP_WORK work;
	XLINK_TCP_WORK_TYPE work_type;
	xsdk_time_t lastPingTime;
} XLINK_TCP_CLIENT;

#ifdef  __cplusplus
}
#endif
#endif /* XLINK_TCP_TYPE_H_ */
