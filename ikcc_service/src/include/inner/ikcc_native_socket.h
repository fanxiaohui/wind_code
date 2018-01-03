/**
* @file ikcc_native_socket.h
* @brief �ṩ�����������
* @version 1.1
* @author cairj
* @date 2016/08/09
*/

#ifndef IKCC_NATIVE_SOCKET
#define IKCC_NATIVE_SOCKET

#include "hw_management.h"
#include "ft_def.h"
#include "ft_queue.h"
#include "ikcc_common.h"


/**
* @ Description: ��ʼ������socket
* @ port:   socket�˿� 
* @ return: �ɹ�:socket fd ʧ��:-1
*/
int ikcc_init_native_socket(const char* sun_path);  //AF_UNIX


#endif /* IKCC_NATIVE_SOCKET */
