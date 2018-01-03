/*
 * main.c
 *
 *  Created on: 2016年11月29日
 *      Author: john
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

//#include "src/XGDebug.h"
#include "src/XGageway.h"
#include "XDevice.h"
#include "sqlite/db.h"
#include <time.h>
#include "sqlite/dbDevice.h"
#include "Config.h"
#include "print.h"
#include "ikcc_common.h"
//#include "shell_script.h"
#include "platform_type.h"
#include "print.h"

extern unsigned char eth0_mac[6];

static time_t g_StartAllowAddTime = 0;
static time_t g_AllowAddTimeout = 0;
static char g_GatewayID[64] = { 0x0 };

static int OnDeviceLogin_t(int deviceid, int result, struct sockaddr_in *address) {
	Info("++++++++++++++++++++++++++++++++++++++++++++++++++++++++fun(%s)", __FUNCTION__);
	if (dbDeviceExisted(deviceid)) {
		dbSetDeviceStatus(deviceid, 1);
		return 0;
	}
	Info("************************************device not existed deviceid=%d", deviceid);
	return 1;
}

static void OnLoginout_t(int deviceid, int result) {
	if (dbDeviceExisted(deviceid)) {
		dbSetDeviceStatus(deviceid, 0);
	}
}

static void OnRecvDataFormDevice_t(int deviceid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype) {
	Debug("++++++++++++++++++++++++++ toid=%d", toid);
	Info("deviceid[%d] toid[%d] msgid[%d] data[%s], lenth[%d]", deviceid, toid, msgid, data, datalength);
	XGSendDataToCloud(deviceid, toid, msgid, data, datalength, pipetype);
	XDControlData(deviceid, data, datalength);
}

static void OnRecvDataFormCloud_t(int deviceid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype) {
	Debug("++++++++++++++++++++++++++ toid=%d", toid);
	Print_hex(data, datalength);
	XGSendDataToDevice(deviceid, toid, msgid, data, datalength, pipetype);
}

static int OnDiscoverDevice_t(char *productid, unsigned char *mac, unsigned int maclen, int deviceid, struct sockaddr_in *address) {
	int i = 0, ret = 0;
	Info("discover devcie product id %s", productid);
	Info("discover devcie mac length %d", maclen);
	char strmac[128]={0};
	sprintf(strmac, "%02x:%02x:%02x:%02x:%02x:%02", mac[0], mac[1],mac[2],mac[3],mac[4],mac[5]);
	Info("discover devcie mac %s", strmac);
	Info("discover devcie id %d", deviceid);

	Info("*********************************DISCOVER DEVICE!***************************************");

	if (dbDeviceExisted(deviceid)) {
		Info("---------------------device existed");
		return 0;
	}

	time_t c_time = time(NULL);
	if ((c_time - g_StartAllowAddTime) > g_AllowAddTimeout) {
		return 1;
	}
	ret = dbAddDevice(deviceid, productid, mac, maclen, address);
	XDNewDeviceJoinNotify(deviceid, productid, mac, maclen);
	Info("New Device Join deviceid[%d] productid[%s]", deviceid, productid);
	Info("---------------------Add device %d", ret);

	return ret;
}

static int OnDeviceRestartNotify_t(char *gatewayid, char *productid, unsigned char *mac, unsigned int maclen, int deviceid, struct sockaddr_in *address) {
	int i = 0;
	Info("device Restart product id %s", productid);
	Info("device Restart  mac length %d", maclen);
	Info("device Restart  mac ");
	for (i = 0; i < maclen; i++) {
		Info("%02X ", mac[i]);
	}
	Info("\r\ndevice Restart device id %d", deviceid);
	Info("device Restart gateway %s=%s", (char *) g_GatewayID,gatewayid);
	if (strcmp(gatewayid, (char *) g_GatewayID) != 0) {
		Info("gataway id !=");
		return -1; //not response
	}
	Info("gataway id ==");

	if (dbDeviceExisted(deviceid)) {
		dbUpdateDeviceAddress(deviceid, address);
		Info("response ok");
		return 0;//response ok
	}
	Info("response already delete");
	return 1; //already delete
}

XGConfig xgconfig = { //
	.OnDeviceLogin = OnDeviceLogin_t, //
	.OnLoginout = OnLoginout_t, //
	.OnRecvDataFormDevice = OnRecvDataFormDevice_t, //
	.OnRecvDataFormCloud = OnRecvDataFormCloud_t, //
	.OnDiscoverDevice = OnDiscoverDevice_t, //
	.OnDeviceRestartNotify = OnDeviceRestartNotify_t, //
	};



int ConsoleDiscoverDevice(char *args) {
	g_StartAllowAddTime = time(NULL);
	if (args == NULL) {
		g_AllowAddTimeout = 10;
	} else {
		unsigned short time = *((unsigned short *) args);
		g_AllowAddTimeout = time;
	}
	return XGDiscover();
}

static int ConsoleRestartNofify(char *args) {
	return XGRestartNotify();
}

static int ConsoleResetSdk(char *args) {
	XDResetSdk();
	return 0;
}

static int ConsoleRemove(char *args) {
	unsigned char buffer[1024] = { 0x00 };
	int size = dbGetAllDeviceID(buffer, 1024);
	if (size == 0) {
		Info("not device");
		return 0;
	}

	int deviceid = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
	Info("device id=%d", deviceid);
	struct sockaddr_in address;
	int ret = dbGetDeviceInfo(deviceid, NULL, NULL, NULL, &address);
	if (ret == 0) {
		dbDeleteDevice(deviceid);
		Info("device port=%d ip=%s", htons(address.sin_port), inet_ntoa(address.sin_addr));
		//address.sin_port = htons(5987);
		XGRemoveDevice(&address, deviceid);
	} else
		Info("get devcie address failed");
	return 0;
}

static int ConsoleGetAck(char *args) {
	unsigned char buffer[1024] = { 0x00 };
	int size = dbGetAllDeviceID(buffer, 1024);
	if (size == 0) {
		Info("not device");
		return 0;
	}

	int deviceid = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
	Info("device id=%d", deviceid);
	struct sockaddr_in address;
	unsigned char MacBuffer[32] = { 0x00 };
	unsigned char product_id[33]={0x00};
	int maclength = 0;
	int ret = dbGetDeviceInfo(deviceid, product_id, MacBuffer, &maclength, &address);
	if (ret != 0) {
		Info("get devcie info failed");
		return 0;
	}
	Debug("get accesskey address %s:%d\r\n", inet_ntoa(address.sin_addr), htons(address.sin_port));
	ret = XGGetAccessKey(&address, 400,product_id, MacBuffer, maclength);
	Debug("get accesskey ret=%d\r\n", ret);
	return 0;
}

static int ConsoleSetAck(char *args) {
	unsigned char buffer[1024] = { 0x00 };
	int size = dbGetAllDeviceID(buffer, 1024);
	if (size == 0) {
		Info("not device");
		return 0;
	}

	int deviceid = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
	Info("device id=%d", deviceid);
	struct sockaddr_in address;
	int ret = dbGetDeviceInfo(deviceid, NULL, NULL, NULL, &address);
	if (ret != 0) {
		Info("get devcie info failed");
		return 0;
	}
	Debug("get accesskey address %s:%d\r\n", inet_ntoa(address.sin_addr), htons(address.sin_port));
	int ack = 25879999;
	ret = XGSetAccessKey(ack, 400, &address);
	Debug("set accesskey ret=%d\r\n", ret);
	return 0;
}

static int ConsoleGetSubkey(char *args) {
	unsigned char buffer[1024] = { 0x00 };
	int size = dbGetAllDeviceID(buffer, 1024);
	if (size == 0) {
		Info("not device");
		return 0;
	}

	int deviceid = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
	Info("device id=%d", deviceid);
	struct sockaddr_in address;
	int ret = dbGetDeviceInfo(deviceid, NULL, NULL, NULL, &address);
	if (ret != 0) {
		Info("get devcie info failed");
		return 0;
	}
	Debug("get accesskey address %s:%d\r\n", inet_ntoa(address.sin_addr), htons(address.sin_port));
	int ack = 25879999;
	ret = XGGetSubKey(ack, 400, &address);
	Debug("get subkey ret=%d\r\n", ret);
	return 0;
}

static int ConsoleHelp(char *args);

struct {
	const char *type;
	const char *doc;
	const int length;
	int (*Console)(char *args);
} ConsoleFun[] = { //
		{ "join", "allow device join and 10 second", 4, ConsoleDiscoverDevice }, //
				{ "scan", "send gateway restart notify", 4, ConsoleRestartNofify }, //
				{ "reset", "reset sdk", 5, ConsoleResetSdk }, //
				{ "h", "gateway  cmd help", 1, ConsoleHelp }, //
				{ "remove", "delete device at once", 6, ConsoleRemove }, //
				{ "getack", "get device ack at once", 6, ConsoleGetAck }, //
				{ "setack", "set device ack at once", 6, ConsoleSetAck }, //
				{ "getsubkey", "get device subkey at once", 9, ConsoleGetSubkey }, //

				{ NULL, NULL, 0, NULL }, //
		};

static int ConsoleHelp(char *args) {
	int i = 0;
	Info("\r\n------------------- xlink gateway demo help ---------------");
	for (i = 0;; i++) {
		if (ConsoleFun[i].type == NULL)
			break;
		Info("cmd:【%s】==>:%s", ConsoleFun[i].type, ConsoleFun[i].doc);
	}
	Info("------------------- xlink gateway demo help over ---------------");
	return 0;
}

static int CheckMac(unsigned char *machex, char *macstring) {
	int len = strlen(macstring);
	int i = 0;
	if ((len % 2) != 0) {
		Error("Mac length error=%d\r\n", len);
		return 0;
	}
	len = len / 2;
	char temp[3] = { 0x00 };
	for (i = 0; i < len; i++) {
		memcpy(temp, &macstring[i * 2], 2);
		machex[i] = (unsigned char) strtol(temp, NULL, 16);
	}
	return len;
}

static void CreateGateId() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	snprintf(g_GatewayID, 64, "%ld", (tv.tv_sec * 1000 + tv.tv_usec / 1000));
	XDWriteString("gatewayid", g_GatewayID);
}

int ft_xlink_gateway_init(unsigned char *mac)
{
	int ret = 0;

	char productIDString[64] = { 0x00 };
	char productKeyString[64] = { 0 };
	char MacString[64] = { 0x00 };
	unsigned char MacHex[32] = { 0x00 };
	char Host[128] = { 00 };
	int maclength = 0;
	Debug("Build time %s %s", __DATE__, __TIME__);
/*
	ret = XDReadString("productid", productIDString, 64);
	if (ret == 0) {
		XDWriteString("productid", "");
		Error("read product id failed\r\n");
		return 0;
	}
	ret = XDReadString("productkey", productKeyString, 64);
	if (ret == 0) {
		Error("read product key failed\r\n");
		return 0;
	}*/
	//strcpy(productIDString, "1607d2af6773d4021607d2af6773d403");
	
	
	//strcpy(productKeyString, "cc2540cf5f853261fea924ee32409852");
	strcpy(productIDString, "1607d4b07f982a001607d4b07f982a01");
	strcpy(productKeyString, "32302522c02aa1771c662967ef935277");
	/*
	ret = XDReadString("mac", MacString, 64);
	if (ret == 0) {
		Error("read mac failed\r\n");
		return 0;
	}
	*/

	ret = XDReadString("host", Host, 128);
	if (ret == 0) {
		strcpy(Host, "cloud.fotile.com");
		//strcpy(Host, "42.121.122.23");
	}

	ret = XDReadString("gatewayid", g_GatewayID, 64);
	if (ret == 0) {
		CreateGateId();
	}
	/*
	maclength = CheckMac(MacHex, MacString);
	if (maclength == 0) {
		Error("check mac failed\r\n");
		return 0;
	}
	*/
	Debug("Read Config ProductID:%s", productIDString);
	Debug("Read Config ProductKey:%s", productKeyString);
	//Debug("Read Config ProductMac:%s", MacString);
	Debug("Read Config ProductHost:%s", Host);
	Debug("Read Config gatewayid:%s", g_GatewayID);

	ret = dbInit();
	if (ret != 0) {
		Error(" db init failed\r\n");
		return 0;
	}
	dbCreateDeviceTable();

	XGSetHost(Host);
	XDSetHost(Host);

	ret = XGInit("12548", productIDString, (char *) g_GatewayID, &xgconfig);
	if (ret < 0) {
		Error(" XGCoreStart failed");
	}
	
//	unsigned char eth0_mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
	{
		//auto get mac from eth0
		char tmp[32] = {0};
		
		//get_eth0_mac(tmp, eth0_mac, 32);
		Info("eth0[%02X:%02X:%02X:%02X:%02X:%02X]", 
			eth0_mac[0], eth0_mac[1],eth0_mac[2],eth0_mac[3],eth0_mac[4],eth0_mac[5]);
	}
	//启动网关云链接
	XDeviceStart(productIDString, productKeyString, mac, 6); //user eth0 mac
	//XDeviceStart(productIDString, productKeyString, MacHex, maclength);
	//延时500 毫秒发送网关重启信息
	usleep(500000);
	ConsoleRestartNofify(NULL);

#if 0	
	//命令行输入
	char input[1024] = { 0x00 };
	int i = 0;
	while (1) {
		memset(input, 0, 1024);
		fgets(input, 1024, stdin);
		for (i = 0;; i++) {
			if (ConsoleFun[i].type == NULL)
				break;
			if (strncasecmp(input, ConsoleFun[i].type, ConsoleFun[i].length) == 0) {
				ret = ConsoleFun[i].Console(NULL);
				Fatal("Start Console function ret=%d\r\n", ret);
			}
		}
	}
#endif	
	return 0;
}

int ft_xlink_gateway_deinit(void) 
{
	XGUninit();
	
	return 0;
}

int ft_xlink_console(int argc, char argv[][64])
{
#if 0
    int i = 0;
    int ret = 0;
    for (i = 0;; i++) 
    {
        if (ConsoleFun[i].type == NULL)
        {
            break;
        }	
        if (strncasecmp(s, ConsoleFun[i].type, ConsoleFun[i].length) == 0) 
        {
            ret = ConsoleFun[i].Console(NULL);
            Warn("Start Console function ret=%d\r\n", ret);
        }
    }
#endif
	
	if(!FT_STRCMP(argv[1], "?") || !strcmp(argv[1], "h")|| !strcmp(argv[1], "help")){
			ConsoleHelp(NULL);
		}else if(!FT_STRCMP(argv[1], "join")){
			Debug("join");
			ConsoleDiscoverDevice(NULL);
		}else if(!FT_STRCMP(argv[1], "scan")){
			Debug("scan");
			ConsoleRestartNofify(NULL);
		}else if(!FT_STRCMP(argv[1], "t1")){
			Debug("t1");
			//ConsoleTest(atoi(argv[2]));
		}else if(!FT_STRCMP(argv[1], "t2")){
			Debug("t2");
			//ConsoleTest2(atoi(argv[2]));
		}else if(!FT_STRCMP(argv[1], "add")){
			Debug("add");
			unsigned char mac[6] = {0x7c, 0xc7, 0x09, 0x7b, 0xb7, 0x59};
			XDNewDeviceJoinNotify(452578839, "1607d6b05791c0001607d6b05791c001", mac, 6);
		}else if(!FT_STRCMP(argv[1], "reset")){
			Debug("reset");
			ConsoleResetSdk(NULL);
		}else if(!FT_STRCMP(argv[1], "remove")){
			Debug("remove");
			ConsoleRemove(NULL);
		}else{
			Warn("unknow %s command!", argv[0]);
		}
}

