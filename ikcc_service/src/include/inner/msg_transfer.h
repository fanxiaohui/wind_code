/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : msg_transfer.h
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/18
* @ Last Modified :
  brief   : ��Ϣ�ַ�ģ��ͷ�ļ�,������Ϣ��صĽṹ�庯������
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/9/18
* @   Author      : wangzhengxue
* @   Modification: Created file

*/

#ifndef MSG_TRANSFER_H
#define MSG_TRANSFER_H

#include "ft_queue.h"
#include "ikcc_common.h"

#define MAX_MSG_THREAD	(32)	///�ɴ������������̵߳�����߳���


/**��Ϣ���нṹ*/
typedef struct _st_cmd_queue_service {	    
	st_ft_list_head dev_msg_queue;		///�豸������Ϣ����
	
	st_ft_list_head clound_msg_queue;		///�ƶ�������Ϣ����

	FT_PTHREAD_TYPE	thread[MAX_MSG_THREAD];	///��Ϣ�����߳�ID
}st_cmd_queue_service;


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
int msg_transfer_init(void);


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
int push_dev_msg(st_msg_entity* p_msg_entity);


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
int push_cloud_msg(st_msg_entity* p_msg_entity);


#endif /* MSG_TRANSFER_H */
