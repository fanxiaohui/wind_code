/*
 * GAProtocol.h
 *
 *  Created on: 2016年12月20日
 *      Author: john
 */

#ifndef GAPROTOCOL_H_
#define GAPROTOCOL_H_



typedef void (*FunSendCall)(unsigned char *data, unsigned int datalength, int id);

extern void GAPStartProtocol(unsigned char *data, unsigned int datalength, int id, FunSendCall SendCall);



extern int  UtilGetInt32(unsigned char *buffer);
extern int  UtilGetInt16(unsigned char *buffer);
extern int UtilSetInt32(unsigned char *buffer,unsigned int data);
extern int UtilSetInt16(unsigned char *buffer,unsigned short data);
void Http_Post(char* head,char* url,char* szJsonData,char* DataBack);
unsigned int Post_Callback(void *buffer, unsigned int size, unsigned int nmemb, void *stream);
static unsigned int downLoadPackage(void *ptr, unsigned int size, unsigned int nmemb, void *userdata);
void DownLoad_firmware(char* tarURL);


#endif /* GAPROTOCOL_H_ */
