/*
 * xlink_data.h
 *
 *  Created on: 2014骞�2鏈�2鏃�
 *      Author: john
 */

#ifndef XLINK_DATA_H_
#define XLINK_DATA_H_
#ifdef  __cplusplus
extern "C" {
#endif
#include "Xlink_Head_Adaptation.h"

#define XLINK_DATAPOINT_MAX_BYTES  1000


#define XLINK_CONFIG_INDEX_CONFIG       (0)
#define XLINK_CONFIG_INDEX_NAME         (1)
#define XLINK_CONFIG_INDEX_AUTH         (2)
//#define XLINK_CONFIG_INDEX_PWD          (3)
//#define XLINK_CONFIG_INDEX_TEST         (4)
#define XLINK_CONFIG_INDEX_SHARE_KEY    (6)
#define XLINK_CONFIG_INDEX_SUBKEY_KEY   (7)
#define XLINK_CONFIG_INDEX_DOMAIN_IP    (8)

/*network*/
typedef struct XLINK_NET_INFO_T {
	/*Domain name or IP */
	//char* hostNameOrIp;
	x_int32 ServerPort;
	unsigned char tcpPingCount;
	int tcp_fd;
	int udp_fd;
	/*Last connection to the server time*/
	xsdk_time_t tcp_last_connect_server;
	union {
		unsigned char all;
		struct {
			unsigned char isFirstConnectSuccess :1;
			unsigned char isLogined :1;
			unsigned char is80Port :1;
			unsigned char is80ConnectSuccess :1;
			unsigned char isConnectWifi :1;
			unsigned char isScanEnable :1;
			unsigned char Res :2;
		} Bit;
	} tcpFlag;
} XLINK_NET_INFO;

/*client*/
typedef struct XLINK_CLIENT_T {
	xlink_addr Address;
	xsdk_time_t LastReceiveDataTime;
	x_uint8 checkValue;
	x_uint8 ArrayIndex;
	short keepAliveTime;
	xsdk_time_ms lastsendtime;
	xsdk_time_ms lastRecvPipeTime;
	unsigned short messageid[2]; //优先级的消息ID号
	unsigned char isActive;
	unsigned short sendcount;
	unsigned short sendmessageid;
} XLINK_CLIENT;

/*device*/
typedef struct XLINK_DEVICE_INFO_T {
	char *type;
	char product_id[XLINK_SIZE_33];
	char product_key[XLINK_SIZE_33];
	xsdk_time_t lastGetServerTimeTime;
	xsdk_time_t getServerTimeDelay;
	union {
		unsigned char all;
		struct {
			unsigned char isGetServerTimeTask :1;
			unsigned char res :7;
		} Bit;
	} flags;
} XLINK_DEVICE_INFO;

typedef struct XLINK_INFO_T {
	XLINK_NET_INFO net_info; /*Network Information*/
	XLINK_DEVICE_INFO dev_info;/*Device Information*/
	xlink_SdkConfig config;

	union {
		unsigned char byte;
		struct {
			unsigned char clientCount :1;
			unsigned char bit1 :1;
			unsigned char bit2 :1;
			unsigned char res :5;
		} bit;
	} flag;
	xsdk_time_t g_XlinkSdkTime;

	int domain_connect_times;
} G_XLINK_INFO;

//share version 3
//#define XLINK_AGREEMENT_VERSION2 2
#define XLINK_AGREEMENT_VERSION3 3
#define XLINK_AGREEMENT_VERSION4 4

#define UTIL_INT32_GET_BIT3(X)  (unsigned char)(((X) & 0xFF000000) >> 24)
#define UTIL_INT32_GET_BIT2(X)  (unsigned char)(((X) & 0x00FF0000) >> 16)
#define UTIL_INT32_GET_BIT1(X)  (unsigned char)(((X) & 0x0000FF00) >> 8)
#define UTIL_INT32_GET_BIT0(X)   (unsigned char)((X) & 0x000000FF)

//#define UTIL_INT16_GET_BITH(X)  (unsigned char)(((X)&0xff00)>>8)
//#define UTIL_INT16_GET_BITL(X)   (unsigned char)((X)&0x00ff)
unsigned char inline UTIL_INT16_GET_BITH(unsigned int X);

unsigned char inline UTIL_INT16_GET_BITL(unsigned int X);

#define UTIL_INT32_SET(H,N,K,L) (((H)& 0x000000FF) << 24)+(((N) & 0x000000FF) << 16)+(((K) & 0x000000FF) << 8)+((L) & 0x000000FF)
#define UTIL_INT16_SET(H,L) (((H)<<8)+(L))

#define LE_BE_32(x)  ((((unsigned int) (x) & 0xff000000) >> 24) | ((((unsigned int) (x) & 0x00ff0000) >> 8)) \
			| (((unsigned int) (x) & 0x0000ff00) << 8) | (((unsigned int) (x) & 0x000000ff) << 24))


extern G_XLINK_INFO g_xlink_info;
extern XLINK_USER_CONFIG *g_xlink_user_config;

extern XLINK_FUNC void XlinkSdkDataInit(const char* product_id, const char *product_key);

extern XLINK_FUNC void XlinkSdkAppSetDeviceName(char* name, unsigned short nameLength);

#ifdef  __cplusplus
}
#endif
#endif /* XLINK_DATA_H_ */
