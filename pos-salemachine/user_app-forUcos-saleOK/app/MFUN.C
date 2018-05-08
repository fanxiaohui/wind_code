#include "MFUN.H"
#include "config.h"
#include "UART.h"
#include "stdlib.h"

unsigned char fj_cmds[16][3] = {
	{'P','D'},
//	{'P','W'},
	{'C','R'},
//	{'c','r'},
	{'C','P'},
//	{'U','R'},
	{'C','W'},
	{'C','E'},
	{'L','R'},
	{'N','S'},
	{'S','L'} 
};

unsigned char RSP_PW[6] = {STX,'P','W','0',ETX,'?'};
unsigned char RSP_PD[6] = {STX,'P','D','0',ETX,'?'};
unsigned char RSP_CR_CPOK[10] = {STX,'C','R',0x00,0x00,0x00,0x00,'0',ETX,'?'};
unsigned char RSP_CR_CPNOK[10] = {STX,'C','R',0x04,0x00,0x00,0x00,'0',ETX,'?'};
unsigned char RSP_CR_CP_BAL[13] = {STX,'c','r',0x00,0x40,0x00,0x00,0x56,0x34,0x12,'0',ETX,'?'};
unsigned char RSP_CP_OK[6] = {STX,'C','P','0',ETX,'?'};
unsigned char RSP_CW_OK[6] = {STX,'C','W','0',ETX,'?'};
unsigned char RSP_CW_NOK[6] = {STX,'C','W','1',ETX,'?'};
unsigned char RSP_CE_OK[6] = {STX,'C','E','0',ETX,'?'};
unsigned char RSP_LR_OK[6] = {STX,'L','R','A',ETX,'?'};
unsigned char RSP_NS_OK[6] = {STX,'N','S','0',ETX,'?'};
unsigned char RSP_SL_OK[6] = {STX,'S','L','0',ETX,'?'};

u8 glb_vts_buffer[VTS_LEN_MAX+1] = {0};
vu8 GLB_VTS_RX_STA;

u8 POS_DEAL_RET_CODE = 0;
u8 glb_pos_buffer[POS_LEN_MAX+1] = {0};
vu8 GLB_POS_RX_STA;

vu8 VTS_MAIL_FLAG=0;

u32 LEFT = 0;
u32 VTS_BALANCE = 0;
u8 NO_WAIT = 0;

void dump_info(u8 *msg, u8 len);

void MFUNTask(void *pdata)
{

	while(1)
	{
		OSTimeDlyHMSM(0, 0, 2, 0);
	}

}

void Card1_Data_In(u8 Dat)
{
	
}

void dump_info(u8 *msg, u8 len)
{
	u8 i;
	u8 strs[256]={0};
	u8 *pt = strs;
	if(len>80) len=80;
	for(i=0;i<len;i++)
	{
		sprintf((char*)pt,"%02x,",msg[i]);
		pt+=3;
	}
	//vts_uart_send_string(strs,strlen(strs));
	debug("[%d]%s\r\n",len,strs);
}

//1. 一次发送多条指令，会被一次接收完毕（CRC错误）；但只处理第一条（没有CRC）；
//2. 两条命令发送间隔时间必须大于字符间隔时间（100ms）
int handle_fj_pro(u8 *buffer, u8 length)
{
	static u8 pd_end = 0;
	static u8 cp_ok = 0;
	static u8 SL_FLAG = 0;
	
	u8 outlen;
	enum _fj_code cscode = END;
	unsigned char outresp[32] = {0x0};
	u8 lrc = 0;
	int i = 0;
	u32 balance = 0;
	u8 tmp[3] = "";
	u32 *left = NULL;
	u8 merr = 0;
	for(i=0; i < END; i++) {
		if(fj_cmds[i][0] == buffer[1] && fj_cmds[i][1] == buffer[2])
			break;
	}
	
	cscode = (enum _fj_code)i;
	switch(cscode) {
	case PD:
		debug("<-----VTS PD.\r\n");
		pd_end = 1;
		memcpy(outresp, RSP_PD, 6);
		outlen = 6;
		break;
	case CR:
		debug("<-----VTS CR. PD_STA=%d.\r\n",pd_end);
		if(0 == pd_end) {
			// send PW
			memcpy(outresp, RSP_PW, 6);
			outlen = 6;
		} else {
			if(cp_ok) {
				//  can recv card ;
				if(SL_FLAG)
				{
					//SL_FLAG = 0;
					
					//must have balance then canbe saled
					// send card balance 0x65 0x43 0x21
					memcpy(outresp, RSP_CR_CP_BAL, 13);
					outlen = 13;
				}
				else
				{
					memcpy(outresp, RSP_CR_CPOK, 10);
					outlen = 10;
				}
				
			} else {
				// can not recv card
				memcpy(outresp, RSP_CR_CPNOK, 10);
				outlen = 10;
			}
		}
		break;
	case CP:
		debug("<-----VTS CP. PD_STA=%d.\r\n",pd_end);
		if(0 == pd_end) {
			memcpy(outresp, RSP_PW, 6);
			outlen = 6;
		} else {
			// can recv card
			cp_ok = 1;
			memcpy(outresp, RSP_CP_OK, 6);
			outlen = 6;
		}
		break;
	case CW:
		debug("<-----VTS CW. PD_STA=%d.\r\n",pd_end);
		
		if(0 == pd_end) {
			memcpy(outresp, RSP_PW, 6);
			outlen = 6;
		} else {
			//parse cw text
			if(length != 10) {
				//debug(LOG_ERR, "CW content length error!\n");
				return -1;
			}
			snprintf((char*)tmp,2,"%x",buffer[3]);
			balance = atoi((const char*)tmp);
			snprintf((char*)tmp,2,"%x",buffer[4]);
			balance += atoi((const char*)tmp)*100;
			snprintf((char*)tmp,2,"%x",buffer[5]);
			balance += atoi((const char*)tmp)*10000;
			
			debug("deduct money int: %d mao\n",balance);
			debug("product number: %c%c\n",buffer[6],buffer[7]);
			
			//send to pos ,waiting for 4 seconds 
			//ret = send_deduct_money2pos(balance,3000);
			VTS_BALANCE = balance;
			merr = OSMboxPost(Wait_VTS_MsgMbox, (void*)&VTS_BALANCE);
			if(merr != 0)
			{
				debug("VTS CW MailBox Post error %d !\r\n",merr);
				memcpy(outresp, RSP_CW_NOK, 6);
				outlen = 6;
				break;
			}
			NO_WAIT = 0;
			// response vts in main process
			return 0;
		#if 0
			debug("VTS CW MailBox wait POS result...\r\n");
			left = (u32*)OSMboxPend(Wait_POS_MsgMbox, 1200, &merr); //wait 4s
			if(left != NULL && merr == 0) {
				// update ok
				debug("VTS CW recv MailBox OK!\r\n");
				debug("POS deal OK. left money:0x%x,0x%x,0x%x,0x%x\r\n",(*left&0xff000000)>>24,
						(*left&0xff0000)>>16,(*left&0xff00)>>8,*left&0xff);
				memcpy(outresp, RSP_CW_OK, 6);
			} else {
				//update card fail
				debug("VTS CW recv MailBox error %d !\r\n",merr);
				memcpy(outresp, RSP_CW_NOK, 6);
			}
			VTS_MAIL_FLAG = 0;
			outlen = 6;
		#endif
		}
		SL_FLAG = 0;
		break;
	case CE:
		debug("<-----VTS CE PD_STA=%d.\r\n",pd_end);
		if(0 == pd_end) {
				// send PW
				memcpy(outresp, RSP_PW, 6);
				outlen = 6;
		} else {
			//clear card
			memcpy(outresp, RSP_CE_OK, 6);
			outlen = 6;
		}
		
		break;
	case LR:
		debug("<-----VTS LR PD_STA=%d.\r\n",pd_end);
		//get card version,def 'A'
		if(0 == pd_end) {
				// send PW
				memcpy(outresp, RSP_PW, 6);
				outlen = 6;
		} else {
			memcpy(outresp, RSP_LR_OK, 6);
			outlen = 6;
		}
		break;
	case NS:
		debug("<-----VTS NS PD_STA=%d.\r\n",pd_end);
		SL_FLAG = 0;
		// not selected 
		if(0 == pd_end) {
				// send PW
				memcpy(outresp, RSP_PW, 6);
				outlen = 6;
		} else {
			//notify POS to cancel,do not repeat
			
			memcpy(outresp, RSP_NS_OK, 6);
			outlen = 6;
		}
		break;
	case SL:
		debug("<-----VTS SL PD_STA=%d.\r\n",pd_end);
		
		// selected
		if(0 == pd_end) {
				// send PW
				memcpy(outresp, RSP_PW, 6);
				outlen = 6;
		} else {
			SL_FLAG = 1;
			//cp_ok = 1; //???
			memcpy(outresp, RSP_SL_OK, 6);
			outlen = 6;
		}
		break;
	case END:
		//invalid CMD
		debug("<-----VTS Invalid cmd, noresp,0x%x,0x%x\r\n",buffer[1],buffer[2]);
		return -1;
	default:
		//error
		debug("<--!!---VTS enum error!\r\n");
		return -1;
	}
	//calc LRC
	i = 2;
	lrc = outresp[1]; //start from CMD, end at ETX;
	for(i=2; i < outlen-1; i++) {
		lrc ^= outresp[i];
	}
	outresp[outlen-1] = lrc;
	
//	debug("VTS response:\r\n");
//	dump_info(outresp, outlen);
	
	vts_uart_send_string(outresp, outlen);
	return outlen;
}

u8 is_crc_valid(u8 *buffer, u8 length)
{
	u8 i = 0;
	u8 lrc;
	if(length < VTS_LEN_MIN || buffer == NULL)
		return 0;
	
	//calc LRC
	i = 2;
	lrc = buffer[1]; //start from CMD, end at ETX;
	for(i=2; i < length-1; i++) {
		lrc ^= buffer[i];
	}
	if(buffer[length-1] != lrc)
	{
		debug("VTS crc error!\n,calc=0x%x,get 0x%x\r\n",lrc,buffer[length-1]);
		return 0;
	}
	return 1;
}

#define VTS_TT_COUNT	(9)
void vts_recv_task(void *pdata)
{
	u8 timeout = 0;
	u8 ret = 0;
	u8 actsize = 0;
	u32 *left = NULL;
	u8 merr = 0;
	u8 outresp[8] = {0};
	u8 i = 0;
	u8 lrc = 0;
	u8 outlen = 0;

	pdata = pdata;  //clear compile warning
	LED3OFF();
	
	while(1)
	{
		if(!NO_WAIT && VTS_MAIL_FLAG && timeout++ < VTS_TT_COUNT) {
			//had send decuct msg,wait result
			debug("VTS CW MailBox wait POS result...\r\n");
			left = (u32*)OSMboxPend(Wait_POS_MsgMbox, 300, &merr); //wait 1s
			if(left != NULL && merr == 0) {
				// update ok
				debug("VTS CW recv MailBox OK!\r\n");
				debug("POS deal OK. left money:0x%x,0x%x,0x%x,0x%x\r\n",(*left&0xff000000)>>24,
						(*left&0xff0000)>>16,(*left&0xff00)>>8,*left&0xff);
				memcpy(outresp, RSP_CW_OK, 6);
				VTS_MAIL_FLAG = 0;
			} else {
				if(timeout >= VTS_TT_COUNT-1) {
					//update card fail, timeout > VTS_TT_COUNT s
					debug("VTS CW recv MailBox timeout > %ds! NOT send cancel\r\n",VTS_TT_COUNT);
					VTS_MAIL_FLAG = 0; //NOT cancel debug
				
					memcpy(outresp, RSP_CW_NOK, 6);
					VTS_BALANCE = 0;
					NO_WAIT = 1;
				#if 0
					merr = OSMboxPost(Wait_VTS_MsgMbox, (void*)&VTS_BALANCE);
					if(merr != 0)
					{
						debug("VTS NS MailBox Post error %d !\r\n",merr);
					}
					//pos waitting and set VTS_MAIL_FLAG = 0;
				#endif
				} else 
					goto CONTINUE;
			}
			outlen = 6;
			//calc LRC
			i = 2;
			lrc = outresp[1]; //start from CMD, end at ETX;
			for(i=2; i < outlen-1; i++) {
				lrc ^= outresp[i];
			}
			outresp[outlen-1] = lrc;
			vts_uart_send_string(outresp, outlen);

			timeout = 0;
			//VTS_MAIL_FLAG = 0;
		}
	CONTINUE:
		if((GLB_VTS_RX_STA & VTS_FIN_MASK))
		{
			LED3ON();
			
			// clear mask, get length
			actsize = GLB_VTS_RX_STA & (~VTS_FIN_MASK);
			
			debug("vts get len=0x%x\r\n",actsize);
			//handle buffer;
			if(glb_vts_buffer[0] == ENQ && actsize == 1)
			{
				debug("vts response ACK\r\n");
				vts_uart_send_char(ACK);
			}
			else if(glb_vts_buffer[0] == STX)
			{
				ret = is_crc_valid(glb_vts_buffer, actsize);
				if(ret == 1)
				{
				//	dump_info(glb_vts_buffer, actsize);
					handle_fj_pro(glb_vts_buffer, actsize);
				}
			}
			else
			{
				debug("VTS data is invalid,len=0x%x,head=0x%x\r\n",actsize,glb_vts_buffer[0]);
			}
			
			//close interrupt ?
			//clear flag
			GLB_VTS_RX_STA = 0;
			memset(glb_vts_buffer, 0, sizeof(glb_vts_buffer));
			//open interrupt ?
		}
		OSTimeDlyHMSM(0, 0, 0, 10);
		LED3OFF();
	}
}

u8 package_deduct_msg(u16 seq, u32 money4B, u8 *outmsg)
{
	u8 crc = 0;
	u8 i = 0;
	
	outmsg[0] = PRO_START;
	//seq
	outmsg[1] = (seq & 0xff00) >> 8;
	outmsg[2] = (seq & 0x00ff);
	outmsg[3] = PRO_TYPE;
	outmsg[4] = CMD_DED;
	//data len
	outmsg[5] = 0x00;
	outmsg[6] = 0x04;
	//payload
	outmsg[7] = (money4B & 0xff000000) >> 24;
	outmsg[8] = (money4B & 0x00ff0000) >> 16;
	outmsg[9] = (money4B & 0x0000ff00) >> 8;
	outmsg[10] = money4B & 0x000000ff;
	//crc
	crc = outmsg[1];
	for(i=2; i < POS_CMD_DED_LEN-1; i++) {
		crc ^= outmsg[i];
	}
	outmsg[11] = crc;
	
	return POS_CMD_DED_LEN;
}

u8 package_cancel_msg(u8 *inmsg, u8 *outmsg)
{
	u8 crc = 0;
	u8 i = 0;
	if(!inmsg || inmsg[0] != PRO_START)
		return 0;

	memcpy(outmsg, inmsg, POS_CMD_DED_LEN);
	outmsg[4] = CMD_CANCEL;

	//crc
	crc = outmsg[1];
	for(i=2; i < POS_CMD_DED_LEN-1; i++) {
		crc ^= outmsg[i];
	}
	outmsg[11] = crc;
	
	return POS_CMD_DED_LEN;
}

u8 is_valid_posret_msg(u8 *msg, u8 len)
{
	u8 crc = 0;
	u8 i = 0;

	if((len < POS_MIN_RET_LEN) || (len > POS_MAX_RET_LEN))
	{
		return 0;
	}

	if(msg[0] != PRO_START)
		return 0;
	
	crc = msg[1];
	for(i=2; i < len-1; i++) {
		crc ^= msg[i];
	}
	if(crc != msg[len-1]) {
		debug("Pos result msg crc error. crc=0x%02x,get=0x%02x\r\n",crc,msg[len-1]);
		return 0;
	}
	return 1;
}

void pos_recv_task(void *pdata)
{
	u16 seq = 0, cur_seq = 0;
	u8 actsize = 0, len = 0, clen = 0;
	u32 *balance = NULL;
	u32 money = 0;
	u8 merr;
	u8 sendmsg[POS_CMD_DED_LEN+1] = {0};
	u8 cancelmsg[POS_CMD_DED_LEN+1] = {0};
	u8 repeat_count = 1;
	u8 st_ret_code = 0;
	pdata = pdata;  //clear compile warning
	
	LED4OFF();
	while(1)
	{
		//get vts deduct money, 10tick = 1000/30 ms; 1s=300tick
		balance = (u32*)OSMboxPend(Wait_VTS_MsgMbox, 10, &merr);
		if(balance != NULL && merr == 0)
		{
			debug("POS get VTS deduct request,money=%u mao from vts\r\n",*balance);
			if(*balance == 0) {
				//NS, to cancel
				repeat_count = 1;
				st_ret_code = 0;
				if(VTS_MAIL_FLAG) {
					VTS_MAIL_FLAG = 0;
					debug("Send Deal cancel msg to pos. seq=%d\n",cur_seq);
					clen = package_cancel_msg(sendmsg,cancelmsg);
					//send cancel to POS?
					if(clen > 0) pos_uart_send_string(cancelmsg, clen);
				}
			} else {
				if(VTS_MAIL_FLAG) {
					debug("VTS_MAIL_FLAG=1,[%d] continue waiting pos ret\r\n",cur_seq);
				} else {
					debug("send new deduct msg to pos\r\n");
					*balance *= 10;
					memset(sendmsg, 0, sizeof(sendmsg));
					cur_seq = seq;
					len = package_deduct_msg(seq++, *balance, sendmsg);

					//dump_info(sendmsg,len);

					//send to POS uart
					if(len > 0) pos_uart_send_string(sendmsg, len);
					
					VTS_MAIL_FLAG = 1;
					
					repeat_count = 1;
					st_ret_code = 0;
				}
			}
#if 0
			//debug always return OK
			money=17;
			merr = OSMboxPost(Wait_POS_MsgMbox, (void*)&money);
			if(merr != 0)
			{
				debug("DBG:POS MailBox Post error %d\r\n",merr);
			}
#endif
		}
		else
		{
			repeat_count++; //timeout
			//debug("POS MailBox Recv error %d!\r\n",merr);
		}
		
		// wait pos result
		if(((GLB_POS_RX_STA & POS_FIN_MASK)))	
		{
			LED4ON();
			
			actsize = GLB_POS_RX_STA & (~POS_FIN_MASK);
			
			debug("<-----pos seq:0x%x,0x%x, buflen=0x%x\r\n",glb_pos_buffer[1],glb_pos_buffer[2],actsize);
			//check sequence.
			if((cur_seq & 0xff00) >> 8 != glb_pos_buffer[1] ||
				    (cur_seq & 0x00ff) != glb_pos_buffer[2] ) 
			{
				debug("!!!<---RSP Seq error, cur=0x%x,get=0x%x,0x%x\n",cur_seq,glb_pos_buffer[1],glb_pos_buffer[2]);
				goto CLEAR;
			}
			if(is_valid_posret_msg(glb_pos_buffer,actsize) != 1) {
				debug("!!!<---RSP Seq format error!\n");
				goto CLEAR;
			}
			if(POS_DEAL_RET_CODE == POS_SUCCESS)
			{
				st_ret_code = POS_SUCCESS;
				debug("<-----pos ret code:0x%x\r\n",glb_pos_buffer[7]);
				
				money = glb_pos_buffer[18] << 24;
				money += glb_pos_buffer[19] << 16;
				money += glb_pos_buffer[20] << 8;
				money += glb_pos_buffer[21];
				
				debug("<-----pos ret payload money[%d fen]:0x%x,0x%x,0x%x,0x%x\r\n",money,
					glb_pos_buffer[18],glb_pos_buffer[19],glb_pos_buffer[20],glb_pos_buffer[21]);
				
				money = glb_pos_buffer[22] << 24;
				money += glb_pos_buffer[23] << 16;
				money += glb_pos_buffer[24] << 8;
				money += glb_pos_buffer[25];
				
				debug("<-----pos ret payload balance[%d fen]:0x%x,0x%x,0x%x,0x%x\r\n",money,
					glb_pos_buffer[22],glb_pos_buffer[23],glb_pos_buffer[24],glb_pos_buffer[25]);
				
				debug("<-----pos ret time:0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\r\n",glb_pos_buffer[26],glb_pos_buffer[27],
					glb_pos_buffer[28],glb_pos_buffer[29],glb_pos_buffer[30],glb_pos_buffer[31],glb_pos_buffer[32]);
				
				if(1 == VTS_MAIL_FLAG)
				{
					merr = OSMboxPost(Wait_POS_MsgMbox, (void*)&money);
					if(merr != 0)
					{
						debug("POS MailBox Post error %d\r\n",merr);
					}
					//VTS waitting and set VTS_MAIL_FLAG = 0;
				}
				else
				{
					debug("POS response OK, BUT HAD NOT GOT VTS DUDECT CMD!\r\n");
				}
			}
			else if(POS_DEAL_RET_CODE == NO_CARD)
			{
				st_ret_code = NO_CARD;
			}
			else if(POS_DEAL_RET_CODE == DEAL_CANCEL)
			{
				debug("POS RSP, Deal Cancel OK.\r\n");
				st_ret_code = DEAL_CANCEL;
			}
			else if(POS_DEAL_RET_CODE == DEAL_TIMEOUT)
			{
				debug("POS RSP, Deal Timeout.\r\n");
				st_ret_code = DEAL_TIMEOUT;
			}
			else
			{
				st_ret_code = POS_DEAL_RET_CODE;
				debug("POS deal failiure, retcode=0x%x\r\n",POS_DEAL_RET_CODE);
			}
		CLEAR:
			GLB_POS_RX_STA = 0;
			POS_DEAL_RET_CODE = 0;
			memset(glb_pos_buffer, 0, sizeof(glb_pos_buffer));
		}
		if( st_ret_code == NO_CARD &&
				VTS_MAIL_FLAG == 1 && 
					repeat_count < 170  && 
					(repeat_count % 40) == 0)
		{
			debug("POS deal No_card, send repeat deduct msg\r\n");
			//send to POS uart
			pos_uart_send_string(sendmsg, len);
		}
		if(repeat_count > 175)
			st_ret_code = 0;
		
		OSTimeDlyHMSM(0, 0, 0, 10);
		LED4OFF();
	}
}
