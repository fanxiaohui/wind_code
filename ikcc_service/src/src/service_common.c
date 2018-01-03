/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : service_common.c
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/19
* @ Last Modified :
  brief   : 提供函数注册、注销服务
* @ Function List :
              register_service
              unregister_service
* @ History       :
* @ 1.Date        : 2016/9/19
* @   Author      : wangzhengxue
* @   Modification: Created file

*/

#include <stdlib.h>
#include <string.h> 
#include "print.h"
#include "service_common.h"
#include "ft_queue.h"
#include "ikcc_common.h"

/**
 Prototype    : register_service
 Description  : 服务注册函数
 Input        : st_list_container *p_list_container  
                st_svr_desp *p_svr_desp              
 Output       : None
 Return Value : 
 Calls        : list_empty_careful(),list_add()
 Called By    :  ds_register_service(),hw_register_service()
 
  History        :
  1.Date         : 2016/9/29
    Author       : wangzhnegxue
    Modification : Created function

  2.Date         : 2016/9/29
    Author       : wangzhnegxue
    Modification : 删除内部内存申请，减少一次内存拷贝，只提供内存管理机制

**/
int register_service(st_list_container *p_list_container, st_svr_desp *p_svr_desp)
{
	int ret = FT_FAILURE;
	st_hw_func *hw_func = NULL,*tmp_func = NULL;
    st_list_container *container = NULL;

	/**参数检查*/
	ASSERT(p_list_container);
	ASSERT(p_svr_desp);

	/**获取链表锁*/
	FT_LOCK(&p_list_container->mutex);

	/**获取链表节点,匹配mac参数,确认函数是否已经注册*/
	if (!list_empty_careful(&p_list_container->head)) {	///链表非空
		list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {	///遍历链表
			/**输入mac和遍历对象mac匹配,匹配成功则退出循环*/
			if (FT_SUCCESS == FT_STRNCMP(hw_func->p_svr_desp->mac, p_svr_desp->mac, sizeof(hw_func->p_svr_desp->mac))) {  
                FT_UNLOCK(&p_list_container->mutex);  ///释放链表锁
                FT_FREE(p_svr_desp);   ///重复注册,释放注册信息的内存 
                return FT_SUCCESS;  ///设备已经注册,直接返回成功
			}
		}		
	}

	/**没有找到设备,说明设备未注册,则增加新注册,添加到函数服务链表*/
	hw_func = FT_MALLOC(sizeof(st_hw_func));	///申请注册需要的内存空间
	if (hw_func) {
		hw_func->p_svr_desp = p_svr_desp;
		list_add(&hw_func->list, &p_list_container->head);				///将函数添加到注册表中
		p_list_container->number++;             ///当前存活在链表中的注册成员
		p_list_container->serial++;             ///记录从设备启动至此已经注册的成员数量
		hw_func->serial_id = p_list_container->serial;  ///当前注册设备的序列号

		/** 初始化client信息 */
		CREATE_QUEUE(&hw_func->client_node.entity.queue);
		hw_func->client_node.entity.fd = p_svr_desp->socketfd;
		hw_func->client_node.entity.heartbeat_count = 0;
		hw_func->client_node.entity.online = FALSE;
		FT_MEMCPY(hw_func->client_node.entity.mac, p_svr_desp->mac,sizeof(hw_func->client_node.entity.mac));

		/** 将client_node添加到clinet_head */
		list_add(&hw_func->client_node.list, &p_list_container->client_head);
        ret = FT_SUCCESS;
	} else {
        FT_FREE(p_svr_desp);        ///事情函数节点失败,释放注册信息的内存
		ret = FT_FAILURE;
		
		Error("malloc st_hw_func size :%d failed", sizeof(st_hw_func));
	}

    Info("register number:%d mac:%s ", p_list_container->number, p_svr_desp->mac);
    
	/**释放链表锁*/
	FT_UNLOCK(&p_list_container->mutex);

	return ret;
}

/**
* @ Prototype    : unregister_service
 Description  : 服务注销函数
* @ Input        : st_list_container *p_list_container  
                const char *mac                      
* @  Output       : None
* @  Return Value : 
* @  Calls        : list_empty_careful(),list_del()
* @  Called By    : ds_unregister_service(), hw_unregister_service()
 
* @   History        :
* @   1.Date         : 2016/9/29
* @     Author       : wangzhengxue
* @     Modification : Created function

* @ 2.Date         : 2016/9/29
* @   Author       : wangzhnegxue
    Modification :  删除内部内存申请，减少一次内存拷贝，只提供内存管理机制

*/
int unregister_service(st_list_container *p_list_container, const char *mac)
{
	st_hw_func *hw_func = NULL, *tmp_func = NULL;

	/**参数检查*/
	ASSERT(p_list_container);
	ASSERT(mac);

	/**获取链表锁*/
	FT_LOCK(&p_list_container->mutex);

	/**遍历链表找到需要注销的设备,删除节点,释放存储空间*/
	if (!list_empty_careful(&p_list_container->head)) {	///链表非空
		list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {	///遍历链表
		
			/**根据mac,匹配设备成功，输出链表节点*/
			if (FT_SUCCESS == FT_STRNCMP(hw_func->p_svr_desp->mac, mac, sizeof(hw_func->p_svr_desp->mac))) {	
			    p_list_container->number--;   ///计数减少
				list_del(&hw_func->list);		///删除节点
				list_del(&hw_func->client_node.list);
				FT_FREE(hw_func->p_svr_desp);   ///删除消息空间
				FT_FREE(hw_func);				///释放存储空间
            	FT_UNLOCK(&p_list_container->mutex);  ///释放资源锁
            	Info("mac:%s number:%d \n",mac, p_list_container->number);
				return FT_SUCCESS;		///查找成功
			}
		}	
	}
	
	/**释放资源锁*/
	FT_UNLOCK(&p_list_container->mutex);

    Error("no find suitable client, mac:%s ", mac);
    
	return FT_SUCCESS; 
}

/**
* @ Prototype    : service_mac_to_socketfd
 Description  : 通过mac找到设备的socketfd
* @ Input        : st_list_container *p_list_container  
                const char *mac                            
                int *p_fd                                  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/30
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int service_mac_to_socketfd(st_list_container *p_list_container, const char *mac, int *p_fd)
{
    st_hw_func *hw_func = NULL, *tmp_func = NULL;

    /**参数检查*/
    ASSERT(p_list_container);
    ASSERT(mac);
    ASSERT(p_fd);

    /**获取链表锁*/
    FT_LOCK(&p_list_container->mutex);

    /**遍历链表找到需要注销的设备,删除节点,释放存储空间*/
    if (!list_empty_careful(&p_list_container->head)) { ///链表非空
        list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {    ///遍历链表
        
            /**根据mac,匹配设备成功，输出链表节点*/
            if (FT_SUCCESS == FT_STRNCMP(hw_func->p_svr_desp->mac, mac, sizeof(hw_func->p_svr_desp->mac))) {    
                *p_fd = hw_func->p_svr_desp->socketfd;
                FT_UNLOCK(&p_list_container->mutex);  ///释放资源锁
                return FT_SUCCESS;      ///查找成功
            }
        }   
    }
    
    /**释放资源锁*/
    FT_UNLOCK(&p_list_container->mutex);

    *p_fd = -1;

    Error("no find suitable client, mac:%s ", mac);
    
    return FT_SUCCESS; 
}

/**
* @ Prototype    : service_socketfd_to_mac
 Description  : 通过socketfd找到设备mac
* @ Input        : st_list_container *p_list_container  
                const int fd                               
                char *mac                                  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/30
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int service_socketfd_to_mac(st_list_container *p_list_container, const int fd, char *mac)
{
    st_hw_func *hw_func = NULL, *tmp_func = NULL;

    /**参数检查*/
    ASSERT(p_list_container);
    ASSERT(mac);

    /**获取链表锁*/
    FT_LOCK(&p_list_container->mutex);

    /**遍历链表找到需要注销的设备,删除节点,释放存储空间*/
    if (!list_empty_careful(&p_list_container->head)) { ///链表非空
        list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {    ///遍历链表
            /** 匹配socketfd */
            if (fd == hw_func->p_svr_desp->socketfd) {  
                FT_MEMCPY(mac,  hw_func->p_svr_desp->mac,sizeof(hw_func->p_svr_desp->mac));
                FT_UNLOCK(&p_list_container->mutex);  ///释放资源锁
                return FT_SUCCESS;      ///查找成功
            }
        }   
    }
    
    /**释放资源锁*/
    FT_UNLOCK(&p_list_container->mutex);

    Error("no find suitable client, mac:%s ", mac);
    
    return FT_SUCCESS; 
}

/**
* @ Prototype    : service_get_clinet
 Description  : 获取client对象
* @ Input        : IN st_list_container *p_list_container  
                IN const char *mac                      
                IN const int socketfd                   
* @  Output       : None
* @  Return Value : st_clinet_entity
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/10/9
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
st_clinet_entity *service_get_clinet(IN st_list_container *p_list_container, IN const char *mac, IN const int socketfd)
{
    st_hw_func *pos = NULL,*n = NULL;
    st_clinet_entity *p_client = NULL;

    /**获取链表锁*/
    FT_LOCK(&p_list_container->mutex);

    /**获取链表节点,匹配mac参数,确认函数是否已经注册*/
    if (!list_empty_careful(&p_list_container->head)) { ///链表非空
        list_for_each_entry_safe(pos, n, &p_list_container->head, list) {    ///遍历链表
                ///mac非空通过获取参数，否则通过socketfd
                if ((NULL != mac && FT_SUCCESS == FT_STRNCMP(pos->p_svr_desp->mac, mac, sizeof(pos->p_svr_desp->mac)))
                || (NULL == mac && socketfd > 0)) {  
                    p_client = &pos->client_node.entity;                    
                    FT_UNLOCK(&p_list_container->mutex);  ///释放链表锁
                    return p_client;  ///设备已经注册,直接返回成功
                } else {
                ///
                Error("input param error\n");
                FT_UNLOCK(&p_list_container->mutex);  ///释放链表锁
                return NULL;  ///设备已经注册,直接返回成功
            }
        }
    }       

    Info("mac:%s no register or has unregister\n", mac);
    
    /**释放链表锁*/
    FT_UNLOCK(&p_list_container->mutex);

    return NULL;
}

