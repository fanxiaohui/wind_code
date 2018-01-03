/**

* @ Copyright (C), 2001-2016,Fotile.

* @ **
* @ File Name     : ft_def.h
* @ Version       : Initial Draft
* @ Author        : cairj
* @ Created       : 2016/10/12
* @ Last Modified :
* @ Description   : 模块间通信协议定义
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/10/12
* @   Author      : wangzhnegxue
* @   Modification: 完善消息类型及消息结构的定义

*/

#ifndef IKCC_DEF_H
#define IKCC_DEF_H

#define FT_STRING_LEN  32
#define MAX_MESSAGE_LEN 2048

#define FT_SUCCESS (0)      ///返回成功
#define FT_FAILURE (-1)     ///返回失败
#define FALSE (0)           ///假
#define TRUE (1)            ///真
#define FT_STRING_LEN  32      
#define IN
#define OUT


/** 消息类型 */
typedef enum _enum_net_msg_type{
	IKCC_DATA	        = 0,    ///数据信息
	IKCC_LOGIN		    = 1,	///登录请求
	IKCC_LOGIN_ACK      = 2,    ///登录请求ack
	IKCC_LOGOUT		    = 3,	///登出请求
	IKCC_LOGOUT_ACK     = 4,    ///登出请求ack
	IKCC_HEARTBEAT	    = 5,	///心跳
	IKCC_HEARTBEAT_ACK	= 6,    ///心跳ack
	IKCC_UPDATE         = 7,	///设备升级	
	IKCC_UPDATE_ACK     = 8,	///设备升级ack
	MSG_NUM
}enum_net_msg_type;

/** 设备信息结构 */
typedef struct _st_client_attr
{
    char username[FT_STRING_LEN];
    char password[FT_STRING_LEN];
    char key[FT_STRING_LEN];            //EG: fotile
    char identify[FT_STRING_LEN];       ///设备的唯一标识,设备未mac地址，云为其它
    char dev_type[FT_STRING_LEN];
}st_client_attr;

/** 数据信息结构 */
typedef struct _st_client_data {
    /** 设备消息为设备地址；云端消息为目标地址 */
    char identify[FT_STRING_LEN];    
    int size;       ///msg size
    
    /** 请参考st_client_msg定义说明，禁止在buffer后添加新成员 */
    char buffer[MAX_MESSAGE_LEN];   ///msg 
}st_client_data;

/** 设备通信消息结构 */
typedef struct _st_client_msg{
    enum_net_msg_type msg_type;
    
    /** 新成员添加开始处*/
    
    /**新成员添加结束处*/
    
    union
    {
        st_client_attr client_attr;
        st_client_data client_data;
    }msgbody;
    /** 由于为了较少无效的消息发送，设计时将消息实体放在buffer最后，禁止
    在msgbody成员后面添加新的成员，如果st_clinet_msg需要添加新成员请
    在msgbody成员之前添加，并更新相应文档说明 */
}st_client_msg;

/** 获取需要发送消息的空间大小 */
#define GET_SEND_DATA_SIZE(client_msg) (IKCC_LOGIN == client_msg.msg_type || IKCC_LOGOUT == client_msg.msg_type)?\
    (sizeof(client_msg.msg_type) + sizeof(client_msg.msgbody.client_attr)):\
    (((unsigned long int)&((typeof(client_msg) *)0)->msgbody.client_data.buffer) + client_msg.msgbody.client_data.size)

#endif /* IKCC_DEF_H */
