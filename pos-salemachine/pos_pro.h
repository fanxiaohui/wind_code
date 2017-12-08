#ifndef _POS_PRO_H_
#define _POS_PRO_H_

#define PRO_START		0x5A
#define PRO_TYPE		0x01
#define CMD_DED		0xA2
#define RSP_DED		0x52

#define POS_CMD_DED_LEN		12

#define POS_OK_RET_LEN		34
#define POS_DEAL_WAIT_LEN   10
#define POS_NORMAL_LEN		9
#define POS_MIN_RET_LEN		8
#define POS_MAX_RET_LEN		34
#define POS_RET_MAX			64

#define DEDUCT_TIMEOUT	3000 //micro seconds
#define DEAL_WAIT_TIME	30	//seconds
#define MIN_CHAR_INTERVAL	10 //ms

enum _deal_ret {
	SYS_ERROR = -1,
	DEAL_OK = 0,
	DEAL_WAITING = 1,
	DEAL_ERROR = 2,
	MSG_CRC_ERR = 3,
	SYS_TIMEOUT = 0x10,
	SYS_CALL_ERR = 0x11 
};

enum _ret_code  {
	SUCCESS = 0,
	CRC_FAIL,
	SEQ_ERR,
	UN_CMD,
	NO_CARD,
	DEAL_FAIL,
	DEAL_WAIT,
	DEAL_TIMEOUT,
	CARD_NUM_ERR,
	
	SYS_ERR = 0x10,
	POS_ERR1 = 0x20,
	POS_ERR2 = 0x21,
	CARD_RD_ERR = 0x31,
	CARD_LIMIT_QUOTA = 0x32,
	CARD_NO_BALANCE = 0x33,
	CARD_INVALID = 0x34,
	CARD_FORBIDDEN = 0x35,
	CARD_WD_ERR	= 0x41,
	RET_END
};

int package_deduct_msg(unsigned short seq, int money4B, unsigned char *outmsg, int *outlen);
int pos_wait_result(int fd, int timeout);

#endif
