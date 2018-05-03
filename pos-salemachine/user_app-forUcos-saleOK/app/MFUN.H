#ifndef  MFUN_H
#define MFUN_H
#include "config.h"

void vts_recv_task(void *pdata);
void pos_recv_task(void *pdata);
void MFUNTask(void *pData);
void FoodPriceDs(u8 *pKeyOK, u8 *pKeyValue, u16 CardMoney);
void Card1_Data_In(u8 Dat);

struct StatePraStr/*״̬����*/
{
    u8 State_KeyValue; /*����ֵ*/
    u8 State_Channel;  /*����ͨ��*/
    u16 State_Money;/*���ۼ۸�*/

};

#define STX	0x02
#define ETX	0x03
#define ENQ	0x05
#define ACK	0x06
#define NAK	0x15

#define VTS_FIN_MASK	(1<<7)
#define VTS_LEN_MAX	(127)
#define VTS_LEN_MIN	5

#define POS_FIN_MASK	(1<<7)
#define POS_LEN_MAX	(127)


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


enum _fj_code {
	PD = 0,
//	PW,
	CR,
//	cr,
	CP,
//	UR,
	CW,
	CE,
	LR,
	NS,
	SL,
	END
};

enum _ret_code  {
	POS_SUCCESS = 0,
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
	DEAL_CANCEL	= 0x42,
	UNSUPPORT = 0xf0,
	RET_END
};

extern u8 POS_DEAL_RET_CODE;
extern u8 glb_vts_buffer[VTS_LEN_MAX+1];
extern vu8 GLB_VTS_RX_STA;
extern u8 glb_pos_buffer[POS_LEN_MAX+1];
extern vu8 GLB_POS_RX_STA;
extern vu8 VTS_MAIL_FLAG;
#endif
