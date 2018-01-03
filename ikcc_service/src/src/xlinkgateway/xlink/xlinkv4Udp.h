/*
 * xlinkv4Udp.h
 *
 *  Created on: 2016-11-24
 *      Author: john
 */

#ifndef XLINKV4UDP_H_
#define XLINKV4UDP_H_

#include "xlink_type.h"
#include "xlink_client.h"
#include "xlink_data.h"

extern XLINK_FUNC void XlinkEnableSubAndScan(void);
extern XLINK_FUNC void XlinkDisableSubAndScan(void);

extern XLINK_FUNC void XlinkV4UdpInit(void);
extern XLINK_FUNC void XlinkV4UdpUnInit(void);
extern XLINK_FUNC void XlinkProcessUdpDataV4(unsigned char * Buffer, unsigned int BufferLen, xlink_addr *addr);
extern XLINK_FUNC void XlinkResend(void);
extern XLINK_FUNC  int XlinkClientSendUdpDataPointSync_V2(unsigned char* DataPoint, unsigned int DataPointSize, unsigned char flag);

#endif /* XLINKV4UDP_H_ */
