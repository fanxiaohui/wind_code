/*
 * xlinkv4Udp.c
 *
 *  Created on: 2016-11-24
 *      Author: john
 */

#include "xlinkv4Udp.h"
#include "sdkDef.h"
#include "tea.h"
#include "xlink_message.h"
#include "Xlink_mem.h"
#include "xsdk_config.h"
#include "xlink_md5.h"
#include "xlink_net.h"
#include "xlink_system.h"
#include "xlink_client.h"


extern int g_udp_listen_port;
extern XLINK_USER_CONFIG *g_xlink_user_config;

static unsigned char udpPipeIndex = 0;
static unsigned short g_udp_msgid = 1;
static SendDataList gs_SendUdpListHeader = { //
		.Next = NULL, //
				.users = 0, //
		};
static unsigned short getUdpMsgID(){
	g_udp_msgid++;
	if(g_udp_msgid > 10000){
		g_udp_msgid=0;
	}
	return g_udp_msgid;
}
XLINK_FUNC void XlinkEnableSubAndScan(void) {
	g_xlink_info.net_info.tcpFlag.Bit.isScanEnable = 1;
}
XLINK_FUNC void XlinkDisableSubAndScan(void) {
	g_xlink_info.net_info.tcpFlag.Bit.isScanEnable = 0;
}
XLINK_FUNC unsigned char XlinkPorcess_UDP_GetScan(void) {
	return g_xlink_info.net_info.tcpFlag.Bit.isScanEnable;
}

XLINK_FUNC static void NSProcessRemoveMessageid(int index, short msgid);

XLINK_FUNC static void XlinkProcessUdpScan_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);
XLINK_FUNC static void XlinkProcessUdpHandshake_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);
XLINK_FUNC static void XlinkProcessUdpSETACK_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);
XLINK_FUNC static void XlinkProcessUdpGetSubKey_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);
XLINK_FUNC static void XlinkProcessUdpPing_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);
XLINK_FUNC static void XlinkProcessUdpProbe_Datapoint_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);
XLINK_FUNC static void XlinkProcessUdpPipe_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);
XLINK_FUNC static void XlinkProcessUdpByby_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);
XLINK_FUNC static void XlinkProcessUdpSet_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);

XLINK_FUNC static void XlinkProcessUdpPipeResponse_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);
XLINK_FUNC static void XlinkProcessUdpSyncResponse_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems);

static struct {
	const char *doc;
	const unsigned char cmd;
	void (*Process)(AgreementUdpHeader_t header, unsigned int Bodylength, xlink_addr *address, xsdk_time_ms c_time);
} gs_UdpCmdFunction[] = { //
		{ "udp scan message", E_TYPE_SCAN, XlinkProcessUdpScan_V4 }, //
				{ "udp handshake message", E_TYPE_HANDSHAKE, XlinkProcessUdpHandshake_V4 }, //
				{ "udp set ack message", E_TYPE_SETACK, XlinkProcessUdpSETACK_V4 }, //
				{ "udp get subkey message", E_TYPE_SUBKEY, XlinkProcessUdpGetSubKey_V4 }, //
				{ "udp get ping message", E_TYPE_PING, XlinkProcessUdpPing_V4 }, //
				{ "udp probe message", E_TYPE_PROBE, XlinkProcessUdpProbe_Datapoint_V4 }, //
				{ "udp pipe message", E_TYPE_PIPE, XlinkProcessUdpPipe_V4 }, //
				{ "udp byebye message", E_TYPE_BYEBYE, XlinkProcessUdpByby_V4 }, //
				{ "udp set message", E_TYPE_SET, XlinkProcessUdpSet_V4 }, //

				{ NULL, 0, NULL }, //
		};

static struct {
	const char *doc;
	const unsigned char cmd;
	void (*Process)(AgreementUdpHeader_t header, unsigned int Bodylength, xlink_addr *address, xsdk_time_ms c_time);
} gs_UdpCmdFunctionResponse[] = { //
		{ "udp response pipe message", E_TYPE_PIPE, XlinkProcessUdpPipeResponse_V4 }, //
				{ "udp response sync message", E_TYPE_SYNC, XlinkProcessUdpSyncResponse_V4 }, //
				{ NULL, 0, NULL }, //
		};

XLINK_FUNC static void XlinkProcess_DebugInfo(xlink_addr *addr) {
	unsigned char sendbuf[100] = { 0x00 };
	int length = 0;
	xlink_memset(sendbuf, 0, 100);
	length = xlink_sprintf((char*) sendbuf, "xver:%d %s %s bufsize %d net=%d", __XLINK_VERSION__, __DATE__,
			__TIME__, __XLINK_BUFFER_PIPE__, g_xlink_info.net_info.tcpFlag.Bit.isLogined);
	XlinkClientSendDataToAddr(sendbuf, length, addr);
}

XLINK_FUNC static int Ddcode(AgreementUdpHeader_t header) {
	int key = 0;
	short length = 0;
	signed int sum = 0;
	int i = 0;
	signed char checkdata = 0;
	unsigned char *data = (unsigned char *) header;
	xsdk_read_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char *) &key, 4);
	length = (header->bodylength);
	TeaDecrypt((data + 6), length, key);
	for (i = 0; i < (length - 2); i++) {
		sum += (signed char) header->data[i];
	}
	checkdata = (signed char) (sum & 0x000000ff);
	if (checkdata == header->flag2) {
		return 0;
	}
	return -1;
}

XLINK_FUNC static void Encode(AgreementUdpHeader_t header) {
	int key = 0;
	short length = 0;
	header->encode = 1;
	signed int sum = 0;
	int i = 0;
	unsigned char *data = (unsigned char *) header;

	xsdk_read_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char *) &key, sizeof(int));
	length = NShtonlInt16(header->bodylength);
	for (i = 0; i < (length - 2); i++) {
		sum += (signed char) header->data[i];
	}
	header->flag2 = (signed char) (sum & 0x000000ff);
	TeaEncrypt((data + 6), length, key);
}

XLINK_FUNC void XlinkV4UdpInit(void) {
	SendData_Item sendItem = NULL;
	sendItem = gs_SendUdpListHeader.Next;
	while (sendItem != NULL) {
		gs_SendUdpListHeader.Next = sendItem->Next;
		XlinkMemFree(_FILE_AND_LINE_, sendItem);
		sendItem = gs_SendUdpListHeader.Next;
	}
	memset(&gs_SendUdpListHeader, 0, sizeof(gs_SendUdpListHeader));
}

XLINK_FUNC void XlinkV4UdpUnInit(void) {

	SendData_Item sendlistItem = NULL;
	sendlistItem = gs_SendUdpListHeader.Next;
	while (sendlistItem != NULL) {
		gs_SendUdpListHeader.Next = sendlistItem->Next;
		XlinkMemFree(_FILE_AND_LINE_, sendlistItem);
		sendlistItem = gs_SendUdpListHeader.Next;
	}
}

XLINK_FUNC void XlinkXGMessage(unsigned char *data, unsigned int datalength, xlink_addr *addr) {
	if (g_xlink_user_config == NULL)
		return;
	if (g_xlink_user_config->OnXGMessage == NULL)
		return;
	g_xlink_user_config->OnXGMessage(data, datalength, addr);
}

XLINK_FUNC void XlinkProcessUdpDataV4(unsigned char * Buffer, unsigned int BufferLen, xlink_addr *addr) {

	AgreementUdpHeader_t header = NULL;
	xsdk_time_ms c_timems = 0;
	int iter = 0;
	if (BufferLen <= 4) {
		if (BufferLen == 3) {
			if (Buffer[0] == 'X' && Buffer[1] == 'D' && Buffer[2] == 'G') {
				XlinkProcess_DebugInfo(addr);
			}
		}
		//return;
	}

	if (Buffer[0] == 0x18) {
		XlinkXGMessage(Buffer, BufferLen, addr);
		return;
	}

	if (BufferLen < 8) {
		return;
	}

	header = (AgreementUdpHeader_t) Buffer;

	if (header->version != XLINK_UDP_VERSION) {
#if __XDEBUG__
		XSDK_DEBUG_ERROR("Agreement version error %d", header->version);
#endif
		return;
	}

	header->bodylength = NShtonlInt16(header->bodylength);
	if (header->bodylength > (BufferLen - 6)) {
		//printf("bodylength %d bufferlen=%d", header->bodylength,BufferLen);
		return;
	}
	if (header->encode) {
		iter = Ddcode(header);
		if (iter != 0) {
#if __XDEBUG__
			XSDK_DEBUG_ERROR("Ddcode data failed");
#endif
			return;
		}
	}

	header->messageid = NShtonlInt16(header->messageid);

//	printf("Header msgid:%d", header->messageid);
//	printf("Header cmd:%02X", header->cmd);
//	printf("Header bodylen:%d", header->bodylength);

	c_timems = g_xlink_user_config->OnGetSystemTimeMs();

	if (header->response) {

		for (iter = 0;; iter++) {
			if (gs_UdpCmdFunctionResponse[iter].doc == NULL) {
				break;
			}
			if (gs_UdpCmdFunctionResponse[iter].cmd == header->cmd) {
#if __XDEBUG__
				XSDK_DEBUG_DEBUG("recv udp : %s", gs_UdpCmdFunctionResponse[iter].doc);
#endif
				gs_UdpCmdFunctionResponse[iter].Process(header, (header->bodylength - 2), addr, c_timems);
			}
		}
		return;
	}

	for (iter = 0;; iter++) {
		if (gs_UdpCmdFunction[iter].doc == NULL) {
			break;
		}
		if (gs_UdpCmdFunction[iter].cmd == header->cmd) {
#if __XDEBUG__
			XSDK_DEBUG_DEBUG("recv udp : %s", gs_UdpCmdFunction[iter].doc);
#endif
			gs_UdpCmdFunction[iter].Process(header, (header->bodylength - 2), addr, c_timems);
		}
	}

}

XLINK_FUNC static void XlinkProcessUdpScan_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {

	unsigned char flag = 0;
	char DeviceName[XLINK_SIZE_17] = { 0x00 };
	char resp_temp[150] = { 0x00 };
	AgreementUdpHeader_t resp_buf = (AgreementUdpHeader_t) resp_temp;
	unsigned char DeviceNameLength = 0;
	int ack = 0;
	int pos = 0;

	unsigned char *Buffer = header->data;

#if __XDEBUG__
	XSDK_DEBUG_DEBUG(" udp SCAN message");
#endif

	flag = Buffer[3];
	if (flag & 0x01) {
		if ((Datalength - 6) < g_xlink_user_config->maclen) {
#if __XDEBUG__
			XSDK_DEBUG_DEBUG("udp SCAN by mac data length error");

#endif
			return;
		}
		if (xlink_strncmp((char* ) (Buffer + 6), (char* ) g_xlink_user_config->mac, g_xlink_user_config->maclen) != 0) {
#if __XDEBUG__
			XSDK_DEBUG_DEBUG(" udp SCAN by mac error");

#endif
			return;
		}
	} else {

		if (g_xlink_info.net_info.tcpFlag.Bit.isScanEnable) { //scan on-off

			if (xlink_strncmp((char* ) (Buffer + 4), g_xlink_info.dev_info.product_id, 32) != 0) {
#if __XDEBUG__
				XSDK_DEBUG_DEBUG("udp SCAN by product error");
#endif
				return;
			}
		} else {
#if __XDEBUG__
			XSDK_DEBUG_DEBUG(" udp SCAN closed");
#endif
			return;
		}
	}

	memset(resp_buf, 0, 150);
	resp_buf->response = 1;
	resp_buf->cmd = E_TYPE_SCAN;
	resp_buf->version = XLINK_UDP_VERSION;
	resp_buf->messageid = NShtonlInt16(header->messageid);

	xlink_memset(DeviceName, 0, XLINK_SIZE_17);
	xsdk_read_config(XLINK_CONFIG_INDEX_NAME, DeviceName, XLINK_SIZE_17 - 1);
	DeviceNameLength = strlen(DeviceName);
	//version
	resp_buf->data[0] = XLINK_UDP_VERSION;
	pos = 1;
	//MAC
	//mac len
	if (g_xlink_user_config->maclen > XLINK_MAC_LEN_MAX) {
		g_xlink_user_config->maclen = XLINK_MAC_LEN_MAX;
	}
	resp_buf->data[pos++] = 0;
	resp_buf->data[pos++] = g_xlink_user_config->maclen;
	//mac
	xlink_memcpy(&resp_buf->data[pos], g_xlink_user_config->mac, g_xlink_user_config->maclen);
	pos += g_xlink_user_config->maclen;

	//XLINK_DEVINFO_PRO_ID_SIZE-1
	resp_buf->data[pos++] = 0;
	resp_buf->data[pos++] = 32;
	xlink_memcpy(&resp_buf->data[pos], g_xlink_info.dev_info.product_id, 32);
	pos += 32;

	// mcu hardware version
	resp_buf->data[pos++] = g_xlink_user_config->wifi_type;

	//mcu software version
	resp_buf->data[pos++] = UTIL_INT16_GET_BITH(g_xlink_user_config->mcuHardwareSoftVersion);
	resp_buf->data[pos++] = UTIL_INT16_GET_BITL(g_xlink_user_config->mcuHardwareSoftVersion);

	//udp port
	resp_buf->data[pos++] = UTIL_INT16_GET_BITH(g_udp_listen_port);
	resp_buf->data[pos++] = UTIL_INT16_GET_BITL(g_udp_listen_port);

	resp_buf->data[pos++] = (g_xlink_user_config->devicetype >> 8) & 0xff;
	resp_buf->data[pos++] = (g_xlink_user_config->devicetype >> 0) & 0xff;

	resp_buf->data[pos++] = flag;
	resp_buf->data[pos++] = 1;
	//master is been set
	xsdk_read_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char *) &ack, 4);
	if (((ack > 0) && (ack <= 999999999)) && ((flag & 0x01) == 0)) {
		resp_buf->data[pos - 1] |= 0x04;	//bit2:set 1 be set
	}

	if (DeviceNameLength >= XLINK_SIZE_17) {
		DeviceNameLength = XLINK_SIZE_17 - 1;
	}
	resp_buf->data[pos++] = 0;
	resp_buf->data[pos++] = DeviceNameLength;
	xlink_memcpy((char* ) &resp_buf->data[pos], DeviceName, DeviceNameLength);
	pos += DeviceNameLength;

	//access key
	if (((ack > 0) && (ack <= 999999999)) && ((flag & 0x01) == 0)) {
		resp_buf->data[pos++] = UTIL_INT32_GET_BIT3(ack);
		resp_buf->data[pos++] = UTIL_INT32_GET_BIT2(ack);
		resp_buf->data[pos++] = UTIL_INT32_GET_BIT1(ack);
		resp_buf->data[pos++] = UTIL_INT32_GET_BIT0(ack);
	}

	resp_buf->bodylength = NShtonlInt16(pos + 2);

	XlinkClientSendDataToAddr((unsigned char *) resp_buf, (pos + 8), addrBro);
	return;
}

XLINK_FUNC static void handshakeErrorResV4(AgreementUdpHeader_t header, xlink_addr * addrBro, unsigned char opt) {

	unsigned char temp[150] = { 0x00 };
	AgreementUdpHeader_t SendBuffer = (AgreementUdpHeader_t) temp;

	SendBuffer->cmd = header->cmd;
	SendBuffer->response = 1;
	SendBuffer->messageid = NShtonlInt16(header->messageid);
	SendBuffer->bodylength = NShtonlInt16(6 + g_xlink_user_config->maclen + 2);
	SendBuffer->version = XLINK_UDP_VERSION;
	SendBuffer->data[0] = opt;
	SendBuffer->data[1] = XLINK_UDP_VERSION;
	//message id
	SendBuffer->data[2] = 0;
	SendBuffer->data[3] = 0;

	SendBuffer->data[4] = 0;
	SendBuffer->data[5] = g_xlink_user_config->maclen;

	xlink_memcpy(&SendBuffer->data[6], g_xlink_user_config->mac, g_xlink_user_config->maclen);
	XlinkClientSendDataToAddr((unsigned char *) SendBuffer, 14 + g_xlink_user_config->maclen, addrBro);
}

XLINK_FUNC static void XlinkProcessUdpHandshake_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {

	unsigned char AckMd5Buffer[16] = { 0x00 };
	unsigned char resp_temp[128] = { 0x00 };
	unsigned char ackbuf[4];
	AgreementUdpHeader_t resp_buf = (AgreementUdpHeader_t) resp_temp;
	unsigned short AppKeepAliveTime = 0;

	unsigned char *Buffer = header->data;
	int accesskey = 0;
	int pos = 0;

	x_uint8 ssidIndex = 0;
	x_uint8 ssidValue = 0; //ssidvalue

	if (header->bodylength < 24) { //Need return error code
		return;
	}

	//check accesskey
	xsdk_read_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char*) (&accesskey), sizeof(int));
	xlink_memset(AckMd5Buffer, 0, 16);

	ackbuf[0] = UTIL_INT32_GET_BIT0(accesskey);
	ackbuf[1] = UTIL_INT32_GET_BIT1(accesskey);
	ackbuf[2] = UTIL_INT32_GET_BIT2(accesskey);
	ackbuf[3] = UTIL_INT32_GET_BIT3(accesskey);

	xlinkGetMd5(AckMd5Buffer, (unsigned char *) (ackbuf), 4);
	if (xlink_strncmp((char* ) (header->data + 3), (char* )AckMd5Buffer, 16) != 0) {
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("handshake ack error");
#endif
		handshakeErrorResV4(header, addrBro, 2);
		return;
	}

	AppKeepAliveTime = UTIL_INT16_SET(Buffer[22], Buffer[23]);

#if __XDEBUG__
	XSDK_DEBUG_DEBUG("  handshake version : %d  keepalive=%d", header->version, AppKeepAliveTime);
#endif
	XlinkClientCheckHeartbeat(g_xlink_info.g_XlinkSdkTime);
	XlinkClientAddclient(AppKeepAliveTime, addrBro, &ssidIndex, &ssidValue);
	if (ssidValue == 0) {
#if __XDEBUG__
		XSDK_DEBUG_DEBUG(" new client connected and get sessid =0 : ");
#endif
		handshakeErrorResV4(header, addrBro, 3);
		return;
	}

	resp_buf->cmd = header->cmd;
	resp_buf->response = 1;
	resp_buf->messageid = NShtonlInt16(header->messageid);
	resp_buf->version = XLINK_UDP_VERSION;
	resp_buf->data[0] = 0;
	resp_buf->data[1] = XLINK_UDP_VERSION;
	pos = 2;
	//message id
	resp_buf->data[pos++] = Buffer[1];
	resp_buf->data[pos++] = Buffer[2];
	//mac length
	resp_buf->data[pos++] = 0;
	resp_buf->data[pos++] = g_xlink_user_config->maclen;
	xlink_memcpy(&resp_buf->data[pos], g_xlink_user_config->mac, g_xlink_user_config->maclen);
	pos += g_xlink_user_config->maclen;
	//device id
	resp_buf->data[pos++] = g_xlink_info.config.Deviceid[0];
	resp_buf->data[pos++] = g_xlink_info.config.Deviceid[1];
	resp_buf->data[pos++] = g_xlink_info.config.Deviceid[2];
	resp_buf->data[pos++] = g_xlink_info.config.Deviceid[3];
	//wifi version
	resp_buf->data[pos++] = UTIL_INT16_GET_BITH(g_xlink_user_config->wifisoftVersion);
	resp_buf->data[pos++] = UTIL_INT16_GET_BITL(g_xlink_user_config->wifisoftVersion);
	//session
	resp_buf->data[pos++] = ssidValue;
	resp_buf->data[pos++] = ssidIndex;
	//res
	resp_buf->data[pos++] = 0;
	resp_buf->bodylength = NShtonlInt16(pos + 2);

	Encode(resp_buf);

	XlinkClientSendDataToAddr((unsigned char *) resp_buf, pos + 8, addrBro);
	return;

}

XLINK_FUNC static void XlinkProcessUdpSETACK_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {

	int accesskey = 0, subkey = 0;
	unsigned char resp_temp[32] = { 0x00 };
	AgreementUdpHeader_t resp_buf = (AgreementUdpHeader_t) resp_temp;
	unsigned char *Buffer = header->data;

	if (Datalength < 9) {
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("packet error,data length too short.");
#endif
		return;
	}

	if (g_xlink_info.net_info.tcpFlag.Bit.isScanEnable == 0) {
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("Set accesskey failed Scan disable.");
#endif
		return;
	}

	xsdk_read_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char*) (&accesskey), 4);

	if ((accesskey > 0) && (accesskey < 999999999)) {
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("alredy set accesskey =%d.", accesskey);
#endif
		resp_buf->cmd = header->cmd;
		resp_buf->response = 1;
		resp_buf->messageid = NShtonlInt16(header->messageid);
		resp_buf->version = XLINK_UDP_VERSION;
		resp_buf->bodylength = NShtonlInt16(3 + 2);

		resp_buf->data[0] = Buffer[0];
		resp_buf->data[1] = Buffer[1];
		resp_buf->data[2] = 1;

		XlinkClientSendDataToAddr((unsigned char *) resp_buf, 8 + 3, addrBro);
		return;
	}

	accesskey = UTIL_INT32_SET(Buffer[5], Buffer[6], Buffer[7], Buffer[8]);
	if ((accesskey <= 0) || (accesskey > 999999999)) {
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("Set accesskey failed data is not legal");
#endif
//		resp_buf->cmd = header->cmd;
//		resp_buf->response = 1;
//		resp_buf->messageid = NShtonlInt16(header->messageid);
//		resp_buf->version = XLINK_UDP_VERSION;
//		resp_buf->bodylength = NShtonlInt16(3 + 2);
//
//		resp_buf->data[0] = Buffer[0];
//		resp_buf->data[1] = Buffer[1];
//		resp_buf->data[2] = 2;
//		XlinkClientSendDataToAddr((unsigned char *) resp_buf, 8 + 3, addrBro);
		return;
	}
	subkey = XlinkBuildSubKey();
	xsdk_write_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char*) (&accesskey), 4);
	xsdk_write_config(XLINK_CONFIG_INDEX_SUBKEY_KEY, (char*) (&subkey), 4);
	xsdk_config_save();

#if __XDEBUG__
	XSDK_DEBUG_DEBUG("set accesskey success is %d.", accesskey);
#endif

	resp_buf->cmd = header->cmd;
	resp_buf->response = 1;
	resp_buf->messageid = NShtonlInt16(header->messageid);
	resp_buf->version = XLINK_UDP_VERSION;
	resp_buf->bodylength = NShtonlInt16(3 + 2);

	resp_buf->data[0] = Buffer[0];
	resp_buf->data[1] = Buffer[1];
	resp_buf->data[2] = 0;

	XlinkClientSendDataToAddr((unsigned char *) resp_buf, 8 + 3, addrBro);
	return;

}

XLINK_FUNC static void SubKeyErrorResV4(AgreementUdpHeader_t header, xlink_addr * addrBro, unsigned char opt) {

	unsigned char resp_temp[32] = { 0x00 };
	AgreementUdpHeader_t resp_buf = (AgreementUdpHeader_t) resp_temp;

	resp_buf->cmd = header->cmd;
	resp_buf->response = 1;
	resp_buf->messageid = NShtonlInt16(header->messageid);
	resp_buf->version = XLINK_UDP_VERSION;
	resp_buf->bodylength = NShtonlInt16(5);
	resp_buf->data[0] = 0;
	resp_buf->data[1] = 0;
	resp_buf->data[2] = opt;

	XlinkClientSendDataToAddr((unsigned char *) resp_buf, 11, addrBro);
}

XLINK_FUNC static void XlinkProcessUdpGetSubKey_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {

	unsigned char AppVersion = 0;
	unsigned char AckMd5Buffer[16] = { 0x00 };
	unsigned char resp_temp[64] = { 0x00 };
	unsigned char ackbuf[4];

	AgreementUdpHeader_t resp_buf = (AgreementUdpHeader_t) resp_temp;
	unsigned char *Buffer = header->data;

	int accesskey = 0, subkey = 0;

	if (Datalength < 20) { //Need return error code
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("Bodylength to short");
#endif
		return;
	}
	memset(resp_temp, 0, 64);
	AppVersion = Buffer[0];

	if (AppVersion == XLINK_AGREEMENT_VERSION4) {
		//check accesskey
		xsdk_read_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char*) (&accesskey), sizeof(int));
		xlink_memset(AckMd5Buffer, 0, 16);

		ackbuf[0] = UTIL_INT32_GET_BIT0(accesskey);
		ackbuf[1] = UTIL_INT32_GET_BIT1(accesskey);
		ackbuf[2] = UTIL_INT32_GET_BIT2(accesskey);
		ackbuf[3] = UTIL_INT32_GET_BIT3(accesskey);

		xlinkGetMd5(AckMd5Buffer, (unsigned char *) (ackbuf), 4);
		if (xlink_strncmp((char* ) (Buffer + 3), (char* )AckMd5Buffer, 16) != 0) {
			SubKeyErrorResV4(header, addrBro, 1);
#if __XDEBUG__
			XSDK_DEBUG_DEBUG("Check ack error ack=%d", accesskey);
#endif
			return;
		}
	} else {
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("not v4 verson=%d", AppVersion);
#endif
		SubKeyErrorResV4(header, addrBro, 1);
		return;
	}

	resp_buf->cmd = header->cmd;
	resp_buf->response = 1;
	resp_buf->messageid = NShtonlInt16(header->messageid);
	resp_buf->version = XLINK_UDP_VERSION;
	resp_buf->data[0] = Buffer[1];
	resp_buf->data[1] = Buffer[2];

	xsdk_read_config(XLINK_CONFIG_INDEX_SUBKEY_KEY, (char*) (&subkey), sizeof(int));
//	if ((subkey <= 0) || (subkey > 999999999)) {
//		SubKeyErrorResV4(header, addrBro, 2);
//#if __XDEBUG__
//		XSDK_DEBUG_DEBUG("sub key error value=%d", subkey);
//#endif
//		return;
//	} else {
	resp_buf->data[2] = 0;
	resp_buf->data[3] = UTIL_INT32_GET_BIT3(subkey);
	resp_buf->data[4] = UTIL_INT32_GET_BIT2(subkey);
	resp_buf->data[5] = UTIL_INT32_GET_BIT1(subkey);
	resp_buf->data[6] = UTIL_INT32_GET_BIT0(subkey);
//	}

	resp_buf->bodylength = NShtonlInt16(9);

#if __XDEBUG__
	XSDK_DEBUG_DEBUG("get sub key response ok subkey=%d", subkey);
#endif
	XlinkClientSendDataToAddr((unsigned char *) resp_buf, 15, addrBro);
	return;

}

XLINK_FUNC static void XlinkProcessUdpPing_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {

	int pos = 0;

	x_uint8 ssidValue = 0;
	x_uint8 ssidIndex = 0;
	x_bool isLogin = xlink_false;
	unsigned char resp_temp[150] = { 0x00 };
	AgreementUdpHeader_t resp_buf = (AgreementUdpHeader_t) resp_temp;
	unsigned char *Buffer = header->data;

	ssidValue = Buffer[0];
	ssidIndex = Buffer[1];

	isLogin = XlinkClientcheckclientLogin(ssidIndex, ssidValue, addrBro);
	if (isLogin == xlink_false) {
		return;
	}
#if __XDEBUG__
	XSDK_DEBUG_DEBUG(" ping client id=%d", ssidIndex);
#endif
	resp_buf->cmd = header->cmd;
	resp_buf->response = 1;
	resp_buf->messageid = NShtonlInt16(header->messageid);
	resp_buf->version = XLINK_UDP_VERSION;
	pos = 0;
	resp_buf->data[pos++] = 0;
	resp_buf->data[pos++] = g_xlink_user_config->maclen;
	xlink_memcpy(&resp_buf->data[pos], g_xlink_user_config->mac, g_xlink_user_config->maclen);
	pos += g_xlink_user_config->maclen;
	resp_buf->bodylength = NShtonlInt16(pos + 2);

	XlinkClientSendToIndex((unsigned char *) resp_buf, pos + 8, ssidIndex);

	return;

}

XLINK_FUNC static void XlinkProcessUdpProbe_Datapoint_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {

	x_uint8 ssidValue = 0;
	x_uint8 ssidIndex = 0;
	int datapointsize = 0;
	x_bool isLogin = xlink_false;

	char DeviceName[XLINK_SIZE_17] = { 0x00 };
	unsigned char resp_temp[XLINK_DATAPOINT_MAX_BYTES + 24] = { 0x00 };
	AgreementUdpHeader_t resp_buf = (AgreementUdpHeader_t) resp_temp;
	unsigned char *Buffer = header->data;

	int pos = 0;
	int index = 0;

	SyncFlags flags;
	x_uint16 DeviceNameLength = 0;

	ssidValue = Buffer[0];
	ssidIndex = Buffer[1];

	isLogin = XlinkClientcheckclientLogin(ssidIndex, ssidValue, addrBro);
	if (isLogin == xlink_false) {
#if __XDEBUG__
		XSDK_DEBUG_ERROR(" probe client ssid error");

#endif

		resp_buf->cmd = header->cmd;
		resp_buf->response = 1;
		resp_buf->messageid = NShtonlInt16(header->messageid);
		resp_buf->version = XLINK_UDP_VERSION;
		resp_buf->bodylength = NShtonlInt16(3);
		resp_buf->data[0] = 2;
		XlinkClientSendToIndex((unsigned char *) resp_buf, 9, ssidIndex);
		return;
	}

	resp_buf->cmd = E_TYPE_SYNC;
	resp_buf->response = 1;
	resp_buf->messageid = NShtonlInt16(header->messageid);
	resp_buf->version = XLINK_UDP_VERSION;

	xsdk_read_config(XLINK_CONFIG_INDEX_NAME, DeviceName, XLINK_SIZE_17 - 1);
	DeviceNameLength = xlink_strlen(DeviceName);

	pos = 0;
	resp_buf->data[pos++] = 0;
	resp_buf->data[pos++] = g_xlink_user_config->maclen;
	xlink_memcpy(&resp_buf->data[pos], g_xlink_user_config->mac, g_xlink_user_config->maclen);
	pos += g_xlink_user_config->maclen;

	flags.all = 0;
	flags.bits.device_name = 1;
	index = pos;
	resp_buf->data[pos++] = flags.all;

	resp_buf->data[pos++] = UTIL_INT16_GET_BITH(DeviceNameLength);
	resp_buf->data[pos++] = UTIL_INT16_GET_BITL(DeviceNameLength);
	xlink_memcpy(&resp_buf->data[pos], DeviceName, DeviceNameLength);
	pos += DeviceNameLength;

	if (g_xlink_user_config->OnGetAllDataPoint != NULL) {
		datapointsize = XLINK_DATAPOINT_MAX_BYTES;
		g_xlink_user_config->OnGetAllDataPoint(&resp_buf->data[pos], &datapointsize);
		if ((datapointsize > 0) && (datapointsize < XLINK_DATAPOINT_MAX_BYTES)) {
			pos += datapointsize;
			resp_buf->data[index] |= 0x06;
		}
	}

	resp_buf->bodylength = NShtonlInt16(pos + 2);

	Encode(resp_buf);

	XlinkClientSendToIndex((unsigned char *) resp_buf, pos + 8, ssidIndex);
}

XLINK_FUNC static void XlinkProcessUdpPipe_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {

	//ssid 0 1
	x_uint8 ssidValue = 0;
	x_uint8 ssidIndex = 0;
	x_bool isLogin = xlink_false;

	unsigned char resp_temp[20] = { 0x00 };
	AgreementUdpHeader_t resp_buf = (AgreementUdpHeader_t) resp_temp;
	unsigned char *Buffer = header->data;

	ssidValue = Buffer[0];
	ssidIndex = Buffer[1];

	isLogin = XlinkClientcheckclientLogin(ssidIndex, ssidValue, addrBro);
	if (isLogin == xlink_false) {
		resp_buf->cmd = header->cmd;
		resp_buf->response = 1;
		resp_buf->messageid = NShtonlInt16(header->messageid);
		resp_buf->version = XLINK_UDP_VERSION;
		resp_buf->bodylength = NShtonlInt16(5);
		resp_buf->data[0] = Buffer[2];
		resp_buf->data[1] = Buffer[3];
		resp_buf->data[2] = 1;
		XlinkClientSendDataToAddr((unsigned char *) resp_buf, 11, addrBro);
		return;
	}

//#if __XDEBUG__
//	XSDK_DEBUG_ERROR(" 接收消息[%lld]  msgid=%d", c_time, header->messageid);
//
//#endif

//	if (g_xlink_client[ssidIndex].lastRecvPipeTime > c_time) {
//		g_xlink_client[ssidIndex].lastRecvPipeTime = c_time;
//	} else if ((c_time - g_xlink_client[ssidIndex].lastRecvPipeTime) < 30) {
//		printf("**************drop package[%lld]  msgid=%d", c_time, header->messageid);
//		return;
//	}
	g_xlink_client[ssidIndex].lastRecvPipeTime = c_timems;

	if (header->isResend) {
		if (g_xlink_client[ssidIndex].messageid[header->Priorty] == header->messageid) {
#if __XDEBUG__
			XSDK_DEBUG_ERROR("recv resend message  msgid=%d", header->messageid);

#endif
			if (header->Ack) {
				resp_buf->cmd = header->cmd;
				resp_buf->response = 1;
				resp_buf->messageid = NShtonlInt16(header->messageid);
				resp_buf->version = XLINK_UDP_VERSION;
				resp_buf->bodylength = NShtonlInt16(5);
				resp_buf->data[0] = Buffer[2];
				resp_buf->data[1] = Buffer[3];
				resp_buf->data[2] = 0;

				XlinkClientSendDataToAddr((unsigned char *) resp_buf, 11, addrBro);
			}
			return;
		} else {
			g_xlink_client[ssidIndex].messageid[header->Priorty] = header->messageid;
		}

	} else {
		g_xlink_client[ssidIndex].messageid[header->Priorty] = header->messageid;
	}
	if (header->Ack) {
		resp_buf->cmd = header->cmd;
		resp_buf->response = 1;
		resp_buf->messageid = NShtonlInt16(header->messageid);
		resp_buf->version = XLINK_UDP_VERSION;
		resp_buf->bodylength = NShtonlInt16(5);
		resp_buf->data[0] = Buffer[2];
		resp_buf->data[1] = Buffer[3];
		resp_buf->data[2] = 0;

		XlinkClientSendToIndex((unsigned char *) resp_buf, 11, ssidIndex);
	}
	udpPipeIndex = ssidIndex;
	if (g_xlink_user_config->OnUdpPipe) {
		g_xlink_user_config->OnUdpPipe(Buffer + 5, Datalength - 5, ssidIndex);
	}
}

XLINK_FUNC static void XlinkProcessUdpByby_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {
	unsigned char *Buffer = header->data;
	unsigned char index = 0, ssidValue = 0;
	x_bool isLogin = 0;
	if (Datalength > 1) {
		index = Buffer[1];
		ssidValue = Buffer[0];
		isLogin = XlinkClientcheckclientLogin(index, ssidValue, addrBro);
		if (isLogin == xlink_true) {
			XlinkClientByby(index, addrBro);
		}
	}
}

XLINK_FUNC static void XlinkProcessUdpSet_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {

	x_uint8 ssidValue = 0, ssidIndex = 0;

	unsigned short BufferIndex = 0;
	char * DeviceName = NULL;
	x_uint8 mUserOpt = 0;
	x_bool isLogin = xlink_false;
	unsigned short DeviceNameLength = 0;
	unsigned char resp_temp[20] = { 0x00 };
	AgreementUdpHeader_t resp_buf = (AgreementUdpHeader_t) resp_temp;
	unsigned char *Buffer = header->data;
	Setflags setFlags;

	ssidValue = Buffer[0];
	ssidIndex = Buffer[1];

	setFlags.all = Buffer[4];

	isLogin = XlinkClientcheckclientLogin(ssidIndex, ssidValue, addrBro);
	if (isLogin == xlink_false) {

		resp_buf->cmd = header->cmd;
		resp_buf->response = 1;
		resp_buf->messageid = NShtonlInt16(header->messageid);
		resp_buf->version = XLINK_UDP_VERSION;
		resp_buf->bodylength = NShtonlInt16(5);

		resp_buf->data[0] = Buffer[2];
		resp_buf->data[1] = Buffer[3];
		resp_buf->data[2] = 1;

		XlinkClientSendDataToAddr((unsigned char *) resp_buf, 11, addrBro);

		return;
	}

	if (header->isResend) {

		if (g_xlink_client[ssidIndex].messageid[header->Priorty] == header->messageid) {
			if (header->Ack) {
				resp_buf->cmd = header->cmd;
				resp_buf->response = 1;
				resp_buf->messageid = NShtonlInt16(header->messageid);
				resp_buf->version = XLINK_UDP_VERSION;
				resp_buf->bodylength = NShtonlInt16(5);
				resp_buf->data[0] = Buffer[2];
				resp_buf->data[1] = Buffer[3];
				resp_buf->data[2] = mUserOpt;
				XlinkClientSendDataToAddr((unsigned char *) resp_buf, 11, addrBro);
			}
			return;
		} else {
			g_xlink_client[ssidIndex].messageid[header->Priorty] = header->messageid;
		}

	} else {
		g_xlink_client[ssidIndex].messageid[header->Priorty] = header->messageid;
	}

	BufferIndex = 5;
	if (setFlags.bits.device_name) {
		DeviceNameLength = UTIL_INT16_SET(Buffer[5], Buffer[6]);
		BufferIndex += 2;
		DeviceName = (char*) &Buffer[7];
		if (DeviceNameLength < XLINK_SIZE_17) {
			XlinkSdkAppSetDeviceName(DeviceName, DeviceNameLength);
		}
		BufferIndex += DeviceNameLength;
	}

	if ((setFlags.bits.datapoint_v2) && (g_xlink_user_config->OnSetDataPoint != NULL)) {
		// update datapoint data
		g_xlink_user_config->OnSetDataPoint(Buffer + BufferIndex, Datalength - BufferIndex);
	}

	resp_buf->cmd = header->cmd;
	resp_buf->response = 1;
	resp_buf->messageid = NShtonlInt16(header->messageid);
	resp_buf->version = XLINK_UDP_VERSION;
	resp_buf->bodylength = NShtonlInt16(5);

	resp_buf->data[0] = Buffer[2];
	resp_buf->data[1] = Buffer[3];
	resp_buf->data[2] = mUserOpt;
	XlinkClientSendDataToAddr((unsigned char *) resp_buf, 11, addrBro);

	return;

}

XLINK_FUNC static void XlinkProcessUdpPipeResponse_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {

	unsigned char *Buffer = header->data;
	int ssidValue = Buffer[0];
	int ssidIndex = Buffer[1];
	int isLogin = XlinkClientcheckclientLogin(ssidIndex, ssidValue, addrBro);
	if (isLogin) {
		NSProcessRemoveMessageid(ssidIndex, header->messageid);
#if __XDEBUG__

		XSDK_DEBUG_DEBUG(" response[%d]  msgid:%d time:%lld", ssidIndex, header->messageid, c_timems);

#endif
	}

}

XLINK_FUNC static void XlinkProcessUdpSyncResponse_V4(AgreementUdpHeader_t header, unsigned int Datalength, xlink_addr * addrBro, xsdk_time_ms c_timems) {
	unsigned char *Buffer = header->data;
	int ssidValue = Buffer[0];
	int ssidIndex = Buffer[1];
	int isLogin = XlinkClientcheckclientLogin(ssidIndex, ssidValue, addrBro);
	if (isLogin) {
		NSProcessRemoveMessageid(ssidIndex, header->messageid);
	}
}

XLINK_FUNC static void NSProcessSendBufferTask();

XLINK_FUNC void XlinkResend(void) {

	NSProcessSendBufferTask();
}

XLINK_FUNC static void NSProcessRemoveClient(int index) {
	SendData_Item sendlist = NULL;
	SendData_Item sendlistItem = NULL;

	sendlist = &gs_SendUdpListHeader;
	sendlistItem = sendlist->Next;

	while (sendlistItem != NULL) {

		BIT_SET_0(sendlistItem->users, index);
		if (sendlistItem->users == 0) {
			sendlist->Next = sendlistItem->Next;
			sendlistItem->Next = NULL;
			XlinkMemFree(_FILE_AND_LINE_, sendlistItem);
			sendlistItem = sendlist->Next;
			continue;
		}
		sendlist = sendlistItem;
		sendlistItem = sendlist->Next;
	}
}

XLINK_FUNC static void NSProcessRemoveMessageid(int index, short msgid) {
	SendData_Item sendlist = NULL;
	SendData_Item sendlistItem = NULL;

	sendlist = &gs_SendUdpListHeader;
	sendlistItem = sendlist->Next;

	while (sendlistItem != NULL) {
		AgreementUdpHeader_t temp = (AgreementUdpHeader_t) sendlistItem->data;
		if (NShtonlInt16(temp->messageid) != msgid) {
			sendlist = sendlistItem;
			sendlistItem = sendlist->Next;
			continue;
		}
		BIT_SET_0(sendlistItem->users, index);
		if (sendlistItem->users == 0) {
			sendlist->Next = sendlistItem->Next;
			XlinkMemFree(_FILE_AND_LINE_, sendlistItem);
		}
		break;
	}
}

XLINK_FUNC static void NSProcessSendClient(XLINK_CLIENT *client, int index, xsdk_time_ms c_timems) {

	int temp = 0;
	SendData_Item sendlistItem = NULL;
	sendlistItem = gs_SendUdpListHeader.Next;

	while (sendlistItem != NULL) {

		if (!(BIT_EQUAL_1(sendlistItem->users, index))) {
			sendlistItem = sendlistItem->Next;
			continue;
		}
		AgreementUdpHeader_t agree = (AgreementUdpHeader_t) sendlistItem->data;
		//说明已经发送过此消息
		if (client->sendmessageid == NShtonlInt16(agree->messageid)) {

			if (client->lastsendtime > c_timems) {
				client->lastsendtime = c_timems;
			}
			temp = c_timems - client->lastsendtime;
			if (temp < 300) {
				break;
			}

			//重发了2次了
			if (client->sendcount > 3) {
				BIT_SET_0(sendlistItem->users, index);
				client->sendcount = 0;
				return;
			}

#if __XDEBUG__
			XSDK_DEBUG_DEBUG(" resend message client id[%d] %d msgid:%d time:%lld", index, client->sendcount, client->sendmessageid, c_timems);
#endif

			agree->isResend = 1;
			XlinkClientSendDataToAddr(sendlistItem->data, (NShtonlInt16(agree->bodylength) + 6), &client->Address);
			agree->isResend = 0;

			client->lastsendtime = c_timems;
			client->sendcount++;

		} else {
			client->sendmessageid = NShtonlInt16(agree->messageid);
#if __XDEBUG__
			XSDK_DEBUG_DEBUG(" send a message for the fist time,client id[%d] msgid:%d  time:%lld", index, client->sendmessageid, c_timems);
#endif
			XlinkClientSendDataToAddr(sendlistItem->data, (NShtonlInt16(agree->bodylength) + 6), &client->Address);
			if (agree->Ack == 0) {
				BIT_SET_0(sendlistItem->users, index);
				client->lastsendtime = c_timems;
				client->sendcount = 0;
				break;
			}

			client->lastsendtime = c_timems;
			client->sendcount = 0;
		}

		break;
	}
}

XLINK_FUNC static void NSProcessClearNotUserData(void) {
	SendData_Item sendlist = NULL;
	SendData_Item sendlistItem = NULL;

	sendlist = &gs_SendUdpListHeader;
	sendlistItem = sendlist->Next;

	while (sendlistItem != NULL) {
		if (sendlistItem->users == 0) {
			sendlist->Next = sendlistItem->Next;
			sendlistItem->Next = NULL;
			XlinkMemFree(_FILE_AND_LINE_, sendlistItem);
			sendlistItem = sendlist->Next;
			continue;
		}
		sendlist = sendlistItem;
		sendlistItem = sendlist->Next;
	}
}

XLINK_FUNC static void NSProcessSendBufferTask() {

	int i = 0;
	xsdk_time_ms c_timems = 0;
	if (gs_SendUdpListHeader.Next == NULL) {
		return;
	}

	for (i = 0; i < XLINK_CLIENT_SIZE; i++) {
		XLINK_CLIENT *client = &g_xlink_client[i];
		if (client->isActive) {
			c_timems = g_xlink_user_config->OnGetSystemTimeMs();
			NSProcessSendClient(client, i, c_timems);
		} else {
			NSProcessRemoveClient(i);
		}
	}
	//删除没有用户的数据
	NSProcessClearNotUserData();

}

XLINK_FUNC int XlinkClientSendUdpDataPointSync_V2(unsigned char* DataPoint, unsigned int DataPointSize, unsigned char flag) {

	SendData_Item sendlistItem = NULL;
	int iteration = 0;
	int pos = 0;
	AgreementUdpHeader_t header = NULL;
	iteration = sizeof(SendDataList) + DataPointSize + 32;
	SendData_Item item = (SendData_Item) XlinkMemMalloc(_FILE_AND_LINE_, iteration);
	if (item == NULL) {
		return -1;
	}

	header = (AgreementUdpHeader_t) item->data;

	item->Next = NULL;
	item->users = 0;

	memset(header, 0, DataPointSize + 32);
	header->Ack = 1;
	header->cmd = E_TYPE_SYNC;
	header->Priorty = 1;
	header->encode = 0;
	header->version = XLINK_UDP_VERSION;
	header->messageid = NShtonlInt16(getUdpMsgID());

	pos = 0;
	header->data[pos++] = 0;
	header->data[pos++] = g_xlink_user_config->maclen;
	xlink_memcpy(&header->data[pos], g_xlink_user_config->mac, g_xlink_user_config->maclen);
	pos += g_xlink_user_config->maclen;

	header->data[pos] = 0x06;
	if (flag == 1) {
		header->data[pos] |= 0x08; //DataPoint
	}

	pos++;

	xlink_memcpy(&header->data[pos], DataPoint, DataPointSize);
	pos += DataPointSize;

	header->bodylength = NShtonlInt16(pos + 2);

	Encode(header);

	sendlistItem = &gs_SendUdpListHeader;

	while (sendlistItem->Next != NULL) {
		sendlistItem = sendlistItem->Next;
	}
	sendlistItem->Next = item;

	for (iteration = 0; iteration < XLINK_CLIENT_SIZE; iteration++) {
		if (g_xlink_client[iteration].isActive == 1 && g_xlink_client[iteration].checkValue) {
			BIT_SET_1(item->users, iteration);
		}
	}
	return 0;
}

XLINK_FUNC x_int32 XlinkSendUdpPipe(const unsigned char *data, const unsigned int datalen, int index, int ack) {

	int sendlen = 0;
	int pos = 0;
	char *temp = NULL;
	SendData_Item sendlistItem = NULL;
	int iteration = 0;
	SendData_Item item = NULL;
	AgreementUdpHeader_t header = NULL;

	if (g_xlink_info.flag.bit.clientCount == 0) {
		return -1;
	}
	if (datalen > __XLINK_BUFFER_PIPE__)
		return -2;
	if (index >= XLINK_CLIENT_SIZE) {
		return -3;
	}
	if (index >= 0) {
		if (g_xlink_client[index].isActive == 1 && g_xlink_client[index].checkValue) {

		} else {
			return -4;
		}
	}

	iteration = sizeof(SendDataList) + datalen + 32;

	item = (SendData_Item) XlinkMemMalloc(_FILE_AND_LINE_, iteration);
	if (item == NULL) {
		return -5;
	}
	item->Next = NULL;
	item->users = 0;
	header = (AgreementUdpHeader_t) item->data;

	header->Ack = ack ? 1 : 0;
	header->cmd = E_TYPE_PIPE;
	header->Priorty = 1;
	header->encode = 0;
	header->version = XLINK_UDP_VERSION;
	header->messageid = NShtonlInt16(getUdpMsgID());

	pos = 0;
	header->data[pos++] = 0;
	header->data[pos++] = g_xlink_user_config->maclen;
	xlink_memcpy(&header->data[pos], g_xlink_user_config->mac, g_xlink_user_config->maclen);
	pos += g_xlink_user_config->maclen;

	//cnt++;
	header->data[pos++] = 0;
	header->data[pos++] = 0;

	if (g_xlink_user_config->pipetype == 1) {
		header->data[pos++] = 1;
	} else {
		header->data[pos++] = 0;
	}
	temp = (char*) &header->data[pos];
	xlink_memcpy((unsigned char* )temp, data, datalen);
	pos += datalen;

	header->bodylength = NShtonlInt16(pos + 2);

	Encode(header);

	if (index == -1) {
		for (iteration = 0; iteration < XLINK_CLIENT_SIZE; iteration++) {
			if (g_xlink_client[iteration].isActive == 1 && g_xlink_client[iteration].checkValue) {
				BIT_SET_1(item->users, iteration);
			}
		}
	} else {
		if (g_xlink_client[index].isActive == 1 && g_xlink_client[index].checkValue) {
			BIT_SET_1(item->users, index);
		}
	}

	sendlistItem = &gs_SendUdpListHeader;

	while (sendlistItem->Next != NULL) {
		sendlistItem = sendlistItem->Next;
	}
	sendlistItem->Next = item;

	return sendlen;
}

