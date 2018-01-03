/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : ft_timer.h
* @ Version       : Initial Draft
* @ Author        : wangzhnegxue
* @ Created       : 2016/9/25
* @ Last Modified :
* @ Description   : ft_timer.c header file
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/9/25
* @   Author      : wangzhnegxue
* @   Modification: Created file

*/

#ifndef FT_TIMER_H
#define FT_TIMER_H

#include <semaphore.h>
#include "ft_queue.h"
#include "ikcc_common.h"


///链表操作宏定义
#define CREATE_LIST_HEAD(head) CREATE_QUEUE(head)
#define DESTORY_LIST_HEAD(head) DESTORY_QUEUE(head)
#define FT_TIMER_HZ (100)       ///定时器时钟频率


/** 定时器jiffes定义 */
typedef struct _st_jiffes{
    unsigned long int timer_tick;         ///时钟统计,经过的滴答数
    FT_MUTEX_TYPE mutex;    ///数据锁
}st_jiffes;

/** 定义系统定时器管理单元 */
typedef struct _st_ft_timer_obj {
    int init_flag;         ///timer初始化标识
    int thread_exit;       ///
    st_jiffes    jiffes;
    FT_MUTEX_TYPE lock;    ///数据锁
    st_ft_list_head timer_vec0;      ///优化查找的时间,timer按时间的大小分别在不同的链表中   
    st_ft_list_head timer_vec1;       ///暂时未使用

    st_ft_list_head run_queue;      ///函数执行队列
    sem_t run_sem;                  ///执行队列信号量
    
    FT_PTHREAD_TYPE tick_thread_id;      ///滴答线程号
    FT_PTHREAD_TYPE handle_thread_id;      ///滴答线程号
}st_ft_timer_obj;

/** 定义定时器资源 */
typedef struct _st_ft_timer_node {
    int (*func)(unsigned long);
    unsigned int timeout;
    unsigned int time_expires;
    struct list_head list;
}st_ft_timer_list;


/** 对外接口申明 */
int init_timer(st_ft_timer_list *timer);
int add_timer(st_ft_timer_list *timer);
int del_timer(st_ft_timer_list *timer);


#endif /* FT_TIMER_H */
