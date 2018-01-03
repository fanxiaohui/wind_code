/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : msg_transfer.c
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/19
* @ Last Modified :
  brief   : 实现消息分发功能
* @ Function List :
              message_handle_thread
              msg_transfer_init
              push_cloud_msg
              push_dev_msg
* @ History       :
* @ 1.Date        : 2016/9/19
* @   Author      : wangzhengxue
* @   Modification: Created file

*/

#include <stdlib.h>
#include "msg_transfer.h"


/*-------------------------------------------------------*/
/**静态全局变量定义*/

static st_cmd_queue_service cmd_service;    ///消息服务体定义


/*-------------------------------------------------------*/
/**静态函数定义*/

static void *message_handle_thread(void *args);     ///消息处理线程申明


/**
* @ Prototype    : message_handle_thread
* @  Description  : 消息分发线程,从设备消息队列获云端消息队列取出数据,将数据发送给下一个模块，
                    线程不会退出，当发现有消息时会将所有消息全部处理才会进行休眠
* @ Input        : void *args  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static void *message_handle_thread(void *args)
{
	st_msg_entity *p_msg_entity = NULL;		///消息指针申明
	
	while(1) {	
		/**处理设备端消息*/		
		POP_FROMQUEUE(p_msg_entity,list,&cmd_service.dev_msg_queue);	///从设备消息队列将消息取出，移除链表	
		
		while(p_msg_entity) {	///消息非空将一直处理，直到所有完成，保证实时性

			ds_send_msg_tocloud(p_msg_entity);	///将设备消息转发给数据服务模块，交由后端进一步处理
			
			FT_FREE(p_msg_entity);		///释放消息内存	
			p_msg_entity = NULL;		///防止野指针

			/**获取下一个设备消息*/
			POP_FROMQUEUE(p_msg_entity,list,&cmd_service.dev_msg_queue);			
		}


		/**处理云端消息*/		
		POP_FROMQUEUE(p_msg_entity,list,&cmd_service.clound_msg_queue);	///获取云端消息
		
		while(p_msg_entity) {	///消息非空，处理
		
			hw_send_msg_toclient(p_msg_entity);	///将消息发送给网络连接模块
			FT_FREE(p_msg_entity);	///释放消息存储空间
			p_msg_entity = NULL;		///防止野指针	

			/**获取下一个云端消息*/
			POP_FROMQUEUE(p_msg_entity,list,&cmd_service.clound_msg_queue);			
		}

		usleep(10000);  /// schedule,sleep 10ms
	}
	
}

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
int msg_transfer_init(void)
{
    /**定义消息指针*/
    st_cmd_queue_service *p_cmd_service = &cmd_service;

    /**创建设备命令和云端命令消息队列*/
    CREATE_QUEUE(&p_cmd_service->dev_msg_queue);
    CREATE_QUEUE(&p_cmd_service->clound_msg_queue);

    /**创建消息处理线程*/
    FT_THREAD_CREATE(&p_cmd_service->thread[0], message_handle_thread,NULL);
    FT_THREAD_DETACH(p_cmd_service->thread[0]); ///设置处理子线程由自身回收线程资源

    return FT_SUCCESS;
}

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
int push_dev_msg(st_msg_entity* p_msg_entity)
{
    /**定义消息指针*/
    st_cmd_queue_service *p_cmd_service = &cmd_service;

	PUSH_ENQUEUE(p_msg_entity, list, &p_cmd_service->dev_msg_queue);	///入队列
	
    return FT_SUCCESS;
}

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
int push_cloud_msg(st_msg_entity* p_msg_entity)
{
    /**定义消息指针*/
    st_cmd_queue_service *p_cmd_service = &cmd_service;

	PUSH_ENQUEUE(p_msg_entity, list, &p_cmd_service->clound_msg_queue);   ///入队列
	
    return FT_SUCCESS;
}


