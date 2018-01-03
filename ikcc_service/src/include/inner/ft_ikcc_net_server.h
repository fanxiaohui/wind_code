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
	struct list_head list;			///��Ϣ�ڵ�
}st_client_device;

/**
* @ Description: ��ʼ���������ģ��
* @ param port:   �˿�
* @ return: �ɹ�:0 ʧ��:-1
*/
int ikcc_init_net_server(const int port);

#endif
