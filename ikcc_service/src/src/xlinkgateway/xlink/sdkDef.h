#ifndef SDKDEF_H_
#define SDKDEF_H_

#include "Xlink_Head_Adaptation.h"
#include "xlink_client.h"

#define  XLINK_UDP_VERSION  4

//--------------tool------------------------
#define WORDS_BIGENDIAN  0

#if WORDS_BIGENDIAN ==1
typedef volatile union {
	unsigned short data;
	struct {
		unsigned char h;
		unsigned char l;
	}byte;
}ShortFormat;
#else
typedef volatile union {
	unsigned short data;
	struct {
		unsigned char l;
		unsigned char h;
	} byte;
} ShortFormat;
#endif

#if WORDS_BIGENDIAN ==1
typedef volatile union {
	unsigned int data;
	struct {
		unsigned char h_h;
		unsigned char h_l;
		unsigned char l_h;
		unsigned char l_l;
	}byte;
}Int32Format, AppId;
#else
typedef volatile union {
	unsigned int data;
	struct {
		unsigned char l_l;
		unsigned char l_h;
		unsigned char h_l;
		unsigned char h_h;
	} byte;
} Int32Format, AppId;
#endif

typedef struct AgreementUdpHeader {
	unsigned char version :8;
	//flag 1
#if WORDS_BIGENDIAN ==1
	unsigned char flag1_res :3;
	unsigned char encode :1;
	unsigned char response :1;
	unsigned char isResend :1;
	unsigned char Priorty :1;
	unsigned char Ack :1;
#else
	unsigned char Ack :1;
	unsigned char Priorty :1;
	unsigned char isResend :1;
	unsigned char response :1;
	unsigned char encode :1;
	unsigned char flag1_res :3;
#endif
	unsigned short messageid;
	unsigned short bodylength;
	unsigned char cmd :8;
	unsigned char flag2 :8;
	unsigned char data[];
}*AgreementUdpHeader_t;

typedef struct SendDataList {
	struct SendDataList *Next;
	AgreementUdpHeader_t m_data;
	unsigned int users;
	unsigned char  data[];
} SendDataList, *SendData_Item;

#define  BIT_EQUAL_1(value,bit)  (((value) &(0x01<<bit))?1:0)
#define  BIT_SET_1(value,bit)    (value=((value) |(0x01<<bit)))
#define  BIT_SET_0(value,bit)    (value=((value) &(~(0x01<<bit))))

signed int NShtonlInt32(signed int s);
signed short NShtonlInt16(signed short s);
extern int UtilHexToString(char *string, const unsigned char *hexBuffer, const unsigned int hexlength);
extern unsigned char UtilStringToHex(char *string);

#endif /* SDKDEF_H_ */
