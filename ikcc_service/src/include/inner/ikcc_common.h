/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : ikcc_common.h
* @ Version       : Initial Draft
* @ Author        : wangzhnegxue
* @ Created       : 2016/9/20
* @ Last Modified :
* @ Description   : ikcc内部公用结构体
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/9/20
* @   Author      : wangzhnegxue
* @   Modification: Created file

* @ 2.Date         : 2016/9/29
* @   Author       : wangzhnegxue
    Modification : 修改st_hw_func结构定义，注册函数内存由外部管理，必须以堆的形式存在

*/

#ifndef IKCC_COMMON_H
#define IKCC_COMMON_H

#include "list.h"
#include <assert.h>
#include <pthread.h>
#include "platform_type.h"
#include "ft_def.h"

/** 消息类型 */
typedef enum _enum_msg_type {
	DEV_DATA_UP = 0,            ///新数据上报 
	DEV_HEARTBEAT = 1,          /// 心跳信息 
	DEV_ONLINE,                 /// 设备上线 
	DEV_OFFLINE,                /// 设备离线 
	DEV_GET_INFO,               /// 获取设备信息 
	DEV_NET_ON_LINE = 5,        /// 设备网络上线 
	DEV_NET_OFF_LINE,           /// 设备网络断线 
	DEV_NET_CONNTECTING,        /// 设备网络连接中 
	CLOUND_CMD_TO_DEV,          /// 云端发送给设备消息 
	CLOUND_GET_STATUS,    
	SYS_MSG = 10,               ///系统消息
	DEV_MSG_NR,
} enum_msg_type;

/** 消息存储类型 */
typedef enum _enum_format_type {
	FORMAT_JSON =	0,				///json 消息
	FORMAT_XML =	1,				///xml 消息
	FORMAT_HEX =	2,				///16进制 消息
	FORMAT_ASSII =	3,				///ascii 消息
	FORMAT_UNKNOW 					///未知 消息
} enum_format_type;

/** 消息存储类型 */
typedef enum _enum_link_type {
	LINK_APP =		0,			///app
	LINK_LINKAGE =	1,			///联动
	LINK_ALI =		2,			///阿里云
	LINK_JINGDONG =	3,			///京东
	LINK_UNKNOW 
} enum_link_type;


/** 设备基本信息描述 */
typedef struct _st_msg_entity {
    enum_msg_type e_msg_type;  			///消息类型
    enum_format_type format;      		///消息存储类型,0-JSON 1-XML 2-HEX 3-ASCII 0xff-Unknown 
    enum_link_type link_type;           ///连接类型,0-app 1-linkage 2-ali 3-jindong .... 0xff-Unknown
	char src_mac[FT_STRING_LEN];		///消息源设备mac地址,设备唯一标识,用于区分不同设备	
	char dst_mac[FT_STRING_LEN];		///消息目标设备mac地址,设备唯一标识,用于区分不同设备
	char serial_number[FT_STRING_LEN];  //设备序列号
	int size;                   		//消息大小
	char payload[MAX_MESSAGE_LEN];  	///消息空间
	struct list_head list;				///消息节点
} st_msg_entity;              

typedef int (*callback_recv_msg_func)(st_msg_entity *p_msg_entity);

/** 注册服务描述 */
typedef struct _st_svr_desp {
    int socketfd;                       ///clinet fd
	char name[FT_STRING_LEN];          ///server name: socket, native_socket, bluetooth, zigbee 
	char mac[FT_STRING_LEN];           ///设备mac地址, 云标记 EG:fotile_cloud native_socket ali
	char key[FT_STRING_LEN];           ///unuseful
	callback_recv_msg_func callback_recv_msg;    ///注册函数
} st_svr_desp;              //dev register struct


/** 服务对象描述对象定义 */
typedef struct _st_list_container
{
   	struct list_head head;			///回调函数链表头节点
	FT_MUTEX_TYPE mutex;				///回调函数锁
	int number;                 ///已注册的设备统计数 
	int serial;                 ///注册的设备序号,一直增加
	struct list_head client_head;   ///客户端表
}st_list_container;

/**队列头结构体定义*/
typedef struct _st_ft_list_head {
	struct list_head list;      ///头结点
	FT_MUTEX_TYPE lock;         ///队列锁
	int length;     ///队列长度
}st_ft_list_head;

/** 客户端基本信息定义 */
typedef struct _st_clinet_entity {
    int fd;         ///socket fd
    int online;     ///stat
	int heartbeat_count;    ///heart	
	st_ft_list_head   queue;  ///msg queue
	char mac[FT_STRING_LEN];           ///mac地址,设备唯一标识,用于区分不同设备	
}st_clinet_entity;

typedef struct _st_clinet_node {
    st_clinet_entity entity;
	struct list_head list;      ///注册节点
}st_clinet_node;
/** 硬件管理函数注册管理结构 */
typedef struct _st_hw_func {
	st_svr_desp *p_svr_desp;       ///注册函数信息,st_hw_func指向空间以堆的形式存在
	st_clinet_node client_node;     ///客户端信息
    int serial_id;              ///注册序号
	struct list_head list;      ///注册节点
}st_hw_func;

#endif /* IKCC_COMMON_H */
