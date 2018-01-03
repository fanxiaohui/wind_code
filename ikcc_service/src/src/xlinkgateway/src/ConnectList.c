/*
 * ConnectList.c
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#include <string.h>
#include "ConnectList.h"
#include "XGMem.h"
#include "XGByteQueue.h"

Connect_t XGConnectFactory() {
	Connect_t con = (Connect_t) XGMemFactory(sizeof(struct Connect));
	//memset(con, 0, sizeof(struct Connect));
	return con;
}

void XGConnectRelease(Connect_t conn) {
	XGQueueItem temp = NULL;
	temp = conn->client.sendQueue.next;
	while (temp != NULL) {
		conn->client.sendQueue.next = temp->next;
		XGDataQueueRelease(temp);
		temp = conn->client.sendQueue.next;
	}

	temp = conn->server.sendQueue.next;
	while (temp != NULL) {
		conn->server.sendQueue.next = temp->next;
		XGDataQueueRelease(temp);
		temp = conn->server.sendQueue.next;
	}
	XGMemRelease(conn);
	conn = NULL;
}
