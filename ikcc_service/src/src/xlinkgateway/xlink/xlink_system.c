#include "xlink_system.h"
#include "xlink_net.h"
#include "xlink_client.h"
#include "xlink_md5.h"
#include "xlink_tcp_type.h"
#include "xlink_tcp_client.h"
#include "xlinkv4Udp.h"
#include "xsdk_config.h"

#if __ALL_DEVICE__
#include "XlinkByteQueue.h"
extern ByteQueue sg_queue;
#endif

extern G_XLINK_INFO g_xlink_info;

static unsigned char isSystemInited = 0;
static XLINK_SYS_TIME m_currentSystemTime;
static xsdk_time_t m_old_getTime = 0;
static int dayOfMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static union {
	unsigned char byte;
	struct {
		unsigned char isInitSystem :1;
		unsigned char res :7;
	} bit;
} m_systemFlag;



XLINK_FUNC static void xlinkSysTimeCount(xsdk_time_t c_time);

XLINK_FUNC char *XlinkVersion(void) {
	static char info[200];
	xlink_memset(info, 0, 200);
	xlink_sprintf(info, "sdk version:%d build time:%s %s buffer size %d ", __XLINK_VERSION__, __DATE__, __TIME__, __XLINK_BUFFER_PIPE__);
	return info;
}

#if __LWIP__ESP_8266 || __MT7681__ || __STM32F107__ || __STM32F103_UIP__ || __ALL_DEVICE__
void XLINK_FUNC setServerStatus_(unsigned char stat,unsigned char is80Port) {

	if (stat == 1) {
		if(is80Port == 1) {
			g_xlink_info.net_info.tcpFlag.Bit.is80Port = 1;
			g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess = 0;
			XlinkTcpInit(TCP_SEND_HTTPCONNECT);
		} else {
			XlinkTcpInit(TCP_ACTIVATION);
			g_xlink_info.net_info.tcpFlag.Bit.is80Port = 0;
			g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess = 1;
		}

		g_xlink_info.net_info.tcp_fd = 1;
	}
	else {
		g_xlink_info.net_info.tcp_fd =0;
	}
}

void xlink_delay(xsdk_time_t x) {

}
#endif

XLINK_FUNC x_bool XlinkInit(char* product_id, char* product_key, XLINK_USER_CONFIG *config) {
	g_xlink_user_config = config;
	if (xlink_strlen(product_id) != XLINK_SIZE_33 - 1) {
		return xlink_false;
	}

	isSystemInited = 0;
	if (config == NULL) {
		return xlink_false;
	}

	if (config->maclen > XLINK_MAC_LEN_MAX)
		return xlink_false;
	if (config->maclen < XLINK_MAC_LEN_MIN)
		return xlink_false;

	isSystemInited = 0;
	if (config->OnWriteConfig == NULL || config->OnReadConfig == NULL) {
		return xlink_false;
	}

	g_xlink_user_config = config;

	xsdk_config_init(product_id, product_key, config->mac);
	/*
	 * This must be initialized
	 */
	XlinkSdkDataInit(product_id, product_key);

	XlinnkClientInit();
	XlinkV4UdpInit();

#if	__ALL_DEVICE__
	sg_queue.BuferLength = config->tcpRecvBuuferLength;
	sg_queue.Buffer = config->tcpRecvBuffer;
#endif

#if !__LWIP__ESP_8266 && !__MT7681__ && !__STM32F107__ && ! __STM32F103_UIP__  && !__ALL_DEVICE__
	XlinkNetInit();
#endif

	//The default is to connect wifi state
	g_xlink_info.net_info.tcpFlag.Bit.isConnectWifi = 1;

#if __LWIP__ESP_8266 || __MT7681__ || __STM32F107__ || __STM32F103_UIP__ || __ALL_DEVICE__
	g_xlink_info.net_info.udp_fd = 1;
#endif
	isSystemInited = 1;
	xlink_memset(&m_currentSystemTime, 0, sizeof(m_currentSystemTime));
	m_systemFlag.byte = 0;
	XlinkGetServerTime();
	return xlink_true;
}

XLINK_FUNC void XlinkReset(void) {
	isSystemInited = 0;
	if (g_xlink_info.net_info.tcp_fd > 0) {
		xlink_close(g_xlink_info.net_info.tcp_fd);
		g_xlink_info.net_info.tcp_fd = 0;
	}
	if (g_xlink_info.net_info.udp_fd > 0) {
		xlink_close(g_xlink_info.net_info.udp_fd);
		g_xlink_info.net_info.udp_fd = 0;
	}
	XlinkV4UdpUnInit();
}

XLINK_FUNC void XlinkSetWifiStatus(unsigned char status) {
	g_xlink_info.net_info.tcpFlag.Bit.isConnectWifi = status;
}

XLINK_FUNC static void xlinkSysTimeCount(xsdk_time_t c_time) {

	int temp = 0; //xlink_ticks_diff(c_time, m_old_getTime);
	if (c_time < m_old_getTime) {
		m_old_getTime = c_time;
	}
	temp = c_time - m_old_getTime;
	if (temp > 0) {
		m_old_getTime = c_time;
		m_currentSystemTime.sec += temp;
		if (m_currentSystemTime.sec >= 60) {
			m_currentSystemTime.sec -= 60;
			m_currentSystemTime.min++;
			if (m_currentSystemTime.min >= 60) {
				m_currentSystemTime.min = 0;
				m_currentSystemTime.hour++;
				if (m_currentSystemTime.hour >= 24) {
					m_currentSystemTime.hour = 0;
					m_currentSystemTime.week++;

					if (m_currentSystemTime.week == 7) {
						m_currentSystemTime.week = 0;
					}
					m_currentSystemTime.day++;

					if (m_currentSystemTime.day > dayOfMonth[m_currentSystemTime.mon - 1]) {
						m_currentSystemTime.day = 1;
						m_currentSystemTime.mon++;
						if (m_currentSystemTime.mon > 12) {
							m_currentSystemTime.mon = 1;
							m_currentSystemTime.year++;
							if (m_currentSystemTime.year % 400 == 0 || (m_currentSystemTime.year % 100 != 0 && m_currentSystemTime.year % 4 == 0)) {
								dayOfMonth[1] = 29;
							} else {
								dayOfMonth[1] = 28;
							}
						}
					}
				}
			}
		}
	}

}

XLINK_FUNC void XlinkLoop(xsdk_time_t c_time, x_int32 timeout_ms) {

	g_xlink_info.g_XlinkSdkTime = c_time;
	if (isSystemInited == 1) {
		xlink_net_loop(c_time, timeout_ms);
		XlinkResend();
	} else {
		xlink_msleep(timeout_ms);
	}
	if (m_systemFlag.bit.isInitSystem == 1) {
		xlinkSysTimeCount(c_time);
	}

}

XLINK_FUNC void XlinkSetDeviceName(char *NameStr) {
	unsigned char nameLength = 0;
	char temp[XLINK_SIZE_17];

	if (g_xlink_info.config.flag.Bit.isAppSetName == 1) {
		return;
	}

	nameLength = xlink_strlen(NameStr);
	if (nameLength + 1 > XLINK_SIZE_17) {
		nameLength = XLINK_SIZE_17 - 1;
	}

	xlink_memset(temp, 0, XLINK_SIZE_17);
	xlink_memcpy(temp, NameStr, nameLength);
	xsdk_write_config(XLINK_CONFIG_INDEX_NAME, temp, XLINK_SIZE_17 - 1);
	xsdk_config_save();

}

XLINK_FUNC void XlinkGetServerTime(void) {
	g_xlink_info.dev_info.flags.Bit.isGetServerTimeTask = 1;
}

XLINK_FUNC void xlink_net_setSystemTime(XLINK_SYS_TIME *pTime) {
	m_systemFlag.bit.isInitSystem = 1;
	m_old_getTime = g_xlink_info.g_XlinkSdkTime;
	m_currentSystemTime.day = pTime->day;
	m_currentSystemTime.hour = pTime->hour;
	m_currentSystemTime.min = pTime->min;
	m_currentSystemTime.mon = pTime->mon;
	m_currentSystemTime.sec = pTime->sec;
	m_currentSystemTime.week = pTime->week;
	m_currentSystemTime.year = pTime->year;
	m_currentSystemTime.zones = pTime->zones;
	if (m_currentSystemTime.year % 400 == 0 || (m_currentSystemTime.year % 100 != 0 && m_currentSystemTime.year % 4 == 0)) {
		dayOfMonth[1] = 29;
	} else {
		dayOfMonth[1] = 28;
	}
}

XLINK_FUNC int XlinkGetSystemTime(XLINK_SYS_TIME *pTime) {

	pTime->day = m_currentSystemTime.day;
	pTime->hour = m_currentSystemTime.hour;
	pTime->min = m_currentSystemTime.min;
	pTime->mon = m_currentSystemTime.mon;
	pTime->sec = m_currentSystemTime.sec;
	pTime->week = m_currentSystemTime.week;
	pTime->year = m_currentSystemTime.year;
	pTime->zones = m_currentSystemTime.zones;

	return m_systemFlag.bit.isInitSystem;
}

XLINK_FUNC int XlinkSystemTcpLoop(void) {
	int ret = 0;
#if (!__LWIP__ESP_8266) && !__MT7681__ && !__STM32F107__ && !__STM32F103_UIP__  && !__ALL_DEVICE__
	if (isSystemInited == 1 && g_xlink_info.net_info.tcpFlag.Bit.isConnectWifi != 0) {
		ret = xlink_net_loop_Domain_name();
	}
#endif
	return ret;
}

XLINK_FUNC int XlinkGetMacString(char *RetMacBuffer, const int bufflen) {
	const char HexStringBuffer[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	int i = 0;
	if (g_xlink_user_config == NULL) {
		return 0;
	}

	if (bufflen < (g_xlink_user_config->maclen * 2))
		return 0;

	for (i = 0; i < g_xlink_user_config->maclen; i++) {
		RetMacBuffer[i * 2] = HexStringBuffer[((g_xlink_user_config->mac[i] & 0xf0) >> 4)];
		RetMacBuffer[i * 2 + 1] = HexStringBuffer[(g_xlink_user_config->mac[i] & 0x0f)];
	}
	return (i * 2);
}

XLINK_FUNC int XlinkGetDeviceID(void) {
	int ret = 0;
	if (g_xlink_info.config.flag.Bit.isActivation == 0)
		return 0;
	ret = UTIL_INT32_SET(g_xlink_info.config.Deviceid[0], g_xlink_info.config.Deviceid[1], g_xlink_info.config.Deviceid[2], g_xlink_info.config.Deviceid[3]);
	return ret;
}

XLINK_FUNC int XlinkUpdateDataPointUdp(unsigned short handle, unsigned char* data, x_uint16 datalen, unsigned char flag) {
	int ret = -1;
	if (datalen > XLINK_DATAPOINT_MAX_BYTES)
		return -1;
	if (g_xlink_info.flag.bit.clientCount == 0) {
		return -1;
	}
	ret = XlinkClientSendUdpDataPointSync_V2(data, datalen, flag);
	return ret;
}

XLINK_FUNC int XlinkUpdateDataPointTcp(unsigned short handle, unsigned char* data, x_uint16 datalen, unsigned char flag) {
	int ret = -1;
	if (datalen > XLINK_DATAPOINT_MAX_BYTES)
		return -1;
	ret = xsdk_tcp_SendSync_V2_cb(handle, data, datalen, flag);
	return ret;
}







