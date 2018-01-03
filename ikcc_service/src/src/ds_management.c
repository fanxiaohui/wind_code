/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : ds_management.c
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/19
* @ Last Modified :
  brief   : 实现数据管理注册、注销等管理功能
* @ Function List :
              update_device_data
              del_device_data
              ds_get_device_info
              ds_init
              ds_register_service
              ds_send_msg_tobuf
              ds_send_msg_tocloud
              ds_unregister_service
* @ History       :
* @ 1.Date        : 2016/9/19
* @   Author      : wangzhengxue
* @   Modification: Created file

*/

#include <stdlib.h>
#include <string.h>
#include "ds_management.h"
#include "print.h"

/**静态变量定义*/
static st_ds_service ds_service;	///<数据服务结构定义?

/**
* @ Prototype    : ds_init
 Description  : 初始化硬件服务
* @ Input        : void  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int ds_init(void)
{
    /** 初始化回调 函数链表*/
	INIT_CONTAINER(ds_service.callback_container);

    /** 初始化数据链表 */
	INIT_CONTAINER(ds_service.data_container);
	
    return FT_SUCCESS;
}


/**
* @ Prototype    : ds_register_service
 Description  : 数据服务注册，云端连接后调用此接口，告知有新连接
* @ Input        : st_svr_desp *p_svr_desp  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int ds_register_service(st_svr_desp *p_svr_desp)
{
	return register_service(&ds_service.callback_container, p_svr_desp);
}

/**
* @ Prototype    : ds_unregister_service
 Description  : 数据服务注销，云端断开后调用此接口，告知连接断开
* @ Input        : st_svr_desp *p_svr_desp  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int ds_unregister_service(const char *identify)
{
	return unregister_service(&ds_service.callback_container, identify);
}


/**
* @ Prototype    : ds_send_msg_tobuf
 Description  :  云端向消息中心发送数据
* @ Input        : 
                st_msg_entity* p_msg_entity         
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int ds_send_msg_tobuf(st_msg_entity* p_msg_entity)
{
	ASSERT(p_msg_entity);

    /**根据e_msg_type做不同处理,默认将消息发送给消息分发模块*/
	switch (p_msg_entity->e_msg_type) {	
		default:
		{
			push_cloud_msg(p_msg_entity);
			break;
		}
	}

	return FT_SUCCESS;
}


/**
* @ Prototype    : ds_send_msg_tocloud
 Description  : 消息中心将数据发送给云端
* @ Input        : st_msg_entity *p_msg_entity  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int ds_send_msg_tocloud(st_msg_entity *p_msg_entity)
{
	st_hw_func *hw_func = NULL, *tmp_func = NULL;
	int ret = FT_FAILURE;
	int exist = FALSE;
    st_list_container *p_list_container = &ds_service.callback_container; ///查找注册函数表

	/**将数据添加到本地数据链表*/
	update_device_data(p_msg_entity);
	
	FT_LOCK(&p_list_container->mutex);																						

	/**遍历成员,给所有注册者发送消息*/
	if (!list_empty_careful(&p_list_container->head)) {																	
		list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {		
		        /** 注册函数非空， 向所有注册者都发送数据 */
                if (hw_func->p_svr_desp->callback_recv_msg) {
                    ret = hw_func->p_svr_desp->callback_recv_msg(p_msg_entity);   
                    if (FT_SUCCESS != ret) {
                        Error("send msg to clound failed\n, ret:%d ", ret);
                    }                
                    FT_UNLOCK(&p_list_container->mutex);                                                                                      
    				return ret;
				}
		}	
	}	
	
	FT_UNLOCK(&p_list_container->mutex);																						

	return ret;
}

/**
* @ Prototype    : ds_get_device_info
 Description  : 获取设备数据信息
* @ Input        : st_msg_entity* p_msg_entity  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

* @ 2.Date         : 2016/10/8
* @   Author       : wangzhnegxue
    Modification : 获取信息前将节点删除，成功再添加
*/
int ds_get_device_info(st_msg_entity* p_msg_entity)
{
	st_msg_entity *pos = NULL,*n = NULL;
	int ret = FT_FAILURE;
    st_list_container *p_list_container = &ds_service.data_container;   ///查找数据表

	ASSERT(p_msg_entity);	///参数检查

	FT_LOCK(&p_list_container->mutex);	

    /**链表非空,遍历所有成员,如果匹配成功则将数据拷贝*/
	if (!list_empty_careful(&p_list_container->head)) {	///链表非空	
		list_for_each_entry_safe(pos, n, &p_list_container->head, list) { 	///遍历链表
		    /**信息匹配成功,拷贝数据,跳出循环*/
			if (FT_SUCCESS == FT_STRNCMP(pos->src_mac, p_msg_entity->src_mac, sizeof(p_msg_entity->src_mac))) {	
			    list_del(&pos->list);
				FT_MEMCPY((void *)p_msg_entity, (void *)pos, sizeof(pos));	
				list_add(&pos->list, &p_list_container->head);
				ret = FT_SUCCESS;
				break;																						
			}
		}
	}	
	
	FT_UNLOCK(&p_list_container->mutex);																				//释放锁
	
	return ret;
}

/**
* @ Prototype    : update_device_data
 Description  : 添加设备数据信息
* @ Input        : st_msg_entity* p_msg_entity  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

* @ 2.Date         : 2016/9/30
* @   Author       : wangzhnegxue
    Modification : 修复list节点异常修改问题

* @ 3.Date         : 2016/10/8
* @   Author       : wangzhnegxue
    Modification : 消息更新时只更新playload信息
*/
int update_device_data(st_msg_entity* p_msg_entity)
{
	st_msg_entity *n = NULL;	
	st_msg_entity *pos = NULL;
	st_list_container *p_list_container = &ds_service.data_container;  ///查找数据表

	ASSERT(p_msg_entity);	///输入参数检查																									

	FT_LOCK(&p_list_container->mutex);																						

    /**链表已经纯在数据,遍历查找是否已存在链表中,如果已存在,只需更新数据*/
	if (!list_empty_careful(&p_list_container->head)) {	///链表非空
		list_for_each_entry_safe(pos, n, &p_list_container->head, list) {	///遍历所有设备信息
		
            /**匹配设备信息成功，设备已存在链表，先删除节点，直接更新内容,然后重新添加到链表头,
            将活跃的设备放在链表头可以提升遍历效率*/
			if (FT_SUCCESS == FT_STRNCMP(pos->src_mac, p_msg_entity->src_mac, sizeof(pos->src_mac))) {	
                list_del(&pos->list);   
				FT_MEMCPY(pos->payload, p_msg_entity->payload, sizeof(pos->payload));	
				list_add(&pos->list, &p_list_container->head);
                FT_UNLOCK(&p_list_container->mutex);               
                return FT_SUCCESS;
			}				
		}
	}
    /**为新添加设备,重新申请新的存储空间,并加入到数据链表*/
    pos = FT_MALLOC(sizeof(st_msg_entity));																
    if (pos) {																							
        FT_MEMCPY(pos, p_msg_entity, sizeof(st_msg_entity));		///拷贝数据信息
        list_add(&pos->list,&p_list_container->head);  //将新设备数据信息添加到数据链表
    } else {
        Error("malloc failed!!! size:%d ", sizeof(st_msg_entity));  ///申请内存失败处理
    }
	
	FT_UNLOCK(&p_list_container->mutex);
	
	return FT_SUCCESS;
}


/**
* @ Prototype    : del_device_data
 Description  : 删除设备数据信息
* @ Input        : const char *mac  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int del_device_data(const char *mac)
{
	st_msg_entity *pos = NULL,*n = NULL;
	st_list_container *p_list_container = &ds_service.data_container;    ///查找数据表

	ASSERT(mac);	///输入参数检查

	/**获取锁*/
	FT_LOCK(&p_list_container->mutex);																				

	/**遍历数据链表,如果设备存在匹配成功,将节点删除,释放存储空间*/
	if (!list_empty_careful(&p_list_container->head)) {//链表非空
		list_for_each_entry_safe(pos, n , &p_list_container->head, list) {	///遍历链表所有元素

		    /**匹配所有链表中的源设备mac地址,匹配成功则直接将节点删除*/
			if (FT_SUCCESS == FT_STRNCMP(pos->src_mac, mac, sizeof(pos->src_mac))) {			
				list_del(&pos->list);																			
				FT_FREE(pos);																					
				pos = NULL;
				n  = NULL;
				break;																												
			}
		}
	} else { ///没有可以删除的资源,返回失败
        FT_UNLOCK(&p_list_container->mutex);      
        return FT_FAILURE;
	}

	/**释放锁*/
	FT_UNLOCK(&p_list_container->mutex);								
	
	return FT_SUCCESS;
}
