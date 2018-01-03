/*
 * XDevice.h
 *
 *  Created on: 2016年12月14日
 *      Author: john
 */

#ifndef XDEVICE_H_
#define XDEVICE_H_

extern void XDeviceStart(char *pid,char *pkey,unsigned char *mac,int maclen);
extern void XDeviceStop();
extern void XDAddSendData(unsigned char *data,unsigned int datalength);
extern void XDNewDeviceJoinNotify(int deviceid,char *productid,unsigned char *mac,unsigned int maclength);
extern void XDControlData(int deviceid,unsigned char *data,unsigned int datalegnth);
extern void XDResetSdk();
extern void XDSetHost(char *host);
extern int AlowDeviceJoin(int timeout);
#endif /* XDEVICE_H_ */
