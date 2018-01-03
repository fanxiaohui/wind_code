/*
 * md5.h
 *
 *  Created on: 2015年1月9日
 *      Author: john
 */

#ifndef _XMD5_H_
#define _XMD5_H_
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
extern void xlinkHttpMd5Init(XGMD5_CTX *ctx);
extern void XlinkHttpMd5Update(XGMD5_CTX *ctx, unsigned char *data, unsigned int datalen);
extern void XlinkHttpM5dFinal(XGMD5_CTX *ctx, unsigned char *RetBuffer);
extern void xlinkGetMd5(unsigned char *RetBuffer,unsigned char *data,unsigned int datalen);
extern void XlinkHttpM5dFinalString(XGMD5_CTX *ctx, char *RetBuffer);
#ifdef  __cplusplus
}
#endif

#endif /* MD5_H_ */
