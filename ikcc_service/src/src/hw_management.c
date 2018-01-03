/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : hw_management.c
* @ Version       : Initial Draft
* @ Author        : wangzhnegxue
* @ Created       : 2016/9/28
* @ Last Modified :
* @ Description   : s
* @ Function List :
              hw_add_device
              hw_del_device
              hw_get_client
              hw_init
              hw_modify_device_option_param
              hw_register_service
              hw_send_msg_tobuf
              hw_send_msg_toclient
              hw_unregister_service
* @ History       :
* @ 1.Date        : 2016/9/28
* @   Author      : wangzhnegxue
* @   Modification: Created file

* @ 2.Date         : 2016/9/28
* @   Author       : wangzhnegxue
    Modification : 增加查询，修改设备参数接口

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "print.h"
#include "hw_management.h"

/** 设备参数修改类型 */
typedef enum _enum_hw_param_option{
    HW_HEART_BEAT,  ///更新心跳
    HW_ONLINE,      ///设备上线
    HW_OFFLINE,     ///设备离线
    HW_NONE,
}enum_hw_param_option;


/** 静态变量定义 */
static st_hw_service hw_service;    ///设备管理定义


/**
* @ Prototype    : hw_init
 Description  : 初始化硬件服务
* @ Input        : void  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/20
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int hw_init(void)
{
	/**创建hw 回调服务函数链表*/
    INIT_CONTAINER(hw_service.callback_container);
}


/**
* @ Prototype    : hw_register_service
 Description  : 连接子模块注册
* @ Input        : st_svr_desp *p_svr_desp  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/20
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int hw_register_service(st_svr_desp *p_svr_desp)
{
	return register_service(&hw_service.callback_container, p_svr_desp);
}

/**
* @ Prototype    : hw_unregister_service
 Description  : 连接子模块注销
* @ Input        : st_svr_desp *p_svr_desp  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/20
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int hw_unregister_service(const char *identify)
{
	unregister_service(&hw_service.callback_container, identify);
	
    return del_device_data(identify);
}

/**
* @ Prototype    : hw_send_msg_tobuf
 Description  : 连接模块发送设备消息给上层
* @ Input        :  
                st_msg_entity* p_msg_entity   
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/20
* @     Author       : wangzhengxue
* @     Modification : Created function

* @ 2.Date         : 2016/9/28
* @   Author       : wangzhnegxue
    Modification : 传参类型修改

* @ 3.Date         : 2016/9/29
* @   Author       : wangzhnegxue
    Modification : 明确接口功能,此接口只用于数据的发送，设备登录登出信息使用其它接口
*/
int hw_send_msg_tobuf(st_msg_entity* p_msg_entity)
{

    ASSERT(p_msg_entity);
    
    /** 将消息推送给消息中心 */
    push_dev_msg(p_msg_entity);

    return FT_SUCCESS;
}

/**
* @ Prototype    : hw_send_msg_toclient
 Description  : 消息中心发送消息给设备客户端
* @ Input        : st_msg_entity *p_msg_entity  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/20
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int hw_send_msg_toclient(st_msg_entity *p_msg_entity)
{
	st_hw_func *hw_func = NULL, *tmp_func = NULL;
	int ret = FT_SUCCESS;
	int exist = FALSE;
	st_list_container *p_list_container = &hw_service.callback_container;    ///查找注册函数表
    
	FT_LOCK(&p_list_container->mutex);
    /**遍历回调函数链表,匹配正确设备,发送消息*/
	if (!list_empty_careful(&p_list_container->head)) {	
		list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {

		    /** 消息的目的地址和注册地址匹配 */
			if (FT_SUCCESS == FT_STRNCMP(hw_func->p_svr_desp->mac, p_msg_entity->dst_mac, sizeof(hw_func->p_svr_desp->mac))) {	
			    /**调用回调函数将消息发送给客户端*/
                ret = hw_func->p_svr_desp->callback_recv_msg(p_msg_entity);
                if (FT_SUCCESS != ret) {    ///发送消息失败
                    Error("hw send msg to client failed");                    
                    FT_UNLOCK(&p_list_container->mutex);
                    return ret;
                }
				break;
			}
		}	
	}

	FT_UNLOCK(&p_list_container->mutex);

	return ret;
}

/**
* @ Prototype    : hw_add_device
 Description  : 添加终端设备
* @ Input        : const char *mac  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/27
* @     Author       : wangzhengxue
* @     Modification : Created function

* @ 2.Date         : 2016/9/28
* @   Author       : wangzhnegxue
    Modification : 增加设备状态的初始化

* @ 3.Date         : 2016/10/8
* @   Author       : wangzhnegxue
    Modification : 删除设备状态单独存储，修改为和注册设备关联，设备上线只修
                   改状态
*/
int hw_add_device(const char *mac)
{
	return hw_modify_device_option_param(HW_ONLINE, mac);
}

/**
* @ Prototype    : hw_del_device
 Description  : 只更新设备状态标注位
* @ Input        : const char *mac  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/27
* @     Author       : wangzhengxue
* @     Modification : Created function

* @ 2.Date         : 2016/9/28
* @   Author       : wangzhnegxue
    Modification : 修改传参

* @ 3.Date         : 2016/9/29
* @   Author       : wangzhnegxue
    Modification : 设备删除时将数据信息同时删除

* @ 4.Date         : 2016/10/8
* @   Author       : wangzhnegxue
    Modification : 删除时只更改状态标志位

*/
int hw_del_device(const char *mac)
{
	hw_modify_device_option_param(HW_OFFLINE, mac);

	return FT_SUCCESS;
}

/**
* @ Prototype    : hw_get_client
 Description  : 获取设备的参数信息
* @ Input        : const char *mac             
                const int socketfd          
                st_clinet_entity *p_client  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/10/9
* @     Author       : wangzhengxue
* @     Modification : Created function

* @ 2.Date         : 2016/10/9
* @   Author       : wangzhnegxue
    Modification : 根据mac或socketfd查找需要信息，mac非空则通过mac查询，否则
                   通过socketfd查询

*/
int hw_get_client(IN const char *mac, IN const int socketfd, OUT st_clinet_entity **p_client, struct list_head **client_head)
{
    if (client_head != NULL) {
        *client_head = &hw_service.callback_container.client_head;
    }
    *p_client = service_get_clinet(&hw_service.callback_container, mac, socketfd);
    if (NULL == *p_client) {
        return FT_FAILURE;
    } else {
        return FT_SUCCESS;
    }
}

/**
* @ Prototype    : hw_modify_device_option_param
 Description  : 更新设备状态信息
* @ Input        : int option       
                const char *mac  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/30
* @     Author       : wangzhengxue
* @     Modification : Created function

* @ 2.Date         : 2016/10/8
* @   Author       : wangzhnegxue
    Modification : 增加设备离线状态设置
*/
int hw_modify_device_option_param(int option, const char *mac)
{
    st_hw_func *pos = NULL,*n = NULL;
    st_list_container *p_list_container = &hw_service.callback_container;

    /**参数检查*/
    ASSERT(p_list_container);

    /**获取链表锁*/
    FT_LOCK(&p_list_container->mutex);

    /**获取链表节点,匹配mac参数,确认函数是否已经注册*/
    if (!list_empty_careful(&p_list_container->head)) { ///链表非空
        list_for_each_entry_safe(pos, n, &p_list_container->head, list) {    ///遍历链表
            /**输入mac和遍历对象mac匹配,匹配成功则退出循环*/
            if (FT_SUCCESS == FT_STRNCMP(pos->p_svr_desp->mac, mac, sizeof(pos->p_svr_desp->mac))) {  
			    /** client param update */
                switch(option){
                    case HW_HEART_BEAT:
                    {
                        pos->client_node.entity.heartbeat_count++;
                        break;
                    }
                    case HW_ONLINE:
                    {
                        pos->client_node.entity.online = TRUE;
                        FT_MEMCPY(pos->client_node.entity.mac, mac, sizeof(pos->client_node.entity.mac));
                        break;
                    }
                    case HW_OFFLINE:
                    {
                        pos->client_node.entity.online = FALSE;                        
                        FT_MEMCPY(pos->client_node.entity.mac, '\0', sizeof(pos->client_node.entity.mac));
                        break;
                    }
                    default:
                    {
                        Info("option:%d is unknow\n",option);
                        break;
                    }
                }
                FT_UNLOCK(&p_list_container->mutex);  ///释放链表锁
                return FT_SUCCESS;  ///设备已经注册,直接返回成功
            }
        }       
    }

    Info("mac:%s no register or has unregister\n", mac);
    
    /**释放链表锁*/
    FT_UNLOCK(&p_list_container->mutex);

    return FT_FAILURE;
}

