/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : hw_management.h
* @ Version       : Initial Draft
* @ Author        : wangzhnegxue
* @ Created       : 2016/9/20
* @ Last Modified :
* @ Description   : 为硬件设备操作提供接口
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/9/20
* @   Author      : wangzhnegxue
* @   Modification: Created file

* @ 2.Date         : 2016/9/28
* @   Author       : wangzhnegxue
    Modification : 修改设备描述及增加查询设备参数和修改设备参数接口
*/

#ifndef HW_MANAGEMENT_H
#define HW_MANAGEMENT_H

#include "msg_transfer.h"
#include "service_common.h"
#include "ikcc_common.h"


/** 硬件服务结构类型重定义 */
typedef st_service_obj st_hw_service;

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
int hw_init(void);

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
int hw_register_service(st_svr_desp *p_svr_desp);

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
int hw_unregister_service(const char *mac);

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

*/
int hw_send_msg_tobuf(st_msg_entity* p_msg_entity);

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
int hw_send_msg_toclient(st_msg_entity *p_msg_entity);

/**
* @ Description: 添加设备
* @ type: st_msg_entity* p_msg_entity 设备基本信息
* @ return: 0成功 -1 失败
*/

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

int hw_add_device(const char *mac);

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

int hw_del_device(const char *mac);

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

*/
int hw_get_client(IN const char *mac, IN const int socketfd, OUT st_clinet_entity **p_client, struct list_head **client_head);


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

*/
int hw_modify_device_option_param(int option, const char *mac);


#endif
