#ifndef TIMER_H
#define TIMER_H

/**
* @file timer.h
* @brief ʵ�ֶ�ʱ����ع���
* @version 1.1
* @author cairj
* @date 2016/09/22
*/


typedef void (*on_time)(const int id);

/**
* @ Description: ����һ����ʱ��
* @ param func: �ص����� 
* @ param interval: �ص����ʱ�� (����)
* @ return: 
*/
int create_timer(on_time func, const int interval, const int id);

#endif /* TIMER_H */
