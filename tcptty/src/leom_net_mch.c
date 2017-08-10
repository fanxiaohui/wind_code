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
#include "leom_utils.h"
#include "leom_sensor_handle.h"


#define HEART_EXIT_COUNT	(5)

typedef enum _machine_stat {
	IDLE = 0,
	TO_LOGIN,
	TO_HEART,
	TO_DEVSTATUS,
	TO_WORKSTATUS,
	TO_RESPSTATUS,
	TO_RESP,
	NET_DISCONN,
	MAX_END
}MACH_STAT;

enum {
	E_DEV_INIT,
	E_DEV_ONLINE = 0x01,
	E_DEV_TAKEUP = 0x0b,
	E_DEV_RUNERROR	= 0x0e
};

extern st_glb_cfg glb_cfg;
static int dev_work_status = E_DEV_INIT;
unsigned int glb_seq = 0;

#include "leom_net_cmd.c"

int check_valid_id_name()
{
	if(deviceid[0] == 0x0 || devicename[0] == 0x0) {
		return 0;
	}
	return 1;
}

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
    } else {
		set_socket_keepalive(cli_fd);
		debug(LOG_NOTICE, "[None-blk-2]Connect OK.%s:%d\n",ip_addr,port);
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

/*
 * recv data from socket with timeout(ms)
 * @out：*pro, should be malloced outside of func
 * @return: -1:sock err; 0:intp or timeout; >0: recv length
*/
int socket_recv_tt(int socket , PRO *pro, int tt_ms)
{
	unsigned char recvbuf[ONLINE_MAX_LEN] = {0};
	int ret = 0;
	
	fd_set fds;
	struct timeval tv;
	if(tt_ms <= 0)
		tt_ms = 5000;

	while(socket >= 0 && pro) {
		
		FD_ZERO(&fds);
		FD_SET(socket, &fds); 
		tv.tv_sec = tt_ms/1000;
		tv.tv_usec = (tt_ms%1000)*1000;

		//server handle socket event
		if((ret = select(socket+1, &fds, NULL, NULL, &tv)) < 0) {
			if(errno == EINTR) {
				debug(LOG_NOTICE, "server socket select EINTR\n");
				continue;
			} else {
				debug(LOG_ERR, "select error:%d\n",errno);
				return -1;
			}
		} else if(ret == 0) {
			debug(LOG_NOTICE, "recv timeout...\n");
			return 0;
		}
		if(FD_ISSET(socket, &fds) <= 0) {
			debug(LOG_ERR, "something wrong while waiting for socket,error:%d\n",errno);
			return -1;
		}

		/* Firstly read PRO_HD */
		/* if ret < sizeof(PRO_HD); switch char* to PRO_HD cause crash ?? */
		//if((ret = recv(socket,&(pro->hd),sizeof(PRO_HD), 0)) <= 0) goto DISCONN;
		if((ret = recv(socket,recvbuf,sizeof(PRO_HD), 0)) <= 0) {
			debug(LOG_ERR, "=== recv socket error! %d:%s\n",errno,strerror(errno));
			return -1;
		}
		if(ret != sizeof(PRO_HD)) {
			debug(LOG_WARNING, "=== recv len %d < PRO_HD Len %d! ===\n",ret,sizeof(PRO_HD));
			return ret;
		} else {
			//pro->hd = *(PRO_HD*)recvbuf;
			memcpy((void*)&pro->hd, (void*)recvbuf, sizeof(PRO_HD));
		}
		/* Switch Net_Endian to Host_Endian */
		pro_net2host(&pro->hd);
		dump_phd(&pro->hd);
		
		if(pro->hd.len == 0) {
			debug(LOG_NOTICE, "=== Has No payload! ===\n");
			return sizeof(PRO_HD);
		} else {
			if(pro->hd.len > ONLINE_MAX_LEN) {
				debug(LOG_WARNING, "=== payload len %d > MAX_LEN %d; Cut to MAX_LEN! ===\n",pro->hd.len,ONLINE_MAX_LEN);
				pro->hd.len = ONLINE_MAX_LEN;
			}
			/* Read payload */
			if((ret = recv(socket, pro->payload, pro->hd.len, 0)) <= 0) {
				debug(LOG_ERR, "=== recv socket error! %d:%s\n",errno,strerror(errno));
				return -1;
			}
			if(ret != pro->hd.len) {
				debug(LOG_WARNING, "=== recv len %d < PRO payload Len %d! ===\n",ret,pro->hd.len);
			}
			//dump_pro(pro);
			return ret + sizeof(PRO_HD);
		}
	}
	return -1;
}

int loop_socket_handle(int socket, int tt_ms)
{
	unsigned char outhexbuf[ONE_TCP_MAX_LEN] = {0};
	PRO *pro = (PRO*)malloc(sizeof(PRO));
	memset(pro, 0, sizeof(PRO));
	pro->payload = (unsigned char*)malloc(ONLINE_MAX_LEN);
	memset(pro->payload, 0, (ONLINE_MAX_LEN));
	
	int ret = 0;
	int login_ok = 0;
	unsigned int sendseq = 0;
	int outinx = 0;
	int hexlen = 0;
	MACH_STAT stat = IDLE;
	
	while(1) {
		debug(LOG_NOTICE, "stat=%d,login_flag=%d,outinx=%d,last_cmd=%04x,last_recv_ret=%d,dev_work_status=%d\n",
							stat,login_ok,outinx,pro->hd.cmd,ret,dev_work_status);
		switch(stat) {
			case IDLE:
				//sleep_intp_s(5); //will block tcp recving
				break;
				
			case TO_LOGIN:
				memset(&pro->hd, 0, sizeof(PRO_HD));
				memset(pro->payload, 0, (ONLINE_MAX_LEN));
				send_login_info(socket, pro, sendseq++);
				break;
				
			case TO_DEVSTATUS:
				//recv_uart();
				//send_dev_data();
				if(glb_cfg.glb_sock != socket)
					glb_cfg.glb_sock = socket; // uart thread will write glb_sock directly.
				break;
				
			case TO_WORKSTATUS:
				memset(pro->payload, 0, (ONLINE_MAX_LEN));
				if(dev_work_status == E_DEV_TAKEUP) {
					send_busy_info(socket, pro, dev_work_status, sendseq++);
				} else {
					send_status_info(socket, pro, dev_work_status, sendseq++);
				}
				stat = IDLE;
				break;

			case TO_RESPSTATUS:
				memset(pro->payload, 0, (ONLINE_MAX_LEN));
				response_status_info(socket, pro, dev_work_status, sendseq++);
				stat = IDLE;
				break;
			
			case TO_HEART:
				memset(&pro->hd, 0, sizeof(PRO_HD));
				send_heart_info(socket, &pro->hd, sendseq++);
				if(outinx++ > HEART_EXIT_COUNT)
					goto EXIT_ERR;
				else
					stat = IDLE;
				break;
				
			case TO_RESP:
				//for testing;all back
				pro->hd.stat = PRO_RSP;
				memset(outhexbuf, 0, sizeof(outhexbuf));
				hexlen = 0;
				pro_pro2hexbuf(pro,outhexbuf,&hexlen);
				send(socket, outhexbuf, hexlen, sendseq++);
				debug(LOG_DEBUG, "Send Resp Len:%d\n",hexlen);
				stat = IDLE;
				break;
				
			case NET_DISCONN:
				goto EXIT_ERR;
				break;
			
			default:
				break;
		}
		
		//memset(pro, 0, sizeof(PRO)); //NOTE***: will init payload as NULL; this will cause crash!!!
		memset(&pro->hd, 0, sizeof(PRO_HD));
		memset(pro->payload, 0, (ONLINE_MAX_LEN));
		ret = socket_recv_tt(socket, pro, tt_ms);
		if(ret == 0) {
			// timeout
			if(!login_ok) {
				stat = TO_LOGIN;
			} else {
				stat = TO_HEART;
			}
		} else if(ret < 0) {
			// socket error
			goto EXIT_ERR;
			
		} else if( (ret != sizeof(PRO_HD)) && 
				   (ret != sizeof(PRO_HD)+pro->hd.len) ) {
			debug(LOG_NOTICE, "Recv Length %d Error!\n",ret);
			continue;
		} else {
			// handle data...
			switch(pro->hd.cmd) {
				case IDM_DEV_LOGIN:
					debug(LOG_NOTICE, "---< server back: login\n");
					login_ok = 1;
					if(dev_work_status != E_DEV_TAKEUP)
						dev_work_status = E_DEV_ONLINE;
					stat = TO_DEVSTATUS;
					break;
					
				case IDM_DEV_HEART:
					debug(LOG_NOTICE, "---< server back: heart\n");
					outinx = 0;
					break;
				
				case IDM_GETDEV:
					debug(LOG_NOTICE, "---< server cmd: get dev\n");
					stat = TO_RESPSTATUS;
					break;

				/* for response testing */
				case 0xfffe:
					debug(LOG_NOTICE, "---< server cmd: test response\n");
					stat = TO_RESP;
					break;
					
				case IDM_TAKEUPDEV:
				case IDM_RELEASEDEV:
				case IDM_RESET:
				case IDM_DOSTART:
				case IDM_DOSTOP:
				case IDM_SETPARS:
				case IDM_DELPARS:
					if(0 == memcmp(taskid, pro->payload+32, 32)) {
						break;
					} else {
						if(dev_work_status == E_DEV_TAKEUP) {
							debug(LOG_WARNING, "---< server taskid is not correct!\n");
							stat = TO_WORKSTATUS;
							continue;
						} else {
							/* Firstly takeup device */
							break;
						}
					}
				default:
					debug(LOG_ERR, "==== Handle: Unknow Server CMD 0x%02x\n", pro->hd.cmd);
					continue;
			}
			
			switch(pro->hd.cmd) {
				/* handle at local */
				case IDM_TAKEUPDEV:
					debug(LOG_NOTICE, "---< server cmd: takeup dev\n");
					dev_work_status = E_DEV_TAKEUP;
					stat = TO_WORKSTATUS;
					memcpy(taskid, pro->payload+32, sizeof(taskid)); /* skip devid 32*/
					break;

				case IDM_RELEASEDEV:
					debug(LOG_NOTICE, "---< server cmd: release dev\n");
					dev_work_status = E_DEV_ONLINE;
					stat = TO_WORKSTATUS;
					memset(taskid, 0, sizeof(taskid));
					break;
				
				/* send to uart */
				case IDM_RESET:
					debug(LOG_NOTICE, "---< server cmd: reset dev\n");
					dev_work_status = E_DEV_ONLINE;
					stat = TO_WORKSTATUS;
					/* stop and clear */
					dev_reset(pro->payload, pro->hd.len);
					break;

				case IDM_DOSTART:
					debug(LOG_NOTICE, "---< server cmd: start dev\n");
					glb_cfg.rsp_cmd_type = SVR_NEED_TASK_RET;
					glb_cfg.rsp_cmd_type |= SVR_NEED_TASK_PERCT;
					/* only send payload to uart */
					ret = dev_start(pro->payload, pro->hd.len);
					debug(LOG_DEBUG, "---Send To Com ret %d\n",ret);
					break;

				case IDM_DOSTOP:
					debug(LOG_NOTICE, "---< server cmd: stop dev\n");
					glb_cfg.rsp_cmd_type = SVR_NEED_TASK_RET;
					/* only send payload to uart */
					ret = dev_stop(pro->payload, pro->hd.len);
					debug(LOG_DEBUG, "---Send To Com ret %d\n",ret);
					break;

				case IDM_SETPARS:
					debug(LOG_NOTICE, "---< server cmd: set params dev\n");
					/* only send params to uart */
					ret = dev_setpar(pro->payload+64+2, (pro->hd.len-66)>0?(pro->hd.len-66):0);
					debug(LOG_DEBUG, "---Send To Com ret %d\n",ret);
					break;

				case IDM_DELPARS:
					debug(LOG_NOTICE, "---< server cmd: del params\n");
					/* only send payload to uart */
					ret = dev_clrpar(pro->payload, pro->hd.len);
					debug(LOG_DEBUG, "---Send To Com ret %d\n",ret);
					break;
			}
		}
		
		continue;
	} /* end while(1) */
	
EXIT_ERR:
	debug(LOG_NOTICE, "<--- Socket recv out!\n");
	glb_cfg.glb_sock = -1;
	glb_cfg.rsp_cmd_type = 0;
	if(socket > 0) close(socket);
	if(pro && pro->payload) {
		free(pro->payload);
		free(pro);
	}
	return -1;
}
