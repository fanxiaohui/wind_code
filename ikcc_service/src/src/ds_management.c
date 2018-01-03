/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : ds_management.c
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/19
* @ Last Modified :
  brief   : ʵ�����ݹ���ע�ᡢע���ȹ�����
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

/**��̬��������*/
static st_ds_service ds_service;	///<���ݷ���ṹ����?

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
int ds_init(void)
{
    /** ��ʼ���ص� ��������*/
	INIT_CONTAINER(ds_service.callback_container);

    /** ��ʼ���������� */
	INIT_CONTAINER(ds_service.data_container);
	
    return FT_SUCCESS;
}


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
int ds_register_service(st_svr_desp *p_svr_desp)
{
	return register_service(&ds_service.callback_container, p_svr_desp);
}

/**
* @ Prototype    : ds_unregister_service
 Description  : ���ݷ���ע�����ƶ˶Ͽ�����ô˽ӿڣ���֪���ӶϿ�
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
int ds_send_msg_tobuf(st_msg_entity* p_msg_entity)
{
	ASSERT(p_msg_entity);

    /**����e_msg_type����ͬ����,Ĭ�Ͻ���Ϣ���͸���Ϣ�ַ�ģ��*/
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
int ds_send_msg_tocloud(st_msg_entity *p_msg_entity)
{
	st_hw_func *hw_func = NULL, *tmp_func = NULL;
	int ret = FT_FAILURE;
	int exist = FALSE;
    st_list_container *p_list_container = &ds_service.callback_container; ///����ע�ắ����

	/**��������ӵ�������������*/
	update_device_data(p_msg_entity);
	
	FT_LOCK(&p_list_container->mutex);																						

	/**������Ա,������ע���߷�����Ϣ*/
	if (!list_empty_careful(&p_list_container->head)) {																	
		list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {		
		        /** ע�ắ���ǿգ� ������ע���߶��������� */
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

* @ 2.Date         : 2016/10/8
* @   Author       : wangzhnegxue
    Modification : ��ȡ��Ϣǰ���ڵ�ɾ�����ɹ������
*/
int ds_get_device_info(st_msg_entity* p_msg_entity)
{
	st_msg_entity *pos = NULL,*n = NULL;
	int ret = FT_FAILURE;
    st_list_container *p_list_container = &ds_service.data_container;   ///�������ݱ�

	ASSERT(p_msg_entity);	///�������

	FT_LOCK(&p_list_container->mutex);	

    /**����ǿ�,�������г�Ա,���ƥ��ɹ������ݿ���*/
	if (!list_empty_careful(&p_list_container->head)) {	///����ǿ�	
		list_for_each_entry_safe(pos, n, &p_list_container->head, list) { 	///��������
		    /**��Ϣƥ��ɹ�,��������,����ѭ��*/
			if (FT_SUCCESS == FT_STRNCMP(pos->src_mac, p_msg_entity->src_mac, sizeof(p_msg_entity->src_mac))) {	
			    list_del(&pos->list);
				FT_MEMCPY((void *)p_msg_entity, (void *)pos, sizeof(pos));	
				list_add(&pos->list, &p_list_container->head);
				ret = FT_SUCCESS;
				break;																						
			}
		}
	}	
	
	FT_UNLOCK(&p_list_container->mutex);																				//�ͷ���
	
	return ret;
}

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

* @ 2.Date         : 2016/9/30
* @   Author       : wangzhnegxue
    Modification : �޸�list�ڵ��쳣�޸�����

* @ 3.Date         : 2016/10/8
* @   Author       : wangzhnegxue
    Modification : ��Ϣ����ʱֻ����playload��Ϣ
*/
int update_device_data(st_msg_entity* p_msg_entity)
{
	st_msg_entity *n = NULL;	
	st_msg_entity *pos = NULL;
	st_list_container *p_list_container = &ds_service.data_container;  ///�������ݱ�

	ASSERT(p_msg_entity);	///����������																									

	FT_LOCK(&p_list_container->mutex);																						

    /**�����Ѿ���������,���������Ƿ��Ѵ���������,����Ѵ���,ֻ���������*/
	if (!list_empty_careful(&p_list_container->head)) {	///����ǿ�
		list_for_each_entry_safe(pos, n, &p_list_container->head, list) {	///���������豸��Ϣ
		
            /**ƥ���豸��Ϣ�ɹ����豸�Ѵ���������ɾ���ڵ㣬ֱ�Ӹ�������,Ȼ��������ӵ�����ͷ,
            ����Ծ���豸��������ͷ������������Ч��*/
			if (FT_SUCCESS == FT_STRNCMP(pos->src_mac, p_msg_entity->src_mac, sizeof(pos->src_mac))) {	
                list_del(&pos->list);   
				FT_MEMCPY(pos->payload, p_msg_entity->payload, sizeof(pos->payload));	
				list_add(&pos->list, &p_list_container->head);
                FT_UNLOCK(&p_list_container->mutex);               
                return FT_SUCCESS;
			}				
		}
	}
    /**Ϊ������豸,���������µĴ洢�ռ�,�����뵽��������*/
    pos = FT_MALLOC(sizeof(st_msg_entity));																
    if (pos) {																							
        FT_MEMCPY(pos, p_msg_entity, sizeof(st_msg_entity));		///����������Ϣ
        list_add(&pos->list,&p_list_container->head);  //�����豸������Ϣ��ӵ���������
    } else {
        Error("malloc failed!!! size:%d ", sizeof(st_msg_entity));  ///�����ڴ�ʧ�ܴ���
    }
	
	FT_UNLOCK(&p_list_container->mutex);
	
	return FT_SUCCESS;
}


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
int del_device_data(const char *mac)
{
	st_msg_entity *pos = NULL,*n = NULL;
	st_list_container *p_list_container = &ds_service.data_container;    ///�������ݱ�

	ASSERT(mac);	///����������

	/**��ȡ��*/
	FT_LOCK(&p_list_container->mutex);																				

	/**������������,����豸����ƥ��ɹ�,���ڵ�ɾ��,�ͷŴ洢�ռ�*/
	if (!list_empty_careful(&p_list_container->head)) {//����ǿ�
		list_for_each_entry_safe(pos, n , &p_list_container->head, list) {	///������������Ԫ��

		    /**ƥ�����������е�Դ�豸mac��ַ,ƥ��ɹ���ֱ�ӽ��ڵ�ɾ��*/
			if (FT_SUCCESS == FT_STRNCMP(pos->src_mac, mac, sizeof(pos->src_mac))) {			
				list_del(&pos->list);																			
				FT_FREE(pos);																					
				pos = NULL;
				n  = NULL;
				break;																												
			}
		}
	} else { ///û�п���ɾ������Դ,����ʧ��
        FT_UNLOCK(&p_list_container->mutex);      
        return FT_FAILURE;
	}

	/**�ͷ���*/
	FT_UNLOCK(&p_list_container->mutex);								
	
	return FT_SUCCESS;
}
