/*
 * ConnectTask.h
 *
 *  Created on: 2017年2月15日
 *      Author: john
 */

#ifndef SRC_CONNECTTASK_H_
#define SRC_CONNECTTASK_H_


typedef enum Connect_Task_Stat{
	E_START_CONNECT,
}Connect_Task_Stat_t;

typedef struct ConnectTask {
	struct ConnectTask *next;
	unsigned int device_id;
	int fd;
	Connect_Task_Stat_t stata;
}*ConnectTask_t;

extern ConnectTask_t XGConnectTaskFactory();
extern void XGConnectTaskRelease(ConnectTask_t conn);


#endif /* SRC_CONNECTTASK_H_ */
