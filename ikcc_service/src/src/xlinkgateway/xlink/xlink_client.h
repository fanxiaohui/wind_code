/*
 * xlink_client.h
 *
 *  Created on: 2014年12月22日
 *      Author: john
 */

#ifndef XLINK_CLIENT_H_
#define XLINK_CLIENT_H_
#ifdef  __cplusplus
extern "C" {
#endif
#include "xlink_type.h"
#include "xlink_message.h"
#include "xlink_data.h"

#define XLINK_CLIENT_SIZE 32

extern XLINK_CLIENT g_xlink_client[XLINK_CLIENT_SIZE];
extern XLINK_FUNC void XlinnkClientInit(void);
extern XLINK_FUNC x_int16 XlinkClientAddclient(unsigned short AppKeepAliveTime, xlink_addr *addr, x_uint8 *ssidIndex, x_uint8 *ssidValue);
extern XLINK_FUNC void XlinkClientQuit(x_uint8 index);
extern XLINK_FUNC void XlinkClientByby(x_uint16 ssid, xlink_addr *AddrBro);
extern XLINK_FUNC void xlinkClientReSet(xlink_addr *AddrBro);
extern XLINK_FUNC void XlinkClientCheckHeartbeat(xsdk_time_t c_time);
extern XLINK_FUNC x_int32 XlinkClientSendDataToAllOlineClient(void *data, unsigned int datalen);
extern XLINK_FUNC x_int32 XlinkClientSendDataToAddr(x_uint8 *data, unsigned int datalen, xlink_addr *addr);
extern XLINK_FUNC x_int32 XlinkClientSendToIndex(x_uint8 *data, unsigned int datalen, x_uint8 index);
extern XLINK_FUNC x_bool XlinkClientcheckclientLogin(x_uint8 ssidIndex, x_uint8 ssidValue, xlink_addr *AddrBro);
extern XLINK_FUNC int XlinkClientSendUdpPipe(const unsigned char *data,const unsigned int datalen,const unsigned char index);



#ifdef  __cplusplus
}
#endif
#endif /* XLINK_CLIENT_H_ */
