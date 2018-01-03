#include "xlink_process_tcp_data.h"
#include  "xlink_tcp_type.h"
#include "xlink_message.h"
#include "xsdk_config.h"
#include "xlink_net.h"
#include "xlink_md5.h"

#if __ALL_DEVICE__
#include "xlink_system.h"
#include "XlinkByteQueue.h"
unsigned char isQueeuInit = 0;
ByteQueue sg_queue = {NULL, 0, 0, 0, 0};
#endif

XLINK_FUNC static x_int32 xlink_process_tcp_active(unsigned char * Buffer, unsigned int BufferLen);
XLINK_FUNC static void xlink_process_tcp_event(unsigned char * Buffer, unsigned int BufferLen);
XLINK_FUNC static void xlink_process_tcp_subscribe_V3(unsigned char * Buffer, unsigned int BufferLen);
XLINK_FUNC static void xlink_process_tcp_set_V2(unsigned char * Buffer, unsigned int BufferLen);
XLINK_FUNC static void xlink_process_tcp_probe_V2(unsigned char * data, unsigned int datalen);
XLINK_FUNC static void xlink_process_tcp_connect(unsigned char * data, unsigned int datalen);
XLINK_FUNC static void xlink_process_tcp_upgrade(unsigned char * Buffer, unsigned int BufferLen);
XLINK_FUNC static void xlink_process_tcp_pipe2(unsigned char * data, unsigned int datalen);
XLINK_FUNC static void xlink_process_tcp_pipe(unsigned char * Buffer, unsigned int datalen);

extern XLINK_FUNC unsigned char XlinkPorcess_UDP_GetScan(void);

static XLINK_FUNC void process_time(unsigned char * Buffer, x_int16 BufferLen) {
	static XLINK_SYS_TIME temptime;

	temptime.year = ((Buffer[0] << 8) & 0xff00) + Buffer[1];
	temptime.mon = Buffer[2];
	temptime.day = Buffer[3];
	temptime.week = Buffer[4] - 1; //0-6
	temptime.hour = Buffer[5];
	temptime.min = Buffer[6];
	temptime.sec = Buffer[7];
	temptime.zones = ((Buffer[8] << 8) & 0xff00) + Buffer[9];
	g_xlink_info.dev_info.getServerTimeDelay = 599;
	g_xlink_info.dev_info.flags.Bit.isGetServerTimeTask = 0;
	xlink_net_setSystemTime(&temptime);
	if (g_xlink_user_config->OnServerTime) {
		g_xlink_user_config->OnServerTime(&temptime);
	}
}

XLINK_FUNC void xlink_process_tcp_event_res(unsigned char * data, x_int16 datalen) {
	unsigned int type = data[0];
	switch (type) {
	case 2:
//		if (g_xlink_info.config.flag.Bit.isUpgrade == 1) {
//			g_xlink_info.config.flag.Bit.isUpgrade = 0;
//			g_xlink_info.config.CurrentSoftVersion = g_xlink_user_config->wifisoftVersion;
//			xsdk_write_config(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));
//			xsdk_config_save();
//		}
		break;
	case 13:
		process_time(data + 1, datalen - 1);
		break;
	default:
		break;
	}
}

XLINK_FUNC void xlink_process_tcp_data(unsigned char * data, x_int16 datalen, x_uint32 bodylen) {

	xlink_Message header;
	unsigned short __handle = 0;

#if __LWIP__ESP_8266 || __MT7681__ || __STM32F107__ || __STM32F103_UIP__ || __ALL_DEVICE__
	if (g_xlink_info.net_info.tcpFlag.Bit.is80Port == 1) {
		if (g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess == 0) {
			g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess = 1;
			XlinkTcpInit(TCP_ACTIVATION);
			return;
		}
	}
	bodylen = UTIL_INT32_SET(data[1],data[2],data[3],data[4]);
#endif

	header.byte = data[0];

#if __XDEBUG__
	XSDK_DEBUG_DEBUG("header.bits.type=%d ", header.bits.type);
#endif

	if (check_typeOk_t(header.byte)) {
		XlinkTcpPing_Clear();
	}

	switch (header.bits.type) {
	case TCP_TYPE_ACTIVATE:
		xlink_process_tcp_active(&data[5], bodylen);
		break;
	case TCP_TYPE_CONNECT:
		xlink_process_tcp_connect(&data[5], bodylen);
		break;
	case TCP_TYPE_SET:
		if (header.bits.resp == 1) {
			break;
		}
		if (data[11] & 0x07) { //flag:bit2
			xlink_process_tcp_set_V2(&data[5], bodylen);
		}
		break;
	case TCP_TYPE_SYNC:
		if (g_xlink_user_config != NULL) {
			if (g_xlink_user_config->OnTcpDatapointSendCb != NULL) {
				if (datalen > 7) {
					__handle = data[5];
					__handle <<= 8;
					__handle += data[6];
					g_xlink_user_config->OnTcpDatapointSendCb(__handle, data[7] == 0 ? 1 : 0);
				}
			}
		}
		break;
	case TCP_TYPE_SETPWD:
		break;
	case TCP_TYPE_COLLECT:
		break;
	case TCP_TYPE_PIPE:
		if (header.bits.resp == 1) {
			if (g_xlink_user_config != NULL) {
				if (g_xlink_user_config->OnTcpPipeSendCb != NULL) {
					if (datalen > 11) {
						__handle = data[9];
						__handle <<= 8;
						__handle += data[10];
						g_xlink_user_config->OnTcpPipeSendCb(__handle, data[11] == 0 ? 1 : 0);
					}
				}
			}
			break;
		}
		xlink_process_tcp_pipe(&data[5], bodylen);
		break;
	case TCP_TYPE_PIPE_2:

		if (header.bits.resp == 1) {
			if (g_xlink_user_config != NULL) {
				if (g_xlink_user_config->OnTcpPipe2SendCb != NULL) {
					if (datalen > 7) {
						__handle = data[5];
						__handle <<= 8;
						__handle += data[6];
						g_xlink_user_config->OnTcpPipe2SendCb(__handle, data[7] == 0 ? 1 : 0);
					}
				}
			}
			break;
		}
		xlink_process_tcp_pipe2(&data[5], bodylen);
		break;
	case TCP_TYPE_PROBE:
		if (header.bits.resp == 1) {
			break;
		}
//		if (data[11] & 0x04) {//flag:bit2
		xlink_process_tcp_probe_V2(&data[5], bodylen);

//		}
#if __XDEBUG__

		XSDK_DEBUG_DEBUG("TCP_TYPE_PROBE=%d ", data[11]);

#endif
		break;
	case TCP_TYPE_SUBSCRIBE:
		if (header.bits.resp == 1) {
			break;
		}

		if (data[27] & 0x04) {
			xlink_process_tcp_subscribe_V3(&data[5], bodylen);
//            XlinkUartSend("TCP_TYPE_SUBSCRIBE v3",23);
		}
		break;
	case TCP_TYPE_PING:
		xlinkChangeWork(TCP_PING);
		break;
	case TCP_TYPE_DISCONNECT:
		break;
	case TCP_TYPE_EVENT:
		if (header.bits.resp == 1) {
			xlink_process_tcp_event_res(&data[5], bodylen);
			break;
		}
		xlink_process_tcp_event(&data[5], bodylen);
		break;
	default:
		break;
	}
}

XLINK_FUNC static x_int32 xlink_process_tcp_active(unsigned char * Buffer, unsigned int BufferLen) {

	x_uint16 mAuthLength = 0;
	char *mAuthString = NULL;
	char authTemp[XLINK_SIZE_17];
#if __XDEBUG__
	XSDK_DEBUG_DEBUG("recv tcp active package ");
#endif
	if (BufferLen < 7) {
		return -1;
	}

	//How to return not equal to zero, that is, the error message
	if (Buffer[0] != 0) {
#if __XDEBUG__

		XSDK_DEBUG_DEBUG("device active failed %d", Buffer[0]);

#endif
		return -1;
	}
	//read device id
	g_xlink_info.config.Deviceid[0] = Buffer[1];
	g_xlink_info.config.Deviceid[1] = Buffer[2];
	g_xlink_info.config.Deviceid[2] = Buffer[3];
	g_xlink_info.config.Deviceid[3] = Buffer[4];
#if __XDEBUG__

	XSDK_DEBUG_DEBUG(" RECV device id %d:%d:%d:%d", Buffer[1], Buffer[2], Buffer[3], Buffer[4]);

#endif
	mAuthLength = UTIL_INT16_SET(Buffer[5], Buffer[6]);
	if (mAuthLength + 6 > BufferLen && mAuthLength < 17) {
		return -1;
	}

	mAuthString = (char*) &Buffer[7];

	xlink_memset(authTemp, 0, XLINK_SIZE_17);
	if (mAuthLength + 1 > XLINK_SIZE_17) {
#if __XDEBUG__
		XSDK_DEBUG_ERROR(" get tcp active auth length %d", mAuthLength);
#endif
		mAuthLength = 16;
	}
	xlink_memcpy(authTemp, mAuthString, mAuthLength);

	xsdk_write_config(XLINK_CONFIG_INDEX_AUTH, authTemp, XLINK_SIZE_17 - 1);
#if __XDEBUG__
	XSDK_DEBUG_DEBUG("xlink active get auth %s", authTemp);
#endif

	g_xlink_info.config.flag.Bit.isActivation = 1;

	xsdk_write_config(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));
	xsdk_config_save();
	xlinkChangeWork(TCP_CONNECT);

	return 0;

}

XLINK_FUNC static void xlink_process_tcp_event(unsigned char * Buffer, unsigned int BufferLen) {
	unsigned int type = Buffer[0];

#if __XDEBUG__
	XSDK_DEBUG_DEBUG(" recv tcp event package event type %d data length %d", type, BufferLen);

#endif

	switch (type) {
	case 1:
		xlink_process_tcp_upgrade(&Buffer[1], BufferLen - 1);
		break;
	case 10:
		xlink_process_tcp_notify(&Buffer[0], BufferLen);
		break;
	default:
		break;
	}
}

XLINK_FUNC void xlink_process_tcp_notify(unsigned char * Buffer, unsigned int BufferLen) {
	unsigned short notifytype = 0;

	if (BufferLen < 10) {
		return;
	}
	notifytype = Buffer[8];
	notifytype <<= 8;
	notifytype += Buffer[9];
	if (g_xlink_user_config->OnTcpNotify != NULL) {
		g_xlink_user_config->OnTcpNotify(notifytype, Buffer + 10 + 2, BufferLen - 10 - 2);
	}
}

XLINK_FUNC static void xlink_process_tcp_subscribe_V3(unsigned char * Buffer, unsigned int BufferLen) {
	//app id 0 1 2 3
	unsigned char mRetOpt = 0;
	unsigned char passwordBuffer[XLINK_SIZE_17] = { 0x00 };
	unsigned char SendBuffer[17] = { 0x00 };
	unsigned char ackbuf[4];
	int subkey = 0;
	xlink_Message mRetHeader;

//    unsigned char retBuffer[100] = { 0x00 };
//    int retlen;

	xsdk_read_config(XLINK_CONFIG_INDEX_SUBKEY_KEY, (char*) (&subkey), 4);

	ackbuf[0] = UTIL_INT32_GET_BIT0(subkey);
	ackbuf[1] = UTIL_INT32_GET_BIT1(subkey);
	ackbuf[2] = UTIL_INT32_GET_BIT2(subkey);
	ackbuf[3] = UTIL_INT32_GET_BIT3(subkey);

	if (subkey < 0) {
		mRetOpt = 2;
	} else {
		xlink_memset(passwordBuffer, 0, 16);
		xlinkGetMd5(passwordBuffer, (unsigned char *) (ackbuf), 4);
		if (xlink_strncmp( (Buffer + 4), passwordBuffer, XLINK_SIZE_17 - 1) != 0) {
#if __XDEBUG__
			XSDK_DEBUG_DEBUG(" info tcp subscribe check pwd failed md5=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", passwordBuffer[0], passwordBuffer[1],
					passwordBuffer[2], passwordBuffer[3], passwordBuffer[4], passwordBuffer[5], passwordBuffer[6], passwordBuffer[7], passwordBuffer[8], passwordBuffer[9],
					passwordBuffer[10], passwordBuffer[11], passwordBuffer[12], passwordBuffer[13], passwordBuffer[14], passwordBuffer[15]);

#endif
			mRetOpt = 2;
		} else {
			mRetOpt = 0;
		}
	}

//XlinkUartSend("retun pkg.",12);

	mRetHeader.byte = 0;
	mRetHeader.bits.resp = 1;
	mRetHeader.bits.type = TCP_TYPE_SUBSCRIBE;
	mRetHeader.bits.version = PROTOCOL_VERSION;
	SendBuffer[0] = mRetHeader.byte;
	//
	SendBuffer[1] = 0;
	SendBuffer[2] = 0;
	SendBuffer[3] = 0;
	SendBuffer[4] = 11;
	//App id
	SendBuffer[5] = Buffer[0];
	SendBuffer[6] = Buffer[1];
	SendBuffer[7] = Buffer[2];
	SendBuffer[8] = Buffer[3];
	//Message id
	SendBuffer[9] = Buffer[20];
	SendBuffer[10] = Buffer[21];

	SendBuffer[11] = mRetOpt;

	xsdk_read_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char*) (&subkey), 4);

	//version 3 ,read accesskey
	SendBuffer[12] = UTIL_INT32_GET_BIT3(subkey);
	SendBuffer[13] = UTIL_INT32_GET_BIT2(subkey);
	SendBuffer[14] = UTIL_INT32_GET_BIT1(subkey);
	SendBuffer[15] = UTIL_INT32_GET_BIT0(subkey);

	XlinkTcpSendData(SendBuffer, 16);
}

XLINK_FUNC static void xlink_process_tcp_set_V2(unsigned char * Buffer, unsigned int BufferLen) {

	Set_flags flag;
	x_uint16 NameLength = 0;
	char * mNameString = NULL;
	int BufferIndex = 0;
	x_uint8 mUserOpt = 0;
	xlink_Message mRetHeader;
	unsigned char sendbuf[13] = { 0x00 };

	if (BufferLen < 9) {
		return;
	}

	flag.all = Buffer[6];
	BufferIndex = 7;

	if (flag.bits.device_name) {
		//mNameString
		NameLength = UTIL_INT16_SET(Buffer[7], Buffer[8]);
		BufferIndex += 2;
		if (NameLength + 8 > BufferLen) {
			return;
		}
		mNameString = (char*) &Buffer[9];
		BufferIndex += NameLength;
		if (NameLength < XLINK_SIZE_17) {
			XlinkSdkAppSetDeviceName(mNameString, NameLength);
		}
	}

	//datapoint v2
	if (flag.bits.datapoint_v2) {
		if (g_xlink_user_config->OnSetDataPoint != NULL) {
			g_xlink_user_config->OnSetDataPoint(Buffer + BufferIndex, BufferLen - BufferIndex);
		}
	}

	mRetHeader.bits.type = TCP_TYPE_SET;
	mRetHeader.bits.resp = 1;
	mRetHeader.bits.version = PROTOCOL_VERSION;

	//Header
	sendbuf[0] = mRetHeader.byte;
	//Body len
	sendbuf[1] = 0;
	sendbuf[2] = 0;
	sendbuf[3] = 0;
	sendbuf[4] = 7;
	//Device Id
	sendbuf[5] = Buffer[0];
	sendbuf[6] = Buffer[1];
	sendbuf[7] = Buffer[2];
	sendbuf[8] = Buffer[3];
	//Message id
	sendbuf[9] = Buffer[4];
	sendbuf[10] = Buffer[5];
	//
	sendbuf[11] = mUserOpt;

	XlinkTcpSendData(sendbuf, 12);
}

XLINK_FUNC static void xlink_process_tcp_probe_V2(unsigned char * data, unsigned int datalen) {

	xlink_Message mRetHeader;
	int datapointsize = 0;
	char DeviceName[XLINK_SIZE_17] = { 0x00 };
	unsigned char DeviceNameLength = 0;
	int BodyLength = 0;
	unsigned char resp_buf[XLINK_DATAPOINT_MAX_BYTES + 20] = { 0x00 };
	int resp_buflen = 0;

	if (datalen < 7) {
		return;
	}

	mRetHeader.bits.resp = 1;
	mRetHeader.bits.version = PROTOCOL_VERSION;
	mRetHeader.bits.type = TCP_TYPE_PROBE;

	//datapointsize = xlinkGetAllDataPointSize();
	xlink_memset(DeviceName, 0, 17);
	xsdk_read_config(XLINK_CONFIG_INDEX_NAME, DeviceName, XLINK_SIZE_17 - 1);
	DeviceNameLength = strlen(DeviceName);
	//8=deviceid+msgid+code+flag
	//2= name length
//	BodyLength = DeviceNameLength + datapointsize + 8 + 2;

//	resp_buflen = BodyLength + 5;	
	//Header
	resp_buf[0] = mRetHeader.byte;
	//Body len
//	resp_buf[1] = UTIL_INT32_GET_BIT3(BodyLength);
//	resp_buf[2] = UTIL_INT32_GET_BIT2(BodyLength);
//	resp_buf[3] = UTIL_INT32_GET_BIT1(BodyLength);
//	resp_buf[4] = UTIL_INT32_GET_BIT0(BodyLength);
	//Device id
	resp_buf[5] = data[0];
	resp_buf[6] = data[1];
	resp_buf[7] = data[2];
	resp_buf[8] = data[3];
	//Message id
	resp_buf[9] = data[4];
	resp_buf[10] = data[5];
	//Code
	resp_buf[11] = 0;
	//Name and DataPoint flag
	resp_buf[12] = 0x01;
	if (XlinkPorcess_UDP_GetScan()) {
		resp_buf[12] |= 0x08;
	}

//Name Data
	resp_buf[13] = 0;
	resp_buf[14] = DeviceNameLength;
	xlink_memcpy(&resp_buf[15], DeviceName, DeviceNameLength);

	// DataPoint 
	if (g_xlink_user_config->OnGetAllDataPoint != NULL) {
		datapointsize = XLINK_DATAPOINT_MAX_BYTES;
		g_xlink_user_config->OnGetAllDataPoint(&resp_buf[15 + DeviceNameLength], &datapointsize);
		resp_buf[12] |= 0x02 | 0x04;
	}

	BodyLength = DeviceNameLength + datapointsize + 8 + 2;
	resp_buflen = BodyLength + 5;

	resp_buf[1] = UTIL_INT32_GET_BIT3(BodyLength);
	resp_buf[2] = UTIL_INT32_GET_BIT2(BodyLength);
	resp_buf[3] = UTIL_INT32_GET_BIT1(BodyLength);
	resp_buf[4] = UTIL_INT32_GET_BIT0(BodyLength);

	XlinkTcpSendData(resp_buf, resp_buflen);

//	xlink_free(resp_buf);
	return;
}

XLINK_FUNC static void xlink_process_tcp_connect(unsigned char * data, unsigned int datalen) {

	x_int8 code = data[0];

	char domainip[2 + XLINK_DOMAIN_IP_MAX_LEN + 2];
	int domainiplen = 0;

	if (data[0] != 0) {

#if __XDEBUG__

		XSDK_DEBUG_DEBUG("recv tcp active package，flag=%d", data[0]);

#endif
		if (data[0] == 12) {
			xlink_memset(domainip, 0, 2+XLINK_DOMAIN_IP_MAX_LEN+2);

			domainiplen = datalen - 2 - 4;
			if (domainiplen <= XLINK_DOMAIN_IP_MAX_LEN) {
				g_xlink_info.domain_connect_times = 0;
				xlink_memcpy(domainip, data + 2, 2);
				xlink_memcpy(domainip + 2, data + 2 + 4, domainiplen);
				xsdk_write_config(XLINK_CONFIG_INDEX_DOMAIN_IP, domainip, XLINK_DOMAIN_IP_MAX_LEN);
				xsdk_config_save();
#if __XDEBUG__
				XSDK_DEBUG_DEBUG("recv tcp active package,new port=%d,ip=%s", (domainip[0] << 8) + domainip[1], domainip + 2);
#endif
			}
		}
		xsdk_closeTCP(1);
		return;
	}
	if (code == 0) {
		g_xlink_info.net_info.tcpFlag.Bit.isLogined = 1;
		xlinkChangeWork(TCP_PING);
		if (g_xlink_user_config->OnStatus) {
			g_xlink_user_config->OnStatus(XLINK_WIFI_STA_LOGIN_SUCCESS);
		}
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("xlink login server success");
#endif
		if (g_xlink_info.config.flag.Bit.isChangedPassword == 1) {
			g_xlink_info.config.flag.Bit.isChangedPassword = 0;
			xsdk_write_config(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));
			xsdk_config_save();
		}
	} else {
		g_xlink_info.config.flag.Bit.isActivation = 0;
		xsdk_write_config(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));
		xsdk_config_save();
	}
}

XLINK_FUNC static void xlink_process_tcp_upgrade(unsigned char * Buffer, unsigned int BufferLen) {

	XLINK_UPGRADE UpGrade;
	unsigned short mSoftVersion = 0;
	unsigned char isHashCheck = 0;
	unsigned int mFileSize = 0;
	unsigned short urlLeng = 0;
	char *url = NULL;
	unsigned int Index = 0;
	char *md5Check = NULL;
	unsigned short checkleng = 0;
	unsigned char mUpgradeType = 0;

#if __XDEBUG__
	XSDK_DEBUG_DEBUG("recv upgrade package data length %d", BufferLen);

#endif

	if (BufferLen < 15) {
		return;
	}

	mUpgradeType = Buffer[0]; //=2 mcu ,=1 wifi

	if (g_xlink_user_config->wifi_type != Buffer[1]) {
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("recv upgrade package wifi get type %d  local type %d", Buffer[1], g_xlink_user_config->wifi_type);

#endif
		return;
	}

	xlink_memset(&UpGrade, 0, sizeof(XLINK_UPGRADE));

	//wifi version
	mSoftVersion = UTIL_INT16_SET(Buffer[2], Buffer[3]);
	if (mSoftVersion <= g_xlink_user_config->wifisoftVersion) {
		//UpGrade.retOpt = 3;	
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("recv upgrade package wifi currentVer %d  upgrade ver %d,old ver %d", g_xlink_user_config->wifisoftVersion, mSoftVersion,
				g_xlink_info.config.CurrentSoftVersion);

#endif
		xlinkSendUpgrade(0, mSoftVersion, g_xlink_info.config.CurrentSoftVersion);
		g_xlink_info.config.flag.Bit.isUpgrade = 0;
		g_xlink_info.config.CurrentSoftVersion = g_xlink_user_config->wifisoftVersion;
		xsdk_write_config(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));
		xsdk_config_save();
		g_xlink_user_config->OnUpgrade(&UpGrade);
	} else {
		isHashCheck = Buffer[4];

		mFileSize = UTIL_INT32_SET(Buffer[5], Buffer[6], Buffer[7], Buffer[8]);

		urlLeng = UTIL_INT16_SET(Buffer[9], Buffer[10]);

		if (BufferLen < (11 + urlLeng)) {
#if __XDEBUG__
			XSDK_DEBUG_DEBUG("xlink recv upgrade package data ", BufferLen);

#endif
			return;
		}

		url = (char*) &Buffer[11];
		Index = 11 + urlLeng;
		if (Index > BufferLen) {
#if __XDEBUG__
			XSDK_DEBUG_DEBUG("recv upgrade return");

#endif
			return;
		}
		Index++;
		checkleng = Buffer[Index++];
		if (checkleng > 0) {
			md5Check = (char*) &Buffer[Index];
		} else {
			checkleng = 0;
			md5Check = NULL;
		}

		UpGrade.checkStr = md5Check;
		UpGrade.checkStrLength = checkleng;
		UpGrade.urlstr = url;
		UpGrade.urlLength = urlLeng;
		UpGrade.isWifi = (mUpgradeType == 1) ? 1 : 0; //=2 mcu ,=1 wifi
		UpGrade.mCheckFlag = isHashCheck;
		UpGrade.mHardVersion = Buffer[1];
		UpGrade.mSoftVersion = mSoftVersion;
		UpGrade.mCurrentVersion = g_xlink_user_config->wifisoftVersion;
		UpGrade.fileSize = mFileSize;
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("recv upgrade file size %d type %d ver %d  current ver %d", mFileSize, UpGrade.mHardVersion, UpGrade.mSoftVersion,
				g_xlink_user_config->wifisoftVersion);

#endif

		if (g_xlink_user_config->OnUpgrade) {
			g_xlink_info.config.flag.Bit.isUpgrade = 1;
			g_xlink_info.config.CurrentSoftVersion = g_xlink_user_config->wifisoftVersion;
			xsdk_write_config(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));
			xsdk_config_save();
			g_xlink_user_config->OnUpgrade(&UpGrade);
		} else {

			xlinkSendUpgrade(0, mSoftVersion, g_xlink_user_config->wifisoftVersion);
		}
	}

	//if (UpGrade.retOpt == 0) {
	//g_xlink_info.config.flag.Bit.isUpgrade = 1;
	//g_xlink_user_config->writeConfig(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));
	//return;
	//}

}

XLINK_FUNC static void xlink_process_tcp_pipe2(unsigned char * data, unsigned int datalen) {

	x_uint8 mUserOpt = 0;

	if (datalen < 7) {
#if __XDEBUG__
		XSDK_DEBUG_ERROR("process tcp pipe2 datalen < 7  length %d\n", datalen);

#endif
		return;
	}

	if (g_xlink_user_config->OnTcpPipe2) {
		g_xlink_user_config->OnTcpPipe2(&data[7], datalen - 7, &mUserOpt);
	}
}

XLINK_FUNC static void xlink_process_tcp_pipe(unsigned char * Buffer, unsigned int datalen) {

	x_uint32 mDevice_id = 0;
	x_uint8 mUserOpt = 0;
	unsigned char sendbuf[15] = { 0x00 };
	xlink_Message mRetHeader;

	if (datalen < 7) {
		return;
	}

	mDevice_id = UTIL_INT32_SET(Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
	if (mDevice_id != 0) {
		mRetHeader.bits.version = PROTOCOL_VERSION;
		mRetHeader.bits.resp = 1;
		mRetHeader.bits.type = TCP_TYPE_PIPE;

		sendbuf[0] = mRetHeader.byte;
		sendbuf[1] = 0;
		sendbuf[2] = 0;
		sendbuf[3] = 0;
		sendbuf[4] = 7;

		sendbuf[5] = Buffer[0];
		sendbuf[6] = Buffer[1];
		sendbuf[7] = Buffer[2];
		sendbuf[8] = Buffer[3];

		sendbuf[9] = Buffer[4];
		sendbuf[10] = Buffer[5];

		sendbuf[11] = mUserOpt;

		XlinkTcpSendData(sendbuf, 12);
	}
	if (g_xlink_user_config->OnTcpPipe) {
		g_xlink_user_config->OnTcpPipe(&Buffer[7], datalen - 7, mDevice_id, &mUserOpt);
	}

}

#if __ALL_DEVICE__

static int IsHeadOk(unsigned char data) {
	xlink_Message header;
	header.byte = data;
	switch (header.bits.type) {
		case TCP_TYPE_ACTIVATE:
		case TCP_TYPE_CONNECT:
		case TCP_TYPE_SET:
		case TCP_TYPE_SYNC:
		case TCP_TYPE_SETPWD:
		case TCP_TYPE_COLLECT:
		case TCP_TYPE_PIPE:
		case TCP_TYPE_PIPE_2:
		case TCP_TYPE_PROBE:
		case TCP_TYPE_SUBSCRIBE:
		case TCP_TYPE_PING:
		case TCP_TYPE_DISCONNECT:
		case TCP_TYPE_EVENT:
		return 1;
		break;
		default:
		break;
	}
	return 0;
}

//void outC() {

//	xlink_printf("\r\n");
//	xlink_printf("totalsize=%d", sg_queue.TotalDataLength);
//	xlink_printf("start=%d", sg_queue.StartIndex);
//	xlink_printf("end=%d", sg_queue.EndIndex);
//	xlink_printf("\r\n");
//}

XLINK_FUNC int XlinkPushData(unsigned char * data, x_int16 datalen) {
	unsigned int ret = 0;

	if (isQueeuInit == 0) {
		ByteQueueInit(&sg_queue);
		isQueeuInit = 1;
	}
	ret = ByteQueuePushArray(&sg_queue, data, datalen);
	if (ret == E_B_NOT_FREE_BUFFER)
	return E_TCP_NO_MEM;
	return E_TCP_CONTINUE;
}
XLINK_FUNC void XlinkInitData(void) {
	ByteQueueInit(&sg_queue);
}

XLINK_FUNC int XlinkTcpDataIn(unsigned char * data, x_int16 datalen) {
	//BYTE_Q_t ret =
	unsigned char Buffer[5] = {0x00};
	unsigned int Qsize = 0, ret = 0, bodyLength = 0,i=0;
	static unsigned char m_ProtoBuffer[1492];

	if (isQueeuInit == 0) {
		ByteQueueInit(&sg_queue);
		isQueeuInit = 1;
	}
	if (data != NULL) {
		ret = ByteQueuePushArray(&sg_queue, data, datalen);
		if (ret == E_B_NOT_FREE_BUFFER)
		return E_TCP_NO_MEM;
	}

	Qsize = ByteQueueSize(&sg_queue);
	if (Qsize < 5) {
		return E_TCP_CONTINUE;
	}

	ret = ByteQueuePeerArray(&sg_queue, Buffer, 5);

	//outC();
	if (ret == 5) {
		ret = IsHeadOk(Buffer[0]);
		if (ret == 1) {
			bodyLength = UTIL_INT32_SET(Buffer[1], Buffer[2], Buffer[3], Buffer[4]);
			if(bodyLength > (__XLINK_BUFFER_PIPE__ + 10)) {	//加上标志数据
				xsdk_closeTCP(1);
				XlinkInitData();
				return E_TCP_HEAD_ERROR;
			}
			if (Qsize >= (bodyLength + 5)) {
				ret = ByteQueuePeerArray(&sg_queue, m_ProtoBuffer, (bodyLength + 5));
				if (ret == (bodyLength + 5)) {
					ByteQueuePopArray(&sg_queue, ret);
//                    xlink_printf("start process recv data: ");
//                    for(i=0;i < ret;i++)
//                    	xlink_printf("%02X ",m_ProtoBuffer[i]);
//                    xlink_printf("\r\n");
					xlink_process_tcp_data(m_ProtoBuffer, ret, bodyLength);
//                    outC();
					return E_TCP_SUCCESS;
				} else {
					return E_TCP_READ_MEM_ERROR;
				}
			} else {
				return E_TCP_CONTINUE;
			}

		} else { // header error
			ByteQueueInit(&sg_queue);
			return E_TCP_HEAD_ERROR;
		}
	}

	return E_TCP_SUCCESS;
}
XLINK_FUNC int XlinkProcessTCPData(void) {
	return XlinkTcpDataIn(NULL, 0);
}
#endif

