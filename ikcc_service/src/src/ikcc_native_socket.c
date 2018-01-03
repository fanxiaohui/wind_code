#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <pthread.h>
#include "ft_queue.h"
#include "tool/ft_set.h"
#include "ikcc_native_socket.h"
#include "ds_management.h"
#include "print.h"
#include "ikcc_common.h"

static st_maxdata_set s_fdset;				//native socket fd set
static st_ft_list_head s_cloud_msg_queue;	



static int __push_msg_to_cloud(const st_msg_entity* p_msg_entity)
{
	if (p_msg_entity == NULL ) {
		Error("p_msg_entity[%p] !", p_msg_entity);
		return FT_FAILURE;
	}
	st_msg_entity* dev_msg = FT_MALLOC(sizeof(st_msg_entity));
	FT_MEMCPY(dev_msg, p_msg_entity, sizeof(st_msg_entity));

	Debug("%d, %s, %d", p_msg_entity->e_msg_type, p_msg_entity->dst_mac, p_msg_entity->size);
	PUSH_ENQUEUE(dev_msg, list, &s_cloud_msg_queue);

}

static int __callback_recv_dev_msg(st_msg_entity *p_msg_entity)
{
	return __push_msg_to_cloud(p_msg_entity);
}

static void set_native_msg_entity(st_msg_entity* msg, const enum_msg_type type, const char* mac, const char* payload, const int len)
{
	if (msg == NULL || mac == NULL || payload == NULL) {
		Error("msg[%p]mac[%p]payload[%p]!", msg, mac, payload);
	}
	msg->e_msg_type = type;
	msg->size = len;
	msg->format = FORMAT_HEX;					//hex
	msg->link_type = LINK_UNKNOW;				//unknow
	FT_MEMCPY(msg->dst_mac, mac, FT_STRING_LEN);
	FT_MEMCPY(msg->payload, payload, len);
}

static int __cloud_login(const int client_fd, const st_client_attr* p_client_attr)       //uart protocol
{
	if (p_client_attr == NULL ) {
		Error("p_client_attr[%p] !", p_client_attr);
		return FT_FAILURE;
	}
	st_client_msg client_msg = {0};
	// check cloud
	char result;
	if (!FT_STRCMP(p_client_attr->key, "fotile_cloud")) {
		/**********send login success response******/
		result = 0;
		
		st_msg_entity msg_entity;
		set_native_msg_entity(&msg_entity, DEV_ONLINE, p_client_attr->identify, &result, sizeof(result));
		
		Info("send login success response!");
		__push_msg_to_cloud(&msg_entity);
		/******************************************/

	} else {
		/**********send login  fail response*******/
		result = 1;
	
		st_msg_entity msg_entity;
		set_native_msg_entity(&msg_entity, DEV_ONLINE, p_client_attr->identify, &result, sizeof(result));

		Info("send login fail response!");
		__push_msg_to_cloud(&msg_entity);
		/******************************************/
	}
	
	//register
	st_svr_desp* svr_desp = FT_MALLOC(sizeof(st_svr_desp));
	FT_MEMSET(svr_desp, 0, sizeof(st_svr_desp));
	FT_STRNCPY(svr_desp->name, "native_socket", FT_STRING_LEN-1);		
	FT_STRNCPY(svr_desp->mac, p_client_attr->identify, FT_STRING_LEN-1);		//云没有mac
	FT_STRNCPY(svr_desp->key, p_client_attr->key, FT_STRING_LEN-1);
	svr_desp->callback_recv_msg = __callback_recv_dev_msg;

	return ds_register_service(svr_desp);
}
static int __cloud_msg(const st_client_data* p_client_data)
{
	ASSERT(p_client_data);
	if (p_client_data->identify == NULL || p_client_data->buffer == NULL) {
		Error("msg[%p] mac[%p]!", p_client_data->buffer, p_client_data->identify);
		return FT_FAILURE;
	}
	st_msg_entity *pmsg = FT_MALLOC(sizeof(st_msg_entity));
	FT_MEMSET(pmsg, 0, sizeof(st_msg_entity));

	set_native_msg_entity(pmsg, DEV_DATA_UP, p_client_data->identify, p_client_data->buffer, p_client_data->size);

    return ds_send_msg_tobuf(pmsg);
}
static int __cloud_logout(const st_client_attr* p_client_attr)
{
	if (p_client_attr == NULL) {
			Error("info is NULL!");
			return FT_FAILURE;
	}

	// if user right
	char result = 0;		//0:success 1:user error 2:password error
	
	/**********send login response*************/
	st_msg_entity msg_entity;
	set_native_msg_entity(&msg_entity, DEV_OFFLINE, p_client_attr->identify, &result, sizeof(result));
	Info("send logout response");
	__push_msg_to_cloud(&msg_entity);
	/******************************************/
	
    return ds_unregister_service(p_client_attr->identify);
}

static int __send_to_cloud(const st_msg_entity *p_msg_entity, int fd)
{
	st_client_msg client_msg = {0};

	if (p_msg_entity == NULL) {
		Error("p_msg is NULL!");
		return FT_FAILURE;
	}
	if (fd < 0) {
		Error("fd[%d] error!!", fd);
		return FT_FAILURE;
	}
	
    switch (p_msg_entity->e_msg_type) {
        case DEV_ONLINE:
        {
            client_msg.msg_type = IKCC_LOGIN_ACK;
            break;
        };
        case DEV_OFFLINE:
        {
            client_msg.msg_type = IKCC_LOGOUT_ACK;
            break;
        };
        case DEV_HEARTBEAT:
        {
            client_msg.msg_type = IKCC_HEARTBEAT_ACK;
            break;
        };
		case DEV_DATA_UP:
        {
            client_msg.msg_type = IKCC_DATA;
            break;
        };
        default:
        {
            client_msg.msg_type = IKCC_DATA;
            break;
        }
    }

	FT_STRNCPY(client_msg.msgbody.client_data.identify, p_msg_entity->dst_mac, FT_STRING_LEN);
	client_msg.msgbody.client_data.size = p_msg_entity->size;
	FT_MEMCPY(client_msg.msgbody.client_data.buffer, p_msg_entity->payload, p_msg_entity->size);
   	int len = GET_SEND_DATA_SIZE(client_msg);	

	int ret = send(fd, &client_msg, len, 0);
	if(ret <= 0) {
        Error("send failed!!! ret[%d] fd:%d len:%d", ret, fd, len);
        return FT_FAILURE;
	} else {
		return FT_SUCCESS;
	}
}

static int __on_rcv_cloud_msg(const int client_fd, const char* msg, int len)
{
	int ret = 0;
    st_client_msg *p_client_msg = (st_client_msg *)msg;

	if (msg == NULL){
		Error("msg is NULL!");
		return FT_FAILURE;
	}

    switch (p_client_msg->msg_type) {
        case IKCC_LOGIN:
        {
            ret =  __cloud_login(client_fd, &p_client_msg->msgbody.client_attr);            
            break;
        };
        case IKCC_LOGOUT:
        {            
            ret = __cloud_logout(&p_client_msg->msgbody.client_attr);            
            break;
        };
        case IKCC_DATA:
        {
            ret = __cloud_msg(&p_client_msg->msgbody.client_data);            
            break;
        }
 
        default:
        {
            Warn("unknow msg_type:%d \n", p_client_msg->msg_type);
            ret = FT_FAILURE;
            break;
        }
    }
	
    return ret;
}


void thread_msgio(void)
{
    int ret = pthread_detach(pthread_self());
    Debug("pthread_self():%lu, ret[%d]", pthread_self(), ret);
	while (1) {
		fd_set rset, wset;
		FD_ZERO(&rset);
		//FD_ZERO(&wset);
		int i, maxfd, rcvfd;
		maxfd = set_get_max(&s_fdset);
		for(i=0;i<set_get_elem_num(&s_fdset);i++)
		{
			FD_SET(set_at(&s_fdset, i),&rset);
		}
		//FD_SET(socket_fd,&wset);
		struct timeval tval;
		tval.tv_sec = 0;
		tval.tv_usec = 10000;
		if(select(maxfd+1, &rset, NULL, NULL,&tval) > 0) {
			int i;
			for(i=0;i<set_get_elem_num(&s_fdset);i++) {
				if(FD_ISSET(set_at(&s_fdset, i), &rset)) {
					rcvfd = set_at(&s_fdset, i);
					break;
				}
			}
			
			char szRcv[MAX_MESSAGE_LEN] = {0};
			int ret = recv(rcvfd, szRcv, MAX_MESSAGE_LEN, 0);		
			if (ret > 0) {
				Print_hex(szRcv, ret);
				__on_rcv_cloud_msg(rcvfd, szRcv, ret);
				
				//Debug("[fd:%d]rcv data: %s", rcvfd, szRcv);
			} else {
				Warn("[fd:%d]rcv error! close socket", rcvfd);
				set_erase(rcvfd, &s_fdset);
				close(rcvfd);
			}
		} else {
			st_msg_entity * msg_entity;
			POP_FROMQUEUE(msg_entity, list, &s_cloud_msg_queue);
			if (msg_entity == NULL) {
				continue;
			} else {
				int i;
				for(i=0; i<set_get_elem_num(&s_fdset);i++) {		//循环发送
					__send_to_cloud(msg_entity, set_at(&s_fdset, i));
				}
	            FT_FREE(msg_entity);
				msg_entity = NULL;
			}
			
		}
	}

}

void native_listen_thread(void* arg)
{
	int ret = pthread_detach(pthread_self());
	long native_socketfd = (long)arg;
    Debug("pthread_self():%lu, ret[%d]", pthread_self(), ret);
	struct sockaddr_un their_addr;
	unsigned int sin_size = sizeof(struct sockaddr_un);
    int new_fd = 0;
	while (1) {
		Info("s_fdset size[%d]wait connect...", set_get_elem_num(&s_fdset));
		if ((new_fd = accept(native_socketfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) { 
		    Error("accept error!");
			perror("accept!\n"); 
			continue;
		} 
		Info("native sockfd[%d]got connection from %s", native_socketfd, their_addr.sun_path);
		set_insert(new_fd, &s_fdset);
	}
}

int ikcc_init_native_socket(const char* sun_path)
{
	ikcc_init_native_tcp(sun_path);
}

int ikcc_init_native_tcp(const char* sun_path)
{
    //msg_init(&s_msg);
	set_init(&s_fdset);

	CREATE_QUEUE(&s_cloud_msg_queue);
		
	struct sockaddr_un my_addr; /* 本机地址信息 */ 
	struct sockaddr_un their_addr; /* 客户地址信息 */ 
	unsigned int sin_size, lisnum; 
	long native_socketfd;
	lisnum = 5;
	unlink(sun_path);
	if ((native_socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        Error("init socket error!");
        perror("socket"); 
		return FT_FAILURE; 
	} 
	Info("socket[%d] AF_UNIX create ok! ", native_socketfd);
	my_addr.sun_family=AF_UNIX;
	strcpy(my_addr.sun_path, sun_path);
	Info("my_addr.sunpath[%s]", my_addr.sun_path);
	//my_addr.sun_port=htons(myport); 
	//my_addr.sun_addr.s_addr = INADDR_ANY; 
	//bzero(&(my_addr.sin_zero), 0);

	if (bind(native_socketfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) { 
	    Error("bind error!");
		perror("bind");
		return FT_FAILURE;  
	} 
	Info("bind ok ");
	if (listen(native_socketfd, lisnum) == -1) {
        Error("listen error!");
		perror("listen"); 
		return FT_FAILURE; 
	}
	Info("listen ok ");

	pthread_t listenid, ioid;
	int listenret, ioret;
	listenret=pthread_create(&listenid,NULL,(void *)native_listen_thread,(void*)native_socketfd);
	if(listenret!=0) {
		Error("Create nativeListenThread error!");
		return FT_FAILURE;
	}
	ioret=pthread_create(&ioid,NULL,(void *)thread_msgio,NULL);
	if(ioret!=0) {
		Error("Create nativeListenThread error!");
		return FT_FAILURE;
	}
    return FT_SUCCESS;
}


