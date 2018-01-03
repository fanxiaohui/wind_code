/*
 * ConnectList.h
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#ifndef CONNECTLIST_H_
#define CONNECTLIST_H_

#include <arpa/inet.h>

#include "DataQueue.h"
#include "XGByteQueue.h"
#include "XGTime.h"

typedef enum {
	E_SER_WAIT_CLIENT_LOGIN, //等待客户端的登录包。
	E_SER_WAIT_CONECT,//等待去连接服务器
	E_SER_CONNECTING,//已经开启了连接服务器任务，等待任务返回结果
	E_SER_WAIT_LOGIN_RESULT,//等待登录服务器的返回信息。
	//E_CONNECTED,//已经连接服务器
	E_SER_LOGINED,//已经登录
	//E_CLOSING,//
	//E_CLOSED,//
	//E_LOCAL,//
} e_server_status_t;

typedef enum {
	UNKNOWN, CONNECT_DEVICE, CONNECT_APP,
} e_connect_type_t;

typedef enum {
	E_CLI_WAIT_LOGIN,E_CLI_LOGINED,
}e_client_status_t;
//enum {
//	E_SERVER_CONNECT, //yi jing lian jie le yun duan
//	E_LOCAL_CONNECT// zhi you ben di lian jie
//};

struct Client {
	int SocketHandle;
	ByteQueue recvBuffer;
	int islogined;
	struct XGQueue sendQueue;
	struct sockaddr_in Addr;
	e_client_status_t status;
	XGTimeMs time;
};

struct Server {
	int SocketHandle;
	ByteQueue recvBuffer;
	int islogined;
	struct XGQueue sendQueue;
	e_server_status_t status;
	XGTimeMs time;
};

enum DisconnectReason {
	E_DIS_CLIENT_CLOSE,//子设备连接关闭
	//E_DIS_SERVER_CLOSE,//服务器通讯关闭
	E_DIS_CONNECT_REQUEST_ERROR,//子设备的connect协议错误
	E_DIS_CLIENT_LOGIN_TIMEOUT,//等待子设备的登录包超时。
	E_DIS_CLIENT_PING_TIMEOUT,//子设备心跳超时
};


typedef struct Connect {
	struct Connect *next;
	struct Client client;
	struct Server server;
	unsigned int device_id;
	//unsigned int islogined;
	//unsigned int isLocalConnect;//biao zhi shi fou dai li le yun duan tong xun.
	e_connect_type_t type;
	unsigned int isActive;
	unsigned int stata;
	int disconnectReason;
	unsigned char loginData[64];
	unsigned int  loginDatalength;
}*Connect_t;

extern Connect_t XGConnectFactory();
extern void XGConnectRelease(Connect_t conn);

#endif /* CONNECTLIST_H_ */
