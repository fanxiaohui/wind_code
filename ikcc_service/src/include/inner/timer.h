#ifndef TIMER_H
#define TIMER_H

/**
* @file timer.h
* @brief 实现定时器相关功能
* @version 1.1
* @author cairj
* @date 2016/09/22
*/


typedef void (*on_time)(const int id);

/**
* @ Description: 启动一个定时器
* @ param func: 回调函数 
* @ param interval: 回调间隔时间 (毫秒)
* @ return: 
*/
int create_timer(on_time func, const int interval, const int id);

#endif /* TIMER_H */
