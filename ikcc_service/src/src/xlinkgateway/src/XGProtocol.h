/*
 * XGProtocol.h
 *
 *  Created on: 2016年12月2日
 *      Author: john
 */

#ifndef XGPROTOCOL_H_
#define XGPROTOCOL_H_

#include "XGCore.h"
#include "ConnectList.h"

#ifndef UTIL_INT32_GET_BIT3
#define UTIL_INT32_GET_BIT3(X)  (unsigned char)(((X) & 0xFF000000) >> 24)
#endif

#ifndef UTIL_INT32_GET_BIT2
#define UTIL_INT32_GET_BIT2(X)  (unsigned char)(((X) & 0x00FF0000) >> 16)
#endif

#ifndef UTIL_INT32_GET_BIT1
#define UTIL_INT32_GET_BIT1(X)  (unsigned char)(((X) & 0x0000FF00) >> 8)
#endif

#ifndef UTIL_INT32_GET_BIT0
#define UTIL_INT32_GET_BIT0(X)   (unsigned char)((X) & 0x000000FF)
#endif

#ifndef UTIL_INT16_GET_BITH
#define UTIL_INT16_GET_BITH(X)  (unsigned char)(((X)&0xff00)>>8)
#endif

#ifndef UTIL_INT16_GET_BITL
#define UTIL_INT16_GET_BITL(X)   (unsigned char)((X)&0x00ff)
#endif

#ifndef UTIL_INT32_SET
#define UTIL_INT32_SET(H,N,K,L) (((H)& 0x000000FF) << 24)+(((N) & 0x000000FF) << 16)+(((K) & 0x000000FF) << 8)+((L) & 0x000000FF)
#endif

#ifndef UTIL_INT16_SET
#define UTIL_INT16_SET(H,L) (((H)<<8)+(L))
#endif

enum _Relay {
	E_CONTINUE_RELAY,//需要转发数据到云端
	E_STOP_RELAY,//停止转发这个数据
};

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

} XLINK_TCP_EMESSAGETYPE;

/**
 * 固定协议头定义
 */
typedef volatile union {
	/*unsigned*/char byte; /**< the whole byte */
#if defined(REVERSED)
	struct
	{
		unsigned int type : 4; /** 任务类型 */
		unsigned int resp : 1; /** bit or request(0) or response(1) */
		unsigned int version : 3;/** 预留3 bits */
	}bits;
#else
	struct {
		unsigned int version :3;/** 预留3 bits */
		unsigned int resp :1; /** bit or request(0) or response(1) */
		unsigned int type :4; /** 任务类型 */
	} bits;
#endif
} XGMessage;

extern void XGProtocol(Connect_t conn, XGCoreCtx ctx);

#endif /* XGPROTOCOL_H_ */
