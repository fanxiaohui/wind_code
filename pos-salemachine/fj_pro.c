#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <termio.h>

#include "fj_pro.h"
#include "debug.h"
#include "handle.h"
#include "utils.h"

unsigned char fj_cmds[16][3] = {
	{'P','D'},
	{'P','W'},
	{'C','R'},
	{'c','r'},
	{'C','P'},
	{'U','R'},
	{'C','W'},
	{'C','E'},
	{'L','R'},
	{'N','S'},
	{'S','L'} };

unsigned char RSP_PW[6] = {STX,'P','W','0',ETX,'?'};
unsigned char RSP_PD[6] = {STX,'P','D','0',ETX,'?'};
unsigned char RSP_CR_CPOK[10] = {STX,'C','R',0x00,0x00,0x00,0x00,'0',ETX,'?'};
unsigned char RSP_CR_CPNOK[10] = {STX,'C','R',0x04,0x00,0x00,0x00,'0',ETX,'?'};
unsigned char RSP_CR_CP_BAL[13] = {STX,'c','r',0x00,0x00,0x00,0x00,0x01,0x02,0x03,'0',ETX,'?'};
unsigned char RSP_CP_OK[6] = {STX,'C','P','0',ETX,'?'};
unsigned char RSP_CW_OK[6] = {STX,'C','W','0',ETX,'?'};
unsigned char RSP_CW_NOK[6] = {STX,'C','W','1',ETX,'?'};
unsigned char RSP_CE_OK[6] = {STX,'C','E','0',ETX,'?'};
unsigned char RSP_LR_OK[6] = {STX,'L','R','A',ETX,'?'};
unsigned char RSP_NS_OK[6] = {STX,'N','S','0',ETX,'?'};
unsigned char RSP_SL_OK[6] = {STX,'S','L','0',ETX,'?'};

static int fj_handle_pro(int fd, unsigned char *buffer, int length)
{
	int outlen = 0;
	static char pd_end = 0;
	static char cp_ok = 0;
	enum _fj_code cscode = END;
	int balance = 0x00;
	int lrc = 0;
	int ret = -1;
	unsigned char outresp[32] = {0x0};
	char tmp[3] = "";
	
	int i = 0;
	for(i=0; i < END; i++) {
		if(fj_cmds[i][0] == buffer[1] && fj_cmds[i][1] == buffer[2])
			break;
	}
	
	cscode = i;
	switch(cscode) {
		case PD:
			debug(LOG_DEBUG, "<------ PD, reset confirm.\n");
			pd_end = 1;
			memcpy(outresp, RSP_PD, 6);
			outlen = 6;
			break;
		case CR:
			debug(LOG_DEBUG, "<------ CR, check info.\n");
			if(0 == pd_end) {
				// send PW
				memcpy(outresp, RSP_PW, 6);
				outlen = 6;
			} else {
				if(cp_ok) {
					//  recv card ok;
					memcpy(outresp, RSP_CR_CPOK, 10);
				} else {
					// can not recv card
					memcpy(outresp, RSP_CR_CPNOK, 10);
				}
				outlen = 10;
				
				//debug: send card balance 0x01 0x02 0x03
				//memcpy(outresp, RSP_CR_CP_BAL, 13);
				//outlen = 13;
			}
			break;
		case CP:
			debug(LOG_DEBUG, "<------ CP, can recv card\n");
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
			debug(LOG_DEBUG, "<------ CW, money update.\n");
			if(0 == pd_end) {
				memcpy(outresp, RSP_PW, 6);
				outlen = 6;
			} else {
				//parse cw text
				if(length != 10) {
					debug(LOG_ERR, "CW content length error!\n");
					return -1;
				}
				snprintf(tmp,2,"%x",buffer[3]);
				balance = atoi(tmp);
				snprintf(tmp,2,"%x",buffer[4]);
				balance += atoi(tmp)*100;
				snprintf(tmp,2,"%x",buffer[5]);
				balance += atoi(tmp)*10000;
				
				debug(LOG_INFO, "deduct money: %d\n",balance);
				debug(LOG_INFO, "product number: %c%c\n",buffer[6],buffer[7]);
				
				//send to pos ,waiting for 3 seconds 
				ret = send_deduct_money2pos(balance,3000);
				if(ret == 0) {
					// update ok
					memcpy(outresp, RSP_CW_OK, 6);	
				} else {
					//update card fail
					memcpy(outresp, RSP_CW_NOK, 6);
				}
				outlen = 6;
			}
			break;
		case CE:
			debug(LOG_DEBUG, "<------ CE, clear card.\n");
			//clear card
			memcpy(outresp, RSP_CE_OK, 6);
			outlen = 6;
			break;
		case LR:
			debug(LOG_DEBUG, "<------ LR, version confirm.\n");
			//get card version,def 'A'
			memcpy(outresp, RSP_LR_OK, 6);
			outlen = 6;
			break;
		case NS:
			debug(LOG_DEBUG, "<------ NS, not selected.\n");
			// not selected 
			memcpy(outresp, RSP_NS_OK, 6);
			outlen = 6;
			break;
		case SL:
			debug(LOG_DEBUG, "<------ SL, product selected.\n");
			// selected
			cp_ok = 1; //???
			memcpy(outresp, RSP_SL_OK, 6);
			outlen = 6;
			break;
		case END:
			debug(LOG_WARNING, "=====Can not found CMD_SUB [0x%02x][0x%02x]!\n",buffer[1],buffer[2]);
			return -1;
		default:
			debug(LOG_WARNING, "==def===Can not found CMD_SUB [0x%02x][0x%02x]!\n",buffer[1],buffer[2]);
			return -1;
	}

	//calc LRC
	i = 2;
	lrc = outresp[1]; //start from CMD, end at ETX;
	for(i=2; i < outlen-1; i++) {
		lrc ^= outresp[i];
	}
	outresp[outlen-1] = lrc;
	
	debug(LOG_DEBUG, "---->resp to fj:\n");
	debug_buf(outresp,outlen);
	
	ret = write(fd, outresp, outlen);
	fsync(fd);
	return ret;
}

int fj_recv_loop(int fd, int timeout)
{
	int ret = 0, ch_num = 0, stflag = 0 , edflag = 0;
	unsigned char fjbuf[32] = {0};
	unsigned char bak = 0, ch = 0;
	fd_set readfds;
    struct timeval tv;
    if(timeout <= 0)
    	timeout = 100; //every char should be sent within 100 ms
	
	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000)*1000;
		ret = select(fd+1,&readfds,NULL,NULL,&tv);
		if(ret == 0) {
			//one package ending ???
			// restart
			ch = stflag = ch_num = edflag = 0;
			memset(fjbuf,0,sizeof(fjbuf));
			continue;
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
		
		switch(ch) {
			case ENQ:
				if(0 == stflag) {
					debug(LOG_DEBUG, "-----< get ENQ\n");
					bak = ACK;
					write(fd, &bak, 1);
					fsync(fd);
					break;
				} else {
					//ENQ is data content. go down.
				}
				
			case STX:
				if(0 == stflag) {
					debug(LOG_DEBUG, "-----< get STX\n");
					//data start here.
					stflag = 1;
					edflag = 0;
					memset(fjbuf,0,sizeof(fjbuf));
					ch_num = 0;
					fjbuf[ch_num++] = STX;
					break;
				} else {
					//STX is data content, not start flag. go down.
				}
				
			default:
				if(stflag) {
					fjbuf[ch_num++] = ch;
					if(ch_num >= FJ_NORMAL_CMD_LEN) {
						if(fjbuf[1] == 'C' && fjbuf[2] == 'W') {
							if(ch_num >= FJ_CW_CMD_LEN) {
								// CW end.
								edflag = 1;
								break;
							} else {
								continue;
							}
						} else {
							//end. except CW need 10 bytes
							edflag = 1;
							break;
						}
					} else {
						continue;
					}
				} else {
					debug(LOG_DEBUG, "-----< get OTHERS:0x%02x\n",ch);
					break;
				}
		}
		if(edflag) {
			debug(LOG_DEBUG, "get buf length %d,dump content:\n",ch_num);
			debug_buf(fjbuf, ch_num);
			fj_handle_pro(fd, fjbuf, ch_num);
			
			ch = stflag = ch_num = edflag = 0;
			memset(fjbuf,0,sizeof(fjbuf));
		}
	}
	return 0;
}
