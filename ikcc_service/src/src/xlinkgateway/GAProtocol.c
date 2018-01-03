/*
 * GAProtocol.c
 *
 *  Created on: 2016年12月20日
 *      Author: john
 */
#include <stdio.h>
#include <string.h>
#include "GAProtocol.h"
#include "main.h"
#include "GAPHeader.h"
#include "sqlite/dbDevice.h"
#include "src/XGageway.h"
//#include "src/XGDebug.h"
#include "xlink/xlink_system.h"
#include "print.h"
#include "XDevice.h"
#include "shell_script.h"
#include <stdlib.h>   
#include <string.h>   
#include <curl/curl.h> 
#include "cJSON.h"

static void processAllowDeviceJoin(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processGetDeviceList(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processGetDeviceInfo(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processGetDeviceAck(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processSetDeviceAck(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processGetDeviceSubkey(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);

static void processControlDevice(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processDeleteDevice(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);

static void processGetGatewayStatue(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processControlLamp(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processOpenTmpSSID(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processSetOnlySSID(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processOpenEasylink(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processSetSystem(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processSetNetMode(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
static void processSetUpdateUrl(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);



struct {
	const char *doc;
	const unsigned short minlength;
	const unsigned char type;
	void (*process)(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);
} FunProtocol[] = { //
		{ "Allow device join", 3, GAP_CMD_ALLOW_DEVICE_JOIN, processAllowDeviceJoin }, //

				{ "get device list", 3, GAP_GET_DEVICESLIST, processGetDeviceList }, //
				{ "get device info", 7, GAP_GET_DEVICES_INFO, processGetDeviceInfo }, //
				{ "get device ack", 11, GAP_GET_DEVICE_ACK, processGetDeviceAck }, //
				{ "set device ack", 11, GAP_SET_DEVICE_ACK, processSetDeviceAck }, //
				{ "get device subkey", 11, GAP_GET_DEVICE_SUBKEY, processGetDeviceSubkey }, //
				{ "control device", 10, GAP_CONTROL_DEVICE, processControlDevice }, //
				{ "delete device", 9, GAP_DELETE_DEVICE, processDeleteDevice }, //
				{ "get gateway statue", 7, GAP_GET_GATEWAY_STATUE, processGetGatewayStatue }, //
				{ "control lamp", 8, GAP_CONTROL_LAMP, processControlLamp }, //
				{ "open tmp ssid", 7, GAP_OPEN_TMP_SSID, processOpenTmpSSID }, //
				{ "set only ssid", 47, GAP_SET_ONLY_SSID, processSetOnlySSID }, //
				{ "open easylink", 7, GAP_OPEN_EASYLINK, processOpenEasylink }, //
				{ "set system", 8, GAP_SET_SYSTEM, processSetSystem }, //
				{ "set net mode", 8, GAP_SET_NET_MODE, processSetNetMode }, //
				{ "set update url", 7, GAP_SET_UPDATE_URL, processSetUpdateUrl }, //
				{ NULL, 0, 0, NULL }, //
		};

int UtilGetInt32(unsigned char *buffer) {
	return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
}

int UtilGetInt16(unsigned char *buffer) {
	return (buffer[2] << 8) + buffer[3];
}

int UtilSetInt32(unsigned char *buffer, unsigned int data) {
	buffer[0] = (unsigned char) (((data) & 0xFF000000) >> 24);
	buffer[1] = (unsigned char) (((data) & 0x00FF0000) >> 16);
	buffer[2] = (unsigned char) (((data) & 0x0000FF00) >> 8);
	buffer[3] = (unsigned char) (((data) & 0x000000FF));
	return 4;
}

int UtilSetInt16(unsigned char *buffer, unsigned short data) {
	buffer[0] = (unsigned char) (((data) & 0xFF00) >> 8);
	buffer[1] = (unsigned char) (((data) & 0x00FF));
	return 2;
}

void GAPStartProtocol(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall) {
	int i = 0;
	for (i = 0;; i++) {
		if (FunProtocol[i].doc == NULL){
			Warn("doc NULL!");
			break;
		}
		if (FunProtocol[i].minlength > datalength) {
			//Warn("funprotocol[%d].minlenth[%d] > datalenth[%d]", FunProtocol[i].minlength, datalength);
			continue;
		}
		if (FunProtocol[i].type == data[0]) {
			Debug("-----Gataway msg %s", FunProtocol[i].doc);
			FunProtocol[i].process(data, datalength, id, SendCall);
		}
	}
}

static void processAllowDeviceJoin(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall) {
	unsigned short timeout = (data[1] << 8) + data[2];
	Debug("-----Gataway allow device join timeout %d", timeout);
	if(timeout > 300){
		timeout = 30;
	}
	AlowDeviceJoin(timeout);
	ConsoleDiscoverDevice((char *) &timeout);
}

static void processGetDeviceList(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall) {
	unsigned char buffer[1024] = { 0x00 };
	int devicecount = dbGetDeviceCount();
	Debug("Device count = %d, id[%d]", devicecount, id);
	devicecount = devicecount < 0 ? 0 : devicecount;
	int index = 0;
	buffer[index++] = GAP_GET_DEVICESLIST_Ret;
	buffer[index++] = data[1];
	buffer[index++] = data[2];
	buffer[index++] = devicecount;
	if (devicecount > 0) {
		index += dbGetAllDeviceID(&buffer[index], 1024 - index);
	}
	Warn("index[%d], id[%d]", index, id);
	Print_hex(buffer, index);
	SendCall(buffer, index, id);
	
}

static void processGetDeviceInfo(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall) {
	int deviceid = UtilGetInt32(data + 1);
	unsigned char buffer[512] = { 0x00 };
	unsigned char mac[128] = { 0x00 };
	int length = 0;
	int index = 7, ret = 0, status = 0;
	buffer[0] = GAP_GET_DEVICES_INFO_Ret;
	buffer[1] = data[1];
	buffer[2] = data[2];
	buffer[3] = data[3];
	buffer[4] = data[4];
	buffer[5] = data[5];
	buffer[6] = data[6];

	Debug("id[%d]", id);
	if (dbDeviceExisted(deviceid) == 0) {
		buffer[index++] = 1; //没有设备
		SendCall(buffer, index, id);
		return;
	}

	buffer[index++] = 0; //返回成功
	ret = dbGetDeviceInfo(deviceid, (char *) &buffer[index], mac, &length, NULL);
	if (ret != 0 || length > 32 || length < 6) {
		buffer[8] = 2; //数据错误
		SendCall(buffer, 9, id);
		return;
	}
	dbGetDeviceStatus(deviceid, &status);
	index += 32;
	buffer[index++] = length;
	memcpy(&buffer[index], mac, length);
	index += length;
	buffer[index++] = (status ==0)?0:1;
	Warn("");
	Print_hex(buffer, index);
	SendCall(buffer, index, id);
}

static void processGetDeviceAck(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall) {
	int deviceid = UtilGetInt32(data + 1);
	int timeout = UtilGetInt32(data + 7);
	Info("deviceid[%d] timeout[%d]", deviceid, timeout);
	unsigned char buffer[512] = { 0x00 };
	int index = 7, status = 0, ret = 0;
	buffer[0] = GAP_GET_DEVICE_ACK_Ret;
	buffer[1] = data[1];
	buffer[2] = data[2];
	buffer[3] = data[3];
	buffer[4] = data[4];
	buffer[5] = data[5];
	buffer[6] = data[6];

	if (timeout > 1000 * 3) {
		buffer[index++] = 1; //timeout value error
		SendCall(buffer, index, id);
		Warn("time out value error!");
		return;
	}

	if (dbDeviceExisted(deviceid) == 0) {
		buffer[index++] = 2; //没有设备
		SendCall(buffer, index, id);
		Warn("no device!");
		return;
	}

	dbGetDeviceStatus(deviceid, &status);
	if (status == 0) {
		buffer[index++] = 3; //device not online
		SendCall(buffer, index, id);
		Warn("device not online!");
		return;
	}

	struct sockaddr_in address;
	unsigned char MacBuffer[32] = { 0x00 };
	char product_id[33] = { 0x00 };
	int maclength = 0;
	ret = dbGetDeviceInfo(deviceid, product_id, MacBuffer, &maclength, &address);
	if (ret != 0) {
		buffer[index++] = 4; //获取设备地址失败
		SendCall(buffer, index, id);
		return;
	}
	Debug("get accesskey address %s:%d", inet_ntoa(address.sin_addr), htons(address.sin_port));
	ret = XGGetAccessKey(&address, timeout, product_id,MacBuffer, maclength);
	Debug("get accesskey ret=%d", ret);
	if (ret > 0) {
		buffer[index++] = 0;
		index += UtilSetInt32(&buffer[index], ret);
	} else {
		buffer[index++] = ret;
	}
	SendCall(buffer, index, id);
	return;
}
static void processSetDeviceAck(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall) {
	int deviceid = UtilGetInt32(data + 1);
	int ack = UtilGetInt32(data + 7);
	int timeout = UtilGetInt32(data + 11);
	Info("deviceid[%d] timeout[%d] ack[%d]", deviceid, timeout, ack);
	unsigned char buffer[512] = { 0x00 };
	int index = 7, status = 0, ret = 0;
	buffer[0] = GAP_SET_DEVICE_ACK_Ret;
	buffer[1] = data[1];
	buffer[2] = data[2];
	buffer[3] = data[3];
	buffer[4] = data[4];
	buffer[5] = data[5];
	buffer[6] = data[6];
	if (timeout > 1000 * 3) {
		buffer[index++] = 1; //timeout value error
		SendCall(buffer, index, id);
		Warn("time out value error!");
		return;
	}

	if (dbDeviceExisted(deviceid) == 0) {
		buffer[index++] = 2; //没有设备
		SendCall(buffer, index, id);
		Warn("no device!");
		return;
	}

	dbGetDeviceStatus(deviceid, &status);
	if (status == 0) {
		buffer[index++] = 3; ////not online
		SendCall(buffer, index, id);
		Warn("device not online!");
		return;
	}

	struct sockaddr_in address;
	ret = dbGetDeviceInfo(deviceid, NULL, NULL, NULL, &address);
	if (ret != 0) {
		buffer[index++] = 4; //获取设备地址失败
		SendCall(buffer, index, id);
		return;
	}
	Debug("Set accesskey address %s:%d ack=%d", inet_ntoa(address.sin_addr), htons(address.sin_port), ack);
	ret = XGSetAccessKey(ack, timeout, &address);
	Debug("Set access key ret=%d", ret);
	buffer[index++] = ret;
	SendCall(buffer, index, id);
	return;
}

static void processGetDeviceSubkey(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall) {
	int deviceid = UtilGetInt32(data + 1);
	int ack = UtilGetInt32(data + 7);
	int timeout = UtilGetInt32(data + 11);
	Info("deviceid[%d] timeout[%d] ack[%d]", deviceid, timeout, ack);
	unsigned char buffer[512] = { 0x00 };
	int index = 7, status = 0, ret = 0;
	buffer[0] = GAP_GET_DEVICE_SUBKEY_Ret;
	buffer[1] = data[1];
	buffer[2] = data[2];
	buffer[3] = data[3];
	buffer[4] = data[4];
	buffer[5] = data[5];
	buffer[6] = data[6];

	if (timeout > 1000 * 3) {
		buffer[index++] = 1; //timeout value error
		SendCall(buffer, index, id);
		Warn("time out value error!");
		return;
	}

	if (dbDeviceExisted(deviceid) == 0) {
		buffer[index++] = 2; //没有设备
		SendCall(buffer, index, id);
		Warn("no device!");
		return;
	}

	dbGetDeviceStatus(deviceid, &status);
	if (status == 0) {
		buffer[index++] = 3; //not online
		SendCall(buffer, index, id);
		Warn("device not online!");
		return;
	}

	struct sockaddr_in address;
	ret = dbGetDeviceInfo(deviceid, NULL, NULL, NULL, &address);
	if (ret != 0) {
		buffer[index++] = 4; //获取设备地址失败
		SendCall(buffer, index, id);
		return;
	}
	Debug("get subkey address %s:%d ack=%d", inet_ntoa(address.sin_addr), htons(address.sin_port), ack);
	ret = XGGetSubKey(ack, timeout, &address);
	Debug("get subkey ret=%d", ret);
	if (ret > 0) {
		buffer[index++] = 0;
		index += UtilSetInt32(&buffer[index], ret);
	} else {
		buffer[index++] = ret;
	}
	SendCall(buffer, index, id);
	return;
}

static void processControlDevice(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall) {
	int deviceid = UtilGetInt32(data + 1);
	unsigned char buffer[512] = { 0x00 };
	int index = 7, status = 0, ret = 0;
	buffer[0] = GAP_CONTROL_DEVICE_Ret;
	buffer[1] = data[1];
	buffer[2] = data[2];
	buffer[3] = data[3];
	buffer[4] = data[4];
	buffer[5] = data[5];
	buffer[6] = data[6];
	Debug("id[%d]", id);

	if (dbDeviceExisted(deviceid) == 0) {
		Debug("Not find device devid=%d",deviceid);
		buffer[index++] = 1; //没有设备
		SendCall(buffer, index, id);
		return;
	}

	ret = dbGetDeviceStatus(deviceid, &status);
	if (ret != 0) {
		Debug("get device status failed devid=%d",deviceid);
		buffer[index++] = 2; //获取设备在线状态失败
		SendCall(buffer, index, id);
		return;
	}

	if (status == 0) {
		Debug("get device status not online devid=%d",deviceid);
		buffer[index++] = 3; //设备不在线
		SendCall(buffer, index, id);
		return;
	}

	ret= XGSendDataToDevice(deviceid, 0, 0, data + 9, datalength - 9, 0);
	Debug("Send control data ret=%d",ret);
	buffer[index++] = 0; //success
	Warn("");
	Print_hex(buffer, index);
	SendCall(buffer, index, id);
	return;
}

static void processDeleteDevice(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall) {
	int deviceid = UtilGetInt32(data + 1);
	unsigned char buffer[512] = { 0x00 };
	unsigned char mac[128] = { 0x00 };
	int length = 0;
	int index = 7, ret = 0;
	buffer[0] = GAP_DELETE_DEVICE_Ret;
	buffer[1] = data[1];
	buffer[2] = data[2];
	buffer[3] = data[3];
	buffer[4] = data[4];
	buffer[5] = data[5];
	buffer[6] = data[6];
	Debug("id[%d]", id);

	if (dbDeviceExisted(deviceid) == 0) {
		buffer[index++] = 1; //没有设备
		SendCall(buffer, index, id);
		Warn("no device!");
		return;
	}

	struct sockaddr_in address;
	ret = dbGetDeviceInfo(deviceid, NULL, mac, &length, &address);
	if (ret != 0) {
		buffer[index++] = 3; //获取设备地址失败
		SendCall(buffer, index, id);
		Warn("get address error");
		return;
	}
	length = length > 32 ? 6 : length;

	dbDeleteDevice(deviceid);
	XGRemoveDevice(&address,deviceid);
	buffer[index++] = 0; //success
	buffer[index++] = length; //mac length
	memcpy(&buffer[index], mac, length);
	index += length;

	Warn("");
	Print_hex(buffer, index);
	
	ret = XlinkSendUdpPipe(buffer, index, -1, 1);
	Debug("delete devcie success ret=%d",ret);
	XlinkSendTcpBroadcast(0, buffer, index);
}

static void processGetGatewayStatue(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall)
{
	Debug("~~~~~~receive gatewaystatue request~~~~~~");
	unsigned char buffer[1024] = { 0x00 };
	int index = 0;
	buffer[index++] = GAP_GET_GATEWAY_STATUE_Ret;
	buffer[index++] = data[1];
	buffer[index++] = data[2];
	buffer[index++] = data[3];
	buffer[index++] = data[4];
	buffer[index++] = data[5];
	buffer[index++] = data[6];
	//buffer[index++] = 0;		// to do ~~~~~~~~~~~
	//Warn("index[%d], id[%d]", index, id);
	//Print_hex(buffer, index);
	FILE *fp; 
	int len,current_version;           
	char buf[1024];  
	if((fp = fopen("/sbin/system_version", "rb")) == NULL){
	Debug("fail to read system_version");
	}
	else{
		while(fgets(buf,1024,fp) != NULL){
			len = strlen(buf);
			buf[len] = '\0'; 
		}
		fclose(fp);
		current_version = atoi(buf);
	}
	Debug("current_version:::::%d||%s",current_version,buf);
	buffer[index++]=(current_version&0xFF00)>>8;
	buffer[index++]=current_version&0x00FF;
	Debug("current_version111:::::%d||%d",current_version^0xFF00,current_version^0x00FF);
	buffer[index++]=0;
	buffer[index++]=0;
	buffer[index++]=0;
	buffer[index++]=0;
	Print_hex(buffer, index);
	Warn("index[%d], id[%d]", index, id);
	SendCall(buffer, index, id);
}


static void processControlLamp(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall)
{
	unsigned char buffer[1024] = { 0x00 };
	int index = 0;
	buffer[index++] = GAP_CONTROL_LAMP_Ret;
	buffer[index++] = data[1];
	buffer[index++] = data[2];
	buffer[index++] = data[3];
	buffer[index++] = data[4];
	buffer[index++] = data[5];
	buffer[index++] = data[6];
	buffer[index++] = 0;		//待修改 
	Warn("index[%d], id[%d]", index, id);
	Print_hex(buffer, index);

	int open = data[7];
	//if (FT_SUCCESS == set_lamp_option(open))
	
	SendCall(buffer, index, id);
}

static void processOpenTmpSSID(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall)
{
	unsigned char buffer[1024] = { 0x00 };
	int index = 0;
	buffer[index++] = GAP_OPEN_TMP_SSID_Ret;
	buffer[index++] = data[1];
	buffer[index++] = data[2];
	buffer[index++] = data[3];
	buffer[index++] = data[4];
	buffer[index++] = data[5];
	buffer[index++] = data[6];
	buffer[index++] = 0;
	Warn("index[%d], id[%d]", index, id);
	Print_hex(buffer, index);

//	ft_tmp_ssid();
	
	SendCall(buffer, index, id);

}

static void processSetOnlySSID(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall)
{
	unsigned char buffer[1024] = { 0x00 };
	int index = 0;
	buffer[index++] = GAP_GET_GATEWAY_STATUE_Ret;
	buffer[index++] = data[1];
	buffer[index++] = data[2];
	buffer[index++] = data[3];
	buffer[index++] = data[4];
	buffer[index++] = data[5];
	buffer[index++] = data[6];
	buffer[index++] = 0;		// to do ~~~~~~~~~~~
	Warn("index[%d], id[%d]", index, id);
	Print_hex(buffer, index);
	
	SendCall(buffer, index, id);
}

static void processOpenEasylink(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall)
{
	unsigned char buffer[1024] = { 0x00 };
	int index = 0;
	buffer[index++] = GAP_OPEN_EASYLINK_Ret;
	buffer[index++] = data[1];
	buffer[index++] = data[2];
	buffer[index++] = data[3];
	buffer[index++] = data[4];
	buffer[index++] = data[5];
	buffer[index++] = data[6];
	buffer[index++] = 0;
	Warn("index[%d], id[%d]", index, id);
	Print_hex(buffer, index);

	//ft_easylink();
	
	SendCall(buffer, index, id);

}

static void processSetSystem(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall)
{
	unsigned char buffer[1024] = { 0x00 };
	
	Warn("id[%d]",  id);
	if (data[7] == 1){
		//ft_system_reboot();
	}else if (data[7] == 2){
		//ft_system_firstboot();
	}
	
}

static void processSetNetMode(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall)
{
	unsigned char buffer[1024] = { 0x00 };
	int index = 0;
	buffer[index++] = GAP_SET_NET_MODE_Ret;
	buffer[index++] = data[1];
	buffer[index++] = data[2];
	buffer[index++] = data[3];
	buffer[index++] = data[4];
	buffer[index++] = data[5];
	buffer[index++] = data[6];
	buffer[index++] = 0;		// to do ~~~~~~~~~~~
	// ft_set_net_mode();
	Warn("index[%d], id[%d]", index, id);
	Print_hex(buffer, index);
	
	SendCall(buffer, index, id);

}
#if 0
void Http_Post(char* head,char* url,char* szJsonData,char* DataBack){
	char m_Result[65535];
	CURL *curl;       
	CURLcode res;
	struct curl_slist *headers = NULL;

	headers = curl_slist_append(headers,"Content-Type:application/json"); 
	if(strcmp(head,"")){
		headers = curl_slist_append(headers,head); 
	}
	
	curl = curl_easy_init();       
	   
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 
	curl_easy_setopt(curl, CURLOPT_URL,url);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	if(strcmp(szJsonData,"")){
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, szJsonData);   
	}
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Post_Callback);       
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &m_Result);
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, 5 );
	curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 3 );
		
	res = curl_easy_perform(curl);      
	if(res != CURLE_OK)      
		printf("curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
	strcpy(((char*)DataBack),(char *)m_Result);
	curl_easy_cleanup(curl); 
}
#else
void Http_Post(char* head,char* url,char* szJsonData,char* DataBack)
{
			
}
#endif

unsigned int Post_Callback(void *buffer, unsigned int size, unsigned int nmemb, void *stream) { 
	strcpy(((char*) stream),(char *)buffer); 
	return nmemb*size; 
} 

static unsigned int downLoadPackage(void *ptr, unsigned int size, unsigned int nmemb, void *userdata)
{
    FILE *fp = (FILE*)userdata;
    unsigned int written = fwrite(ptr, size, nmemb, fp);
    return written;
}

#if 0
void DownLoad_firmware(char* tarURL){
	FILE *fp = fopen("/tmp/openwrt-ramips-mt7628-wrtnode2p-squashfs-sysupgrade.bin", "wb");
    if (! fp)
    {
        Debug("can not create file !!");
    }
	CURL *curl;       
	CURLcode res;
	struct curl_slist *headers = NULL;
	curl = curl_easy_init();  
	curl_easy_setopt(curl, CURLOPT_URL,tarURL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downLoadPackage);       
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, 120 );
	curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 3 );
	res = curl_easy_perform(curl); 
	if(res != CURLE_OK)      
		printf("curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
	curl_easy_cleanup(curl); 
	fclose(fp);  
    Debug("succeed downloading package to /tmp/");
}
#else
	
#endif
void DownLoad_firmware(char* tarURL){
}

#if 0
static void processSetUpdateUrl(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall)
{
	char GetData[1024];
	char auth_buffer[24]={0x00};
	char JsonData[1024]; 
	char header[128]; 
	char URL[256];
	char tarURL[256];
	char tarMD5[40];
	int tarVer;
	int device_id;	
	char access_token[100];
	char mac_tmp[200];
	char* strMAC = "";
	char* strAUTH = "";
	memset(JsonData, 0, sizeof(JsonData)); 
	memset(GetData, 0, sizeof(GetData));	
	unsigned char eth0_mac[6] = {0x00};
	char tmp[32] = {0};	
	get_eth0_mac(tmp, eth0_mac, 32);
	sprintf(mac_tmp,"%X%X%X%X%X%X",eth0_mac[0], eth0_mac[1],eth0_mac[2],eth0_mac[3],eth0_mac[4],eth0_mac[5]);
	strMAC=mac_tmp;
	XlinkGetAuthCode(auth_buffer);
	strAUTH=auth_buffer;
	sprintf(JsonData,"{\"product_id\" : \"1607d4b07f982a001607d4b07f982a01\",\"mac\" : \"%s\",\"authorize_code\" : \"%s\"}",strMAC,strAUTH);
	Http_Post("Content-Type:application/json","http://api.fotile.com:80/v2/device_login",JsonData,GetData);
	Debug("acc_token get ok");
	cJSON *json , *json_value , *root;  
	json = cJSON_Parse(GetData);
	if (!json){
		Error("Error before: [%s]\n",cJSON_GetErrorPtr()); 
		return;
		}
	else{
		if(cJSON_HasObjectItem(json , "access_token")){
			json_value = cJSON_GetObjectItem( json , "access_token");
			if( json_value->type == cJSON_String ){
				strcpy(access_token,json_value->valuestring);
				}
			}else{
			Error("access token get failed!");
			cJSON_Delete(json);  
			return;
			}
		if(cJSON_HasObjectItem(json , "device_id")){
			json_value = cJSON_GetObjectItem( json , "device_id");
			if( json_value->type == cJSON_Number  ){
				device_id = json_value->valueint;
				}
			}else{
			Error("device id get failed!");
			cJSON_Delete(json);  
			return;
			}
        cJSON_Delete(json);  
   	 }  
	FILE *fp; 
	int len;           
	char buf[1024];  
	if((fp = fopen("/sbin/system_version", "rb")) == NULL){
	Debug("fail to read system_version");
	}
	else{
		while(fgets(buf,1024,fp) != NULL){
			len = strlen(buf);
			buf[len] = '\0'; 
		}
		fclose(fp);
		int current_version;
		int tar_version;
		current_version = atoi(buf);
		sprintf(JsonData,"{\"product_id\":\"1607d4b07f982a001607d4b07f982a01\", \"device_id\":\"%d\", \"type\":\"1\", \"current_version\":\"%d\", \"identify\":\"0\"}",device_id,current_version);
		sprintf(header,"Access-Token:%s",access_token);
		sprintf(URL,"http://api.fotile.com:80/v2/upgrade/firmware/check/%d",device_id);
		Http_Post(header,URL,JsonData,GetData);
		Debug("URL get ok");
		root = cJSON_Parse(GetData);
		if (!root){
		Error("Error before: [%s]\n",cJSON_GetErrorPtr()); 
		return;
		}
		else{
			if(cJSON_HasObjectItem(root , "target_version_url")){
				json_value = cJSON_GetObjectItem( root , "target_version_url");
				if( json_value->type == cJSON_String ){
					strcpy(tarURL,json_value->valuestring);
					}
				}
			else{
				Error("target version url get failed!");
				cJSON_Delete(root);  
				return;
			}
			if(cJSON_HasObjectItem(root , "target_version_md5")){
				json_value = cJSON_GetObjectItem( root , "target_version_md5");
				if( json_value->type == cJSON_String ){
					strcpy(tarMD5,json_value->valuestring);
				}
			}
			else{
				Error("target version md5 get failed!");
				cJSON_Delete(root);  
				return;
			}
			if(cJSON_HasObjectItem(root , "target_version")){
				json_value = cJSON_GetObjectItem( root, "target_version");
				if( json_value->type == cJSON_Number  ){
					tarVer = json_value->valueint;
					}
				}
			else{
				Error("target version get failed!");
				cJSON_Delete(root);  
				return;
			}
        	cJSON_Delete(root);  
   	 	}  
		
		tar_version = tarVer;
		Debug("\ncurrent version:%d\ntarget version:%d",current_version,tar_version);
		if(current_version<tar_version){
			DownLoad_firmware(tarURL);
			Notice("firmware download end!");
			sprintf(tmp,"fw_setenv current_version %d",current_version);
			system(tmp);
			ft_update_system(tarMD5,tar_version);
		}
		else{
			Error("firmware version error! update canceled");
		}
		}	

	//ft_set_update_url(url);
	
	//SendCall(buffer, index, id);
}
#else
static void processSetUpdateUrl(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall)
{
	return;
}
#endif
