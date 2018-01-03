#include "ft_set.h"
#include "print.h"
#include "platform_type.h"

void set_init(st_maxdata_set* p)
{
	pthread_mutex_init(&(p->mu), NULL);
	p->maxData=-1;
	p->size=0;
	int i;
	for (i=0;i<SET_SIZE;i++)
	{
		p->fdarray[i]=-1;
	}
}

void set_destroy(st_maxdata_set* p)
{
	pthread_mutex_destroy(&p->mu);
}

int set_insert(int Q, st_maxdata_set* p)
{
	//需要判断队列满
	pthread_mutex_lock(&(p->mu));
	if(set_find(Q, p))
	{
		pthread_mutex_unlock(&(p->mu));
		return 0;
	}
	p->fdarray[p->size++] = Q;
	p->maxData = Q > p->maxData?Q:p->maxData;
	pthread_mutex_unlock(&(p->mu));
	return 1;
}
int __set_updateMax(st_maxdata_set* p)
{
	p->maxData = -1;
	int i;
	for (i=0;i<p->size;i++)
	{
		p->maxData = p->fdarray[i] > p->maxData?p->fdarray[i]:p->maxData;
	}
}

int set_find(int Q, st_maxdata_set* p)
{
	int i;
	for(i=0; i<p->size; i++)
	{
		if (p->fdarray[i] == Q)
		{
			return 1;
		}
	}
	
	return 0;
}

int set_erase(int Q, st_maxdata_set* p)
{
	//需要判断队列空
	pthread_mutex_lock(&(p->mu));
	int i,j;
	for (i=0;i<p->size;i++)
	{
		if (p->fdarray[i] == Q)
		{
			for (j=i;j+1 < p->size;j++)
			{
				p->fdarray[j] = p->fdarray[j+1];
			}
			p->size-=1;
		}
	}
	if (Q == p->maxData)
		__set_updateMax(p);
	pthread_mutex_unlock(&(p->mu));
	return 0;
}

int set_is_empty(st_maxdata_set* p)
{
	pthread_mutex_lock(&(p->mu));
	if(p->size == 0)
	{	
		pthread_mutex_unlock(&(p->mu));
		return 1;
	}
	pthread_mutex_unlock(&(p->mu));
	return 0;
}

int set_get_elem_num(st_maxdata_set* p)
{
	pthread_mutex_lock(&(p->mu));
	int len = p->size;
	pthread_mutex_unlock(&(p->mu));
	return len;
}

int set_get_max(st_maxdata_set* p)
{
	pthread_mutex_lock(&(p->mu));
	int max = p->maxData;
	pthread_mutex_unlock(&(p->mu));
	return max;
}

int set_at(st_maxdata_set* p, int index)
{
	if (index < 0 || index >= SET_SIZE)
	{
		return -1;
	}
	pthread_mutex_lock(&(p->mu));
	int data = p->fdarray[index];
	pthread_mutex_unlock(&(p->mu));
	return data;
}


void set_dump(st_maxdata_set* p)
{
	pthread_mutex_lock(&(p->mu));
	Info("size[%d], maxData[%d] array:", p->size, p->maxData);
	int i;
	for(i=0;i<p->size;i++)
	{
		Info(" %d", p->fdarray[i]);
	}
	pthread_mutex_unlock(&(p->mu));
}
