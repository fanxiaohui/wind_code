/*
 * DataQueue.c
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#include "DataQueue.h"
#include <string.h>
#include "XGMem.h"

XGQueueItem XGDataQueueFactory(int datasize) {
	XGQueueItem ret = (XGQueueItem) XGMemFactory(sizeof(struct XGQueue) + datasize);
	if (ret != NULL) {
		ret->next = NULL;
	}
	return ret;
}

void XGDataQueueRelease(XGQueueItem item) {
	XGMemRelease(item);
	item = NULL;
}
