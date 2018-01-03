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


///��������궨��
#define CREATE_LIST_HEAD(head) CREATE_QUEUE(head)
#define DESTORY_LIST_HEAD(head) DESTORY_QUEUE(head)
#define FT_TIMER_HZ (100)       ///��ʱ��ʱ��Ƶ��


/** ��ʱ��jiffes���� */
typedef struct _st_jiffes{
    unsigned long int timer_tick;         ///ʱ��ͳ��,�����ĵδ���
    FT_MUTEX_TYPE mutex;    ///������
}st_jiffes;

/** ����ϵͳ��ʱ������Ԫ */
typedef struct _st_ft_timer_obj {
    int init_flag;         ///timer��ʼ����ʶ
    int thread_exit;       ///
    st_jiffes    jiffes;
    FT_MUTEX_TYPE lock;    ///������
    st_ft_list_head timer_vec0;      ///�Ż����ҵ�ʱ��,timer��ʱ��Ĵ�С�ֱ��ڲ�ͬ��������   
    st_ft_list_head timer_vec1;       ///��ʱδʹ��

    st_ft_list_head run_queue;      ///����ִ�ж���
    sem_t run_sem;                  ///ִ�ж����ź���
    
    FT_PTHREAD_TYPE tick_thread_id;      ///�δ��̺߳�
    FT_PTHREAD_TYPE handle_thread_id;      ///�δ��̺߳�
}st_ft_timer_obj;

/** ���嶨ʱ����Դ */
typedef struct _st_ft_timer_node {
    int (*func)(unsigned long);
    unsigned int timeout;
    unsigned int time_expires;
    struct list_head list;
}st_ft_timer_list;


/** ����ӿ����� */
int init_timer(st_ft_timer_list *timer);
int add_timer(st_ft_timer_list *timer);
int del_timer(st_ft_timer_list *timer);


#endif /* FT_TIMER_H */
