/******************************************************************************

                  ��Ȩ���� (C), 2001-2016,  ������̫�������޹�˾

 ******************************************************************************
  �� �� ��   : service_common.h
  �� �� ��   : ����
  ��    ��   : ss
  ��������   : 2016��9��14��
  ����޸�   :
  ��������   : �������ṹ��ע�ᣬע������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��9��14��
    ��    ��   : wangzhengxue
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef SERVICE_COMMON_H
#define SERVICE_COMMON_H


#include "ikcc_common.h"

/**�豸������,��������������*/
typedef struct _st_service_obj {

	st_list_container callback_container;   ///�����ص�ע��

	st_list_container data_container;       ///����ע��
	/*............*/
}st_service_obj;

/**
 �� �� ��  : register_service
 ��������  : ����ע�ắ��
 �������  : st_service_obj *p_service  
             st_svr_desp *p_svr_desp    
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��9��14��
    ��    ��   : wangzhengxue
    �޸�����   : �����ɺ���

*/
int register_service(st_list_container *p_list_container, st_svr_desp *p_svr_desp);


/**
 �� �� ��  : unregister_service
 ��������  : ����ע������
 �������  : st_service_obj * p_service  
             st_svr_desp *p_svr_desp     
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��9��14��
    ��    ��   : wangzhengxue
    �޸�����   : �����ɺ���

*/
int unregister_service(st_list_container *p_list_container, const char *mac);

/**
* @ Prototype    : service_mac_to_socketfd
 Description  : ͨ��mac�ҵ��豸��socketfd
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
 Description  : ͨ��socketfd�ҵ��豸mac
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
 Description  : ��ȡclient����
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
    /**��ʼ������ͷ*/\
    INIT_LIST_HEAD(&(container).head);\
    INIT_LIST_HEAD(&(container).client_head);\
    /**��ʼ���ص�������*/\
    FT_LOCK_INIT(&(container).mutex);\
    }


#endif /* SERVICE_COMMON_H */
