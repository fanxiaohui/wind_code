/*
 * XlinkByteQueue.h
 *
 *  Created on: 2016年4月23日
 *      Author: john
 */

#ifndef XGBYTEQUEUE_H_
#define XGBYTEQUEUE_H_

#define BQ_BUFFER_SIZE 2048

typedef struct BQ {
	unsigned char Buffer[BQ_BUFFER_SIZE];
	int StartIndex;
	int EndIndex;
	unsigned char lock;
} ByteQueue;


extern unsigned int XGByteQueueInit(ByteQueue *queue);
extern void XGByteQueueReSet(ByteQueue *queue);
extern int XGByteQueuePush(ByteQueue *queue, const unsigned char *data, unsigned int length);
extern unsigned int XGByteQueuePeer(ByteQueue *queue, unsigned char *retBuffer, unsigned int BufferLength);
extern void XGByteQueuePop(ByteQueue *queue, unsigned int length);
extern unsigned int XGByteQueueSize(ByteQueue *queue);
extern int  XGByteQueueTryLock(ByteQueue *queue);
extern void XGByteQueueLock(ByteQueue *queue);
extern void XGByteQueueUnLock(ByteQueue *queue);

#endif /* XGBYTEQUEUE_H_ */
