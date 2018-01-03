/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : ikcc_common.h
* @ Version       : Initial Draft
* @ Author        : wangzhnegxue
* @ Created       : 2016/9/20
* @ Last Modified :
* @ Description   : ikcc�ڲ����ýṹ��
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/9/20
* @   Author      : wangzhnegxue
* @   Modification: Created file

* @ 2.Date         : 2016/9/29
* @   Author       : wangzhnegxue
    Modification : �޸�st_hw_func�ṹ���壬ע�ắ���ڴ����ⲿ���������Զѵ���ʽ����

*/

#ifndef IKCC_COMMON_H
#define IKCC_COMMON_H

#include "list.h"
#include <assert.h>
#include <pthread.h>
#include "platform_type.h"
#include "ft_def.h"

/** ��Ϣ���� */
typedef enum _enum_msg_type {
	DEV_DATA_UP = 0,            ///�������ϱ� 
	DEV_HEARTBEAT = 1,          /// ������Ϣ 
	DEV_ONLINE,                 /// �豸���� 
	DEV_OFFLINE,                /// �豸���� 
	DEV_GET_INFO,               /// ��ȡ�豸��Ϣ 
	DEV_NET_ON_LINE = 5,        /// �豸�������� 
	DEV_NET_OFF_LINE,           /// �豸������� 
	DEV_NET_CONNTECTING,        /// �豸���������� 
	CLOUND_CMD_TO_DEV,          /// �ƶ˷��͸��豸��Ϣ 
	CLOUND_GET_STATUS,    
	SYS_MSG = 10,               ///ϵͳ��Ϣ
	DEV_MSG_NR,
} enum_msg_type;

/** ��Ϣ�洢���� */
typedef enum _enum_format_type {
	FORMAT_JSON =	0,				///json ��Ϣ
	FORMAT_XML =	1,				///xml ��Ϣ
	FORMAT_HEX =	2,				///16���� ��Ϣ
	FORMAT_ASSII =	3,				///ascii ��Ϣ
	FORMAT_UNKNOW 					///δ֪ ��Ϣ
} enum_format_type;

/** ��Ϣ�洢���� */
typedef enum _enum_link_type {
	LINK_APP =		0,			///app
	LINK_LINKAGE =	1,			///����
	LINK_ALI =		2,			///������
	LINK_JINGDONG =	3,			///����
	LINK_UNKNOW 
} enum_link_type;


/** �豸������Ϣ���� */
typedef struct _st_msg_entity {
    enum_msg_type e_msg_type;  			///��Ϣ����
    enum_format_type format;      		///��Ϣ�洢����,0-JSON 1-XML 2-HEX 3-ASCII 0xff-Unknown 
    enum_link_type link_type;           ///��������,0-app 1-linkage 2-ali 3-jindong .... 0xff-Unknown
	char src_mac[FT_STRING_LEN];		///��ϢԴ�豸mac��ַ,�豸Ψһ��ʶ,�������ֲ�ͬ�豸	
	char dst_mac[FT_STRING_LEN];		///��ϢĿ���豸mac��ַ,�豸Ψһ��ʶ,�������ֲ�ͬ�豸
	char serial_number[FT_STRING_LEN];  //�豸���к�
	int size;                   		//��Ϣ��С
	char payload[MAX_MESSAGE_LEN];  	///��Ϣ�ռ�
	struct list_head list;				///��Ϣ�ڵ�
} st_msg_entity;              

typedef int (*callback_recv_msg_func)(st_msg_entity *p_msg_entity);

/** ע��������� */
typedef struct _st_svr_desp {
    int socketfd;                       ///clinet fd
	char name[FT_STRING_LEN];          ///server name: socket, native_socket, bluetooth, zigbee 
	char mac[FT_STRING_LEN];           ///�豸mac��ַ, �Ʊ�� EG:fotile_cloud native_socket ali
	char key[FT_STRING_LEN];           ///unuseful
	callback_recv_msg_func callback_recv_msg;    ///ע�ắ��
} st_svr_desp;              //dev register struct


/** ����������������� */
typedef struct _st_list_container
{
   	struct list_head head;			///�ص���������ͷ�ڵ�
	FT_MUTEX_TYPE mutex;				///�ص�������
	int number;                 ///��ע����豸ͳ���� 
	int serial;                 ///ע����豸���,һֱ����
	struct list_head client_head;   ///�ͻ��˱�
}st_list_container;

/**����ͷ�ṹ�嶨��*/
typedef struct _st_ft_list_head {
	struct list_head list;      ///ͷ���
	FT_MUTEX_TYPE lock;         ///������
	int length;     ///���г���
}st_ft_list_head;

/** �ͻ��˻�����Ϣ���� */
typedef struct _st_clinet_entity {
    int fd;         ///socket fd
    int online;     ///stat
	int heartbeat_count;    ///heart	
	st_ft_list_head   queue;  ///msg queue
	char mac[FT_STRING_LEN];           ///mac��ַ,�豸Ψһ��ʶ,�������ֲ�ͬ�豸	
}st_clinet_entity;

typedef struct _st_clinet_node {
    st_clinet_entity entity;
	struct list_head list;      ///ע��ڵ�
}st_clinet_node;
/** Ӳ��������ע�����ṹ */
typedef struct _st_hw_func {
	st_svr_desp *p_svr_desp;       ///ע�ắ����Ϣ,st_hw_funcָ��ռ��Զѵ���ʽ����
	st_clinet_node client_node;     ///�ͻ�����Ϣ
    int serial_id;              ///ע�����
	struct list_head list;      ///ע��ڵ�
}st_hw_func;

#endif /* IKCC_COMMON_H */
