#include "timer.h"
#include "time.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "ikcc_common.h"
#include "print.h"

typedef struct _st_thread_param{
	on_time func;
	int interval;
	int id;
}st_thread_param;

typedef struct timeval timeval;

static timeval __add(timeval a, timeval b){
	timeval result;
	result.tv_sec = a.tv_sec + b.tv_sec + (a.tv_usec + b.tv_usec) >= 1000000?1:0;
	result.tv_usec = (a.tv_usec + b.tv_usec) % 1000000;
	return result;
}

static timeval __reduce(timeval a, timeval b){
	timeval result;
	result.tv_sec = a.tv_sec - b.tv_sec + (a.tv_usec - b.tv_usec) < 0?-1:0;
	result.tv_usec = (a.tv_usec + 1000000 - b.tv_usec) % 1000000;
	return result;
}


static void* __timer_thread(void* param){
	on_time func = ((st_thread_param*)param)->func;
	int interval = ((st_thread_param*)param)->interval;
	int id = ((st_thread_param*)param)->id;
	FT_FREE(param);
	param = NULL;
	
	timeval std_time;
	gettimeofday(&std_time, NULL);
	//Debug("%d,%d\n", std_time.tv_sec, std_time.tv_usec);
	usleep(interval * 1000);
	while(1){
		timeval tv, now, interval_time;
		gettimeofday(&now, NULL);
		//Debug("%d,%d\n", now.tv_sec, now.tv_usec);
		tv.tv_sec = interval / 1000;
		tv.tv_usec = interval % 1000;

		interval_time = __reduce(__add(__add(tv, tv),std_time), now);
		interval_time = interval_time.tv_sec>0?interval_time:tv;
		//Debug("%d,%d\n", interval_time.tv_sec, interval_time.tv_usec);
		select( 0, 0, 0, 0, &interval_time);
		func(id);
		std_time = now;
		//usleep(interval*1000);
	}

}

int create_timer(on_time function, const int _interval, const int id){
	pthread_t tid;
	
	if(function == NULL || _interval <= 0)
	{
		Warn("param error!function[%p] _interval[%d]", function, _interval);
	}
	st_thread_param *p = FT_MALLOC(sizeof(st_thread_param));
	p->func = function;
	p->interval = _interval;
	p->id = id;
	FT_THREAD_CREATE(&tid, __timer_thread, p);
	return tid;
}

