/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : msg_transfer.h
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/18
* @ Last Modified :
  brief   : 消息分发模块头文件,包含消息相关的结构体函数定义
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/9/18
* @   Author      : wangzhengxue
* @   Modification: Created file

*/

#ifndef MSG_TRANSFER_H
#define MSG_TRANSFER_H

#include "ft_queue.h"
#include "ikcc_common.h"

#define MAX_MSG_THREAD	(32)	///可创建消除处理线程的最大线程数


/**消息队列结构*/
typedef struct _st_cmd_queue_service {	    
	st_ft_list_head dev_msg_queue;		///设备命令消息队列
	
	st_ft_list_head clound_msg_queue;		///云端命令消息队列

	FT_PTHREAD_TYPE	thread[MAX_MSG_THREAD];	///消息处理线程ID
}st_cmd_queue_service;


/**
* @ Prototype    : msg_transfer_init
 Description  : :消息分发初始化
* @ Input        : void  
* @  Output       : None
* @  Return Value : FT_SUCCESS,消息分发初始化成功;FT_FAILURE,消息分发初始化失败
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int msg_transfer_init(void);


/**
* @ Prototype    : push_dev_msg
 Description  : 通过此接口将上报的消息放入消息队列
* @ Input        : st_msg_entity* p_msg_entity  
* @  Output       : None
* @  Return Value : FT_SUCCESS,加入设备消息队列成功;FT_FAILURE,加入设备消息队列失败
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int push_dev_msg(st_msg_entity* p_msg_entity);


/**
* @ Prototype    : push_cloud_msg
 Description  : 通过此接口将云端消息放入消息云端消息队列
* @ Input        : st_msg_entity* p_msg_entity  
* @  Output       : None
* @  Return Value : FT_SUCCESS,加入云端消息队列成功;FT_FAILURE,加入云端消息队列失败
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int push_cloud_msg(st_msg_entity* p_msg_entity);


#endif /* MSG_TRANSFER_H */
