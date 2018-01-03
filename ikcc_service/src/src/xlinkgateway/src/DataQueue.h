/*
 * DataQueue.h
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#ifndef DATAQUEUE_H_
#define DATAQUEUE_H_

typedef struct XGQueue {
	struct XGQueue *next;
	unsigned int deviceid;
	unsigned int dataSize;
	unsigned char data[];
}*XGQueueItem;

extern XGQueueItem XGDataQueueFactory(int datasize);
extern void XGDataQueueRelease(XGQueueItem item);

#endif /* DATAQUEUE_H_ */
