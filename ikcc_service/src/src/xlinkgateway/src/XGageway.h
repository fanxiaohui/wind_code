/*
 * XGageway.h
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#ifndef XGAGEWAY_H_
#define XGAGEWAY_H_

#include <arpa/inet.h>

typedef struct {
	int (*OnDeviceLogin)(int deviceid, int result, struct sockaddr_in *address);
	void (*OnLoginout)(int deviceid, int result);
	void (*OnRecvDataFormDevice)(int deviceid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype);
	void (*OnRecvDataFormCloud)(int deviceid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype);
	int (*OnDiscoverDevice)(char *productid, unsigned char *macString, unsigned int maclen, int deviceid, struct sockaddr_in *address);
	int (*OnDeviceRestartNotify)(char *gatewayid, char *productid, unsigned char *macString, unsigned int maclen, int deviceid, struct sockaddr_in *address);
	int (*OnRecvUdpData)(unsigned char *data,unsigned int datalength,struct sockaddr_in *address);
} XGConfig;

extern int XGInit(char *corp_id, char *pid, char *gatewayid, XGConfig *config);
extern int XGUninit();
extern int XGSendDataToDevice(int formid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype);
extern int XGSendDataToCloud(int formid, int toid, short msgid, unsigned char *data, unsigned int datalength, int pipetype);
extern int XGDisconnect(int deviceid);
extern int XGSendUdpData(unsigned char *data, int datalength, struct sockaddr_in *address);
extern int XGDiscover(void);
extern int XGRestartNotify(void);
extern int XGRemoveDevice(struct sockaddr_in *address,int device_id);
extern int XGGetAccessKey(struct sockaddr_in *addr, unsigned int timeoutms,const char *product_id,unsigned char *mac,unsigned int maclength);
extern int XGSetAccessKey(int accesskey, unsigned int timeoutms, struct sockaddr_in *addr);
extern int XGGetSubKey(int accesskey, unsigned int timeoutms, struct sockaddr_in *addr);
extern int XGSetHost(char *host);
#endif /* XGAGEWAY_H_ */
