/**
* @file ds_management.h
* @brief 数据服务管理接口
* @version 1.1
* @author cairj
* @date 2016/08/25
*/

#ifndef DS_MANAGEMENT_H
#define DS_MANAGEMENT_H
#include "msg_transfer.h"
#include "service_common.h"
#include "ikcc_common.h"


/** 类型重定义 */
typedef st_service_obj st_ds_service;


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
int ds_init(void);


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
int ds_register_service(st_svr_desp *p_svr_desp);


/**
* @ Prototype    : ds_unregister_service
 Description  : 数据服务注销，云端断开后调用此接口，告知连接断开
* @ Input        : (const char *mac)  
* @  Output       : None
* @  Return Value : 
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int ds_unregister_service(const char *mac);


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
int ds_send_msg_tobuf(st_msg_entity* p_msg_entity);

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
int ds_send_msg_tocloud(st_msg_entity *p_msg_entity);

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

*/
int ds_get_device_info(st_msg_entity* p_msg_entity);

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

*/
int update_device_data(st_msg_entity* p_msg_entity);


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
int del_device_data(const char *mac);


#endif /* DATA_MANAGEMENT_H */
