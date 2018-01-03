/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : service_common.c
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/19
* @ Last Modified :
  brief   : �ṩ����ע�ᡢע������
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
 Description  : ����ע�ắ��
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
    Modification : ɾ���ڲ��ڴ����룬����һ���ڴ濽����ֻ�ṩ�ڴ�������

**/
int register_service(st_list_container *p_list_container, st_svr_desp *p_svr_desp)
{
	int ret = FT_FAILURE;
	st_hw_func *hw_func = NULL,*tmp_func = NULL;
    st_list_container *container = NULL;

	/**�������*/
	ASSERT(p_list_container);
	ASSERT(p_svr_desp);

	/**��ȡ������*/
	FT_LOCK(&p_list_container->mutex);

	/**��ȡ����ڵ�,ƥ��mac����,ȷ�Ϻ����Ƿ��Ѿ�ע��*/
	if (!list_empty_careful(&p_list_container->head)) {	///����ǿ�
		list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {	///��������
			/**����mac�ͱ�������macƥ��,ƥ��ɹ����˳�ѭ��*/
			if (FT_SUCCESS == FT_STRNCMP(hw_func->p_svr_desp->mac, p_svr_desp->mac, sizeof(hw_func->p_svr_desp->mac))) {  
                FT_UNLOCK(&p_list_container->mutex);  ///�ͷ�������
                FT_FREE(p_svr_desp);   ///�ظ�ע��,�ͷ�ע����Ϣ���ڴ� 
                return FT_SUCCESS;  ///�豸�Ѿ�ע��,ֱ�ӷ��سɹ�
			}
		}		
	}

	/**û���ҵ��豸,˵���豸δע��,��������ע��,��ӵ�������������*/
	hw_func = FT_MALLOC(sizeof(st_hw_func));	///����ע����Ҫ���ڴ�ռ�
	if (hw_func) {
		hw_func->p_svr_desp = p_svr_desp;
		list_add(&hw_func->list, &p_list_container->head);				///��������ӵ�ע�����
		p_list_container->number++;             ///��ǰ����������е�ע���Ա
		p_list_container->serial++;             ///��¼���豸���������Ѿ�ע��ĳ�Ա����
		hw_func->serial_id = p_list_container->serial;  ///��ǰע���豸�����к�

		/** ��ʼ��client��Ϣ */
		CREATE_QUEUE(&hw_func->client_node.entity.queue);
		hw_func->client_node.entity.fd = p_svr_desp->socketfd;
		hw_func->client_node.entity.heartbeat_count = 0;
		hw_func->client_node.entity.online = FALSE;
		FT_MEMCPY(hw_func->client_node.entity.mac, p_svr_desp->mac,sizeof(hw_func->client_node.entity.mac));

		/** ��client_node��ӵ�clinet_head */
		list_add(&hw_func->client_node.list, &p_list_container->client_head);
        ret = FT_SUCCESS;
	} else {
        FT_FREE(p_svr_desp);        ///���麯���ڵ�ʧ��,�ͷ�ע����Ϣ���ڴ�
		ret = FT_FAILURE;
		
		Error("malloc st_hw_func size :%d failed", sizeof(st_hw_func));
	}

    Info("register number:%d mac:%s ", p_list_container->number, p_svr_desp->mac);
    
	/**�ͷ�������*/
	FT_UNLOCK(&p_list_container->mutex);

	return ret;
}

/**
* @ Prototype    : unregister_service
 Description  : ����ע������
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
    Modification :  ɾ���ڲ��ڴ����룬����һ���ڴ濽����ֻ�ṩ�ڴ�������

*/
int unregister_service(st_list_container *p_list_container, const char *mac)
{
	st_hw_func *hw_func = NULL, *tmp_func = NULL;

	/**�������*/
	ASSERT(p_list_container);
	ASSERT(mac);

	/**��ȡ������*/
	FT_LOCK(&p_list_container->mutex);

	/**���������ҵ���Ҫע�����豸,ɾ���ڵ�,�ͷŴ洢�ռ�*/
	if (!list_empty_careful(&p_list_container->head)) {	///����ǿ�
		list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {	///��������
		
			/**����mac,ƥ���豸�ɹ����������ڵ�*/
			if (FT_SUCCESS == FT_STRNCMP(hw_func->p_svr_desp->mac, mac, sizeof(hw_func->p_svr_desp->mac))) {	
			    p_list_container->number--;   ///��������
				list_del(&hw_func->list);		///ɾ���ڵ�
				list_del(&hw_func->client_node.list);
				FT_FREE(hw_func->p_svr_desp);   ///ɾ����Ϣ�ռ�
				FT_FREE(hw_func);				///�ͷŴ洢�ռ�
            	FT_UNLOCK(&p_list_container->mutex);  ///�ͷ���Դ��
            	Info("mac:%s number:%d \n",mac, p_list_container->number);
				return FT_SUCCESS;		///���ҳɹ�
			}
		}	
	}
	
	/**�ͷ���Դ��*/
	FT_UNLOCK(&p_list_container->mutex);

    Error("no find suitable client, mac:%s ", mac);
    
	return FT_SUCCESS; 
}

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
int service_mac_to_socketfd(st_list_container *p_list_container, const char *mac, int *p_fd)
{
    st_hw_func *hw_func = NULL, *tmp_func = NULL;

    /**�������*/
    ASSERT(p_list_container);
    ASSERT(mac);
    ASSERT(p_fd);

    /**��ȡ������*/
    FT_LOCK(&p_list_container->mutex);

    /**���������ҵ���Ҫע�����豸,ɾ���ڵ�,�ͷŴ洢�ռ�*/
    if (!list_empty_careful(&p_list_container->head)) { ///����ǿ�
        list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {    ///��������
        
            /**����mac,ƥ���豸�ɹ����������ڵ�*/
            if (FT_SUCCESS == FT_STRNCMP(hw_func->p_svr_desp->mac, mac, sizeof(hw_func->p_svr_desp->mac))) {    
                *p_fd = hw_func->p_svr_desp->socketfd;
                FT_UNLOCK(&p_list_container->mutex);  ///�ͷ���Դ��
                return FT_SUCCESS;      ///���ҳɹ�
            }
        }   
    }
    
    /**�ͷ���Դ��*/
    FT_UNLOCK(&p_list_container->mutex);

    *p_fd = -1;

    Error("no find suitable client, mac:%s ", mac);
    
    return FT_SUCCESS; 
}

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
int service_socketfd_to_mac(st_list_container *p_list_container, const int fd, char *mac)
{
    st_hw_func *hw_func = NULL, *tmp_func = NULL;

    /**�������*/
    ASSERT(p_list_container);
    ASSERT(mac);

    /**��ȡ������*/
    FT_LOCK(&p_list_container->mutex);

    /**���������ҵ���Ҫע�����豸,ɾ���ڵ�,�ͷŴ洢�ռ�*/
    if (!list_empty_careful(&p_list_container->head)) { ///����ǿ�
        list_for_each_entry_safe(hw_func, tmp_func, &p_list_container->head, list) {    ///��������
            /** ƥ��socketfd */
            if (fd == hw_func->p_svr_desp->socketfd) {  
                FT_MEMCPY(mac,  hw_func->p_svr_desp->mac,sizeof(hw_func->p_svr_desp->mac));
                FT_UNLOCK(&p_list_container->mutex);  ///�ͷ���Դ��
                return FT_SUCCESS;      ///���ҳɹ�
            }
        }   
    }
    
    /**�ͷ���Դ��*/
    FT_UNLOCK(&p_list_container->mutex);

    Error("no find suitable client, mac:%s ", mac);
    
    return FT_SUCCESS; 
}

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
st_clinet_entity *service_get_clinet(IN st_list_container *p_list_container, IN const char *mac, IN const int socketfd)
{
    st_hw_func *pos = NULL,*n = NULL;
    st_clinet_entity *p_client = NULL;

    /**��ȡ������*/
    FT_LOCK(&p_list_container->mutex);

    /**��ȡ����ڵ�,ƥ��mac����,ȷ�Ϻ����Ƿ��Ѿ�ע��*/
    if (!list_empty_careful(&p_list_container->head)) { ///����ǿ�
        list_for_each_entry_safe(pos, n, &p_list_container->head, list) {    ///��������
                ///mac�ǿ�ͨ����ȡ����������ͨ��socketfd
                if ((NULL != mac && FT_SUCCESS == FT_STRNCMP(pos->p_svr_desp->mac, mac, sizeof(pos->p_svr_desp->mac)))
                || (NULL == mac && socketfd > 0)) {  
                    p_client = &pos->client_node.entity;                    
                    FT_UNLOCK(&p_list_container->mutex);  ///�ͷ�������
                    return p_client;  ///�豸�Ѿ�ע��,ֱ�ӷ��سɹ�
                } else {
                ///
                Error("input param error\n");
                FT_UNLOCK(&p_list_container->mutex);  ///�ͷ�������
                return NULL;  ///�豸�Ѿ�ע��,ֱ�ӷ��سɹ�
            }
        }
    }       

    Info("mac:%s no register or has unregister\n", mac);
    
    /**�ͷ�������*/
    FT_UNLOCK(&p_list_container->mutex);

    return NULL;
}

