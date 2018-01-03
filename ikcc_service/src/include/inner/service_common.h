/******************************************************************************

                  版权所有 (C), 2001-2016,  宁波方太厨具有限公司

 ******************************************************************************
  文 件 名   : service_common.h
  版 本 号   : 初稿
  作    者   : ss
  生成日期   : 2016年9月14日
  最近修改   :
  功能描述   : 定义服务结构及注册，注销函数
  函数列表   :
  修改历史   :
  1.日    期   : 2016年9月14日
    作    者   : wangzhengxue
    修改内容   : 创建文件

******************************************************************************/

#ifndef SERVICE_COMMON_H
#define SERVICE_COMMON_H


#include "ikcc_common.h"

/**设备服务定义,包含各个链表及锁*/
typedef struct _st_service_obj {

	st_list_container callback_container;   ///函数回调注册

	st_list_container data_container;       ///数据注册
	/*............*/
}st_service_obj;

/**
 函 数 名  : register_service
 功能描述  : 服务注册函数
 输入参数  : st_service_obj *p_service  
             st_svr_desp *p_svr_desp    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月14日
    作    者   : wangzhengxue
    修改内容   : 新生成函数

*/
int register_service(st_list_container *p_list_container, st_svr_desp *p_svr_desp);


/**
 函 数 名  : unregister_service
 功能描述  : 服务注销函数
 输入参数  : st_service_obj * p_service  
             st_svr_desp *p_svr_desp     
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年9月14日
    作    者   : wangzhengxue
    修改内容   : 新生成函数

*/
int unregister_service(st_list_container *p_list_container, const char *mac);

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
int service_mac_to_socketfd(st_list_container *p_list_container, const char *mac, int *p_fd);

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
int service_socketfd_to_mac(st_list_container *p_list_container, const int fd, char *mac);

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
st_clinet_entity *service_get_clinet(IN st_list_container *p_list_container, IN const char *mac, IN const int socketfd);


#define INIT_CONTAINER(container) {\
    /**初始化链表头*/\
    INIT_LIST_HEAD(&(container).head);\
    INIT_LIST_HEAD(&(container).client_head);\
    /**初始化回调函数锁*/\
    FT_LOCK_INIT(&(container).mutex);\
    }


#endif /* SERVICE_COMMON_H */
