#ifndef XLINK_SYSTEM_H_
#define XLINK_SYSTEM_H_
#ifdef  __cplusplus
extern "C" {
#endif
#include "Xlink_Head_Adaptation.h"


#if __ALL_DEVICE__
enum TCP_ {
	E_TCP_SUCCESS, E_TCP_CONTINUE, //
	E_TCP_HEAD_ERROR,//
	E_TCP_NO_MEM,
	E_TCP_READ_MEM_ERROR,
};
#endif

extern XLINK_FUNC char *XlinkVersion(void);

extern XLINK_FUNC x_bool XlinkInit(char * product_id, char * product_key, XLINK_USER_CONFIG *config);

extern XLINK_FUNC void XlinkSetWifiStatus(unsigned char status);

extern XLINK_FUNC void XlinkLoop(xsdk_time_t c_time, x_int32 timeout_ms);

extern XLINK_FUNC void XlinkSetDeviceName(char *NameStr);
extern XLINK_FUNC void XlinkSetHost(const char *domain);

extern XLINK_FUNC void XlinkReset(void);

extern XLINK_FUNC void XlinkNetInit(void);

extern XLINK_FUNC x_int32 XlinkSendTcpPipe(unsigned short handle, const unsigned char * data, const unsigned int datalen, x_uint32 to_id);
extern XLINK_FUNC x_int32 XlinkSendTcpBroadcast(unsigned short handle, const unsigned char * data, const unsigned int datalen);

extern XLINK_FUNC x_int32 XlinkSendUdpPipe(const unsigned char *data, const unsigned int datalen, int index, int ack);

extern XLINK_FUNC void XlinkGetServerTime(void);
extern XLINK_FUNC int XlinkGetSystemTime(XLINK_SYS_TIME *pTime);
extern XLINK_FUNC void XlinkInitEvent(XLINK_SELECT *event);

extern XLINK_FUNC int XlinkSystemTcpLoop(void);
extern XLINK_FUNC int XlinkGetMacString(char *RetMacBuffer, const int bufflen);

extern XLINK_FUNC int XlinkGetDeviceID(void);

extern XLINK_FUNC int XlinkUpdateDataPointUdp(unsigned short handle, unsigned char* data, x_uint16 datalen, unsigned char flag);
extern XLINK_FUNC int XlinkUpdateDataPointTcp(unsigned short handle, unsigned char* data, x_uint16 datalen, unsigned char flag);

extern XLINK_FUNC void XlinkSetACK(int accesskey);
extern XLINK_FUNC int XlinkGetACK(void);

extern XLINK_FUNC void xlinkSendUpgrade(unsigned char RetCode, unsigned short Upver, unsigned short currentVer);

extern XLINK_FUNC void XlinkCloseNet(void);
extern XLINK_FUNC void XlinkCloseTcp(void);

extern XLINK_FUNC int XlinkProcessTCPData(void);
extern XLINK_FUNC int XlinkPushData(unsigned char * data, x_int16 datalen);

#define XLINK_USE_DEFAUT_PORT  0
//code size:16bytes
extern XLINK_FUNC void XlinkGetAuthCode(char * code);
extern XLINK_FUNC int XlinkSendUdpData(unsigned char *data, unsigned int datalength, xlink_addr *addr);

extern XLINK_FUNC void XlinkEnableSubAndScan(void);
extern XLINK_FUNC void XlinkDisableSubAndScan(void);
extern XLINK_FUNC void XlinkReSetSDK(void);

#if __ALL_DEVICE__
extern void XLINK_FUNC setServerStatus_(unsigned char stat,unsigned char is80Port);
extern XLINK_FUNC void XlinkInitData(void);
#endif
#ifdef  __cplusplus
}
#endif
#endif /* XLINK_SYSTEM_H_ */
