#include "xsdk_config.h"
#include "xlink_data.h"
#include "xlink_md5.h"
#include "Xlink_Head_Adaptation.h"
#include "xlink_net.h"

extern XLINK_FUNC void XlinkSetSubKey(int subkey);
extern XLINK_FUNC int XlinkGetSubKey(void);

static struct {
	xlink_SdkConfig sdk;
	unsigned char name[XLINK_SIZE_17 - 1];
	unsigned char auth[XLINK_SIZE_17 - 1];
	unsigned char isXlinkCheck[XLINK_SIZE_17 - 1];
	int accesskey;
	int subkey;
	unsigned char domain_ip[2 + XLINK_DOMAIN_IP_MAX_LEN + 2];
} xsdk_flash;

typedef volatile union {
	unsigned short all;
	struct {
		unsigned char hig :8;
		unsigned char low :8;
	} byte;
} x_short;

const unsigned char m_key[32] = { 0x23, 0x68, 0x59, 0x13, 0x07, 0x31, 0x22, 0x55, //
		0x29, 0x64, 0x19, 0x17, 0x11, 0x26, 0x53, 0x44, //
		0x85, 0x29, 0x71, 0x43, 0x49, 0x51, 0x06, 0x35, //
		0x28, 0x35, 0x18, 0x42, 0x82, 0x99, 0x18, 0x33 };

XLINK_FUNC int xsdk_config_init(char *proID, char *proKey, unsigned char *mac) {
	int iter = 0;
	char xlinkflag[16];
	char saveBuffer[XLINK_CONFIG_BUFFER_SIZE__];
	unsigned char md5String[100] = { 0x00 };
	int ack = -1;
	x_short temp;

	xlink_memset(&xsdk_flash, 0, sizeof(xsdk_flash));
	if (g_xlink_user_config->OnReadConfig) {
		xlink_memset(saveBuffer, 0, XLINK_CONFIG_BUFFER_SIZE__);
		g_xlink_user_config->OnReadConfig(saveBuffer, XLINK_CONFIG_BUFFER_SIZE__);

		for (iter = 0; iter < XLINK_CONFIG_BUFFER_SIZE__; iter++) {
			saveBuffer[iter] = saveBuffer[iter] ^ m_key[iter % 32];
		}

		xsdk_flash.sdk.flag.All = saveBuffer[0];
		xsdk_flash.sdk.Deviceid[0] = saveBuffer[1];
		xsdk_flash.sdk.Deviceid[1] = saveBuffer[2];
		xsdk_flash.sdk.Deviceid[2] = saveBuffer[3];
		xsdk_flash.sdk.Deviceid[3] = saveBuffer[4];
		temp.byte.hig = saveBuffer[5];
		temp.byte.low = saveBuffer[6];
		xsdk_flash.sdk.CurrentSoftVersion = temp.all;

		xlink_memcpy(xsdk_flash.name, &saveBuffer[7], XLINK_SIZE_17 - 1);
		xlink_memcpy(xsdk_flash.auth, &saveBuffer[23], XLINK_SIZE_17 - 1);
		xlink_memcpy(xsdk_flash.isXlinkCheck, &saveBuffer[55], XLINK_SIZE_17 - 1);

		//access key

		xlink_memcpy(&xsdk_flash.accesskey, &saveBuffer[77], 4);
		xlink_memcpy(&xsdk_flash.subkey, &saveBuffer[77 + 4], 4);
		xlink_memcpy(xsdk_flash.domain_ip, &saveBuffer[85], 2+XLINK_DOMAIN_IP_MAX_LEN + 2);
		//        i=xlink_sprintf((char*) md5String, "*******************accesskey=%d",xsdk_flash.accesskey);
		//        XlinkUartSend(md5String, i);
	}

	xlink_sprintf((char*) md5String, "%s%02X%02X%02X%02X%02X%02X%s", proID, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], proKey);
	xlinkGetMd5((unsigned char*) xlinkflag, (unsigned char*) md5String, xlink_strlen((char* )md5String));

	if (xlink_strncmp(xsdk_flash.isXlinkCheck, xlinkflag, 16) != 0) {
		xlink_memset(&xsdk_flash, 0, sizeof(xsdk_flash));
		ack = -1;
		xlink_memcpy(&saveBuffer[77], &ack, 4);

		//access key
		xsdk_flash.subkey = -1;
		xlink_memcpy(&saveBuffer[77 + 4], &ack, 4);
		xlink_memcpy(xsdk_flash.isXlinkCheck, xlinkflag, 16);
		//access key
		xsdk_flash.accesskey = -1;

		//        i=xlink_sprintf((char*) md5String, "*******************erase config");
		//        XlinkUartSend(md5String, i);
#if __XDEBUG__
		XSDK_DEBUG_DEBUG(" erase config");

#endif
		xsdk_config_save();
	}
	return 0;
}

XLINK_FUNC int xsdk_write_config(unsigned int index, char *p_data, unsigned int p_datalen) {

	int ret = 0;
	switch (index) {
	case XLINK_CONFIG_INDEX_CONFIG:
		if (p_datalen == sizeof(xlink_SdkConfig)) {
			xlink_memcpy(&xsdk_flash.sdk, p_data, p_datalen);
		}
		break;
	case XLINK_CONFIG_INDEX_NAME:
		xlink_memset(xsdk_flash.name, 0, XLINK_SIZE_17-1);
		p_datalen = p_datalen < 16 ? p_datalen : 16;
		xlink_memcpy(xsdk_flash.name, p_data, p_datalen);
		break;
	case XLINK_CONFIG_INDEX_AUTH:
		xlink_memset(xsdk_flash.auth, 0, XLINK_SIZE_17-1);
		p_datalen = p_datalen < 16 ? p_datalen : 16;
		xlink_memcpy(xsdk_flash.auth, p_data, p_datalen);
		break;
	case XLINK_CONFIG_INDEX_SHARE_KEY:
		//access key
		xlink_memcpy(&xsdk_flash.accesskey, p_data, 4);
		break;
	case XLINK_CONFIG_INDEX_SUBKEY_KEY:
		xlink_memcpy(&xsdk_flash.subkey, p_data, 4);
		break;
	case XLINK_CONFIG_INDEX_DOMAIN_IP:
		if (p_datalen <= 2 + XLINK_DOMAIN_IP_MAX_LEN) {
			xlink_memset(xsdk_flash.domain_ip, 0, 2 + XLINK_DOMAIN_IP_MAX_LEN + 2);
			xlink_memcpy(xsdk_flash.domain_ip, p_data, p_datalen);
		}
		break;
	default:

		break;
	}
	return ret;
}

XLINK_FUNC int xsdk_config_save(void) {
	int iter = 0;
	char saveBuffer[XLINK_CONFIG_BUFFER_SIZE__];
	x_short temp;
	int ret = 0;
	if (g_xlink_user_config->OnWriteConfig) {
		xlink_memset(saveBuffer, 0, XLINK_CONFIG_BUFFER_SIZE__);
		saveBuffer[0] = xsdk_flash.sdk.flag.All;
		saveBuffer[1] = xsdk_flash.sdk.Deviceid[0];
		saveBuffer[2] = xsdk_flash.sdk.Deviceid[1];
		saveBuffer[3] = xsdk_flash.sdk.Deviceid[2];
		saveBuffer[4] = xsdk_flash.sdk.Deviceid[3];
		temp.all = xsdk_flash.sdk.CurrentSoftVersion;
		saveBuffer[5] = temp.byte.hig;
		saveBuffer[6] = temp.byte.low;
		xlink_memcpy(&saveBuffer[7], xsdk_flash.name, XLINK_SIZE_17 - 1);
		xlink_memcpy(&saveBuffer[23], xsdk_flash.auth, XLINK_SIZE_17 - 1);
		//xlink_memcpy(&saveBuffer[39], xsdk_flash.pwd, XLINK_SIZE_17 - 1);
		xlink_memcpy(&saveBuffer[55], xsdk_flash.isXlinkCheck, XLINK_SIZE_17 - 1);
		//71

		//access key
		xlink_memcpy(&saveBuffer[77], &xsdk_flash.accesskey, 4);
		xlink_memcpy(&saveBuffer[77 + 4], &xsdk_flash.subkey, 4);

		xlink_memcpy(&saveBuffer[85], xsdk_flash.domain_ip, 2 + XLINK_DOMAIN_IP_MAX_LEN + 2);

		for (iter = 0; iter < XLINK_CONFIG_BUFFER_SIZE__; iter++) {
			saveBuffer[iter] = saveBuffer[iter] ^ m_key[iter % 32];
		}
		ret = g_xlink_user_config->OnWriteConfig(saveBuffer, XLINK_CONFIG_BUFFER_SIZE__);

//        iter=xlink_sprintf((char*) saveBuffer, "*******************save accesskey=%d,flag=%d",xsdk_flash.accesskey,xsdk_flash.sdk.flag.Bit.isActivation);
//        XlinkUartSend(saveBuffer, iter);
	}
	return ret;
}

XLINK_FUNC int xsdk_read_config(unsigned int index, char *p_Buffer, unsigned int p_datalen) {

	int ret = 0;
	switch (index) {
	case XLINK_CONFIG_INDEX_CONFIG:
		if (p_datalen == sizeof(xlink_SdkConfig)) {
			xlink_memcpy(p_Buffer, &xsdk_flash.sdk, sizeof(xlink_SdkConfig));
			ret = sizeof(xlink_SdkConfig);
		}
		break;
	case XLINK_CONFIG_INDEX_NAME:
		p_datalen = p_datalen > 16 ? 16 : p_datalen;
		xlink_memcpy(p_Buffer, xsdk_flash.name, p_datalen);
		ret = p_datalen;
		break;
	case XLINK_CONFIG_INDEX_AUTH:

		p_datalen = p_datalen > 16 ? 16 : p_datalen;
		xlink_memcpy(p_Buffer, xsdk_flash.auth, p_datalen);
		ret = p_datalen;
		break;
	case XLINK_CONFIG_INDEX_SHARE_KEY:
		xlink_memcpy(p_Buffer, (char * )&xsdk_flash.accesskey, sizeof(int));
		break;
	case XLINK_CONFIG_INDEX_SUBKEY_KEY:
		xlink_memcpy(p_Buffer, (char * )&xsdk_flash.subkey, sizeof(int));
		break;
	case XLINK_CONFIG_INDEX_DOMAIN_IP:

		xlink_memcpy(p_Buffer, xsdk_flash.domain_ip, p_datalen);
		break;
	default:

		break;
	}
	return ret;
}

XLINK_FUNC void xsdk_timer_set(xlink_timer *timer, unsigned int outtime) {
	if (timer == NULL) {
		return;
	}
	timer->outtime = outtime;
	timer->time = g_xlink_info.g_XlinkSdkTime;
}

XLINK_FUNC int xsdk_tiemr_timerout(xlink_timer *timer) {
	xsdk_time_t ctime;
	if (timer == NULL) {
		return 0;
	}
	ctime = g_xlink_info.g_XlinkSdkTime;
	if (ctime >= timer->time) {
		if (ctime >= (timer->outtime + timer->time)) {
			return 1;
		} else {
			return 0;
		}
	} else {
		timer->time = ctime;
		return 1;
	}

}
XLINK_FUNC void XlinkReSetSDK(void) {


	xsdk_flash.accesskey = -1;
	xsdk_flash.subkey = -1;
	xsdk_flash.sdk.flag.Bit.isActivation = 0;
	g_xlink_info.config.flag.Bit.isActivation = 0;
	xsdk_config_save();

	xsdk_closeTCP(0);
}

XLINK_FUNC void XlinkSetACK(int accesskey) {
	int subkey = 0;
	subkey = XlinkGetSubKey();
	if ((subkey <= 0) || (subkey > 999999999)) {
		subkey = XlinkBuildSubKey();
		xsdk_write_config(XLINK_CONFIG_INDEX_SUBKEY_KEY, (char *) (&subkey), 4);
	}
	xsdk_write_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char *) (&accesskey), 4);

	xsdk_config_save();
}
XLINK_FUNC int XlinkGetACK(void) {
	int accesskey = -1;
	xsdk_read_config(XLINK_CONFIG_INDEX_SHARE_KEY, (char *) (&accesskey), 4);
	return accesskey;
}
XLINK_FUNC void XlinkSetSubKey(int subkey) {
	xsdk_write_config(XLINK_CONFIG_INDEX_SUBKEY_KEY, (char *) (&subkey), 4);
	xsdk_config_save();
}
XLINK_FUNC int XlinkGetSubKey(void) {
	int subkey = -1;
	xsdk_read_config(XLINK_CONFIG_INDEX_SUBKEY_KEY, (char *) (&subkey), 4);
	return subkey;
}
//G_XLINK_INFO g_xlink_info;
XLINK_FUNC int XlinkBuildSubKey(void) {
	int subkey = -1;
	int i = 0;
	for (i = 0; i < g_xlink_user_config->maclen; i++) {
		subkey += g_xlink_user_config->mac[i];
	}
	subkey += xsdk_flash.isXlinkCheck[0];
	subkey += xsdk_flash.isXlinkCheck[1];
	subkey += xsdk_flash.isXlinkCheck[2];
	subkey += g_xlink_info.g_XlinkSdkTime;
	subkey &= 0x2ffffff;
	subkey ^= 0xCDEF7A;
	if ((subkey <= 0) || (subkey > 999999999))
		subkey = 0xCDEF7A;
	return subkey;
}
//code size:16bytes
XLINK_FUNC void XlinkGetAuthCode(char * code) {
	xsdk_read_config(XLINK_CONFIG_INDEX_AUTH, code, 16);
}

