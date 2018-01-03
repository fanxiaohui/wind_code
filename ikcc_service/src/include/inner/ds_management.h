/**
* @file ds_management.h
* @brief ���ݷ������ӿ�
* @version 1.1
* @author cairj
* @date 2016/08/25
*/

#ifndef DS_MANAGEMENT_H
#define DS_MANAGEMENT_H
#include "msg_transfer.h"
#include "service_common.h"
#include "ikcc_common.h"


/** �����ض��� */
typedef st_service_obj st_ds_service;


/**
* @ Prototype    : ds_init
 Description  : ��ʼ��Ӳ������
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
 Description  : ���ݷ���ע�ᣬ�ƶ����Ӻ���ô˽ӿڣ���֪��������
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
 Description  : ���ݷ���ע�����ƶ˶Ͽ�����ô˽ӿڣ���֪���ӶϿ�
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
 Description  :  �ƶ�����Ϣ���ķ�������
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
 Description  : ��Ϣ���Ľ����ݷ��͸��ƶ�
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
 Description  : ��ȡ�豸������Ϣ
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
 Description  : ����豸������Ϣ
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
 Description  : ɾ���豸������Ϣ
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
