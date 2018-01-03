/*
 * dbDevice.h
 *
 *  Created on: 2016年12月20日
 *      Author: john
 */

#ifndef DBDEVICE_H_
#define DBDEVICE_H_

#include <arpa/inet.h>
//return 0 ok , -1 error
extern int dbCreateDeviceTable(void);
extern int dbAddDevice(unsigned int deviceid, char *productid, unsigned char *mac,int maclength,struct sockaddr_in *address);
extern int dbDeleteDevice(unsigned int deviceid);
extern int dbDeviceExisted(unsigned int deviceid);
extern int dbUpdateDeviceAddress(unsigned int deviceid,struct sockaddr_in *address);
extern int dbSetDeviceStatus(unsigned int deviceid,int status);
extern int dbGetDeviceStatus(unsigned int deviceid,int *status);
extern int dbGetDeviceCount();
extern int dbGetAllDeviceID(unsigned char *buffer,int buffersize);
extern int dbGetDeviceInfo(unsigned int deviceid,char *outProductID,unsigned char *outMac,int *outMaclen,struct sockaddr_in *outAddress);

#endif /* DBDEVICE_H_ */
