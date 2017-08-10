#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <termios.h>
#include <sys/un.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
/* for wait() */
#include <sys/wait.h>
#include "leom_dbg.h"
#include "leom_pro.h"
#include "leoman.h"

static void set_socket_keepalive(int socket)
{
	int keepalive = 1;
	int keepidle = 60; //def 7200s
	int keepinterval = 5;  //def 75s
	int keepcount = 3;  //def 9
	setsockopt(socket,SOL_SOCKET,SO_KEEPALIVE,(void*)&keepalive,sizeof(keepalive));
	setsockopt(socket,SOL_TCP,TCP_KEEPIDLE,(void*)&keepidle,sizeof(keepidle));
	setsockopt(socket,SOL_TCP,TCP_KEEPINTVL,(void*)&keepinterval,sizeof(keepinterval));
	setsockopt(socket,SOL_TCP,TCP_KEEPCNT,(void*)&keepcount,sizeof(keepcount));
}


int init_connect(const char *ip_addr, int port, int conn_out_flag)
{
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;

	bzero(&client_addr,sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);
	
	int cli_fd = socket(AF_INET, SOCK_STREAM,0);
	if(cli_fd < 0){
        debug(LOG_ERR, "Create Socket Failed!%d:%s\n",errno,strerror(errno));
        return -1;
    }

//   int nRecvBuf=50*1024;//设置为32K
//	setsockopt(cli_fd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));

    if(bind(cli_fd, (struct sockaddr*)&client_addr,sizeof(client_addr))) {
        debug(LOG_ERR, "Client Bind Port Failed! %d:%s\n",errno,strerror(errno)); 
        goto EXIT;
    }
	
	bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(ip_addr, &server_addr.sin_addr) == 0){
        debug(LOG_ERR, "inet_aton Server IP Address Error! %d:%s\n",errno,strerror(errno));
        goto EXIT;
    }
    server_addr.sin_port = htons(port);
	debug(LOG_NOTICE, "Connecting %s:%d...\n",ip_addr,port);
	
if(conn_out_flag) {
	int flags = fcntl(cli_fd,F_GETFL,0);
    fcntl(cli_fd,F_SETFL,flags | O_NONBLOCK);
	
	int n = connect(cli_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(n < 0)
	{
        if(errno != EINPROGRESS && errno != EWOULDBLOCK)
		{
			debug(LOG_ERR, "connect is not EINPROGRESS! %d:%s\n",errno,strerror(errno));
			goto EXIT;
		}

        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(cli_fd,&wset);
        n = select(cli_fd+1,NULL,&wset,NULL,&tv);
        if(n < 0) {
			debug(LOG_ERR, "Connect select() error. %d:%s\n",errno,strerror(errno));
			goto EXIT;
        } else if (0 == n) {
            debug(LOG_ERR, "Connect select time out.\n");
			goto EXIT;
        } else {
			set_socket_keepalive(cli_fd);
            debug(LOG_NOTICE, "[None-blk]Connect OK.%s:%d\n",ip_addr,port);
        }
    }
    
    //fcntl(cli_fd,F_SETFL,flags & ~O_NONBLOCK);
} else {
	//def connect timeout 75s
	if(connect(cli_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        debug(LOG_ERR, "Can NOT connect! %d:%s\n",errno,strerror(errno));
        goto EXIT;
    }
    set_socket_keepalive(cli_fd);
    debug(LOG_NOTICE, "[Blk] Connect OK.%s:%d\n",ip_addr,port);
}

    return cli_fd;
EXIT:
	if(cli_fd >= 0) close(cli_fd);
	return -1;
}

static int sock_send(int sock, char *send_buf,int send_len)
{
	int ret = 0;
	if((ret=send(sock,send_buf,send_len,0)) <= 0) {
		debug(LOG_ERR, "socket send error!%d:%s\n",errno,strerror(errno));
		return -1;
	}
	if(ret != send_len)
		debug(LOG_WARNING, "Warning! socket send %d < %d\n",ret,send_len);

	return ret;
}

int send_board_info(int socket, int index)
{
	char info[1024] = {0};
	time_t tt;
    time(&tt);

   	snprintf(info,sizeof(info)-1,"\n[%d] Board's Time: %s\n",index,ctime(&tt));
   	return sock_send(socket,info,strlen(info));
}


int loop_socket_recv(int socket ,int tt_ms)
{
	unsigned char recvbuf[MAX_PAYLEN] = {0};
	int info_count = 0, ret = 0;
	int outinx = 0;
	unsigned int hexlen = 0;
	
	fd_set fds;
	struct timeval tv;
	if(tt_ms <= 0)
		tt_ms = 5000;
	PRO *pro = (PRO*)malloc(sizeof(PRO));
	memset(pro, 0, sizeof(PRO));
	pro->payload = (unsigned char*)malloc(MAX_PAYLEN);
	memset(pro->payload, 0, sizeof(MAX_PAYLEN));
	
	while(socket >= 0) {
		FD_ZERO(&fds);
		FD_SET(socket, &fds); 
		tv.tv_sec = tt_ms/1000;
		tv.tv_usec = (tt_ms%1000)*1000;

		//server handle socket event
		if((ret = select(socket+1, &fds, NULL, NULL, &tv)) < 0) {
			if(errno == EINTR) {
				//gettimeofday(&last_tv,NULL);
				debug(LOG_NOTICE, "server socket select EINTR\n");
				
				continue;
			} else {
				debug(LOG_ERR, "select error:%d\n",errno);
				goto DISCONN;
			}
		} else if(ret == 0) {
			debug(LOG_NOTICE, "timeout...%d\n",outinx);
			if(outinx++ > 10) {
				debug(LOG_NOTICE, "Get Server heartbeat timeout!\n");
				goto DISCONN;
			}
			ret = send_board_info(socket,info_count++);
			if(ret <= 0) {
				debug(LOG_ERR, "Error send gpsinfo!\n");
				goto DISCONN;
			} else {
				continue;
			}
		}
		if(FD_ISSET(socket, &fds) <= 0) {
			debug(LOG_ERR, "something wrong while waiting for socket,error:%d\n",errno);
			goto DISCONN;
		}
		outinx = 0;
		memset(recvbuf,0,sizeof(recvbuf));
		//memset(pro,0,sizeof(PRO)); will clear payload as NULL
		memset(pro->payload,0,MAX_PAYLEN);
		
		/* Firstly read PRO_HD */
		/* if ret < sizeof(PRO_HD); switch char* to PRO_HD cause crash ?? */
		//if((ret = recv(socket,&(pro->hd),sizeof(PRO_HD), 0)) <= 0) goto DISCONN;
		if((ret = recv(socket,recvbuf,sizeof(PRO_HD), 0)) <= 0) goto DISCONN;
		if(ret != sizeof(PRO_HD)) {
			debug(LOG_WARNING, "=== recv len %d < PRO_HD Len %d! ===\n",ret,sizeof(PRO_HD));
			continue;
		} else {
			pro->hd = *(PRO_HD*)recvbuf;
			dump_phd(&pro->hd);
		}
		/* Switch Net_Endian to Host_Endian */
		pro_net2host(&pro->hd);
		dump_phd(&pro->hd);
		if(pro->hd.len == 0) {
			debug(LOG_NOTICE, "=== Has No payload! ===\n");
		} else {
			/* Read payload */
			if((ret = recv(socket, pro->payload, pro->hd.len, 0)) <= 0) goto DISCONN;
			if(ret != pro->hd.len) {
				debug(LOG_WARNING, "=== recv len %d < PRO payload Len %d! ===\n",ret,pro->hd.len);
				dump_pro(pro);
				continue;
			}
		}
		/* all to hex to uart */
		//memset(recvbuf,0,sizeof(recvbuf));
		//pro_pro2hexbuf(pro,recvbuf,&hexlen);
		//leom_handle_sock_cmd(recvbuf,hexlen);
		//hexlen = 0;
		
		/* only send payload to uart */
		leom_handle_sock_cmd(pro->payload, pro->hd.len);
		
		continue;

	DISCONN:
		if(socket > 0) close(socket);
		socket = -1;
		if(pro && pro->payload) {
			free(pro->payload);
			free(pro);
		}
		return -1;
	}
	if(socket > 0) close(socket);
	if(pro && pro->payload) {
		free(pro->payload);
		free(pro);
	}
	return 0;
}



