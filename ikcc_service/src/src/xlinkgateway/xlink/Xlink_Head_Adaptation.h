/*
 * Xlink_Head_Adaptation.h
 *
 *  Created on: 2015年3月9日
 *      Author: John
 */

#ifndef SRC_XLINK_HEAD_ADAPTATION_H_
#define SRC_XLINK_HEAD_ADAPTATION_H_
#ifdef  __cplusplus
extern "C" {
#endif
#include "xlink_type.h"

#define HF_SDK                  0//汉枫
#define HF_SDK_A21              0
#define MARVELL_SDK          	0//marvell
#define QCA4004             	0//高通4004
#define REALTEK              	0//realtek
#define REALTEK_8711            0
#define NL6621                  0//新岸线
#define __LWIP__ESP_8266     	0//
#define __HED_10W07SN__         0//联盛德
#define __MT7681__           	0
#define __LINUX__             	1
#define __STM32F107__           0

#define __LSD4WF_2MD05101__     0   //LI ER DA
#define __RTOS_STM32F103__      0
#define __RTOS_STM32F107__      0
#define __STM32F103_UIP__       0
#define __MXCHIP__             	0

#define __ALL_DEVICE__          0

#define __XDEBUG__           	1

#define __XLINK_VERSION__       34700

#define __XLINK_BUFFER_PIPE__   1500

#define __XLINK_STAND_HAIMAN__   0  //海曼科技部署
//#define TEST_SERVER 

#if HF_SDK
#define CLIENT_SSL_ENABLE 0
#include "ic_head/xlink_HF.h"
#elif HF_SDK_A21

#include "ic_head/xlink_HF_A21.h"

#elif  MARVELL_SDK

#include "ic_head/xlink_MARVELL_200.h"

#elif   QCA4004

#include "ic_head/xlink_QCA4004.h"

#elif   REALTEK

#include "ic_head/xlink_REALTEK_8195.h"

#elif REALTEK_8711

#include "ic_head/xlink_REALTEK_8711.h"

#elif   NL6621

#include "ic_head/xlink_NL6621.h"

#elif   __LWIP__ESP_8266

#include "ic_head/xlink_ESP8266.h"

#elif  __MT7681__

#include "ic_head/xlink_MT7681.h"

#elif __STM32F107__

#include "ic_head/xlink_stm32_lwip.h"

#elif __HED_10W07SN__

#include "ic_head/xlink_HED10W07SN.h"

#elif __LINUX__

#include "ic_head/xlink_linux.h"

#elif __LSD4WF_2MD05101__

#include "ic_head/xlink_LSD4WF2MD05101.h"

#elif __RTOS_STM32F103__

#include "ic_head/xlink_stm32_rtos.h"

#elif __STM32F103_UIP__

#include "ic_head/xlink_stm32_uip.h"

#elif __MXCHIP__

#include "ic_head/xlink_MXCHIP_mico.h"

#elif __ALL_DEVICE__

#include "ic_head/xlink_All_Device.h"

#endif

#define XLINK_MAC_LEN_MAX 32
#define XLINK_MAC_LEN_MIN 1

typedef struct XLINK_CALLBACK_FUN {
	//接收服务器广播的回调函数
	void (*OnTcpPipe2)(unsigned char * Buffer, unsigned int BufferLen, x_uint8 *opt);
	//接收APP通过服务器发送的数据的回调函数
	void (*OnTcpPipe)(unsigned char * Buffer, unsigned int BufferLen, x_uint32 AppID, x_uint8 *opt);
	//接收APP通过本地发送的数据的回调函数
	void (*OnUdpPipe)(unsigned char * Buffer, unsigned int BufferLen, int fromAddr);
	//SDK 内部写配置回调函数，需要将此数据写入flash
	int (*OnWriteConfig)(char *Buffer, unsigned int BufferLen);
	//SDK 内部读本地的配置文件，需要读出flash的配置数据
	int (*OnReadConfig)(char *Buffer, unsigned int BufferLen);
	//SDK 内部状态信息回调
	void (*OnStatus)(XLINK_APP_STATUS OnStatus);
	//SDK 收到服务器升级信息回调
	void (*OnUpgrade)(XLINK_UPGRADE *data);
	//SDK 收到服务器同步时间信息回调
	void (*OnServerTime)(XLINK_SYS_TIME *time_p);
	//SDK 内部日志输出回调
	xlink_debug_fn DebugPrintf;
	//WiFi类型
	unsigned char wifi_type;
	//WiFi版本
	unsigned short wifisoftVersion;
	x_uint8 mcuHardwareVersion; /*Hardware Version*/
	x_uint16 mcuHardwareSoftVersion;/*Software Version*/
	//SDK 内部是否使用TCP公网连接
	unsigned char in_internet;
	//设备的MAC地址
	unsigned short maclen;
	unsigned char mac[XLINK_MAC_LEN_MAX];

	//硬件协议透传协议选择,0表示普通透传，1表示透传内容为硬件通讯协议。
	unsigned char pipetype;
	//设备类型
	unsigned short devicetype;

	//TCP服务器消息通知
	void (*OnTcpNotify)(unsigned short messagetpye, unsigned char * data, unsigned int datalen);
	//TCP pipe数据发送回调,handle与发送handle对应，val为0表示失败，1表示成功
	void (*OnTcpPipeSendCb)(unsigned short handle, unsigned char val);
	//TCP pipe2数据发送回调,handle与发送handle对应，val为0表示失败，1表示成功
	void (*OnTcpPipe2SendCb)(unsigned short handle, unsigned char val);
	//udp pipe数据发送回调,handle与发送handle对应，val为0表示失败，1表示成功
	void (*OnUdpPipeSendCb)(unsigned short handle, unsigned char val);
	//TCP 数据端点更新发送回调,handle与发送handle对应，val为0表示失败，1表示成功
	void (*OnTcpDatapointSendCb)(unsigned short handle, unsigned char val);

#if  __LWIP__ESP_8266 || __MT7681__ || __STM32F107__ || __STM32F103_UIP__ || __ALL_DEVICE__
	unsigned char *tcpRecvBuffer;
	unsigned int tcpRecvBuuferLength;
	int (*send_udp)(xlink_addr *addr, unsigned char *data, unsigned int datalen);
	int (*send_tcp)(unsigned char *data, unsigned int datalen);
	void (*setServerStatus)(unsigned char stat,unsigned char is80Port);
	void (*process_udp_data)(unsigned char * Buffer, unsigned int BufferLen, xlink_addr *addrBro);
	void (*process_tcp_data)(unsigned char * data, unsigned int datalen, x_uint32 bodylen);
#endif

	//V2 datapoint
	//datapoint format:[datapoint index:1byte][datalen:2bytes(H L)][data:datalen bytes]...
	void (*OnSetDataPoint)(unsigned char *data, int datalen);	//app->wifi,you can update datapoints by this buf
	void (*OnGetAllDataPoint)(unsigned char *data, int *datalen);	//add all datapoint data to the buf,The maximum length is XLINK_DATAPOINT_MAX_BYTES
	unsigned long long int (*OnGetSystemTimeMs)(void);
	void (*OnXGMessage)(unsigned char *data, unsigned int datalength, xlink_addr *addr);
} XLINK_USER_CONFIG;



#ifdef  __cplusplus
}
#endif
#endif /* SRC_XLINK_HEAD_ADAPTATION_H_ */
