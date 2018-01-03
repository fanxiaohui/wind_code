/*
 * XGThread.h
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#ifndef XGTHREAD_H_
#define XGTHREAD_H_


#include <pthread.h>

#define THREAD_HANDLE(name)   void *name(void *args)
#define CREATE_THREAD(fun,param) do{pthread_t __t;pthread_create(&__t,NULL,fun,param);}while(0)


#endif /* XGTHREAD_H_ */
