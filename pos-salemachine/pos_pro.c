#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "pos_pro.h"
#include "debug.h"
#include "utils.h"

int package_deduct_msg(unsigned short seq, int money4B, unsigned char *outmsg, int *outlen)
{
	unsigned char msg[16] = {0};
	unsigned char crc = 0;
	int i = 0;
	
	msg[0] = PRO_START;
	//seq number
	msg[1] = (seq & 0xff00) >> 8;
	msg[2] = seq & 0x00ff;
	msg[3] = PRO_TYPE;
	msg[4] = CMD_DED;
	//data len
	msg[5] = 0x00;
	msg[6] = 0x04;
	//payload
	msg[7] = (money4B & 0xff000000) >> 24;
	msg[8] = (money4B & 0x00ff0000) >> 16;
	msg[9] = (money4B & 0x0000ff00) >> 8;
	msg[10] = money4B & 0x000000ff;
	//crc
	crc = msg[1];
	for(i=2; i < POS_CMD_DED_LEN-1; i++) {
		crc ^= msg[i];
	}
	msg[11] = crc;
	
	memcpy(outmsg, msg, POS_CMD_DED_LEN);
	*outlen = POS_CMD_DED_LEN;
	
	return 0;
}

static unsigned char calc_crc(unsigned char *msg, int len)
{
	int i = 0;
	unsigned char crc = 0;
	if(!msg || len <= POS_MIN_RET_LEN)
		return 0;

	crc = msg[1];
	for(i=2; i < len-1; i++) {
		crc ^= msg[i];
	}
	return crc;
}

int is_valid_posret_msg(unsigned char *msg, int len)
{
	unsigned char crc = 0;
	int i = 0;
	//debug
	return 0;
	
	if(len < POS_MIN_RET_LEN || len > POS_MAX_RET_LEN)
		return -1;
	if(msg[0] != PRO_START)
		return -1;
	
	debug(LOG_DEBUG, "<-----POS result:\n");
	debug_buf(msg, len);
	
	crc = msg[1];
	for(i=2; i < len-1; i++) {
		crc ^= msg[i];
	}
	if(crc != msg[len-1]) {
		debug(LOG_ERR, "Pos result msg crc error. crc=0x%02x,get=0x%02x\n",crc,msg[len-1]);
		return -1;
	}
	return 0;
}

int pos_wait_result(int fd, int timeout)
{
	int ret = 0, ch_num = 0, stflag = 0 , edflag = 0;
	unsigned char posret[POS_RET_MAX] = {0};
	unsigned char ch = 0;
	int paylen = 0;
	int retcode = 0;
	
	fd_set readfds;
    struct timeval tv;
    if(timeout <= 0)
    	timeout = DEDUCT_TIMEOUT; //first waiting time

	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000)*1000;
		ret = select(fd+1,&readfds,NULL,NULL,&tv);
		if(ret == 0) {
			//one package ending ???
			stflag = edflag = 0;
			debug(LOG_ERR, "POS waiting deduct result timeout,had recved %d bytes!\n",ch_num);
			return -1;
		} else if(ret < 0) {
			if(errno == EINTR) {
				debug(LOG_WARNING, "######cmd get_char interrupt by signal #####\n");
				continue;
			} else
				debug(LOG_ERR, "cmd get_char error,%d:%s\n",errno,strerror(errno));

 			return -1;  // error or closed
		}
		//run here! must be OK ?
		ret = read(fd,&ch,1);
		if(ret != 1) {
			debug(LOG_ERR, "cmd get_char error,%d:%s\n",errno,strerror(errno));
			return -1;
		}
		if(ch == PRO_START) {
			stflag = 1;
			posret[ch_num++] = ch;
			//1. waiting 3s for the first char
			//2. every char should be sent within 10 ms
			timeout = MIN_CHAR_INTERVAL + MIN_CHAR_INTERVAL; //recv safely
			continue;
		}
		if(stflag) {
			posret[ch_num++] = ch;
			if(ch_num >= POS_MIN_RET_LEN) {
				//recv retcode.
				paylen = (posret[5] << 8) + posret[6];
				retcode = posret[7];
				if(paylen == 26 && retcode == SUCCESS) {
					if(ch_num >= POS_OK_RET_LEN) {
						//decode ret OK. recv end.
						if(0 != is_valid_posret_msg(posret, ch_num)) {
							return MSG_CRC_ERR;
						}
						debug(LOG_DEBUG, "<-----pos ret ok. seq:0x%x,0x%x\n",posret[1],posret[2]);
						debug(LOG_DEBUG, "<-----pos ret code:0x%x\n",posret[7]);
						debug(LOG_DEBUG, "<-----pos ret payload money:0x%x,0x%x,0x%x,0x%x\n",posret[18],posret[19],posret[20],posret[21]);
						debug(LOG_DEBUG, "<-----pos ret payload balance:0x%x,0x%x,0x%x,0x%x\n",posret[22],posret[23],posret[24],posret[25]);
						debug(LOG_DEBUG, "<-----pos ret time:0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
								posret[26],posret[27],posret[28],posret[29],posret[30],posret[31],posret[32]);
						
						return DEAL_OK;
					}
				} else if(paylen == 2 && retcode == DEAL_WAIT) {
					if(ch_num >= POS_DEAL_WAIT_LEN) {
						//deal waiting. recv end.
						if(0 != is_valid_posret_msg(posret, ch_num)) {
							return MSG_CRC_ERR;
						}
						
						debug(LOG_DEBUG, "<-----pos deal waiting %d seconds\n",posret[8]);
	
						return DEAL_WAITING;
					}
				} else if(paylen == 1 && retcode >= CRC_FAIL && retcode <= CARD_WD_ERR) {
					if(ch_num >= POS_NORMAL_LEN) {
						//paylen=1. recv end.
						if(0 != is_valid_posret_msg(posret, ch_num)) {
							return MSG_CRC_ERR;
						}
						
						debug(LOG_INFO, "<------ POS deal err,retcode=0x%x\n",retcode);
						return DEAL_ERROR;
					}
				} else {
					//error paylen
					debug(LOG_ERR, "<------ POS result err,paylen=0x%x,retcode=0x%x\n",paylen,retcode);
					return -1;
				}
			}
		} else {
			//other unknow char
		}
	}
	return -1;
}
