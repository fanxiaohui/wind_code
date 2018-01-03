#include "xlink_tcp_client.h"
#include "xlink_type.h"
#include "xlink_system.h"
#include "xsdk_config.h"
#include "xlink_md5.h"
#include "xlink_net.h"

#if CLIENT_SSL_ENABLE	
extern CYASSL* tq_ssl;
#endif

#if __ALL_DEVICE__
#include "XlinkByteQueue.h"
#endif

static XLINK_TCP_CLIENT m_tcp_client;
extern G_XLINK_INFO g_xlink_info;

volatile static xlink_timer tcp_timer;

XLINK_FUNC static void XlinkTcpWorkPing(void);
XLINK_FUNC static void XlinkTcpWorkConnect(XLINK_TCP_CLIENT *tcp_cli);
XLINK_FUNC static int XlinkTcpWorkActivation(void);

XLINK_FUNC void XlinkTcpInit(XLINK_TCP_WORK_TYPE type) {

	m_tcp_client.work_type = type;
	g_xlink_info.net_info.tcpFlag.Bit.isLogined = 0;
	g_xlink_info.net_info.tcpPingCount = 0;

#if __ALL_DEVICE__
	isQueeuInit = 0;
#endif
}

XLINK_FUNC void xlinkChangeWork(int type) {
	m_tcp_client.work_type = type;
	g_xlink_info.net_info.tcpPingCount = 0;
}

XLINK_FUNC x_int32 XlinkTcpSendData(unsigned char* data, x_uint16 datalen) {
	x_int32 ret = -1;
	if (data == NULL) {
		return ret;
	}

#if __XDEBUG__
	XSDK_DEBUG_DEBUG(" tcp send data length %d", datalen);

#endif

#if (__LWIP__ESP_8266)
	if (g_xlink_info.net_info.tcp_fd > 0) {
		ret = g_xlink_user_config->send_tcp(data, datalen);
	}
#elif __MT7681__
	if (g_xlink_info.net_info.tcp_fd > 0) {
		ret = g_xlink_user_config->send_tcp(data, datalen);
	}
#elif __STM32F107__
	if (g_xlink_info.net_info.tcp_fd > 0) {
		ret = g_xlink_user_config->send_tcp(data, datalen);
	}
#elif __ALL_DEVICE__
	if (g_xlink_info.net_info.tcp_fd > 0) {
		ret = g_xlink_user_config->send_tcp(data, datalen);
	}
#elif __STM32F103_UIP__
	if (g_xlink_info.net_info.tcp_fd > 0) {
		ret = g_xlink_user_config->send_tcp(data, datalen);
	}
#else
	//isSendDataed

	if (g_xlink_info.net_info.tcp_fd > 0) {
#if CLIENT_SSL_ENABLE
		ret = xlink_ssl_send(tq_ssl, (char*)data, datalen);
#else
		ret = xlink_send(g_xlink_info.net_info.tcp_fd, (char*) data, datalen, 0);
#endif
		xlink_msleep(10);
	}

#endif

	return ret;
}

XLINK_FUNC static void xlink_tcp_send_http_connect() {
	char data[255] = { 0x00 };
	xlink_memset(data, 0, 255);
	xlink_sprintf(data, "CONNECT %s:%d HTTP/1.1Host: %s:%d\r\n", xlinkNetGetHost(), g_xlink_info.net_info.ServerPort, xlinkNetGetHost(), g_xlink_info.net_info.ServerPort);

	XlinkTcpSendData((unsigned char*) data, xlink_strlen(data));
}

XLINK_FUNC void xlinkSendUpgrade(unsigned char RetCode, unsigned short Upver, unsigned short currentVer) {
	xlink_Message mRetHeader;
	unsigned char RetBuffer[20];
	mRetHeader.byte = 0;
	mRetHeader.bits.type = TCP_TYPE_EVENT;
	mRetHeader.bits.resp = 0;
	mRetHeader.bits.version = PROTOCOL_VERSION;
	RetBuffer[0] = mRetHeader.byte;
	RetBuffer[1] = 0;
	RetBuffer[2] = 0;
	RetBuffer[3] = 0;
	RetBuffer[4] = 11;

	RetBuffer[5] = 2; //event type

	RetBuffer[6] = g_xlink_info.config.Deviceid[0];
	RetBuffer[7] = g_xlink_info.config.Deviceid[1];
	RetBuffer[8] = g_xlink_info.config.Deviceid[2];
	RetBuffer[9] = g_xlink_info.config.Deviceid[3];

	RetBuffer[10] = 0; //type
	RetBuffer[11] = RetCode;
	RetBuffer[12] = UTIL_INT16_GET_BITH(Upver);
	RetBuffer[13] = UTIL_INT16_GET_BITL(Upver);

	RetBuffer[14] = UTIL_INT16_GET_BITH(currentVer);
	RetBuffer[15] = UTIL_INT16_GET_BITL(currentVer);

	XlinkTcpSendData(RetBuffer, 16);
}

XLINK_FUNC void xlink_GetServerTime(xsdk_time_t c_time) {

	xlink_Message mRetHeader;
	unsigned char RetBuffer[20];
	int temp = 0; //xlink_ticks_diff(c_time, g_xlink_info.dev_info.lastGetServerTimeTime);

	if (c_time < g_xlink_info.dev_info.lastGetServerTimeTime) {
		g_xlink_info.dev_info.lastGetServerTimeTime = c_time;
		return;
	}
	temp = c_time - g_xlink_info.dev_info.lastGetServerTimeTime;
	if (temp < g_xlink_info.dev_info.getServerTimeDelay) {
		return;
	}

	g_xlink_info.dev_info.getServerTimeDelay = 20;
	g_xlink_info.dev_info.lastGetServerTimeTime = c_time;

	mRetHeader.byte = 0;
	mRetHeader.bits.type = TCP_TYPE_EVENT;
	mRetHeader.bits.resp = 0;
	mRetHeader.bits.version = PROTOCOL_VERSION;
	RetBuffer[0] = mRetHeader.byte;
	RetBuffer[1] = 0;
	RetBuffer[2] = 0;
	RetBuffer[3] = 0;
	RetBuffer[4] = 1;

	RetBuffer[5] = 12; //event type-time

	XlinkTcpSendData(RetBuffer, 6);

}

XLINK_FUNC void XlinkTcpPing_Clear() {
	m_tcp_client.lastPingTime = g_xlink_info.g_XlinkSdkTime;
	g_xlink_info.net_info.tcpPingCount = 0;
}

XLINK_FUNC void XlinkTcpLoop(xsdk_time_t c_time) {
	x_uint32 time_temp = 0;

//	if (m_tcp_client.work.work_type == TCP_PING) {
//
//		if (time_temp > 20) {
//			m_tcp_client.lastPingTime = c_time;
//			if (g_xlink_info.net_info.tcpFlag.Bit.isLogined == 1) {
//				if (g_xlink_info.net_info.tcpPingCount > 2) {
//					xsdk_closeTCP(1);
//					return;
//				}
//				XlinkTcpWorkPing();
//				g_xlink_info.net_info.tcpPingCount++;
//				return;
//			} else {
//				xsdk_closeTCP(1);
//				return;
//			}
//		} else if (g_xlink_info.dev_info.flags.Bit.isGetServerTimeTask == 1) {
//			if (g_xlink_info.net_info.tcpFlag.Bit.isLogined == 1) {
//				xlink_GetServerTime(c_time);
//			}
//		}
//		xsdk_timer_set((xlink_timer*) (&tcp_timer), 30);
//	}

	switch ((XLINK_TCP_WORK_TYPE) m_tcp_client.work_type) {
	case TCP_NOT_CONNECT:
		break;
	case TCP_SEND_HTTPCONNECT:
		xlink_tcp_send_http_connect();
		m_tcp_client.work_type = TCP_WAIT_HTTP_RESULT;
		xsdk_timer_set((xlink_timer*) (&tcp_timer), 15);
		break;
	case TCP_WAIT_HTTP_RESULT:
	case TCP_WAIT_ACTIVE_RESULT:
	case TCP_WAIT_CONNECT_RESULT:
		if (xsdk_tiemr_timerout((xlink_timer*) (&tcp_timer)) == 1) {
			xsdk_timer_set((xlink_timer*) (&tcp_timer), 15);
			xsdk_closeTCP(1);
		}
		break;
	case TCP_ACTIVATION: {
		if (g_xlink_info.config.flag.Bit.isActivation == 1) {
			m_tcp_client.work_type = TCP_CONNECT;
			break;
		}
		XlinkTcpWorkActivation();
		m_tcp_client.work_type = TCP_WAIT_ACTIVE_RESULT;
		xsdk_timer_set((xlink_timer*) (&tcp_timer), 15);
	}
		break;
	case TCP_CONNECT:
		XlinkTcpWorkConnect(&m_tcp_client);
		m_tcp_client.work_type = TCP_WAIT_CONNECT_RESULT;
		xsdk_timer_set((xlink_timer*) (&tcp_timer), 15);
		break;
	case TCP_PING:
		if (c_time < m_tcp_client.lastPingTime) {
			m_tcp_client.lastPingTime = c_time;
			g_xlink_info.net_info.tcpPingCount = 0;
		} else {
			time_temp = c_time - m_tcp_client.lastPingTime;
		}
		if (time_temp > 20) {
			XlinkTcpWorkPing();
			m_tcp_client.lastPingTime = c_time;
			g_xlink_info.net_info.tcpPingCount++;
			m_tcp_client.work_type = TCP_WAIT_PING_RESULT;
			xsdk_timer_set((xlink_timer*) (&tcp_timer), 20);
		}
		break;
	case TCP_WAIT_PING_RESULT:
		if (xsdk_tiemr_timerout((xlink_timer*) (&tcp_timer)) == 1) {
			if (g_xlink_info.net_info.tcpPingCount > 1) {
				xsdk_closeTCP(1);
				return;
			}
			XlinkTcpWorkPing();
			m_tcp_client.lastPingTime = c_time;
			g_xlink_info.net_info.tcpPingCount++;
			xsdk_timer_set((xlink_timer*) (&tcp_timer), 15);

		}
		break;
	default:
		break;
	}
}

//XLINK_FUNC void XlinkTcpNextWork(void) {
//
//	m_tcp_client.work.work_sta = TCP_STATUS_START;
//	switch ((XLINK_TCP_WORK_TYPE) m_tcp_client.work.work_type) {
//	case TCP_SEND_HTTPCONNECT:
//		break;
//	case TCP_ACTIVATION:
//		m_tcp_client.work.work_type = TCP_CONNECT;
//		break;
//	case TCP_CONNECT:
//		m_tcp_client.work.work_type = TCP_NEED_SEND_UPGRADE;
//		m_tcp_client.last_SendPingTime = g_xlink_info.g_XlinkSdkTime;
//		//m_tcp_client.work.work_type = TCP_PING;
//		break;
//	case TCP_NEED_SEND_UPGRADE:
//		m_tcp_client.last_SendPingTime = g_xlink_info.g_XlinkSdkTime;
//		m_tcp_client.work.work_type = TCP_PING;
//		break;
//	case TCP_PING:
//
//		break;
//	default:
//		break;
//	}
//}

XLINK_FUNC void xsdk_tcp_SendSync_V2(unsigned char* data, x_uint16 datalen, unsigned char flag) {

	xlink_Message header;
	unsigned char sendbuf[XLINK_DATAPOINT_MAX_BYTES + 20] = { 0x00 };
	char *temp = NULL;
	x_uint16 sendlen = 0;

	header.bits.type = TCP_TYPE_SYNC;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	if (datalen > XLINK_DATAPOINT_MAX_BYTES)
		return;

	sendlen = datalen + 5 + 7;

	xlink_memset(sendbuf, 0, XLINK_DATAPOINT_MAX_BYTES+20);

	sendbuf[0] = header.byte;

	sendbuf[1] = UTIL_INT32_GET_BIT3(datalen + 7);
	sendbuf[2] = UTIL_INT32_GET_BIT2(datalen + 7);
	sendbuf[3] = UTIL_INT32_GET_BIT1(datalen + 7);
	sendbuf[4] = UTIL_INT32_GET_BIT0(datalen + 7);

	sendbuf[5] = g_xlink_info.config.Deviceid[0];
	sendbuf[6] = g_xlink_info.config.Deviceid[1];
	sendbuf[7] = g_xlink_info.config.Deviceid[2];
	sendbuf[8] = g_xlink_info.config.Deviceid[3];

	sendbuf[9] = 1; //message id
	sendbuf[10] = 1;

	sendbuf[11] = 0x06; //DataPoint

	if (flag) {
		sendbuf[11] |= 0x08; //DataPoint
	}

	temp = (char *) &sendbuf[12];
	xlink_memcpy(temp, (char* )data, datalen);

	XlinkTcpSendData(sendbuf, sendlen);
}
XLINK_FUNC int xsdk_tcp_SendSync_V2_cb(unsigned short handle, unsigned char* data, x_uint16 datalen, unsigned char flag) {

	xlink_Message header;
	unsigned char sendbuf[XLINK_DATAPOINT_MAX_BYTES + 20] = { 0x00 };
	char *temp = NULL;
	int ret = -1;
	x_uint16 sendlen = 0;

	header.bits.type = TCP_TYPE_SYNC;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	if (datalen > XLINK_DATAPOINT_MAX_BYTES)
		return -1;

	sendlen = datalen + 5 + 7;

	xlink_memset(sendbuf, 0, XLINK_DATAPOINT_MAX_BYTES+20);

	sendbuf[0] = header.byte;

	sendbuf[1] = UTIL_INT32_GET_BIT3(datalen + 7);
	sendbuf[2] = UTIL_INT32_GET_BIT2(datalen + 7);
	sendbuf[3] = UTIL_INT32_GET_BIT1(datalen + 7);
	sendbuf[4] = UTIL_INT32_GET_BIT0(datalen + 7);

	sendbuf[5] = g_xlink_info.config.Deviceid[0];
	sendbuf[6] = g_xlink_info.config.Deviceid[1];
	sendbuf[7] = g_xlink_info.config.Deviceid[2];
	sendbuf[8] = g_xlink_info.config.Deviceid[3];

	sendbuf[9] = handle >> 8; //message id
	sendbuf[10] = handle & 0x00ff;

	sendbuf[11] = 0x06; //DataPoint

	if (flag) {
		sendbuf[11] |= 0x08; //DataPoint
	}

	temp = (char *) &sendbuf[12];
	xlink_memcpy(temp, (char* )data, datalen);

	ret = XlinkTcpSendData(sendbuf, sendlen);

	return ret;
}

XLINK_FUNC x_int32 XlinkSendTcpBroadcast(unsigned short handle, const unsigned char * data, const unsigned int datalen) {
	xlink_Message header;
	x_uint16 sendlen = 0;
	unsigned char sendbuf[__XLINK_BUFFER_PIPE__ + 50] = { 0x00 };
	x_int32 ret = -1;
	if (g_xlink_info.net_info.tcpFlag.Bit.isLogined == 0) {
		return -1;
	}

	if (data == NULL) {
		return -2;
	}
	if (datalen > __XLINK_BUFFER_PIPE__)
		return -3;

	header.bits.type = TCP_TYPE_PIPE_2;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	sendlen = datalen + 7 + 5;

	sendbuf[0] = header.byte;
	sendbuf[1] = UTIL_INT32_GET_BIT3(datalen + 7);
	sendbuf[2] = UTIL_INT32_GET_BIT2(datalen + 7);
	sendbuf[3] = UTIL_INT32_GET_BIT1(datalen + 7);
	sendbuf[4] = UTIL_INT32_GET_BIT0(datalen + 7);

	sendbuf[5] = g_xlink_info.config.Deviceid[0];
	sendbuf[6] = g_xlink_info.config.Deviceid[1];
	sendbuf[7] = g_xlink_info.config.Deviceid[2];
	sendbuf[8] = g_xlink_info.config.Deviceid[3];

	sendbuf[9] = (unsigned char) ((handle & 0xff00) >> 8);
	sendbuf[10] = (unsigned char) (handle & 0x00ff);

	if (g_xlink_user_config->pipetype == 1) {
		sendbuf[11] = 1;
	} else {
		sendbuf[11] = 0;
	}

	xlink_memcpy(&sendbuf[12], data, datalen);

	ret = XlinkTcpSendData(sendbuf, sendlen);
	return ret;
}

XLINK_FUNC x_int32 XlinkSendTcpPipe(unsigned short handle, const unsigned char * data, const unsigned int datalen, x_uint32 to_id) {

	xlink_Message header;
	x_uint16 sendlen = 0;
	x_int32 ret = -1;
	unsigned char sendbuf[__XLINK_BUFFER_PIPE__ + 50];
	if (g_xlink_info.net_info.tcpFlag.Bit.isLogined == 0) {
		return -1;
	}
	if (data == NULL) {
		return -2;
	}
	if (datalen > __XLINK_BUFFER_PIPE__)
		return -3;

	header.bits.type = TCP_TYPE_PIPE;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	sendlen = datalen + 7 + 5;
	xlink_memset(sendbuf, 0, __XLINK_BUFFER_PIPE__ + 50);

	sendbuf[0] = header.byte;

	sendbuf[1] = UTIL_INT32_GET_BIT3(datalen + 7);
	sendbuf[2] = UTIL_INT32_GET_BIT2(datalen + 7);
	sendbuf[3] = UTIL_INT32_GET_BIT1(datalen + 7);
	sendbuf[4] = UTIL_INT32_GET_BIT0(datalen + 7);

	sendbuf[5] = UTIL_INT32_GET_BIT3(to_id);
	sendbuf[6] = UTIL_INT32_GET_BIT2(to_id);
	sendbuf[7] = UTIL_INT32_GET_BIT1(to_id);
	sendbuf[8] = UTIL_INT32_GET_BIT0(to_id);

	sendbuf[9] = (handle >> 8) & 0xff;
	sendbuf[10] = (handle >> 0) & 0xff;

	if (g_xlink_user_config->pipetype == 1) {
		sendbuf[11] = 1;
	} else {
		sendbuf[11] = 0;
	}

	xlink_memcpy(&sendbuf[12], data, datalen);

	ret = XlinkTcpSendData(sendbuf, sendlen);
	return ret;
}
XLINK_FUNC static void XlinkTcpWorkPing(void) {

	unsigned char sendbuf[6] = { 0x00 };
	xlink_Message header;

	header.bits.type = TCP_TYPE_PING;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	sendbuf[0] = header.byte;
	sendbuf[1] = 0;
	sendbuf[2] = 0;
	sendbuf[3] = 0;
	sendbuf[4] = 0;
	XlinkTcpSendData(sendbuf, 5);
}

XLINK_FUNC static void XlinkTcpWorkConnect(XLINK_TCP_CLIENT *tcp_cli) {

	xlink_Message header;
	char AuthBuffer[XLINK_SIZE_17];
	unsigned char SendBuffer[60] = { 0x00 };
	x_uint16 AuthStrLength = 0;
	unsigned short SendBufferIndex = 0;
	xlink_memset(AuthBuffer, 0, XLINK_SIZE_17);

	xsdk_read_config(XLINK_CONFIG_INDEX_AUTH, AuthBuffer, XLINK_SIZE_17 - 1);
	AuthStrLength = xlink_strlen(AuthBuffer);
	if (AuthStrLength > 0) {

	} else {
#if __XDEBUG__
		XSDK_DEBUG_ERROR("tcp send connect get auth failed  length = %d", AuthStrLength);
#endif
		g_xlink_info.config.flag.Bit.isActivation = 0;
		xsdk_write_config(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));
		xsdk_config_save();
		xsdk_closeTCP(1);
		return;
	}

	header.bits.type = TCP_TYPE_CONNECT;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	SendBuffer[0] = header.byte;

	SendBuffer[1] = 0;
	SendBuffer[2] = 0;
	SendBuffer[3] = 0;
	SendBuffer[4] = UTIL_INT32_GET_BIT0(AuthStrLength + 10);

	SendBuffer[5] = XLINK_AGREEMENT_VERSION3;

	//device id
	SendBuffer[6] = g_xlink_info.config.Deviceid[0];
	SendBuffer[7] = g_xlink_info.config.Deviceid[1];
	SendBuffer[8] = g_xlink_info.config.Deviceid[2];
	SendBuffer[9] = g_xlink_info.config.Deviceid[3];

	//auth
	SendBuffer[10] = 0;
	SendBuffer[11] = AuthStrLength;
	xlink_memcpy(&SendBuffer[12], AuthBuffer, AuthStrLength);
	SendBufferIndex = AuthStrLength + 12;

	SendBuffer[SendBufferIndex++] = g_xlink_info.config.flag.Bit.isChangedPassword;
	SendBuffer[SendBufferIndex++] = UTIL_INT16_GET_BITH(TCP_KEEPALIVETIME);
	SendBuffer[SendBufferIndex++] = UTIL_INT16_GET_BITL(TCP_KEEPALIVETIME);

	XlinkTcpSendData(SendBuffer, AuthStrLength + 15);
}
const char HEXBUF[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
XLINK_FUNC static int XlinkGetMACHex(char * str, char *dat, int len) {
	int i = 0;
//    for(i = 0; i < len; i++) {
//        xlink_sprintf(str+i*2, "%02X", dat[i]);
//    }
	for (i = 0; i < len; i++) {
		str[i * 2] = HEXBUF[(dat[i] >> 4) & 0x0f];
		str[i * 2 + 1] = HEXBUF[dat[i] & 0x0f];
	}

	return 0;
}
XLINK_FUNC static int XlinkTcpWorkActivation(void) {

	xlink_Message header;
	unsigned char encrypt[100];
	unsigned char ss[16];
	char ActiveMD5Buffer[64];
	unsigned char SendBuffer[100];

	int pos = 0;

	header.byte = 0;
	header.bits.type = TCP_TYPE_ACTIVATE;
	header.bits.resp = 0;
	header.bits.version = PROTOCOL_VERSION;

	//x_uint8 i;

	xlink_memset(encrypt, 0, 100);

	xlink_memcpy(encrypt, g_xlink_info.dev_info.product_key, XLINK_SIZE_33-1);

	XlinkGetMACHex((char*) (encrypt + 32), (char*) g_xlink_user_config->mac, g_xlink_user_config->maclen);

	xlinkGetMd5(ss, encrypt, strlen((char*) encrypt));

	xlink_memset(ActiveMD5Buffer, 0, 64);
	XlinkGetMACHex(ActiveMD5Buffer, (char *) ss, 16);

	//header
	SendBuffer[0] = header.byte;
	//BodyLen
	SendBuffer[1] = 0;
	SendBuffer[2] = 0;
	SendBuffer[3] = 0;
	SendBuffer[4] = 48;

	//version

	SendBuffer[5] = XLINK_AGREEMENT_VERSION3;

	pos = 6;
	SendBuffer[pos++] = 0;
	SendBuffer[pos++] = g_xlink_user_config->maclen;
	xlink_memcpy(&SendBuffer[pos], g_xlink_user_config->mac, g_xlink_user_config->maclen);
	pos += g_xlink_user_config->maclen;

	//wifi hardware company
	SendBuffer[pos++] = g_xlink_user_config->wifi_type;

	//wifi software version
	SendBuffer[pos++] = UTIL_INT16_GET_BITH(g_xlink_user_config->wifisoftVersion);
	SendBuffer[pos++] = UTIL_INT16_GET_BITL(g_xlink_user_config->wifisoftVersion);
	//mcu hardware version
	SendBuffer[pos++] = g_xlink_user_config->mcuHardwareVersion;
	//SendBuffer[15] = 0;
	//mcu software version
	SendBuffer[pos++] = UTIL_INT16_GET_BITH(g_xlink_user_config->mcuHardwareSoftVersion);
	SendBuffer[pos++] = UTIL_INT16_GET_BITL(g_xlink_user_config->mcuHardwareSoftVersion);

	//need read active code
	SendBuffer[pos++] = 0;
	SendBuffer[pos++] = 32;
	xlink_memcpy(&SendBuffer[pos], ActiveMD5Buffer, 32);
	pos += 32;
	SendBuffer[pos++] = 0;	//reserved

	//BodyLen
	SendBuffer[1] = 0;
	SendBuffer[2] = 0;
	SendBuffer[3] = 0;
	SendBuffer[4] = pos - 5;

	XlinkTcpSendData(SendBuffer, pos);

	return 1;
}

