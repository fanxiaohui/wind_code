#include "xlink_data.h"
#include "xlink_client.h"
#include "xlink_tcp_client.h"
#include "xlink_md5.h"
#include "xlink_system.h"
#include "xsdk_config.h"
#include "xlink_net.h"


G_XLINK_INFO g_xlink_info;
XLINK_USER_CONFIG *g_xlink_user_config = NULL;



#if  __LWIP__ESP_8266 || __MT7681__ || __STM32F107__ || __STM32F103_UIP__ || __ALL_DEVICE__
XLINK_FUNC void xclose(int s) {

}
#endif


XLINK_FUNC void XlinkSdkDataInit(const char* product_id, const char *product_key) {

	xlink_memset(&g_xlink_info, 0, sizeof(G_XLINK_INFO));
	//read device id and configure info
	xsdk_read_config(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));

	g_xlink_info.net_info.udp_fd = 0;
	g_xlink_info.net_info.tcp_fd = 0;
	g_xlink_info.net_info.tcp_last_connect_server = 0;
	g_xlink_info.net_info.ServerPort = 80;
	g_xlink_info.flag.byte = 0;
	

	xlink_memcpy(g_xlink_info.dev_info.product_id, product_id, XLINK_SIZE_33-1);

	g_xlink_info.dev_info.getServerTimeDelay = 0;
	g_xlink_info.dev_info.lastGetServerTimeTime = 0;

	xlink_memcpy(g_xlink_info.dev_info.product_key, product_key, XLINK_SIZE_33-1);
	
	XlinkSetDeviceName("xlink_dev");

}


XLINK_FUNC void XlinkSdkAppSetDeviceName(char* name, unsigned short nameLength) {
	xsdk_write_config(XLINK_CONFIG_INDEX_NAME, name, nameLength);

	g_xlink_info.config.flag.Bit.isAppSetName = 1;

	xsdk_write_config(XLINK_CONFIG_INDEX_CONFIG, (char*) &g_xlink_info.config, sizeof(xlink_SdkConfig));
	xsdk_config_save();
}

unsigned char inline UTIL_INT16_GET_BITH(unsigned int X)
{
	return (unsigned char)(((X)&0xff00)>>8);
}
unsigned char inline UTIL_INT16_GET_BITL(unsigned int X)
{
	return (unsigned char)((X)&0x00ff);
}




