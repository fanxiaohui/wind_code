/**
* @file ft_ikcc_net_server.h
* @brief (to add)
* @version 1.1
* @author cairj
* @date 2016/08/09
*/

#ifndef FT_IKCC_NET_SERVER_H
#define FT_IKCC_NET_SERVER_H
#include "ft_def.h"
#include "ft_queue.h"
#include "ikcc_common.h"

typedef struct _st_client_device{
    char mac[FT_STRING_LEN];
    int fd;
	int online;
	int heartbeat_count;
	st_ft_list_head p_queue;
	struct list_head list;			///消息节点
}st_client_device;

/**
* @ Description: 初始化网络服务模块
* @ param port:   端口
* @ return: 成功:0 失败:-1
*/
int ikcc_init_net_server(const int port);

#endif
