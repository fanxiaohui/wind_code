/*
 * XGCore.h
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#ifndef XGCORE_H_
#define XGCORE_H_
#include <pthread.h>
#include "ConnectList.h"
#include "XGageway.h"
#include "ConnectTask.h"

enum ErrorCode {
	EC_NO_ERROR, EC_ALREADY_INIT, EC_CREATE_TCP_SERVER_FAILED, EC_CREATE_UDP_SERVER_FAILED,
};

//typedef enum NetStatus_t {
//	E_NET_OK, E_NET_CHECK_ING, E_NET_WAIT_CHECK, E_NET_FAILED,
//} NetStatus;
//
//typedef enum CheckNet {
//	E_IS_NOT_NEED, E_IS_NEED,
//} CheckNet;

#define  CORE_IP_ADDRESS_BUFFER_SIZE 32

typedef struct XGCore {
	unsigned int isInited;//是否初始
	unsigned int isQuit;//是否退出
	unsigned int isRuning;//是否在运行标识
	int ServerTcpSocketHandle;
	int ServerUdpSocketHandle;
	int LocalTcpServerPort;
	int LocalUdpServerPort;
	int ErrorCode;
	//int isNeedCheckNet;
	//int isStartCheckNet;
	//NetStatus netStatus;
	char Host[128];
	char *corp;
	char *token;
	char *productid;
	int ServerPort;
	struct Connect Connects;
	struct ConnectTask ConnectTasks;
	struct ConnectTask ConnectTasksResult;
	struct XGQueue ClientSendQueue;
	struct XGQueue CloudSendQueue;
	XGConfig *config;
	pthread_mutex_t lockSendData;
	pthread_mutex_t lockConnectServerTask;
	//pthread_cond_t  condConnectServerTask;
	pthread_mutex_t lockConnectServerResult;
}*XGCoreCtx;

enum {
	E_TO_DEVICE, E_TO_CLOUD,
};

#define XGCORE_SIZE  sizeof(struct XGCore)

extern int XGStartCore(XGCoreCtx ctx);
extern int XGStopCore(XGCoreCtx ctx);
extern void XGIgoreSignal(void);
extern XGCoreCtx XGCoreFactory();
extern void XGCoreRelease(XGCoreCtx ctx);
extern int XGCoreSendData(XGCoreCtx ctx, XGQueueItem item, int where);

#endif /* XGCORE_H_ */
