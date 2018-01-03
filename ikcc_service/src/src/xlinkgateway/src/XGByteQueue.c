/*
 * XlinkByteQueue.c
 *
 *  Created on: 2016年4月23日
 *      Author: john
 */

#include <stdio.h>
#include <string.h>
#include "XGByteQueue.h"

#define min(m, n) (m) < (n) ? (m) : (n)


int  XGByteQueueTryLock(ByteQueue *queue){
	
	if(queue->lock)
		return 0;
	
	queue->lock = 1;
	
	return 1;
}

void XGByteQueueLock(ByteQueue *queue){
	int i = 0;
	while (queue->lock) {
		i++;
	}
	queue->lock = 1;
}

void XGByteQueueUnLock(ByteQueue *queue){
	queue->lock = 0;
}


unsigned int XGByteQueueInit(ByteQueue *queue) {
	if (queue == NULL) {
		return 1;
	}
//	if (queue->Buffer == NULL) {
//		return 1;
//	}
	memset(queue->Buffer, 0, BQ_BUFFER_SIZE);
	queue->EndIndex = 0;
	queue->StartIndex = 0;
	queue->lock = 0;
	return 0;
}


void XGByteQueueReSet(ByteQueue *queue){
	queue->StartIndex = 0;
	queue->EndIndex = 0;
}

int XGByteQueuePush(ByteQueue *queue, const unsigned char *data, unsigned int length) {
	unsigned int temp = 0;

	if (queue == NULL) {
		return 0;
	}

	if(data == NULL)
		return 0;
	
	if(length == 0)
		return 0;

	length = min(length,BQ_BUFFER_SIZE- queue->EndIndex + queue->StartIndex);
	temp = min(length,BQ_BUFFER_SIZE- (queue->EndIndex & (BQ_BUFFER_SIZE - 1)));
	memcpy(queue->Buffer + (queue->EndIndex & (BQ_BUFFER_SIZE - 1)), data, temp);
	memcpy(queue->Buffer, data + temp, length - temp);
	queue->EndIndex += length;
	return length;
}

unsigned int XGByteQueuePeer(ByteQueue *queue, unsigned char *retBuffer, unsigned int BufferLength) {
	unsigned int length = 0;
	if (queue == NULL) {
		return 0;
	}
//	if (queue->Buffer == NULL) {
//		return 0;
//	}
	BufferLength = min(BufferLength, queue->EndIndex - queue->StartIndex);
	length = min(BufferLength,BQ_BUFFER_SIZE - (queue->StartIndex & (BQ_BUFFER_SIZE - 1)));
	memcpy(retBuffer, queue->Buffer + (queue->StartIndex & (BQ_BUFFER_SIZE - 1)), length);
	memcpy(retBuffer + length, queue->Buffer, BufferLength - length);
	return BufferLength;
}

void XGByteQueuePop(ByteQueue *queue, unsigned int length) {

	if (queue == NULL) {
		return;
	}
//	if (queue->Buffer == NULL) {
//		return;
//	}
	length = min(length, queue->EndIndex - queue->StartIndex);
	queue->StartIndex += length;
	if (queue->EndIndex == queue->StartIndex) {
		XGByteQueueReSet(queue);
	}
	return;
}

unsigned int XGByteQueueSize(ByteQueue *queue) {
	if (queue == NULL) {
		return 0;
	}
//	if (queue->Buffer == NULL) {
//		return 0;
//	}
	return queue->EndIndex - queue->StartIndex;
}

