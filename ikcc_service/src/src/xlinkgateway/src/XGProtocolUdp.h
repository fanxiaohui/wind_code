/*
 * XGProtocolUdp.h
 *
 *  Created on: 2016年12月12日
 *      Author: john
 */

#ifndef XGPROTOCOLUDP_H_
#define XGPROTOCOLUDP_H_

#include "XGCore.h"


extern void XGStartProtocolUdp(XGCoreCtx core,unsigned char *buffer,unsigned int bufferlength,struct sockaddr_in *addr);


#endif /* XGPROTOCOLUDP_H_ */
