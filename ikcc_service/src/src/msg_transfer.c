/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : msg_transfer.c
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/19
* @ Last Modified :
  brief   : ʵ����Ϣ�ַ�����
* @ Function List :
              message_handle_thread
              msg_transfer_init
              push_cloud_msg
              push_dev_msg
* @ History       :
* @ 1.Date        : 2016/9/19
* @   Author      : wangzhengxue
* @   Modification: Created file

*/

#include <stdlib.h>
#include "msg_transfer.h"


/*-------------------------------------------------------*/
/**��̬ȫ�ֱ�������*/

static st_cmd_queue_service cmd_service;    ///��Ϣ�����嶨��


/*-------------------------------------------------------*/
/**��̬��������*/

static void *message_handle_thread(void *args);     ///��Ϣ�����߳�����


/**
* @ Prototype    : message_handle_thread
* @  Description  : ��Ϣ�ַ��߳�,���豸��Ϣ���л��ƶ���Ϣ����ȡ������,�����ݷ��͸���һ��ģ�飬
                    �̲߳����˳�������������Ϣʱ�Ὣ������Ϣȫ������Ż��������
* @ Input        : void *args  
* @  Output       : None
* @  Return Value : static
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
static void *message_handle_thread(void *args)
{
	st_msg_entity *p_msg_entity = NULL;		///��Ϣָ������
	
	while(1) {	
		/**�����豸����Ϣ*/		
		POP_FROMQUEUE(p_msg_entity,list,&cmd_service.dev_msg_queue);	///���豸��Ϣ���н���Ϣȡ�����Ƴ�����	
		
		while(p_msg_entity) {	///��Ϣ�ǿս�һֱ����ֱ��������ɣ���֤ʵʱ��

			ds_send_msg_tocloud(p_msg_entity);	///���豸��Ϣת�������ݷ���ģ�飬���ɺ�˽�һ������
			
			FT_FREE(p_msg_entity);		///�ͷ���Ϣ�ڴ�	
			p_msg_entity = NULL;		///��ֹҰָ��

			/**��ȡ��һ���豸��Ϣ*/
			POP_FROMQUEUE(p_msg_entity,list,&cmd_service.dev_msg_queue);			
		}


		/**�����ƶ���Ϣ*/		
		POP_FROMQUEUE(p_msg_entity,list,&cmd_service.clound_msg_queue);	///��ȡ�ƶ���Ϣ
		
		while(p_msg_entity) {	///��Ϣ�ǿգ�����
		
			hw_send_msg_toclient(p_msg_entity);	///����Ϣ���͸���������ģ��
			FT_FREE(p_msg_entity);	///�ͷ���Ϣ�洢�ռ�
			p_msg_entity = NULL;		///��ֹҰָ��	

			/**��ȡ��һ���ƶ���Ϣ*/
			POP_FROMQUEUE(p_msg_entity,list,&cmd_service.clound_msg_queue);			
		}

		usleep(10000);  /// schedule,sleep 10ms
	}
	
}

/**
* @ Prototype    : msg_transfer_init
 Description  : :��Ϣ�ַ���ʼ��
* @ Input        : void  
* @  Output       : None
* @  Return Value : FT_SUCCESS,��Ϣ�ַ���ʼ���ɹ�;FT_FAILURE,��Ϣ�ַ���ʼ��ʧ��
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int msg_transfer_init(void)
{
    /**������Ϣָ��*/
    st_cmd_queue_service *p_cmd_service = &cmd_service;

    /**�����豸������ƶ�������Ϣ����*/
    CREATE_QUEUE(&p_cmd_service->dev_msg_queue);
    CREATE_QUEUE(&p_cmd_service->clound_msg_queue);

    /**������Ϣ�����߳�*/
    FT_THREAD_CREATE(&p_cmd_service->thread[0], message_handle_thread,NULL);
    FT_THREAD_DETACH(p_cmd_service->thread[0]); ///���ô������߳�����������߳���Դ

    return FT_SUCCESS;
}

/**
* @ Prototype    : push_dev_msg
 Description  : ͨ���˽ӿڽ��ϱ�����Ϣ������Ϣ����
* @ Input        : st_msg_entity* p_msg_entity  
* @  Output       : None
* @  Return Value : FT_SUCCESS,�����豸��Ϣ���гɹ�;FT_FAILURE,�����豸��Ϣ����ʧ��
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int push_dev_msg(st_msg_entity* p_msg_entity)
{
    /**������Ϣָ��*/
    st_cmd_queue_service *p_cmd_service = &cmd_service;

	PUSH_ENQUEUE(p_msg_entity, list, &p_cmd_service->dev_msg_queue);	///�����
	
    return FT_SUCCESS;
}

/**
* @ Prototype    : push_cloud_msg
 Description  : ͨ���˽ӿڽ��ƶ���Ϣ������Ϣ�ƶ���Ϣ����
* @ Input        : st_msg_entity* p_msg_entity  
* @  Output       : None
* @  Return Value : FT_SUCCESS,�����ƶ���Ϣ���гɹ�;FT_FAILURE,�����ƶ���Ϣ����ʧ��
* @  Calls        : 
* @  Called By    : 
 
* @   History        :
* @   1.Date         : 2016/9/19
* @     Author       : wangzhengxue
* @     Modification : Created function

*/
int push_cloud_msg(st_msg_entity* p_msg_entity)
{
    /**������Ϣָ��*/
    st_cmd_queue_service *p_cmd_service = &cmd_service;

	PUSH_ENQUEUE(p_msg_entity, list, &p_cmd_service->clound_msg_queue);   ///�����
	
    return FT_SUCCESS;
}


