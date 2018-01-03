/*
 * ConnectTask.c
 *
 *  Created on: 2017年2月15日
 *      Author: john
 */

#include "ConnectTask.h"
#include <stdio.h>
#include "XGMem.h"



ConnectTask_t XGConnectTaskFactory(){
	ConnectTask_t con = (ConnectTask_t) XGMemFactory(sizeof(struct ConnectTask));
	return con;
}

void XGConnectTaskRelease(ConnectTask_t conn){
	XGMemRelease(conn);
	conn = NULL;
}
