/**
* @file ikcc_native_socket.h
* @brief 提供本地网络服务
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
* @ Description: 初始化本地socket
* @ port:   socket端口 
* @ return: 成功:socket fd 失败:-1
*/
int ikcc_init_native_socket(const char* sun_path);  //AF_UNIX


#endif /* IKCC_NATIVE_SOCKET */
