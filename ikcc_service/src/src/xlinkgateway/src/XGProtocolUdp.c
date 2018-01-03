/*
 * XGProtocolUdp.c
 *
 *  Created on: 2016年12月12日
 *      Author: john
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "XGMem.h"
#include "XGProtocolUdp.h"
#include "XGDebug.h"
#include "XGProtocol.h"

extern int g_udp_listen_port;

static void XGUdpDevcieRequestJoin(XGCoreCtx core, unsigned char *buffer, unsigned int bufferlength, struct sockaddr_in *addr);
static void XGDeviceRestartNotify(XGCoreCtx core, unsigned char *buffer, unsigned int bufferlength, struct sockaddr_in *addr);
static void XGDevcieDiscoverGateway(XGCoreCtx core, unsigned char *buffer, unsigned int bufferlength, struct sockaddr_in *addr);

const struct {
	char *doc;
	unsigned char cmd;
	unsigned short minLength;
	void (*Process)(XGCoreCtx core, unsigned char *buffer, unsigned int bufferlength, struct sockaddr_in *addr);
} XGProtocoUdpFuns[] = { //
		{ "device request join", 0x12, 44, XGUdpDevcieRequestJoin }, //
				{ "device restart notify", 0x14, 46, XGDeviceRestartNotify }, //
				{ "device discover gataway", 0x16, 0, XGDevcieDiscoverGateway }, //
				{ NULL, 0, 0, NULL }, //
		};

void XGStartProtocolUdp(XGCoreCtx core, unsigned char *buffer, unsigned int bufferlength, struct sockaddr_in *addr) {

	int i = 0;
	printf("Udp Data:");
	for (i = 0; i < bufferlength; i++) {
		printf("%02X ", buffer[i]);
	}
	printf("\r\n\r\n");
	for (i = 0;; i++) {
		if (XGProtocoUdpFuns[i].doc == NULL) {
			break;
		}
		if (bufferlength < XGProtocoUdpFuns[i].minLength) {
			continue;
		}
		if (XGProtocoUdpFuns[i].cmd == buffer[1]) {
			XGDEBUG("XG Protocol udp : %s\r\n", XGProtocoUdpFuns[i].doc);
			XGProtocoUdpFuns[i].Process(core, (unsigned char *) buffer, bufferlength, addr);
		}
	}
}

static void XGUdpDevcieRequestJoin(XGCoreCtx core, unsigned char *buffer, unsigned int bufferlength, struct sockaddr_in *addr) {

	char productid[33] = { 0x00 };
	unsigned char macstring[33] = { 0x00 };
	int deviceid = 0;
	int maclen = 0;
	memcpy(productid, buffer + 6, 32);
	maclen = buffer[38];
	if (maclen > 32) {
		return;
	}
	memcpy(macstring, buffer + 39, maclen);
	int index = 39 + maclen;
	if (index + 4 > bufferlength) {
		return;
	}
	deviceid = UTIL_INT32_SET(buffer[index], buffer[index + 1], buffer[index + 2], buffer[index + 3]);

	int ret = 0;
	if (core->config) {
		if (core->config->OnDiscoverDevice) {
			ret = core->config->OnDiscoverDevice(productid, macstring, maclen, deviceid, addr);
			if (ret == 0) {
				int corplen = strlen(core->corp);
				int tokenlen = strlen(core->token);
				int index = 0;
				char *response = (char *) XGMemFactory(4 + 32 + 2 + corplen + tokenlen + 16); //header productid,(corplen,tokenlen,) corp,token,(res 16)
				if (response == NULL) {
					return;
				}
				response[0] = 0x18;
				response[1] = 0x22; //cmd

				response[4] = buffer[4]; //msgid
				response[5] = buffer[5];
				response[6] = 0; //result
				memcpy(&response[7], core->productid, 32);
				index = 39;
				response[index++] = corplen;
				memcpy(response + index, core->corp, corplen);
				index += corplen;
				response[index++] = tokenlen;
				memcpy(response + index, core->token, tokenlen);
				index += tokenlen;
				index -= 4;
				response[2] = UTIL_INT16_GET_BITH(index);
				response[3] = UTIL_INT16_GET_BITL(index);
				struct sockaddr_in addrto;
				bzero(&addrto, sizeof(struct sockaddr_in));
				addrto.sin_family = AF_INET;
				addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
				addrto.sin_port = htons(g_udp_listen_port);
				ret = sendto(core->ServerUdpSocketHandle, response, index + 4, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
				XGSleepMs(30);
				ret = sendto(core->ServerUdpSocketHandle, response, index + 4, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
				XGMemRelease(response);
				response = NULL;
			} else {
				char response[32] = { 0x00 };
				response[0] = 0x18;
				response[1] = 0x22; //cmd
				response[2] = 00;
				response[3] = 3;
				response[4] = buffer[4];
				response[5] = buffer[5];
				response[6] = 1;
				ret = sendto(core->ServerUdpSocketHandle, response, 7, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
				XGSleepMs(30);
				ret = sendto(core->ServerUdpSocketHandle, response, 7, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
			}
		}
	}

}

static void XGDeviceRestartNotify(XGCoreCtx core, unsigned char *buffer, unsigned int bufferlength, struct sockaddr_in *addr) {

	char productid[33] = { 0x00 };
	unsigned char macstring[33] = { 0x00 };
	char tokenstring[64] = { 0x00 };
	int deviceid = 0;
	int maclen = 0;
	int tokenlen = 0;
	memcpy(productid, buffer + 6, 32);
	deviceid = UTIL_INT32_SET(buffer[38], buffer[39], buffer[40], buffer[41]);
	maclen = buffer[42];
	if (maclen > 32) {
		return;
	}
	memcpy(macstring, &buffer[43], maclen);
	int index = 43 + maclen;
	tokenlen = buffer[index++];
	if ((index + tokenlen) > bufferlength) {
		return;
	}

	memcpy(tokenstring, &buffer[index], tokenlen);
	int ret = 0;
	if (core->config) {
		if (core->config->OnDeviceRestartNotify) {
			ret = core->config->OnDeviceRestartNotify(tokenstring, productid, macstring, maclen, deviceid, addr);
			if (ret == 0) {
				int corplen = strlen(core->corp);
				int tokenlen = strlen(core->token);
				int index = 0;
				char *response = (char *) XGMemFactory(4 + 32 + 2 + corplen + tokenlen + 16); //header productid,(corplen,tokenlen,) corp,token,(res 16)
				if (response == NULL) {
					return;
				}
				response[0] = 0x18;
				response[1] = 0x24; //cmd

				response[4] = buffer[4]; //msgid
				response[5] = buffer[5];
				response[6] = 0; //result
				memcpy(&response[7], core->productid, 32);
				index = 39;
				response[index++] = corplen;
				memcpy(response + index, core->corp, corplen);
				index += corplen;
				response[index++] = tokenlen;
				memcpy(response + index, core->token, tokenlen);
				index += tokenlen;
				index -= 4;
				response[2] = UTIL_INT16_GET_BITH(index);
				response[3] = UTIL_INT16_GET_BITL(index);
				struct sockaddr_in addrto;
				bzero(&addrto, sizeof(struct sockaddr_in));
				addrto.sin_family = AF_INET;
				addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
				addrto.sin_port = htons(g_udp_listen_port);
				ret = sendto(core->ServerUdpSocketHandle, response, index + 4, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
				XGSleepMs(30);
				ret = sendto(core->ServerUdpSocketHandle, response, index + 4, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
				XGMemRelease(response);
				response = NULL;
			} else if (ret == 1) {
				char response[32] = { 0x00 };
				response[0] = 0x18;
				response[1] = 0x24; //cmd
				response[2] = 00;
				response[3] = 3;
				response[4] = buffer[4];
				response[5] = buffer[5];
				response[6] = 1;
				ret = sendto(core->ServerUdpSocketHandle, response, 7, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
				XGSleepMs(30);
				ret = sendto(core->ServerUdpSocketHandle, response, 7, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
			}
			//not response ,not local gateway id
//			else {
//				char response[32] = { 0x00 };
//				response[0] = 0x18;
//				response[1] = 0x24; //cmd
//				response[2] = 00;
//				response[3] = 3;
//				response[4] = buffer[4];
//				response[5] = buffer[5];
//				response[6] = 1;
//				ret = sendto(core->ServerUdpSocketHandle, response, 7, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
//				XGSleepMs(30);
//				ret = sendto(core->ServerUdpSocketHandle, response, 7, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
//			}
		}
	}
}

static void XGDevcieDiscoverGateway(XGCoreCtx core, unsigned char *buffer, unsigned int bufferlength, struct sockaddr_in *addr) {
	char response[32] = { 0x00 };
	response[0] = 0x18;
	response[1] = 0x26; //cmd
	response[2] = 00;
	response[3] = 00;
	sendto(core->ServerUdpSocketHandle, response, 4, 0, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
}
