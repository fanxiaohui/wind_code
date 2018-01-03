#include "xlink_client.h"
#include "xlink_data.h"
#include "xlink_system.h"
#include "xlink_tcp_client.h"
#include "xsdk_config.h"
#include "xlink_net.h"

XLINK_CLIENT g_xlink_client[XLINK_CLIENT_SIZE];
extern G_XLINK_INFO g_xlink_info;

XLINK_FUNC void XlinnkClientInit(void) {
	x_uint8 i = 0;
	for (; i < XLINK_CLIENT_SIZE; i++) {
		xlink_memset(&g_xlink_client[i], 0, sizeof(XLINK_CLIENT));
		g_xlink_client[i].ArrayIndex = i;
	}
}

XLINK_FUNC x_int16 XlinkClientAddclient(unsigned short AppKeepAliveTime, xlink_addr *addr, x_uint8 *ssidIndex, x_uint8 *ssidValue) {

	x_uint8 index = 1;
	*ssidIndex = 0;
	*ssidValue = 0;

	for (index = 0; index < XLINK_CLIENT_SIZE; index++) {
		if (g_xlink_client[index].isActive == 1) {

			if (g_xlink_client[index].Address.sin_addr.s_addr == addr->sin_addr.s_addr && g_xlink_client[index].Address.sin_port == addr->sin_port) {
				g_xlink_client[index].keepAliveTime = AppKeepAliveTime;
				g_xlink_client[index].isActive = 1;
				g_xlink_client[index].LastReceiveDataTime = g_xlink_info.g_XlinkSdkTime;
#if __XDEBUG__
				XSDK_DEBUG_DEBUG("xlink client reconnect index=%d ip=%d port=%d", index, addr->sin_addr.s_addr, addr->sin_port);
#endif
				xlink_memcpy(&g_xlink_client[index].Address, addr, sizeof(xlink_addr));

				*ssidIndex = index;
				*ssidValue = g_xlink_client[index].checkValue;//g_xlink_client[index].LastReceiveDataTime % 253 + 1 + g_xlink_client[index].checkValue / 13;
				//g_xlink_client[index].checkValue = *ssidValue;
				g_xlink_info.flag.bit.clientCount = 1;
				return index;
			}
		}
	}

	index = 0;
	for (; index < XLINK_CLIENT_SIZE; index++) {
		if (g_xlink_client[index].isActive == 0) {
			g_xlink_client[index].keepAliveTime = AppKeepAliveTime;
			g_xlink_client[index].isActive = 1;
			g_xlink_client[index].LastReceiveDataTime = g_xlink_info.g_XlinkSdkTime;
			xlink_memcpy(&g_xlink_client[index].Address, addr, sizeof(xlink_addr));

			if (g_xlink_user_config->OnStatus) {
				g_xlink_user_config->OnStatus(XLINK_WIFI_STA_APP_CONNECT);
			}
#if __XDEBUG__
			XSDK_DEBUG_DEBUG("xlink client add index=%d ip=%d port=%d", index, addr->sin_addr.s_addr, addr->sin_port);
#endif
			*ssidIndex = index;
			*ssidValue = g_xlink_client[index].LastReceiveDataTime % 253 + 1;
			g_xlink_client[index].checkValue = *ssidValue;
			g_xlink_info.flag.bit.clientCount = 1;
			break;
		}
	}

	return index;
}

XLINK_FUNC void XlinkClientQuit(x_uint8 index) {
	g_xlink_client[index].isActive = 0;
	g_xlink_client[index].checkValue = 0;
	g_xlink_client[index].keepAliveTime = 0;
	g_xlink_client[index].LastReceiveDataTime = 0;

	if (g_xlink_user_config->OnStatus) {
		g_xlink_user_config->OnStatus(XLINK_WIFI_STA_APP_DISCONNECT);
	}
}

XLINK_FUNC void XlinkClientByby(x_uint16 ssid, xlink_addr *AddrBro) {
	//if (g_xlink_client[ssid].Address.sin_addr.s_addr == AddrBro->sin_addr.s_addr) {
	XlinkClientQuit(ssid);
	//}
}

XLINK_FUNC void xlinkClientReSet(xlink_addr *AddrBro) {
	x_uint8 Iteration = 0;
	for (Iteration = 0; Iteration < XLINK_CLIENT_SIZE; Iteration++) {
		if (g_xlink_client[Iteration].isActive == 1) {
			if (g_xlink_client[Iteration].Address.sin_addr.s_addr != AddrBro->sin_addr.s_addr) {
				g_xlink_client[Iteration].checkValue = 0;
				g_xlink_client[Iteration].isActive = 0;
				g_xlink_client[Iteration].LastReceiveDataTime = 0;
				g_xlink_client[Iteration].checkValue = 0;
			}
		}
	}
}

XLINK_FUNC void XlinkClientCheckHeartbeat(xsdk_time_t c_time) {
	x_uint8 i = 0;
	x_uint32 time_temp = 0;
	x_uint8 count = 0;
	for (i = 0; i < XLINK_CLIENT_SIZE; i++) {
		if (g_xlink_client[i].isActive == 1) {
			if (c_time < g_xlink_client[i].LastReceiveDataTime) {
				g_xlink_client[i].LastReceiveDataTime = c_time;
			}
			time_temp = c_time - g_xlink_client[i].LastReceiveDataTime;
			count++;
			if (time_temp >= (g_xlink_client[i].keepAliveTime)) {
				g_xlink_client[i].isActive = 0;
				g_xlink_client[i].checkValue = 0;
				g_xlink_client[i].keepAliveTime = 0;
				g_xlink_client[i].LastReceiveDataTime = 0;
				g_xlink_client[i].messageid[0] = 0;
				g_xlink_client[i].messageid[1] = 0;
				//printf("App timeout index=%d", i);
				if (g_xlink_user_config->OnStatus) {
					g_xlink_user_config->OnStatus(XLINK_WIFI_STA_APP_TIMEOUT);
				}
				count--;
			}
		}
	}
	g_xlink_info.flag.bit.clientCount = count > 0 ? 1 : 0;
}

XLINK_FUNC x_int32 XlinkClientSendDataToAllOlineClient(void *data, unsigned int datalen) {
	x_int32 ret = 0;
	x_uint8 iteration = 0;

	if (g_xlink_info.flag.bit.clientCount) {
		iteration = 0;
		for (iteration = 0; iteration < XLINK_CLIENT_SIZE; iteration++) {
			if (g_xlink_client[iteration].isActive == 1 && g_xlink_client[iteration].checkValue) {
				ret = XlinkClientSendToIndex(data, datalen, iteration);
			}
		}
	}
	return ret;
}

XLINK_FUNC x_int32 XlinkClientSendDataToAddr(x_uint8 *data, unsigned int datalen, xlink_addr *addr) {

#if __MT7681__
	int ret = g_xlink_user_config->send_udp(addr,data,datalen);
	//xlink_msleep(15);

	return ret;
#elif __LWIP__ESP_8266
	int ret = g_xlink_user_config->send_udp(addr,data,datalen);
	//xlink_msleep(15);

	return ret;
#elif __STM32F107__
	int ret = g_xlink_user_config->send_udp(addr,data,datalen);
	//xlink_msleep(15);

	return ret;
#elif 	__ALL_DEVICE__
	int ret = g_xlink_user_config->send_udp(addr,data,datalen);
	//xlink_msleep(15);

	return ret;
#elif __STM32F103_UIP__
	int ret = g_xlink_user_config->send_udp(addr,data,datalen);
	return ret;
#elif __MXCHIP__
	struct sockaddr_t mAddr;
	x_uint16 addrBroLen = sizeof(mAddr);
	int ret = 0;
	mAddr.s_type = addr->type;
	mAddr.s_port = addr->sin_port;
	mAddr.s_ip = addr->sin_addr.s_addr;
	xlink_memcpy(mAddr.s_spares,addr->spares,6);

	ret = xlink_sendto(g_xlink_info.net_info.udp_fd, (char*)data,
			datalen, 0, &mAddr, addrBroLen);
	return ret;
#else
	x_uint16 addrBroLen = sizeof(xlink_addr);
	x_int32 ret = 0;
//	printf("Send:");
//	int i = 0;
//	for (i = 0; i < datalen; i++) {
//		printf("%02X ", data[i]);
//	}
//	printf("");
	if (g_xlink_info.net_info.udp_fd > 0) {
		ret = xlink_sendto(g_xlink_info.net_info.udp_fd, (char*) data, datalen, 0, (struct sockaddr *) addr, addrBroLen);
		xlink_msleep(10);
	}
	return ret;
#endif

}

XLINK_FUNC int XlinkSendUdpData(unsigned char *data, unsigned int datalength, xlink_addr *addr) {
	return XlinkClientSendDataToAddr(data, datalength, addr);
}

XLINK_FUNC x_int32 XlinkClientSendToIndex(x_uint8 *data, unsigned int datalen, x_uint8 index) {
	x_int32 ret = 0;
	if (index < XLINK_CLIENT_SIZE) {
		ret = XlinkClientSendDataToAddr(data, datalen, &g_xlink_client[index].Address);
	}
	return ret;
}

XLINK_FUNC x_bool XlinkClientcheckclientLogin(x_uint8 ssidIndex, x_uint8 ssidValue, xlink_addr *AddrBro) {
#if __XDEBUG__	
	volatile union {
		unsigned int ipAddr;
		struct {
			unsigned char IP3 :8;
			unsigned char IP2 :8;
			unsigned char IP1 :8;
			unsigned char IP0 :8;
		} byte;
	} ip_byte;
#endif
	if (ssidIndex < XLINK_CLIENT_SIZE) {
		if (g_xlink_client[ssidIndex].isActive == 1) { //addr->sin_port
			if (g_xlink_client[ssidIndex].checkValue == ssidValue) {
				if (g_xlink_client[ssidIndex].Address.sin_addr.s_addr == AddrBro->sin_addr.s_addr) {
					if (g_xlink_client[ssidIndex].Address.sin_port == AddrBro->sin_port) {
						g_xlink_client[ssidIndex].LastReceiveDataTime = g_xlink_info.g_XlinkSdkTime;
						g_xlink_info.flag.bit.clientCount = 1;
						return xlink_true;
					}
				}
			}
		}
	}
#if __XDEBUG__

	ip_byte.ipAddr = AddrBro->sin_addr.s_addr;
	XSDK_DEBUG_ERROR("not find client index=%d IP=%d.%d.%d.%d port %d", ssidIndex, ip_byte.byte.IP0, ip_byte.byte.IP1, ip_byte.byte.IP2, ip_byte.byte.IP3, AddrBro->sin_port);
#endif

	return xlink_false;

}

