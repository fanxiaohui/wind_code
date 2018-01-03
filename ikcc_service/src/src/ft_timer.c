/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : ft_timer.c
* @ Version       : Initial Draft
* @ Author        : wangzhnegxue
* @ Created       : 2016/9/25
* @ Last Modified :
* @ Description   : 实现简单的定时器功能,每次添加定时器只能被执行一次
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
    Modification : 增加timer使用例子
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

/** 静态函数申明 */
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

/** 定时器管理对象定义 */
static st_ft_timer_obj s_timer_obj; 

/**
* @ Prototype    : __init_jiffes
 Description  : 初始化系统jiffes
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
 Description  : 系统jieffes销毁
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
 Description  : 获取当前的jiffes值
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
 Description  : 设置jiffes+1
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
 Description  : 初始化系统定时器资源、
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

    /** 创建初始化系统定时器资源 */
    if (!p_timer_obj->init_flag) {  
        FT_LOCK_INIT(&p_timer_obj->lock); 

        FT_LOCK(&p_timer_obj->lock);
        
        if (!p_timer_obj->init_flag) {  
            CREATE_LIST_HEAD(&p_timer_obj->timer_vec0);     ///timer vec0

            CREATE_QUEUE(&p_timer_obj->run_queue);  ///初始化运行队列

            __init_jiffes();     ///初始化时钟

            ret = SEM_INIT(&p_timer_obj->run_sem, 1);     ///run queue 的信号量
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
 Description  : 销毁系统定时器资源
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
    
    /** 销毁系统定时器资源 */
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
 Description  : 将定时器从链表删除
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
 Description  : 等待时钟发生
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

    select(0, NULL, NULL, NULL, &tv);   ///等待事件发生

    return FT_SUCCESS;
}
/**
* @ Prototype    : __timer_pending
 Description  : 定时器在链表处理中
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
 Description  : 定时器实现，滴答统计
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
        
        __increase_current_jiffes();  ///增加jiffes计数

        timer_tick = __get_current_jiffs();

        /** 遍历定时器列表，将超时的定时器放入执行队列中 */
        FT_LOCK(&p_timer_obj->timer_vec0.lock);
        list_for_each_entry_safe(pos, n, &p_timer_obj->timer_vec0.list, list){
            if (timer_tick > pos->time_expires) {            
                list_del(&pos->list);     ///删除定时器
                
                PUSH_ENQUEUE(pos, list, &p_timer_obj->run_queue);    ///放入执行队列中
                
                SEM_POST(&p_timer_obj->run_sem);     ///通知执行线程资源可用
            }
        }        
        FT_UNLOCK(&p_timer_obj->timer_vec0.lock);     
    }

    FT_THREAD_EXIT(NULL);
}

/**
* @ Prototype    : __timer_handle_thread
 Description  : 定时器处理线程
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
        /** 等待资源可用，并执行注册函数 */
        SEM_WAIT(&p_timer_obj->run_sem);

        /** 将所有可用资源全部处理完毕 */        
        
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
 Description  : 创建一个定时器
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
 Description  : 添加到定时器链表中,启动执行
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
    
    /** 设置定时器参数，加入定时器列表，启动定时器 */    
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
 Description  : 将定时器从链表删除,回收资源
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

