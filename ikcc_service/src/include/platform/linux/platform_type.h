/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : platform_type.h
* @ Version       : Initial Draft
* @ Author        : wangzhnegxue
* @ Created       : 2016/10/18
* @ Last Modified :
* @ Description   : platform_type.h header file
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/10/18
* @   Author      : wangzhnegxue
* @   Modification: Created file

*/

#ifndef PLATFORM_TYPE_H
#define PLATFORM_TYPE_H

#include <syslog.h>
#include <pthread.h>

/**通用操作重定义*/
#define FT_LOCK_INIT(mutex) 	pthread_mutex_init(mutex,NULL)
#define FT_LOCK_DESTORY(mutex) 	pthread_mutex_destroy(mutex)
#define FT_LOCK(mutex) 		pthread_mutex_lock(mutex)
#define FT_UNLOCK(mutex) 	pthread_mutex_unlock(mutex)
#define FT_MALLOC(size)		malloc(size)
#define FT_FREE(p)			(free(p),(p)=NULL)
#define FT_MEMCPY(dst, src, size) memcpy(dst, src, size)
#define FT_MEMSET(dst, value, size) memset(dst, value, size)
#define FT_STRCMP(s1, s2)	strcmp(s1, s2)
#define FT_STRNCMP(s1, s2, n)	strncmp(s1, s2, n)
#define FT_STRNCPY(s1, s2, n)	strncpy(s1, s2, n)
#define FT_STRCPY(s1, s2)		strcpy(s1, s2)
#define ASSERT(a)	assert(a)
#define FT_THREAD_CREATE(threadid, func, args)	pthread_create(threadid, NULL, func, args)
#define FT_THREAD_EXIT(ret)    pthread_exit(ret)
#define FT_THREAD_DETACH(threadid) 	pthread_detach(threadid)
#define FT_THREAD_JOIN(threadid)	pthread_join(threadid,NULL)
#define FT_THREAD_SELF() pthread_self()
#define SEM_INIT(sem,value)    sem_init(sem,0,value)
#define SEM_POST(sem)  sem_post(sem)
#define SEM_WAIT(sem)   sem_wait(sem)  
#define FT_STRLEN(s)    strlen(s)

/**通用类型重定义*/
typedef pthread_mutex_t FT_MUTEX_TYPE;
typedef pthread_t FT_PTHREAD_TYPE;

#endif /* PLATFORM_TYPE_H */

