/*
 * xlink_linux.h
 *
 *  Created on: 2015年9月6日
 *      Author: XT800
 */

#ifndef XLINK_LINUX_H_
#define XLINK_LINUX_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "time.h"
#include "unistd.h"

#if CLIENT_SSL_ENABLE
#include <ssl.h>
//#include <internal.h>
//#include <cyassl_config.h>

#define CYASSL          SSL
#define CYASSL_SESSION  SSL_SESSION
#define CYASSL_METHOD   SSL_METHOD
#define CYASSL_CTX      SSL_CTX

#define CYASSL_X509       X509
#define CYASSL_X509_NAME  X509_NAME
#define CYASSL_X509_CHAIN X509_CHAIN
#define CyaSSL_write SSL_write
#define CyaSSL_read SSL_read
#define CyaSSL_connect SSL_connect
#define SSL_SUCCESS 1
#define CyaSSL_get_error SSL_get_error
#define CyaSSL_shutdown //SSL_shutdown
#define CyaSSL_free    SSL_free
#define CyaSSL_CTX_free SSL_CTX_free
#define CyaSSL_Init  SSL_library_init
#define CyaTLSv1_2_client_method TLSv1_client_method
#define CyaSSL_CTX_new SSL_CTX_new
#define CyaSSL_CTX_set_verify SSL_CTX_set_verify
#define CyaSSL_new SSL_new
#define CyaSSL_set_fd  SSL_set_fd
#define CyaSSL_ERR_error_string //
#endif

#define  XLINK_FUNC

#define   xlink_sprintf             sprintf
#define   xlink_strlen(x)      strlen(x)
#define   xlink_strcmp(x,t)    strcmp(x,t)
#define   xlink_strncmp(x,t,y)    strncmp((char*)(x),(char*)(t),y)
#define   xlink_memset(x,d,t)  memset(x,d,t)
#define   xlink_memcpy(x,d,l)  memcpy((char *)(x),(char *)(d),l)
#define   xlink_msleep(n)       usleep(n)

#define   xlink_socket          socket
#define   xlink_bind             bind
#define   xlink_connect       connect
#define   xlink_setsockopt     setsockopt
#define   xlink_recv              recv
#define   xlink_recvfrom      recvfrom
#define   xlink_sendto          sendto
#define   xlink_send             send
#define   xlink_close(x)        close(x)
#define   xlink_set_fd            fd_set
#define   xlink_select(a,b,c,d,e)      select(a,b,c,d,e)

//ssl
#if CLIENT_SSL_ENABLE
#define   xlink_ssl_send           CyaSSL_write
#define   xlink_ssl_recv           CyaSSL_read
#define   xlink_ssl_connect        CyaSSL_connect
#else
#define   xlink_send           		 send
#define   xlink_recv               recv
#endif

typedef unsigned long long int xsdk_time_ms;
typedef unsigned long long int xsdk_time_t;
typedef struct sockaddr_in xlink_addr;

#ifdef  __cplusplus
}
#endif
#endif /* XLINK_LINUX_H_ */
