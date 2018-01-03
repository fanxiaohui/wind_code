/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : ft_timer.c
* @ Version       : Initial Draft
* @ Author        : wangzhnegxue
* @ Created       : 2016/9/25
* @ Last Modified :
* @ Description   : ʵ�ּ򵥵Ķ�ʱ������,ÿ����Ӷ�ʱ��ֻ�ܱ�ִ��һ��
* @ function:init_timer ->add_timer -->del_timer
* @ Function List :
              add_timer
              __deinit_timer_system
              del_timer
              init_timer
              __init_timer_system
              main
              test_func
              __timer_handle_thread
              timer_tick_thread
* @ History       :
* @ 1.Date        : 2016/9/25
* @   Author      : wangzhnegxue
* @   Modification: Created file

* @ 2.Date         : 2016/9/27
* @   Author       : wangzhnegxue
    Modification : ����timerʹ������
    ///example
    int example(unsigned long data)
    {
        st_ft_timer_list *timer = (st_ft_timer_list *)data;
        add_timer(timer);
    }

    int main(void)
    {
        st_ft_timer_list timer;

        init_timer(&timer);

        timer.func = example;
        timer.timeout = 1000;
        add_timer(&timer);

        while(1)        
            sleep(1);
    }
*/

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <ft_timer.h>
#include <stdlib.h>
#include <assert.h>
#include "string.h"
#include "print.h"

/** ��̬�������� */
static void __deinit_jiffes();
static int __del_timer(st_ft_timer_list *timer);
static inline unsigned long int __get_current_jiffs(void);
static inline void __increase_current_jiffes(void);
static void __init_jiffes();
static void *__timer_handle_thread(void *args);
static int __timer_pending(st_ft_timer_list *timer);
static void *__timer_tick_thread(void *args);
static int __timer_wait(unsigned int ms);
static int __init_timer_system(void);
static int __deinit_timer_system(void);

/** ��ʱ����������� */
static st_ft_timer_obj s_timer_obj; 

/**
* @ Prototype    : __init_jiffes
 Description  : ��ʼ��ϵͳjiffes
* @  Input        : None
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/26
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static void __init_jiffes()
{
    s_timer_obj.jiffes.timer_tick = 0;
    FT_LOCK_INIT(&s_timer_obj.jiffes.mutex);
}

/**
* @ Prototype    : __deinit_jiffes
 Description  : ϵͳjieffes����
* @  Input        : None
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/26
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static void __deinit_jiffes()
{
    s_timer_obj.jiffes.timer_tick = 0;
    FT_LOCK_DESTORY(&s_timer_obj.jiffes.mutex);
}


/**
* @ Prototype    : __get_current_jiffs
 Description  : ��ȡ��ǰ��jiffesֵ
* @ Input        : void  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/26
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static inline unsigned long int __get_current_jiffs(void)
{
    unsigned long int time_tick;
    FT_LOCK(&s_timer_obj.jiffes.mutex);
    time_tick = s_timer_obj.jiffes.timer_tick;
    FT_UNLOCK(&s_timer_obj.jiffes.mutex);
    
    return time_tick;
}

/**
* @ Prototype    : __increase_current_jiffes
 Description  : ����jiffes+1
* @ Input        : void  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/26
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static inline void __increase_current_jiffes(void)
{
    FT_LOCK(&s_timer_obj.jiffes.mutex);
    ++s_timer_obj.jiffes.timer_tick;
    FT_UNLOCK(&s_timer_obj.jiffes.mutex);
}


/**
* @ Prototype    : __init_timer_system
 Description  : ��ʼ��ϵͳ��ʱ����Դ��
* @ Input        : static void  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/25
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static int __init_timer_system(void)
{
    int ret = FT_SUCCESS;
    st_ft_timer_obj *p_timer_obj = &s_timer_obj;

    /** ������ʼ��ϵͳ��ʱ����Դ */
    if (!p_timer_obj->init_flag) {  
        FT_LOCK_INIT(&p_timer_obj->lock); 

        FT_LOCK(&p_timer_obj->lock);
        
        if (!p_timer_obj->init_flag) {  
            CREATE_LIST_HEAD(&p_timer_obj->timer_vec0);     ///timer vec0

            CREATE_QUEUE(&p_timer_obj->run_queue);  ///��ʼ�����ж���

            __init_jiffes();     ///��ʼ��ʱ��

            ret = SEM_INIT(&p_timer_obj->run_sem, 1);     ///run queue ���ź���
            if (FT_SUCCESS != ret) {
                FT_UNLOCK(&p_timer_obj->lock);                
                Error("init sem failed\n");   
                return FT_FAILURE;
            }
            

            ret = FT_THREAD_CREATE(&p_timer_obj->tick_thread_id, __timer_tick_thread, NULL);        ///clock tick thread
            if (FT_SUCCESS != ret) {
                FT_UNLOCK(&p_timer_obj->lock);                
                Error("create __timer_tick_thread failed\n");   
                return FT_FAILURE;
            }

            ret = FT_THREAD_CREATE(&p_timer_obj->handle_thread_id, __timer_handle_thread, NULL);    ///handle thread
            if (FT_SUCCESS != ret) {
                FT_UNLOCK(&p_timer_obj->lock);                
                Error("create __timer_handle_thread failed\n");   
                return FT_FAILURE;
            }

            p_timer_obj->init_flag = TRUE;
        }
        
        FT_UNLOCK(&p_timer_obj->lock);
    }

    return FT_SUCCESS;
}

/**
* @ Prototype    : __deinit_timer_system
 Description  : ����ϵͳ��ʱ����Դ
* @ Input        : void  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/25
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int __deinit_timer_system(void)
{
    st_ft_timer_obj *p_timer_obj = &s_timer_obj;

    if (FALSE == p_timer_obj->init_flag) {
        Error("timer system not init or has deinit\n");
        return FT_FAILURE;
    }
    
    /** ����ϵͳ��ʱ����Դ */
    FT_LOCK(&p_timer_obj->lock);
    
    if (TRUE == p_timer_obj->init_flag) {
    
        p_timer_obj->thread_exit = TRUE;

        __deinit_jiffes();
                
        DESTORY_LIST_HEAD(&p_timer_obj->timer_vec0);
        
        DESTORY_QUEUE(&p_timer_obj->run_queue);

        p_timer_obj->init_flag = FALSE;
    }
    
    FT_UNLOCK(&p_timer_obj->lock);

    FT_LOCK_DESTORY(&p_timer_obj->lock);

    return FT_SUCCESS;
}

/**
* @ Prototype    : __del_timer
 Description  : ����ʱ��������ɾ��
* @ Input        : st_ft_timer_list *timer  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/27
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static int __del_timer(st_ft_timer_list *timer)
{
    st_ft_timer_obj *p_timer_obj = &s_timer_obj;

    if(__timer_pending(timer)) {
        FT_LOCK(&p_timer_obj->timer_vec0.lock);
            if (__timer_pending(timer)){
                list_del(&timer->list);
            }
        FT_UNLOCK(&p_timer_obj->timer_vec0.lock);
    }
    
    return FT_SUCCESS;
}


/**
* @ Prototype    : __timer_wait
 Description  : �ȴ�ʱ�ӷ���
* @ Input        : unsigned int ms  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/27
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static int __timer_wait(unsigned int ms)
{
    int ret = 0;
    static int T = ((1000- 1000/FT_TIMER_HZ)/FT_TIMER_HZ);
    struct timeval tv = {0};  
    T = (T<=1)?1:T; 
    tv.tv_sec = 0;
    tv.tv_usec = 1000*T;

    select(0, NULL, NULL, NULL, &tv);   ///�ȴ��¼�����

    return FT_SUCCESS;
}
/**
* @ Prototype    : __timer_pending
 Description  : ��ʱ������������
* @ Input        : st_ft_timer_list *timer  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/27
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static int __timer_pending(st_ft_timer_list *timer)
{
    return (timer->list.prev != NULL);
}
/**
* @ Prototype    : __timer_tick_thread
 Description  : ��ʱ��ʵ�֣��δ�ͳ��
* @ Input        : void *args  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/25
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static void *__timer_tick_thread(void *args)
{
    FT_THREAD_DETACH(FT_THREAD_SELF());
    unsigned long int timer_tick = 0;
    
    st_ft_timer_list *pos = NULL, *n = NULL;
    st_ft_timer_obj *p_timer_obj = &s_timer_obj;    


    while(!p_timer_obj->thread_exit)
    {
        __timer_wait(10); ///wait 10ms 
        
        __increase_current_jiffes();  ///����jiffes����

        timer_tick = __get_current_jiffs();

        /** ������ʱ���б�����ʱ�Ķ�ʱ������ִ�ж����� */
        FT_LOCK(&p_timer_obj->timer_vec0.lock);
        list_for_each_entry_safe(pos, n, &p_timer_obj->timer_vec0.list, list){
            if (timer_tick > pos->time_expires) {            
                list_del(&pos->list);     ///ɾ����ʱ��
                
                PUSH_ENQUEUE(pos, list, &p_timer_obj->run_queue);    ///����ִ�ж�����
                
                SEM_POST(&p_timer_obj->run_sem);     ///ִ֪ͨ���߳���Դ����
            }
        }        
        FT_UNLOCK(&p_timer_obj->timer_vec0.lock);     
    }

    FT_THREAD_EXIT(NULL);
}

/**
* @ Prototype    : __timer_handle_thread
 Description  : ��ʱ�������߳�
* @ Input        : void *args  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/25
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static void *__timer_handle_thread(void *args)
{
    FT_THREAD_DETACH(FT_THREAD_SELF());
    st_ft_timer_obj *p_timer_obj = &s_timer_obj;
    st_ft_timer_list *pos = NULL, *n = NULL;
    
    while(1)
    {
        /** �ȴ���Դ���ã���ִ��ע�ắ�� */
        SEM_WAIT(&p_timer_obj->run_sem);

        /** �����п�����Դȫ��������� */        
        
        FT_LOCK(&p_timer_obj->timer_vec0.lock);
        POP_FROMQUEUE(pos, list, &p_timer_obj->run_queue);    
        FT_UNLOCK(&p_timer_obj->timer_vec0.lock);
        while(pos && pos->func) {
            pos->func((unsigned long)pos);             
            
            FT_LOCK(&p_timer_obj->timer_vec0.lock);
            POP_FROMQUEUE(pos, list, &p_timer_obj->run_queue);            
            FT_UNLOCK(&p_timer_obj->timer_vec0.lock);
        }        
    }
}


/**
* @ Prototype    : create_timer
 Description  : ����һ����ʱ��
* @ Input        : st_ft_timer_list *timer  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/25
* @     Author       : wangzhengxue
* @     Modification : Created function

*/

int init_timer(st_ft_timer_list *timer)
{
    ASSERT(timer);

    __init_timer_system();
    
    FT_MEMSET((void *)timer, 0 , sizeof(st_ft_timer_list));   
    INIT_LIST_HEAD(&timer->list);
    
    return FT_SUCCESS;
}

/**
* @ Prototype    : add_timer
 Description  : ��ӵ���ʱ��������,����ִ��
* @ Input        : st_ft_timer_list *timer  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/25
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int add_timer(st_ft_timer_list *timer)
{
    ASSERT(timer);
    
    st_ft_timer_obj *p_timer_obj = &s_timer_obj;
    st_ft_list_head *p_timer_vec; 
    unsigned long int timer_tick = 0;

    __del_timer(timer);
    
    /** ���ö�ʱ�����������붨ʱ���б�������ʱ�� */    
    p_timer_vec = &p_timer_obj->timer_vec0;
    FT_LOCK(&p_timer_vec->lock);
    
    timer_tick = __get_current_jiffs();    
    timer->time_expires = timer_tick + timer->timeout*FT_TIMER_HZ/1000;
    list_add(&timer->list, &p_timer_vec->list);  
    
    FT_UNLOCK(&p_timer_vec->lock);        

    return FT_SUCCESS;
}

/**
* @ Prototype    : del_timer
 Description  : ����ʱ��������ɾ��,������Դ
* @ Input        : st_ft_timer_list *timer  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/25
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int del_timer(st_ft_timer_list *timer)
{
    ASSERT(timer);    
    
    __del_timer(timer);

    return FT_SUCCESS;
}

