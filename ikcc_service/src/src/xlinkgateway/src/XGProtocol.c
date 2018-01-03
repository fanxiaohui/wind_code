/*
 * XGProtocol.c
 *
 *  Created on: 2016年12月2日
 *      Author: john
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "DataQueue.h"
#include "XGProtocol.h"
#include "XGDebug.h"
#include "XGDebug.h"



static void XGProtocolClient(Connect_t conn, XGCoreCtx ctx);
static void XGProtocolServer(Connect_t conn, XGCoreCtx ctx);

static int XGProDeviceConnectRequest(Connect_t conn, XGQueueItem item, XGCoreCtx ctx);
static int XGProDevicePipeSyncRequest(Connect_t conn, XGQueueItem item, XGCoreCtx ctx);
static int XGProDevicePipeRequest(Connect_t conn, XGQueueItem item, XGCoreCtx ctx);

static int XGProServerDeviceConnectResponse(Connect_t conn, XGQueueItem item, XGCoreCtx ctx);
static int XGProServerPipeRequest(Connect_t conn, XGQueueItem item, XGCoreCtx ctx);

void XGProtocol(Connect_t conn, XGCoreCtx ctx) {
	int size = 0;

	size = XGByteQueueSize(&conn->client.recvBuffer);
	if (size > 4) {
		XGProtocolClient(conn, ctx);
	}
	size += XGByteQueueSize(&conn->server.recvBuffer);
	if (size > 4) {
		XGProtocolServer(conn, ctx);
	}
}

static int XGRemoveConnectByDeviceID(int deviceid, XGCoreCtx ctx) {
	Connect_t conn = ctx->Connects.next;
	while (conn != NULL) {
		if (conn->device_id == deviceid) {
			XGWARNING("remove equal deviceid=%d connect \r\n",deviceid);
			conn->isActive = 0;
			conn->client.islogined = 0;
		}
		conn = conn->next;
	}
	return 0;
}

static int XGProDeviceConnectRequest(Connect_t conn, XGQueueItem item, XGCoreCtx ctx) {
	if (item->dataSize < 10) {
		conn->isActive = 0;
		conn->disconnectReason = E_DIS_CONNECT_REQUEST_ERROR;
		XGWARNING("devcie connect package error and close connect\r\n");
		return E_STOP_RELAY;
	}

	int deviceid = UTIL_INT32_SET(item->data[6], item->data[7], item->data[8], item->data[9]);
	if (deviceid != 0) {
		//移除相同的device id
		XGRemoveConnectByDeviceID(deviceid, ctx);
		conn->device_id = deviceid;
	} else {
		return E_STOP_RELAY;
	}
	XGDEBUG("Devcie connect device id=%d\r\n", deviceid);
	return E_CONTINUE_RELAY;
}

static int XGProDevicePipeSyncRequest(Connect_t conn, XGQueueItem item, XGCoreCtx ctx) {
	if (item->dataSize < 12) {
		return 0;
	}
	//unsigned int deviceid = UTIL_INT32_SET(item->data[5], item->data[6], item->data[7], item->data[8]);
	unsigned short msgid = UTIL_INT16_SET(item->data[9], item->data[10]);
	unsigned int flag = item->data[11];
	unsigned char *data = &item->data[12];
	unsigned int datalength = item->dataSize - 12;
	if (ctx->config) {
		if (ctx->config->OnRecvDataFormDevice) {
			ctx->config->OnRecvDataFormDevice(conn->device_id, 0, msgid, data, datalength, flag);
		}
	}
	return 0;
}
static int XGProDevicePipeRequest(Connect_t conn, XGQueueItem item, XGCoreCtx ctx) {
	if (item->dataSize < 12) {
		return 0;
	}
	unsigned int appid = UTIL_INT32_SET(item->data[5], item->data[6], item->data[7], item->data[8]);
	unsigned short msgid = UTIL_INT16_SET(item->data[9], item->data[10]);
	unsigned int flag = item->data[11];
	unsigned char *data = &item->data[12];
	unsigned int datalength = item->dataSize - 12;
	if (ctx->config) {
		if (ctx->config->OnRecvDataFormDevice) {
			ctx->config->OnRecvDataFormDevice(conn->device_id, appid, msgid, data, datalength, flag);
		}
	}
	return 0;
}
//处理子设备的数据协议
static void XGProtocolClient(Connect_t conn, XGCoreCtx ctx) {
	unsigned char header[5] = { 0x00 };
	int bodylength = 0, ret = 0;
	XGMessage msg;
	int used = XGByteQueueSize(&conn->client.recvBuffer);
	if (used < 5) {
		return;
	}

	XGByteQueuePeer(&conn->client.recvBuffer, header, 5);
	bodylength = (header[1] << 24) + (header[2] << 16) + (header[3] << 8) + header[4];
	if (bodylength + 5 > used) {
		return;
	}

	msg.byte = header[0];
	XGDEBUG("Tcp protocol type=%d\r\n", msg.bits.type);
	if (msg.bits.type == TCP_TYPE_ACTIVATE || msg.bits.type == TCP_TYPE_CONNECT) {
		conn->type = CONNECT_DEVICE;
	} else if (msg.bits.type == TCP_TYPE_LOGIN) {
		conn->type = CONNECT_APP;
	}

	XGQueueItem que = XGDataQueueFactory(bodylength + 5);
	if (que == NULL) {
		XGDEBUG("RecvData Malloc mem failed errno=%d\r\n",errno);
		return;
	}

	que->dataSize = bodylength + 5;
	XGByteQueuePeer(&conn->client.recvBuffer, que->data, bodylength + 5);
	XGByteQueuePop(&conn->client.recvBuffer, bodylength + 5);
	//
	int relay = E_CONTINUE_RELAY;
	if (msg.bits.type == TCP_TYPE_CONNECT) {
		//缓存登录包的信息，用户代理登录云端
		memcpy(conn->loginData, que->data, que->dataSize);
		conn->loginDatalength = que->dataSize;

		relay = XGProDeviceConnectRequest(conn, que, ctx); //记录设备的device id
		if (relay == E_CONTINUE_RELAY) {
			//回调通知用户设备登录
			if (ctx->config->OnDeviceLogin != NULL) {
				ret = ctx->config->OnDeviceLogin(conn->device_id, 0, &conn->client.Addr);
			}
			if (ret == 0) {
				conn->client.islogined = 1;//子设备登录成功
				conn->client.status = E_CLI_LOGINED;
				conn->client.time = XGGetLocalTime();//登录成功的时间， 记录ping 包的时间
				conn->server.status = E_SER_WAIT_CONECT;//进入等待连接云服务器的状态
			}
			//response connect success
			XGMessage resp;
			resp.byte = 0;
			resp.bits.type = TCP_TYPE_CONNECT;
			resp.bits.resp = 1;
			resp.bits.version = msg.bits.version;
			XGQueueItem respitem = XGDataQueueFactory(12);
			if (respitem == NULL) {
				conn->isActive = 0;
				XGDataQueueRelease(que);
				return;
			}
			respitem->next = NULL;
			respitem->dataSize = 7;
			respitem->deviceid = conn->device_id;
			respitem->data[0] = resp.byte;
			respitem->data[1] = 0;
			respitem->data[2] = 0;
			respitem->data[3] = 0;
			respitem->data[4] = 2;
			respitem->data[5] = ret;
			respitem->data[6] = 0;

			XGQueueItem qhead = &conn->client.sendQueue;
			while (qhead->next != NULL) {
				qhead = qhead->next;
			}
			qhead->next = respitem;

			relay = E_STOP_RELAY;
		}
	} else if ((msg.bits.type == TCP_TYPE_PIPE) && (msg.bits.resp == 0)) { //接收到设备的透传数据包
		conn->client.time = XGGetLocalTime();//更新时间
		XGProDevicePipeRequest(conn, que, ctx);
		relay = E_STOP_RELAY;
	} else if ((msg.bits.type == TCP_TYPE_PIPE_2) && (msg.bits.resp == 0)) {//接收到设备的广播数据包
		conn->client.time = XGGetLocalTime();//更新时间
		XGProDevicePipeSyncRequest(conn, que, ctx);
		relay = E_STOP_RELAY;
	} else if (msg.bits.type == TCP_TYPE_PING) {//接收到数据的心跳包
		conn->client.time = XGGetLocalTime();//更新时间
		//if (conn->server.status != E_SER_LOGINED) {//没有登录云端，需要返回ping 包
			XGMessage resp;
			resp.byte = 0;
			resp.bits.type = TCP_TYPE_PING;
			resp.bits.resp = 1;
			resp.bits.version = msg.bits.version;

			XGQueueItem respitem = XGDataQueueFactory(9);
			if (respitem == NULL) {
				XGDataQueueRelease(que);
				return;
			}
			respitem->next = NULL;
			respitem->dataSize = 5;
			respitem->deviceid = conn->device_id;
			respitem->data[0] = resp.byte;
			respitem->data[1] = 0;
			respitem->data[2] = 0;
			respitem->data[3] = 0;
			respitem->data[4] = 0;

			XGQueueItem qhead = &conn->client.sendQueue;
			while (qhead->next != NULL) {
				qhead = qhead->next;
			}
			qhead->next = respitem;

			//relay = E_STOP_RELAY;
		//}
		//已经连接云端，直接将心跳发送到云端
	}

	if (relay == E_CONTINUE_RELAY) {
		XGQueueItem qhead = &conn->server.sendQueue;
		while (qhead->next != NULL) {
			qhead = qhead->next;
		}
		qhead->next = que;
	} else {
		XGDataQueueRelease(que);
	}

}

static int XGProServerDeviceConnectResponse(Connect_t conn, XGQueueItem item, XGCoreCtx ctx) {
	conn->server.status = E_SER_LOGINED;
	conn->server.islogined = 1;
	XGDEBUG("login public server success device id=%d\r\n",conn->device_id);
	return E_CONTINUE_RELAY;
}

static int XGProServerPipeRequest(Connect_t conn, XGQueueItem item, XGCoreCtx ctx) {
	if (item->dataSize < 12) {
		return E_STOP_RELAY;
	}
	unsigned int appid = UTIL_INT32_SET(item->data[5], item->data[6], item->data[7], item->data[8]);
	unsigned short msgid = UTIL_INT16_SET(item->data[9], item->data[10]);
	unsigned int flag = item->data[11];
	unsigned char *data = &item->data[12];
	unsigned int datalength = item->dataSize - 12;
	if (ctx->config) {
		if (ctx->config->OnRecvDataFormCloud) {
			ctx->config->OnRecvDataFormCloud(conn->device_id, appid, msgid, data, datalength, flag);
		}else{
			return E_CONTINUE_RELAY;
		}
	}
	return E_STOP_RELAY;
}

//处理云端返回的数据协议。
static void XGProtocolServer(Connect_t conn, XGCoreCtx ctx) {
	unsigned char header[5] = { 0x00 };
	int bodylength = 0;
	XGMessage msg;

	int used = XGByteQueueSize(&conn->server.recvBuffer);
	if (used < 5) {
		return;
	}

	XGByteQueuePeer(&conn->server.recvBuffer, header, 5);
	bodylength = (header[1] << 24) + (header[2] << 16) + (header[3] << 8) + header[4];
	if (bodylength + 5 > used) {
		return;
	}

	msg.byte = header[0];
	if (msg.bits.type == TCP_TYPE_CONNECT) {
		conn->type = CONNECT_DEVICE;
	} else if (msg.bits.type == TCP_TYPE_LOGIN) {
		conn->type = CONNECT_APP;
	}

	XGQueueItem que = XGDataQueueFactory(bodylength + 5);
	if (que == NULL) {
		XGERROR("RecvData Malloc mem failed errno=%d\r\n",errno);
		return;
	}

	que->dataSize = bodylength + 5;
	XGByteQueuePeer(&conn->server.recvBuffer, que->data, bodylength + 5);
	XGByteQueuePop(&conn->server.recvBuffer, bodylength + 5);

	int relay = E_CONTINUE_RELAY;

	if (msg.bits.type == TCP_TYPE_CONNECT) {
		relay = XGProServerDeviceConnectResponse(conn, que, ctx);
	} else if ((msg.bits.type == TCP_TYPE_PIPE) && (msg.bits.resp == 0)) {//处理云端的透传数据包
		relay = XGProServerPipeRequest(conn, que, ctx);
	}

	if (relay == E_CONTINUE_RELAY) {
		XGQueueItem qhead = &conn->client.sendQueue;
		while (qhead->next != NULL) {
			qhead = qhead->next;
		}
		qhead->next = que;
	} else {
		XGDataQueueRelease(que);
	}
}

