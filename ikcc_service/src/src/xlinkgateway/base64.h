/*
 * base64.h
 *
 *  Created on: 2016年12月13日
 *      Author: john
 */

#ifndef BASE64_H_
#define BASE64_H_

extern char * base64_encode(const unsigned char * bindata, char * base64, int binlength);
extern int base64_decode(const char * base64, unsigned char * bindata);

#endif /* BASE64_H_ */
