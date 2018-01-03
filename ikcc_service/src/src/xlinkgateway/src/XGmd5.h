/*
 * md5.h
 *
 *  Created on: 2015年1月9日
 *      Author: john
 */

#ifndef _XGMD5_H_
#define _XGMD5_H_
#ifdef  __cplusplus
extern "C" {
#endif
//#include "Xlink_Head_Adaptation.h"

//#ifdef __alpha


//#else
//typedef unsigned long uint32;
//#endif

struct XGMD5Context {
	 unsigned int  buf[4];
	 unsigned int  bits[2];
	unsigned char in[64];
};
/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct XGMD5Context XGMD5_CTX;



extern void xlinkGetMd5(unsigned char *RetBuffer,unsigned char *data,unsigned int datalen);

#ifdef  __cplusplus
}
#endif

#endif /* MD5_H_ */
