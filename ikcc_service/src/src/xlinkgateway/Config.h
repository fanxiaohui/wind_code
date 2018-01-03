/*
 * XDConfig.h
 *
 *  Created on: 2016年12月13日
 *      Author: john
 */

#ifndef XDCONFIG_H_
#define XDCONFIG_H_

extern int XDReadBase64Decode(char *key,unsigned char *value,int size);
extern int XDWriteBase64Encode(char *key,unsigned char *value,int size);

extern int XDReadString(char *key, char *value,int size);
extern int XDWriteString(char *key, char *value);


#endif /* XDCONFIG_H_ */
