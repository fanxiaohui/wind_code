/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : hw_management.h
* @ Version       : Initial Draft
* @ Author        : wangzhnegxue
* @ Created       : 2016/9/20
* @ Last Modified :
* @ Description   : ΪӲ���豸�����ṩ�ӿ�
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/9/20
* @   Author      : wangzhnegxue
* @   Modification: Created file

* @ 2.Date         : 2016/9/28
* @   Author       : wangzhnegxue
    Modification : �޸��豸���������Ӳ�ѯ�豸�������޸��豸�����ӿ�
*/

#ifndef HW_MANAGEMENT_H
#define HW_MANAGEMENT_H

#include "msg_transfer.h"
#include "service_common.h"
#include "ikcc_common.h"


/** Ӳ������ṹ�����ض��� */
typedef st_service_obj st_hw_service;

/**
* @ Prototype    : hw_init
 Description  : ��ʼ��Ӳ������
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
 Description  : ������ģ��ע��
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
 Description  : ������ģ��ע��
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
 Description  : ����ģ�鷢���豸��Ϣ���ϲ�
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
 Description  : ��Ϣ���ķ�����Ϣ���豸�ͻ���
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
* @ Description: ����豸
* @ type: st_msg_entity* p_msg_entity �豸������Ϣ
* @ return: 0�ɹ� -1 ʧ��
*/

/**
* @ Prototype    : hw_add_device
 Description  : ����ն��豸
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
    Modification : �����豸״̬�ĳ�ʼ��

* @ 3.Date         : 2016/10/8
* @   Author       : wangzhnegxue
    Modification : ɾ���豸״̬�����洢���޸�Ϊ��ע���豸�������豸����ֻ��
                   ��״̬
*/

int hw_add_device(const char *mac);

/**
* @ Prototype    : hw_del_device
 Description  : ֻ�����豸״̬��עλ
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
    Modification : �޸Ĵ���

* @ 3.Date         : 2016/9/29
* @   Author       : wangzhnegxue
    Modification : �豸ɾ��ʱ��������Ϣͬʱɾ��

* @ 4.Date         : 2016/10/8
* @   Author       : wangzhnegxue
    Modification : ɾ��ʱֻ����״̬��־λ

*/

int hw_del_device(const char *mac);

/**
* @ Prototype    : hw_get_client
 Description  : ��ȡ�豸�Ĳ�����Ϣ
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
 Description  : �����豸״̬��Ϣ
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
