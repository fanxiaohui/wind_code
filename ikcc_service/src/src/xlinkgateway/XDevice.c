/*
 * XDevice.c
 *
 *  Created on: 2016年12月14日
 *      Author: john
 */

#include "XDevice.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include "xlink/Xlink_Head_Adaptation.h"
#include "xlink/xlink_system.h"
#include "Config.h"
#include "GAProtocol.h"
#include "GAPHeader.h"
//#include "src/XGDebug.h"
#include "print.h"
#include "platform_type.h"
#include "shell_script.h"

#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include "cJSON.h"


#define OUT_FUN  Info("[fotile]%s<%d>", __FUNCTION__, __LINE__)


static pthread_mutex_t g_lock; //网关发送到APP的数据资源锁

typedef struct XlinkQueue {
	struct XlinkQueue *next;
	unsigned int datalength;
	unsigned char data[];
} XlinkQueue_t, *XlinkQueueItem;

static char gs_host[128] = { "cloud.fotile.com" };
XlinkQueue_t g_header;

unsigned char isConnectServer = 0;
static unsigned char isRuning = 0;
static unsigned char isQuit = 0;
static unsigned char isResetSdkFlag = 0;
static unsigned char isAllowJoin =0;
static time_t  g_allowtime = 0;

static void SendUdpResponse(unsigned char *data, unsigned int datalength, int id) {
	XlinkSendUdpPipe(data, datalength, id, 1);
}

static void SendTcpResponse(unsigned char *data, unsigned int datalength, int id) {
	XlinkSendTcpPipe(0, data, datalength, id);
}

static void OnTcpPipe_t(unsigned char * Buffer, unsigned int BufferLen, x_uint32 AppID, x_uint8 *opt) {
	OUT_FUN;
	Debug("rcv app msg AppID[%d]", AppID);
	Print_hex(Buffer, BufferLen);
	GAPStartProtocol(Buffer, BufferLen, AppID, SendTcpResponse);
}

static void OnUdpPipe_t(unsigned char * Buffer, unsigned int BufferLen, int fromAddr) {
	OUT_FUN;
	Print_hex(Buffer, BufferLen);
	Debug("fromAddr[%d]", fromAddr);
	GAPStartProtocol(Buffer, BufferLen, fromAddr, SendUdpResponse);
}

static int OnWriteConfig_t(char *Buffer, unsigned int BufferLen) {
	OUT_FUN;
	//写数据到配置文件中
	BufferLen = XDWriteBase64Encode("Xlink.v4sdk.cofnig", (unsigned char *) Buffer, BufferLen);
	return BufferLen;
}

static int OnReadConfig_t(char *Buffer, unsigned int BufferLen) {
	OUT_FUN;
	//从配置文件中读取数据
	BufferLen = XDReadBase64Decode("Xlink.v4sdk.cofnig", (unsigned char *) Buffer, BufferLen);
	return BufferLen;
}

static void OnStatus_t(XLINK_APP_STATUS OnStatus) {
	switch (OnStatus) {
	case XLINK_WIFI_STA_DISCONNCT_SERVER: //与服务器断开链接
	case XLINK_WIFI_STA_CONNECT_SERVER_FAILED: //链接服务器失败的回调，此表示在XLINK v4sdk 中新增加的。
		isConnectServer = 0; //清除链接信息
		//set_lamp_option(IKCC_LED_BLINK, IKCC_LED_RED);
		break;
	case XLINK_WIFI_STA_CONNECT_SERVER:
		Info("%s<%d> connect server success", __FUNCTION__, __LINE__);
		break;
	case XLINK_WIFI_STA_APP_CONNECT:
		Info("%s<%d> app connect", __FUNCTION__, __LINE__);
		break;
	case XLINK_WIFI_STA_APP_DISCONNECT:
		Info("%s<%d> app disconnect", __FUNCTION__, __LINE__);
		break;
	case XLINK_WIFI_STA_APP_TIMEOUT:
		Info("%s<%d> app timeout", __FUNCTION__, __LINE__);
		break;
	case XLINK_WIFI_STA_LOGIN_SUCCESS: //登陆服务器成功，只有成功登陆后才能够算链接成功。
		Info("%s<%d> connect server login success", __FUNCTION__, __LINE__);
		isConnectServer = 1;
		//set_lamp_option(IKCC_LED_NOBLINK, IKCC_LED_RED);
		g_sdk_open = 1;
		break;
	default:
		break;
	}
}

static void OnUpgrade_t(XLINK_UPGRADE *data) {
	OUT_FUN;
}

static void OnServerTime_t(XLINK_SYS_TIME *time_p) {
	OUT_FUN;
}

static void OnSetDataPoint_t(unsigned char *data, int datalen) {
	OUT_FUN;
	Print_hex(data, datalen);
}

static void OnGetAllDataPoint_t(unsigned char *data, int *datalen) {
	*datalen = 0;
	OUT_FUN;
	Print_hex(data, *datalen);
}

static unsigned long long int OnGetSystemTimeMs_t(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void OnTcpNotify_T(unsigned short messagetpye, unsigned char * data, unsigned int datalen) {
	OUT_FUN;
}
static void OnTcpPipeSendCb_t(unsigned short handle, unsigned char val) {
	OUT_FUN;
}
static void OnTcpPipe2SendCb_t(unsigned short handle, unsigned char val) {
	OUT_FUN;
}
static void OnUdpPipeSendCb_t(unsigned short handle, unsigned char val) {
	OUT_FUN;
}
static void OnTcpDatapointSendCb_t(unsigned short handle, unsigned char val) {
	//OUT_FUN;
}

//接收到网关的数据
void OnXGMessage_t(unsigned char *data, unsigned int datalength, xlink_addr *addr) {
	//OUT_FUN;
	//不处理网关消息
	//Debug("-----OnXGMessage_t");
}

static XLINK_USER_CONFIG user_config = { //
#ifdef SYSLOG
		.DebugPrintf = NULL,	//
#else
		.DebugPrintf = printf,
#endif
				//.mac = { 0xAC, 0xCF, 0x00, 0x01, 0x00, 0x09 },	//
				.mac = { 0xAA, 0xBB, 0xCC, 0x00, 0x00, 0x00 },	//
				.maclen = 6,	//
				.in_internet = 1,	//
				.mcuHardwareVersion = 1,	//
				.mcuHardwareSoftVersion = 1,	//
				.wifi_type = 1,	//
				.wifisoftVersion = 1,	//
				.devicetype = 0,	//
				.pipetype = 0,	//
				.OnGetAllDataPoint = OnGetAllDataPoint_t,	//
				.OnGetSystemTimeMs = OnGetSystemTimeMs_t,	//
				.OnReadConfig = OnReadConfig_t,	//
				.OnServerTime = OnServerTime_t,	//
				.OnSetDataPoint = OnSetDataPoint_t,	//
				.OnStatus = OnStatus_t,	//
				.OnTcpDatapointSendCb = OnTcpDatapointSendCb_t,	//
				.OnTcpNotify = OnTcpNotify_T,	//
				.OnTcpPipe = OnTcpPipe_t,	//
				.OnTcpPipe2 = NULL,	//
				.OnTcpPipe2SendCb = OnTcpPipe2SendCb_t,	//
				.OnTcpPipeSendCb = OnTcpPipeSendCb_t,	//
				.OnUdpPipe = OnUdpPipe_t,	//
				.OnUdpPipeSendCb = OnUdpPipeSendCb_t,	//
				.OnUpgrade = OnUpgrade_t,	//
				.OnWriteConfig = OnWriteConfig_t, //
				.OnXGMessage = OnXGMessage_t, //
		};

// TCP 的链接工作线程
static void *TcpWorkThread(void *args) {
	while (!isQuit) {
		XlinkSystemTcpLoop();
		usleep(5000000);
	}
	return NULL;
}

int FT_System(char* cmdstring, char* buf, int len)
{
	int   fd[2]; 
	pid_t pid;
	int   n, count;
	memset(buf, 0, len);
	if (pipe(fd) < 0)
	return -1;
	if ((pid = fork()) < 0)
	return -1;
	else if (pid > 0)     /* parent process */
	{
		close(fd[1]);     /* close write end */
		count = 0;
		while ((n = read(fd[0], buf + count, len)) > 0 && count > len)
		count += n;
		close(fd[0]);
		if (waitpid(pid, NULL, 0) > 0)
		return -1;
	}
	else    /* child process */
	{
	close(fd[0]);     /* close read end */
	if (fd[1] != STDOUT_FILENO)
	{
		if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO)
		{
			return -1;
		}
		close(fd[1]);
	}
	if (execl("/bin/sh", "sh", "-c", cmdstring, (char*)0) == -1)
	return -1;
	}
	return 0;
}

static void *DiscoverntfThread(void *args) {
	int counter = 0,result=0,orign=0;
	char buff[256];
	FT_System("cat /proc/net/arp |grep br-lan | wc -l",buff,256);
	if(buff != ""){
	orign = atoi(buff);
	}else{
	orign = 0;
	}
	Debug("**********orign=%d*********",orign);
	while (!isQuit) {
		sleep(1);
		if(counter <= 300){			
			counter++;
			if((counter%10) == 0){
			kill(1,SIGUSR1);
			Debug("**********feed dog*********");
			}
			}
		else{
			FT_System("cat /proc/net/arp |grep br-lan | wc -l",buff,256);
			result = atoi(buff);
			Debug("**********result=%d*********",result);
			if(result > orign){
			unsigned short timeout = 30;
			Debug("**********DiscoverDevice[result=%d][orign=%d]*********",result,orign);
			AlowDeviceJoin(timeout);
			ConsoleDiscoverDevice((char *) &timeout);
			orign = result;
			}
			counter = 0;
			}
	}
	return NULL;
}

static void *UDPWorkThread(void *args) {
	isRuning = 1;
	int lock = 0;
	XlinkQueueItem item;
	time_t old = time(NULL),olddiscover=0;
	while (!isQuit) {
		//等待30毫秒，用于接收网络数据
		XlinkLoop(time(NULL), 30);
		if (g_header.next != NULL) {
			lock = pthread_mutex_trylock(&g_lock);
			if (lock == 0) {
				item = g_header.next;
				g_header.next = item->next;
				item->next = NULL;
				XlinkSendUdpPipe(item->data, item->datalength, -1, 1);
				XlinkSendTcpBroadcast(0, item->data, item->datalength);
				free(item);
				item = NULL;
				pthread_mutex_unlock(&g_lock);
			}
		}
		if (isResetSdkFlag) {
			isResetSdkFlag = 0;
			XlinkReSetSDK();
		}

		if ((time(NULL) - old) > 20) {
			XGRestartNotify();
			old = time(NULL);
		}

		if(g_allowtime > time(NULL) && (time(NULL)-olddiscover)>=3){
			olddiscover = time(NULL);
			XGDiscover();
		}
	}
	isRuning = 0;
	return NULL;
}

void XDeviceStart(char *pid, char *pkey, unsigned char *mac, int maclen) {
	//int i = 0, ret = 0;
	pthread_mutex_init(&g_lock, NULL);
	g_header.next = NULL;
	g_header.datalength = 0;

	memcpy(user_config.mac, mac, maclen);
	user_config.maclen = maclen;

	//系统初始化
	XlinkInit(pid, pkey, &user_config);
	XlinkSetHost(gs_host);
	//启用UDP扫描功能
	XlinkEnableSubAndScan();
	isQuit = 0;
	isRuning = 0;
	//创建TCP工作线程
	pthread_t threadtcp, threadudp,threaddnf;
	pthread_create(&threadtcp, NULL, TcpWorkThread, NULL);
	//创建UDP工作线程
	pthread_create(&threadudp, NULL, UDPWorkThread, NULL);
	pthread_create(&threaddnf, NULL, DiscoverntfThread, NULL);
	

}

void XDeviceStop() {
	isQuit = 1;
	while (isRuning)
		usleep(20);
	pthread_mutex_destroy(&g_lock);
}

void XDAddSendData(unsigned char *data, unsigned int datalength) {
	Print_hex(data, datalength);
	pthread_mutex_lock(&g_lock);

	XlinkQueueItem item = (XlinkQueueItem) malloc(datalength + sizeof(XlinkQueue_t) + 8);
	if (item == NULL) {
		pthread_mutex_unlock(&g_lock);
		return;
	}
	item->next = NULL;
	item->datalength = datalength;
	memcpy(item->data, data, datalength);

	XlinkQueueItem head = &g_header;
	while (head->next != NULL) {
		head = head->next;
	}
	head->next = item;
	pthread_mutex_unlock(&g_lock);
}

void XDNewDeviceJoinNotify(int deviceid, char *productid, unsigned char *mac, unsigned int maclength) {
	unsigned char buffer[256] = { 0x00 };
	int index = 0;
	buffer[index++] = GAP_CMD_NEW_DEVICE_JOIN_NOTIFY;
	buffer[index++] = (unsigned char) (((deviceid) & 0xFF000000) >> 24);
	buffer[index++] = (unsigned char) (((deviceid) & 0x00FF0000) >> 16);
	buffer[index++] = (unsigned char) (((deviceid) & 0x0000FF00) >> 8);
	buffer[index++] = (unsigned char) (((deviceid) & 0x000000FF));
	memcpy(&buffer[index], productid, 32);
	index += 32;
	buffer[index++] = maclength;
	memcpy(&buffer[index], mac, maclength);
	index += maclength;
	XDAddSendData(buffer, index);
}

void XDControlData(int deviceid, unsigned char *data, unsigned int datalegnth) {

	unsigned char buffer[1600] = { 0x00 };
	if (datalegnth > 1500) {
		return;
	}
	int index = 0;
	buffer[index++] = GAP_CONTROL_DEVICE;
	index += UtilSetInt32(&buffer[index], deviceid);	//deviceid
	buffer[index++] = 0;
	buffer[index++] = 0;								//msgid
	index += UtilSetInt16(&buffer[index], datalegnth);	
	memcpy(&buffer[index], data, datalegnth);			//data len
	index += datalegnth;
	XDAddSendData(buffer, index);
}

void XDResetSdk() {
	isResetSdkFlag = 1;
}

void XDSetHost(char *host) {
	memset(gs_host, 0, 128);
	strcpy(gs_host, host);
}

int AlowDeviceJoin(int timeout){
	isAllowJoin = 1;
	g_allowtime = time(NULL)+timeout;
	return 0;
}
