#ifndef _LEOM_PRO_H_
#define _LEOM_PRO_H_
/*
通信协议格式
长度	2B	      2B	1B	    2B	1B	    4B        	4B	       (LEN-12)B	 2B
定义	Header	LEN	   Version	CMD	Stat	Sequence	Reserve 	Payload	     CRC
描述	头部	命令长度	版本号	命令编码	方向	流水号	预留	数据段	校验位
备注	0xA5A5	从Version起到CRC结束的长度	协议的版本号	不区分方向，应答的一方将该字段原数带回；	0x01:主动发送；0x02:应答数据；	主动发送的一方不断累加，应答的一方原数带回；	默认0x0000_0000	包括通用描述和具体内容	具体验证方式待定
*/

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
#define PACK(n) __attribute__((aligned(n),packed))

#define IN
#define OUT
#define MAX_PAYLEN	(65536)
#define ONLINE_MAX_LEN	(1024)
#define ONE_TCP_MAX_LEN	(1400)

typedef struct _st_pro_hd {
	ushort header;
	ushort len;
	uchar ver;
	ushort cmd;
	uchar stat;
	uint seq;
	uint reserve;
}PACK(1) PRO_HD;

typedef struct _st_pro {
	PRO_HD hd;
	uchar *payload;
	//ushort crc;
}PACK(1) PRO;

void pro_net2host(PRO_HD *phd);

/*
 * switch PRO_HD struct to hexbuf;
 * Then switch to Net-Endian (Big Endian)
 * You can do send(socket,hex,len) directly.
 * length of hex = sizeof(PRO_HD)
*/
void pro_hd2hexbuf(IN PRO_HD *phd, OUT uchar *hex);

/*
 * switch PRO struct to hexbuf;
 * Then switch to Net-Endian (Big Endian)
 * You can do send(socket,hex,len) directly.
 * @len: length of all hexbuf
*/
void pro_pro2hexbuf(IN PRO *pro, OUT uchar *hex, OUT int *len);

/*
 * switch hexbuf to PRO_HD struct;
 * Then switch to Host-Endian (Mips-7688 is Little Endian)
 * You can access the value by phd->xx directly.
*/
void pro_hex2hd(IN uchar *hex, OUT PRO_HD *phd);

/*
 * switch hexbuf to PRO struct;
 * Then switch to Host-Endian (Mips-7688 is Little Endian)
 * You can access the value by pro->xx directly.
 * NOTE: this will malloc for pro->payload which len is pro->hd.len
 * After all you must do free(pro->payload)
 * Before this you must malloc for pro at function outside
*/
void pro_hex2pro(IN uchar *hex, OUT PRO *pro);

void dump_phd(PRO_HD *phd);
void dump_pro(PRO *pro);

#endif
