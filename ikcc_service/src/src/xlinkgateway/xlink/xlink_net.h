
#ifndef XLINK_NET_H_
#define XLINK_NET_H_
#ifdef  __cplusplus
extern "C" {
#endif
#include "xlink_data.h"

extern XLINK_FUNC x_uint8 xlink_net_loop(xsdk_time_t c_time, x_int32 timeout);
extern XLINK_FUNC unsigned char check_typeOk(unsigned char type);
extern XLINK_FUNC unsigned char check_typeOk_t(unsigned char type);

#if (!__LWIP__ESP_8266) && !__MT7681__ && !__STM32F107__ && !__STM32F103_UIP__ && !__ALL_DEVICE__
extern XLINK_FUNC int xlink_net_loop_Domain_name(void);
extern XLINK_FUNC void  xsdk_closeUDP(void);
#endif

extern XLINK_FUNC void xsdk_closeTCP(int flag);
extern XLINK_FUNC unsigned int xlink_getUdpIndex(void);

extern XLINK_FUNC void xlink_net_setSystemTime(XLINK_SYS_TIME *pTime);
extern XLINK_FUNC const char *xlinkNetGetHost(void);

#ifdef  __cplusplus
}
#endif
#endif /* XLINK_NET_H_ */
