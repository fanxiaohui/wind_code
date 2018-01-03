/*
 * XGageway.c
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "XGCore.h"
#include "XGDebug.h"
#include "XGageway.h"
#include "XGProtocol.h"
#include "XGMem.h"
#include "XGmd5.h"

//协议版本
#ifndef PROTOCOL_VERSION
#define  PROTOCOL_VERSION  3
#endif

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

static XGCoreCtx g_ctx = NULL;
static char g_host[128] = { "cm.xlink.cn" };
short g_port = 23778;
extern int g_udp_svr_port;
extern int g_udp_listen_port;
int XGInit(char *corp_id, char *pid, char *gatewayid, XGConfig *config) {
	if (corp_id == NULL || pid == NULL || gatewayid == NULL || config == NULL) {
		return -1;
	}

	if (g_ctx != NULL) {
		return -100;
	}

	g_ctx = XGCoreFactory();
	if (g_ctx == NULL) {
		XGERROR("malloc XGCoreCtx failed\r\n");
		return -1;
	}
	strcpy(g_ctx->Host, g_host);
	g_ctx->ServerPort = g_port;
	g_ctx->LocalTcpServerPort = 23778;
	g_ctx->config = config;
	g_ctx->corp = corp_id;
	g_ctx->productid = pid;
	g_ctx->token = gatewayid;
	g_ctx->LocalUdpServerPort = g_udp_svr_port;
	int ret = XGStartCore(g_ctx);

	return ret;
}

int XGUninit() {
	if (g_ctx != NULL) {
		XGStopCore(g_ctx);
		XGCoreRelease(g_ctx);
		g_ctx = NULL;
	}
	return 0;
}

static int SendPipeClient(int formid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype) {
	XGMessage header;
	int ret = 0;
	if (data == NULL) {
		return -3;
	}

	XGQueueItem item = XGDataQueueFactory(datalength + 32);

	if (item == NULL) {
		return -4;
	}

	header.bits.type = TCP_TYPE_PIPE;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	item->data[0] = header.byte;
	item->data[1] = UTIL_INT32_GET_BIT3(datalength + 7);
	item->data[2] = UTIL_INT32_GET_BIT2(datalength + 7);
	item->data[3] = UTIL_INT32_GET_BIT1(datalength + 7);
	item->data[4] = UTIL_INT32_GET_BIT0(datalength + 7);

	item->data[5] = UTIL_INT32_GET_BIT3(toid);
	item->data[6] = UTIL_INT32_GET_BIT2(toid);
	item->data[7] = UTIL_INT32_GET_BIT1(toid);
	item->data[8] = UTIL_INT32_GET_BIT0(toid);

	item->data[9] = (msgid >> 8) & 0xff;
	item->data[10] = (msgid >> 0) & 0xff;
	item->data[11] = pipetype;
	memcpy(&item->data[12], data, datalength);
	item->dataSize = datalength + 12;
	item->deviceid = formid;

	ret = XGCoreSendData(g_ctx, item, E_TO_DEVICE);
	if (ret < 0) {
		XGDataQueueRelease(item);
	}
	return ret;
}
int XGSendDataToDevice(int formid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype) {
	int ret = 0;
	if (g_ctx == NULL) {
		return -1;
	}

	if (!g_ctx->isRuning) {
		return -2;
	}

	ret = SendPipeClient(formid, toid, msgid, data, datalength, pipetype);

	return ret;
}

static int SendPipeCloud(int formid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype) {
	XGMessage header;
	if (data == NULL) {
		return -3;
	}

	XGQueueItem item = XGDataQueueFactory(datalength + 32);

	if (item == NULL) {
		return -4;
	}

	header.bits.type = TCP_TYPE_PIPE;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	item->data[0] = header.byte;
	item->data[1] = UTIL_INT32_GET_BIT3(datalength + 7);
	item->data[2] = UTIL_INT32_GET_BIT2(datalength + 7);
	item->data[3] = UTIL_INT32_GET_BIT1(datalength + 7);
	item->data[4] = UTIL_INT32_GET_BIT0(datalength + 7);

	item->data[5] = UTIL_INT32_GET_BIT3(toid);
	item->data[6] = UTIL_INT32_GET_BIT2(toid);
	item->data[7] = UTIL_INT32_GET_BIT1(toid);
	item->data[8] = UTIL_INT32_GET_BIT0(toid);

	item->data[9] = (msgid >> 8) & 0xff;
	item->data[10] = (msgid >> 0) & 0xff;
	item->data[11] = pipetype;
	memcpy(&item->data[12], data, datalength);
	item->dataSize = datalength + 12;
	item->deviceid = formid;
	int ret = XGCoreSendData(g_ctx, item, E_TO_CLOUD);
	if (ret < 0) {
		XGDataQueueRelease(item);
	}
	return ret;
}

static int SendPipeSyncCloud(int formid, short msgid, unsigned char *data, unsigned int datalength, int pipetype) {
	XGMessage header;

	if (data == NULL) {
		return -3;
	}

	XGQueueItem item = XGDataQueueFactory(datalength + 32);
	if (item == NULL) {
		return -4;
	}
	header.bits.type = TCP_TYPE_PIPE_2;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	item->data[0] = header.byte;
	item->data[1] = UTIL_INT32_GET_BIT3(datalength + 7);
	item->data[2] = UTIL_INT32_GET_BIT2(datalength + 7);
	item->data[3] = UTIL_INT32_GET_BIT1(datalength + 7);
	item->data[4] = UTIL_INT32_GET_BIT0(datalength + 7);

	item->data[5] = UTIL_INT32_GET_BIT3(formid);
	item->data[6] = UTIL_INT32_GET_BIT2(formid);
	item->data[7] = UTIL_INT32_GET_BIT1(formid);
	item->data[8] = UTIL_INT32_GET_BIT0(formid);

	item->data[9] = (msgid >> 8) & 0xff;
	item->data[10] = (msgid >> 0) & 0xff;
	item->data[11] = pipetype;
	memcpy(&item->data[12], data, datalength);
	item->dataSize = datalength + 12;
	item->deviceid = formid;
	int ret = XGCoreSendData(g_ctx, item, E_TO_CLOUD);
	if (ret < 0) {
		XGDataQueueRelease(item);
	}
	return ret;
}

int XGSendDataToCloud(int formid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype) {
	int ret = 0;
	if (g_ctx == NULL) {
		return -1;
	}
	if (!g_ctx->isRuning) {
		return -2;
	}

	if (toid == 0) {
		ret = SendPipeSyncCloud(formid, msgid, data, datalength, pipetype);
	} else {
		ret = SendPipeCloud(formid, toid, msgid, data, datalength, pipetype);
	}
	return ret;
}

int XGDisconnect(int deviceid) {

	if (g_ctx == NULL) {
		return -1;
	}
	if (!g_ctx->isRuning) {
		return -2;
	}

	return 0;
}
extern int XGSendUdpData(unsigned char *data, int datalength, struct sockaddr_in *address) {
	int ret = 0;
	if (g_ctx == NULL) {
		return -1;
	}
	if (!g_ctx->isRuning) {
		return -2;
	}

	if (g_ctx->ServerUdpSocketHandle <= 0)
		return -4;

	ret = sendto(g_ctx->ServerUdpSocketHandle, (char *) data, datalength, 0, (struct sockaddr*) address, sizeof(struct sockaddr_in));
	return ret;
}

int XGDiscover(void) {
	int ret = 0;
	if (g_ctx == NULL) {
		return -1;
	}
	if (!g_ctx->isRuning) {
		return -2;
	}

	if (g_ctx->ServerUdpSocketHandle <= 0)
		return -4;

	int corplen = strlen(g_ctx->corp);
	int tokenlen = strlen(g_ctx->token);
	int index = 0;
	char *buffer = (char *) XGMemFactory(4 + 32 + 2 + corplen + tokenlen + 16); //header productid,(corplen,tokenlen,) corp,token,(res 16)
	if (buffer == NULL) {
		return -5;
	}
	buffer[0] = 0x18;
	buffer[1] = 0x11; //cmd
	memcpy(&buffer[4], g_ctx->productid, 32);
	index = 36;
	buffer[index++] = corplen;
	memcpy(buffer + index, g_ctx->corp, corplen);
	index += corplen;
	buffer[index++] = tokenlen;
	memcpy(buffer + index, g_ctx->token, tokenlen);
	index += tokenlen;
	index -= 4;
	buffer[2] = UTIL_INT16_GET_BITH(index);
	buffer[3] = UTIL_INT16_GET_BITL(index);

	struct sockaddr_in addrto;
	bzero(&addrto, sizeof(struct sockaddr_in));
	addrto.sin_family = AF_INET;
	addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	addrto.sin_port = htons(g_udp_listen_port);
	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) &addrto, sizeof(struct sockaddr_in));
	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) &addrto, sizeof(struct sockaddr_in));

	addrto.sin_addr.s_addr = inet_addr("192.168.8.255");
	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) &addrto, sizeof(struct sockaddr_in));
	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) &addrto, sizeof(struct sockaddr_in));

	XGMemRelease(buffer);
	buffer = NULL;
	return ret;
}

int XGRestartNotify(void) {
	int ret = 0;
	if (g_ctx == NULL) {
		return -1;
	}
	if (!g_ctx->isRuning) {
		return -2;
	}

	if (g_ctx->ServerUdpSocketHandle <= 0)
		return -4;

	int corplen = strlen(g_ctx->corp);
	int tokenlen = strlen(g_ctx->token);
	int index = 0;
	char *buffer = (char *) XGMemFactory(4 + 32 + 2 + corplen + tokenlen + 16); //header productid,(corplen,tokenlen,) corp,token,(res 16)
	if (buffer == NULL) {
		return -5;
	}
	buffer[0] = 0x18;
	buffer[1] = 0x13; //cmd
	memcpy(&buffer[4], g_ctx->productid, 32);
	index = 36;
	buffer[index++] = corplen;
	memcpy(buffer + index, g_ctx->corp, corplen);
	index += corplen;
	buffer[index++] = tokenlen;
	memcpy(buffer + index, g_ctx->token, tokenlen);
	index += tokenlen;
	index -= 4;
	buffer[2] = UTIL_INT16_GET_BITH(index);
	buffer[3] = UTIL_INT16_GET_BITL(index);

	struct sockaddr_in addrto;
	bzero(&addrto, sizeof(struct sockaddr_in));
	addrto.sin_family = AF_INET;
	addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	addrto.sin_port = htons(g_udp_listen_port);
	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) &addrto, sizeof(struct sockaddr_in));
	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) &addrto, sizeof(struct sockaddr_in));

	addrto.sin_addr.s_addr = inet_addr("192.168.8.255");
	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) &addrto, sizeof(struct sockaddr_in));
	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) &addrto, sizeof(struct sockaddr_in));

	XGMemRelease(buffer);
	buffer = NULL;
	return ret;
}

int XGGetAccessKey(struct sockaddr_in *addr, unsigned int timeoutms,const char *product_id,unsigned char *mac, unsigned int maclength) {
	int ret = 0;
	fd_set fdset;
	if (g_ctx == NULL) {
		return -1;
	}
	if (!g_ctx->isRuning) {
		return -2;
	}

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock <= 0) {
		perror("socket");
		return -3;
	}
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (addr->sin_addr.s_addr == htonl(INADDR_BROADCAST)) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -4;
	}
	unsigned char tempmsgid = XGGetLocalTime() % 126;
	unsigned char buffer[256] = { 0x00 };
	buffer[0] = 0x04; //v4
	buffer[1] = 0x00; //flag
	//message id
	buffer[2] = 0;
	buffer[3] = tempmsgid;
	//bodylength
	buffer[4] = 0;
	buffer[5] = 38;
	//cmd
	buffer[6] = 01;
	//res
	buffer[7] = 0;
	//version
	buffer[8] = 04;
	//port
	buffer[9] = 0;
	buffer[10] = 0;
	//flag scan by pid
	buffer[11] = 2;
	//mac len
	//buffer[12] = 0;
	//buffer[13] = maclength;
	//mac
	memcpy(&buffer[12], product_id, 32);

	ret = sendto(sock, buffer, 44, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
	if (ret <= 0) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -5;
	}
	FD_ZERO(&fdset);
	FD_SET(sock, &fdset);
	struct timeval timeout;
	timeout.tv_sec = timeoutms / 1000;
	timeout.tv_usec = (timeoutms % 1000) * 1000;
	ret = select(sock + 1, &fdset, NULL, NULL, &timeout);
	if (ret < 0) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -6;
	}

	if (ret == 0) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -7;
	}

	if (FD_ISSET(sock, &fdset)) {
		struct sockaddr_in clientAddr;
		socklen_t len = sizeof(clientAddr);
		memset(buffer, 0, 256);
		int bytes = recvfrom(sock, buffer, 256, 0, (struct sockaddr*) &clientAddr, &len);
		if (bytes <= 0) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -8;
		}

		if (bytes < 43) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -9;
		}

		if (buffer[0] != 0x04) //协议版本不正确
				{
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -10;
		}
		if (buffer[3] != tempmsgid) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -11;
		}
		if (buffer[6] != 01) //cmd error
				{
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -12;
		}

		//start maclen + product lenght +product+((version +port)=7)+flag

		int index = 10 + buffer[10] + 2 + 32 + 7 + 1+1;
		index += 2; //device name length

		index += buffer[index]; //device name
		index += 1;//ptr ack start
		printf("get subkey recv data len =%d,index =%d\r\n",bytes,index);
		int i = 0;
		for(i = 0;i < bytes;i++){
			printf("%02X ",buffer[i]);
		}
		printf("\r\n");
		if (index + 1 < bytes) {
			int ack = buffer[index++] << 24;
			ack |= buffer[index++] << 16;
			ack |= buffer[index++] << 8;
			ack |= buffer[index++];
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return ack;
		}
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -13;
	}

	shutdown(sock, SHUT_RDWR);
	close(sock);

	return -14;
}
int XGSetAccessKey(int accesskey, unsigned int timeoutms, struct sockaddr_in *addr) {
	int ret = 0;
	fd_set fdset;
	if (g_ctx == NULL) {
		return -1;
	}
	if (!g_ctx->isRuning) {
		return -2;
	}

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock <= 0) {
		perror("socket");
		return -3;
	}
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (addr->sin_addr.s_addr == htonl(INADDR_BROADCAST)) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -4;
	}
	unsigned char tempmsgid =XGGetLocalTime() % 126;
	unsigned char buffer[32] = { 0x00 };
	buffer[0] = 0x04; //v4
	buffer[1] = 0x01; //flag
	//message id
	buffer[2] = 0;
	buffer[3] = tempmsgid;
	//bodylength
	buffer[4] = 0;
	buffer[5] = 11;
	//cmd
	buffer[6] = 11;
	//res
	buffer[7] = 0;
	//set ack message id
	buffer[8] = 0;
	buffer[9] = 0;
	//flag
	buffer[10] = 0;
	//port
	buffer[11] = 0;
	buffer[12] = 0;
	//ack
	buffer[13] = (unsigned char) (((accesskey) & 0xFF000000) >> 24);
	buffer[14] = (unsigned char) (((accesskey) & 0x00FF0000) >> 16);
	buffer[15] = (unsigned char) (((accesskey) & 0x0000FF00) >> 8);
	buffer[16] = (unsigned char) (((accesskey) & 0x000000FF));

	ret = sendto(sock, buffer, 17, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
	if (ret <= 0) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -5;
	}
	FD_ZERO(&fdset);
	FD_SET(sock, &fdset);
	struct timeval timeout;
	timeout.tv_sec = timeoutms / 1000;
	timeout.tv_usec = (timeoutms % 1000) * 1000;
	ret = select(sock + 1, &fdset, NULL, NULL, &timeout);
	if (ret < 0) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -6;
	}

	if (ret == 0) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -7;
	}

	if (FD_ISSET(sock, &fdset)) {
		struct sockaddr_in clientAddr;
		socklen_t len = sizeof(clientAddr);
		memset(buffer, 0, 32);
		int bytes = recvfrom(sock, buffer, 32, 0, (struct sockaddr*) &clientAddr, &len);
		if (bytes <= 0) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -8;
		}

		if (bytes < 11) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -9;
		}

		if (buffer[0] != 0x04) //协议版本不正确
				{
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -10;
		}

		if (buffer[3] != tempmsgid) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -11;
		}

		if (buffer[6] != 11) //cmd error
				{
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -12;
		}

		//OK
		if (buffer[10] == 0) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return 0;
		} else { //already set ack
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -14;
		}
	}

	shutdown(sock, SHUT_RDWR);
	close(sock);
	return -13;
}

int XGGetSubKey(int accesskey, unsigned int timeoutms, struct sockaddr_in *addr) {

	int ret = 0;
	fd_set fdset;
	if (g_ctx == NULL) {
		return -1;
	}
	if (!g_ctx->isRuning) {
		return -2;
	}

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock <= 0) {
		perror("socket");
		return -3;
	}
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (addr->sin_addr.s_addr == htonl(INADDR_BROADCAST)) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -4;
	}
#define  TEMP_BUFFER_LEN 64
	unsigned char buffer[TEMP_BUFFER_LEN] = { 0x00 };

	buffer[3] = (unsigned char) (((accesskey) & 0xFF000000) >> 24);
	buffer[2] = (unsigned char) (((accesskey) & 0x00FF0000) >> 16);
	buffer[1] = (unsigned char) (((accesskey) & 0x0000FF00) >> 8);
	buffer[0] = (unsigned char) (((accesskey) & 0x000000FF));
	unsigned char tempmsgid = XGGetLocalTime() % 126;
	xlinkGetMd5(&buffer[11], buffer, 4);
	buffer[0] = 0x04; //v4
	buffer[1] = 0x01; //flag
	//message id
	buffer[2] = 0;
	buffer[3] = tempmsgid;
	//bodylength
	buffer[4] = 0;
	buffer[5] = 22;
	//cmd
	buffer[6] = 7;
	//res
	buffer[7] = 0;
	//version
	buffer[8] = 04;

	//message id
	buffer[9] = 0;
	buffer[10] = 0;
	//md5
	//res
	buffer[27] = 0;

	ret = sendto(sock, buffer, 28, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
	ret = sendto(sock, buffer, 28, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
	if (ret <= 0) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -5;
	}
	FD_ZERO(&fdset);
	FD_SET(sock, &fdset);
	struct timeval timeout;
	timeout.tv_sec = timeoutms / 1000;
	timeout.tv_usec = (timeoutms % 1000) * 1000;
	ret = select(sock + 1, &fdset, NULL, NULL, &timeout);
	if (ret < 0) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -6;
	}

	if (ret == 0) {
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -7;
	}

	if (FD_ISSET(sock, &fdset)) {
		struct sockaddr_in clientAddr;
		socklen_t len = sizeof(clientAddr);
		memset(buffer, 0, TEMP_BUFFER_LEN);
		int bytes = recvfrom(sock, buffer, TEMP_BUFFER_LEN, 0, (struct sockaddr*) &clientAddr, &len);
		if (bytes <= 0) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -8;
		}

		if (bytes < 10) {
			XGERROR("Get subkey failed data length to short %d\r\n", bytes);
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -9;
		}

		if (buffer[0] != 0x04) //协议版本不正确{
				{
			XGERROR("Get subkey failed agremment version error datalength =%d\r\n", bytes);
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -10;
		}

		if (buffer[3] != tempmsgid) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -11;
		}

		if (buffer[6] != 7) //cmd error
				{
			XGERROR("Get subkey failed command =%d\r\n", buffer[6]);
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -12;
		}

		if (buffer[10] == 0) {
			unsigned int subkey = (buffer[11] << 24) + (buffer[12] << 16) + (buffer[13] << 8) + (buffer[14]);
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return subkey;
		} else {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -14;
		}
	}
	shutdown(sock, SHUT_RDWR);
	close(sock);
	return -13;
}

int XGRemoveDevice(struct sockaddr_in *address, int device_id) {

	int ret = 0;
	if (g_ctx == NULL) {
		return -1;
	}
	if (!g_ctx->isRuning) {
		return -2;
	}

	if (address->sin_addr.s_addr == htonl(INADDR_BROADCAST)) {
		return -3;
	}
//	if (address->sin_port != htons(g_udp_listen_port)) {
//		return -4;
//	}
	if (g_ctx->ServerUdpSocketHandle <= 0)
		return -5;

	int corplen = strlen(g_ctx->corp);
	int tokenlen = strlen(g_ctx->token);
	int index = 0;
	char *buffer = (char *) XGMemFactory(4 + 32 + 2 + corplen + tokenlen + 16); //header productid,(corplen,tokenlen,) corp,token,(res 16)
	if (buffer == NULL) {
		return -6;
	}
	buffer[0] = 0x18;
	buffer[1] = 0x15; //cmd
	memcpy(&buffer[4], g_ctx->productid, 32);
	index = 36;
	buffer[index++] = corplen;
	memcpy(buffer + index, g_ctx->corp, corplen);
	index += corplen;
	buffer[index++] = tokenlen;
	memcpy(buffer + index, g_ctx->token, tokenlen);
	index += tokenlen;

	buffer[index++] = (unsigned char) (((device_id) & 0xFF000000) >> 24);
	buffer[index++] = (unsigned char) (((device_id) & 0x00FF0000) >> 16);
	buffer[index++] = (unsigned char) (((device_id) & 0x0000FF00) >> 8);
	buffer[index++] = (unsigned char) (((device_id) & 0x000000FF));

	index -= 4;
	buffer[2] = UTIL_INT16_GET_BITH(index);
	buffer[3] = UTIL_INT16_GET_BITL(index);

	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) address, sizeof(struct sockaddr_in));
	ret = sendto(g_ctx->ServerUdpSocketHandle, buffer, index + 4, 0, (struct sockaddr*) address, sizeof(struct sockaddr_in));
	XGMemRelease(buffer);
	buffer = NULL;
	return ret;

}

int XGSetHost(char *host) {
	memset(g_host, 0, 128);
	strcpy(g_host, host);
	return 0;
}
