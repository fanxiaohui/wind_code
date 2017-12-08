#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "uart.h"
#include "pos_pro.h"
#include "debug.h"
#include "config.h"
#include "utils.h"

int send_deduct_money2pos(int money3B, int timeout)
{
	static unsigned short seqnum = 0;
	int fd = -1;
	int ret = -1;
	static unsigned char posmsg[POS_RET_MAX] = {0};
	int poslen = 0;
	
	fd = init_com_dev(POS_DEV, POS_BPS, POS_PAR, POS_BLOCK);
	if(fd <= 0) {
		debug(LOG_ERR, "Pos dev open error:%d\n",errno);
		return -1;
	}
	//clear history
	memset(posmsg, 0, sizeof(posmsg));
	poslen = 0;
	
	//generate new pos deduct msg
	package_deduct_msg(seqnum++, money3B, posmsg, &poslen);
	
	debug(LOG_DEBUG, "====debug, deduct msg to pos:\n");
	debug_buf(posmsg,poslen);
	
	//send to pos
	write(fd, posmsg, poslen);
	
	ret = pos_wait_result(fd, timeout);
	if(ret == 0) {
		close(fd);
		return 0;
	} else if(ret == DEAL_WAITING) {
		// need to waiting 30s for deal result??
		//it's too long for sale machine (5s)
	}
	return -1;
}
