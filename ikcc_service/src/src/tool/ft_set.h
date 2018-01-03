#ifndef _FT_SET_H
#define _FT_SET_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define SET_SIZE 1024
typedef struct _st_maxdata_set{
		int fdarray[SET_SIZE];
		int size;
		int maxData;
		pthread_mutex_t mu;
}st_maxdata_set;

void set_init(st_maxdata_set* p);
void set_destroy(st_maxdata_set* p);

int set_insert(int Q, st_maxdata_set* p);

int set_find(int Q, st_maxdata_set* p);
int set_erase(int Q, st_maxdata_set* p);
int set_is_empty(st_maxdata_set* p);
int set_get_elem_num(st_maxdata_set* p);
int set_get_max(st_maxdata_set* p);

void set_dump(st_maxdata_set* p);

#endif

