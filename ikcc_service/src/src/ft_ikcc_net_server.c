#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include "ft_ikcc_net_server.h"
#include "hw_management.h"
#include "print.h"
#include "tool/ft_map.h"
#include "ikcc_common.h"
#include "ikcc_lua_api.h"
#include "shell_script.h"




#define SECONDS_PER_MINUTE 60

static st_link_list s_dev_map_queue;		//设备信息列表

//进行内存拷贝
static int __push_msg_to_dev(st_msg_entity *p_msg_entity)
{
	if (p_msg_entity == NULL ) {
		Error("p_msg_entity[%p] !", p_msg_entity);
		return FT_FAILURE;
	}
	st_msg_entity* dev_msg = FT_MALLOC(sizeof(st_msg_entity));
	FT_MEMCPY(dev_msg, p_msg_entity, sizeof(st_msg_entity));

	Debug("%d, %s, %d", p_msg_entity->e_msg_type, p_msg_entity->dst_mac, p_msg_entity->size);
	PUSH_ENQUEUE(dev_msg, list, &(find_device_by_mac(&s_dev_map_queue, p_msg_entity->dst_mac))->p_queue);
}

static void set_net_msg_entity(st_msg_entity* msg, const enum_msg_type type, const char* mac, const char* payload, const int len)
{
	if (msg == NULL || mac == NULL || payload == NULL) {
		Error("msg[%p]mac[%p]payload[%p]!", msg, mac, payload);
	}
	msg->e_msg_type = type;
	msg->size = len;
	msg->format = FORMAT_HEX;
	msg->link_type = LINK_UNKNOW;
	FT_MEMCPY(msg->dst_mac, mac, FT_STRING_LEN);
	FT_MEMCPY(msg->payload, payload, len);
}

static void __check_heartbeat()
{
	st_link_list *p = &s_dev_map_queue;
	while (p != NULL && p->next != NULL) {						//遍历设备列表
		p = p->next;
		if (!p->dev_status.online) {			//offline
			continue;
		}
		if((1 + p->dev_status.heartbeat_count++) % SECONDS_PER_MINUTE == 0) {		//隔60秒一个心跳
			//send heartbeat
			Debug("mac[%s]heartbeat_count[%d] send to heartbeat client!", p->dev_status.mac, p->dev_status.heartbeat_count);
			st_msg_entity *st_msg = FT_MALLOC(sizeof(st_msg_entity));
			char heartbeat_flag = 0;	// 0:send 1: rcv
			set_net_msg_entity(st_msg, DEV_HEARTBEAT, p->dev_status.mac, &heartbeat_flag, 1);
			PUSH_ENQUEUE(st_msg, list, &(p->dev_status.p_queue));
			
		}
		if (p->dev_status.heartbeat_count > 3*SECONDS_PER_MINUTE) {					//两个心跳未收到回复则认为超时
			// notify offline msg
			p->dev_status.online = FALSE;
			Warn("%s offline", p->dev_status.mac);
		}
	}
}

void funct(const int fd)
{
	//定时检查设备心跳计数设备发送心跳
	FT_LOCK(&s_dev_map_queue.lock);		//加遍历锁 因为没有定义相关遍历的宏
	__check_heartbeat();
	FT_UNLOCK(&s_dev_map_queue.lock);
}

static int __callback_recv_cloud_msg(st_msg_entity *p_msg_entity)
{
	return __push_msg_to_dev(p_msg_entity);
}

static int __clear_heartbeat_count(const st_client_data *p_client_data)
{
	st_link_list *p = &s_dev_map_queue;
	FT_LOCK(&s_dev_map_queue.lock);
	while (p != NULL && p->next != NULL) {						//遍历设备列表
		p = p->next;
		if(FT_STRNCMP(p->dev_status.mac, p_client_data->identify, FT_STRING_LEN) == 0) {	
			p->dev_status.heartbeat_count = 0;							//清零心跳计数
			p->dev_status.online = TRUE;
		}
	}
	FT_UNLOCK(&s_dev_map_queue.lock);
}

static int __device_login(const int client_fd, const st_client_attr* p_client_attr)       //uart protocol
{
    st_client_msg client_msg = {0};
    int len = 0;
    int ret = 0;

	if (p_client_attr == NULL ) {
		Error("info[%p] !", p_client_attr);
		return FT_FAILURE;
	}
	
	// check user
#ifdef LUA_SCRIPT
	char result = lua_check_user(p_client_attr->username, p_client_attr->password);
#else
	char result = 0;
#endif /* LUA_SCRIPT */
	
	
	if (!FT_STRCMP(p_client_attr->key, "fotile") && result == 0) {	//0:success 1:user error 2:password error	
	} else {
		//if user check fail send fail response to client but not create client thread
        client_msg.msg_type = IKCC_LOGIN_ACK;
		FT_MEMCPY(client_msg.msgbody.client_data.identify, p_client_attr->identify, FT_STRING_LEN);
        client_msg.msgbody.client_data.size = sizeof(result);
        FT_MEMCPY(client_msg.msgbody.client_data.buffer, &result, sizeof(result));
        len = GET_SEND_DATA_SIZE(client_msg);
		
		ret = send(client_fd, &client_msg, len, 0);
		Debug("login result[%d] send ret[%d]", result, ret);
		return FT_FAILURE;
	}

	/**********create msg queue node*************/
	st_client_device dev;
	FT_STRNCPY(dev.mac, p_client_attr->identify, FT_STRING_LEN-1);
	dev.fd = client_fd;
	dev.heartbeat_count = 0;
	dev.online = 1;				// online when login
	//add to dev map
	add_device_to_list(&s_dev_map_queue, &dev);
	//此队列存放 p_dev_desp 消息结构体指针
	CREATE_QUEUE(&(find_device_by_mac(&s_dev_map_queue, p_client_attr->identify))->p_queue);
	/******************************************/

	/**********send login response*************/
	//必须放在创建队列之后
	st_msg_entity msg_entity;
	set_net_msg_entity(&msg_entity, DEV_ONLINE, p_client_attr->identify, &result, sizeof(result));
	Info("send login response");
	__push_msg_to_dev(&msg_entity);
	/******************************************/
	
	//register
	st_svr_desp* svr_desp = FT_MALLOC(sizeof(st_svr_desp));
	FT_MEMSET(svr_desp, 0, sizeof(st_svr_desp));
	FT_STRNCPY(svr_desp->name, "socket", FT_STRING_LEN-1);
	FT_STRNCPY(svr_desp->mac, p_client_attr->identify, FT_STRING_LEN-1);
	FT_STRNCPY(svr_desp->key, p_client_attr->key, FT_STRING_LEN-1);
	svr_desp->callback_recv_msg = __callback_recv_cloud_msg;
	if (FT_SUCCESS == hw_register_service(svr_desp)) {
        hw_add_device(p_client_attr->identify);
	} else {
	}
	return FT_SUCCESS;
	//send online msg
	
}

static int __device_msg(const st_client_data *p_client_data)
{
    ASSERT(p_client_data);
	if (p_client_data->identify == NULL || p_client_data->buffer == NULL) {
		Error("msg[%p] mac[%p]!", p_client_data->buffer, p_client_data->identify);
		return FT_FAILURE;
	}
	__clear_heartbeat_count(p_client_data);
	st_msg_entity *pmsg = FT_MALLOC(sizeof(st_msg_entity));
	FT_MEMSET(pmsg, 0, sizeof(st_msg_entity));
	set_net_msg_entity(pmsg, DEV_DATA_UP, p_client_data->identify, p_client_data->buffer, p_client_data->size);
	
    return hw_send_msg_tobuf(pmsg);
}
static int __device_logout(const st_client_attr* p_client_attr)
{
	if (p_client_attr == NULL) {
		Error("info is NULL!");
		return FT_FAILURE;
	}

	// logout always success
	char result = 0;		//0:success 1:user error 2:password error	
	/**********send login response*************/
	st_msg_entity msg_entity;
	set_net_msg_entity(&msg_entity, DEV_OFFLINE, p_client_attr->identify, &result, sizeof(result));
	Info("send logout response");
	__push_msg_to_dev(&msg_entity);
	/******************************************/

    if (hw_unregister_service(p_client_attr->identify)) {
        return hw_del_device(p_client_attr->identify);
	} else {
		return FT_FAILURE;
	}
}
static int __device_heartbeat(st_client_data *p_client_data)
{
	if (p_client_data == NULL) {
		Error("msg[%p]", p_client_data->buffer);
	}
	Info("rcv dev mac[%s] heartbeat response[%d]", p_client_data->identify, p_client_data->buffer[0]);
	__clear_heartbeat_count(p_client_data);
}

static int __send_to_dev(st_msg_entity *p_msg_entity, int fd)
{
	int len = 0;
	int ret = 0;
    st_client_msg client_msg = {0};

	if (p_msg_entity == NULL) {
		Error("p_msg is NULL!");
		return FT_FAILURE;
	}
	
    switch(p_msg_entity->e_msg_type)
    {
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
            client_msg.msg_type = IKCC_HEARTBEAT;
            break;
        };
        default:
        {
            client_msg.msg_type = IKCC_DATA;
            break;
        }
    }

	FT_MEMCPY(client_msg.msgbody.client_data.identify, p_msg_entity->dst_mac, FT_STRING_LEN);
	client_msg.msgbody.client_data.size = p_msg_entity->size;
	FT_MEMCPY(client_msg.msgbody.client_data.buffer, p_msg_entity->payload, p_msg_entity->size);
    len = GET_SEND_DATA_SIZE(client_msg);	
	
	ret = send(fd, &client_msg, len, 0);
	if(ret <= 0) {
        Error("send failed!!! ret[%d] fd:%d len:%d", ret, fd, len);
        return FT_FAILURE;
	}
	
	Debug("msg_type:%d send ret[%d] fd:%d len:%d",client_msg.msg_type, ret, fd, len);

	return FT_SUCCESS;
}



static int __on_rcv_client_msg(const int client_fd, const char* msg, int len)
{
    int ret = 0;
    st_client_msg *p_client_msg = (st_client_msg *)msg;

	if (msg == NULL) {
		Error("msg is NULL!");
		return FT_FAILURE;
	}

	//update back door
	if (FT_STRCMP(msg, "update_ipk") == 0){
#ifdef OPENWRT
		ft_update_ipk();
#else
		Warn("rcv update cmd!");
#endif
	}

    switch(p_client_msg->msg_type)
    {
        case IKCC_LOGIN:
        {
            ret =  __device_login(client_fd, &p_client_msg->msgbody.client_attr);            
            break;
        };
        case IKCC_LOGOUT:
        {            
            ret = __device_logout(&p_client_msg->msgbody.client_attr);            
            break;
        };
        case IKCC_DATA:
        {
            ret = __device_msg(&p_client_msg->msgbody.client_data);            
            break;
        }
        case IKCC_HEARTBEAT_ACK:
        {
            ret = __device_heartbeat(&p_client_msg->msgbody.client_data);
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

void clientThread(void* arg)
{
	int ret = pthread_detach(pthread_self());
	unsigned long client_fd = (unsigned long)arg;

	while (1) {
		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(client_fd, &rset);
		struct timeval tval;
		tval.tv_sec = 0;
		tval.tv_usec = 10000;
		int n;
		if(( n=select(client_fd+1, &rset, /*&wset*/NULL, NULL,&tval)) > 0) {
			char szRcv[MAX_MESSAGE_LEN] = {0};
			int ret = recv(client_fd, szRcv, MAX_MESSAGE_LEN, 0);		
			if (ret > 0) {
				//Debug("[fd:%d]rcv len[%d]data: %s", client_fd, ret, szRcv);
				Print_hex(szRcv, ret);
				__on_rcv_client_msg(client_fd, szRcv, ret);
				continue;
			} else {
				Warn("[fd:%d]rcv error! close thread", client_fd);
				return ;
			}
		} else {
			st_msg_entity *p_msg_entity = NULL;	
			st_client_device* device = find_device_by_fd(&s_dev_map_queue, client_fd);	
			if (device == NULL) {				
				continue;
			}
			POP_FROMQUEUE(p_msg_entity, list, &device->p_queue);
			if (p_msg_entity == NULL) {			
				continue;
			} else {
				__send_to_dev(p_msg_entity, client_fd);			
				FT_FREE(p_msg_entity);
				p_msg_entity = NULL;
			}
		}
	}
}

void listenThread(void* arg)
{
	int ret = pthread_detach(pthread_self());
	long svr_socketfd = (long)arg;
	unsigned int sin_size = sizeof(struct sockaddr_in);
	struct sockaddr_in their_addr; /* 客户地址信息 */ 
	int new_fd = 0;
	while (1) {
		Info("wait connect...");
		if ((new_fd = accept(svr_socketfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) { 
			Error("accept error!");
			perror("accept!\n"); 
			continue;
		} 
		Info("sockfd[%d]got connection from %s fd[%d] check user...", svr_socketfd, inet_ntoa(their_addr.sin_addr), new_fd);

		/*********wait login*********/
		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(new_fd, &rset);
		struct timeval tval;
		tval.tv_sec = 5;
		tval.tv_usec = 0;		//5 second timeout
		if(select(new_fd+1, &rset, NULL, NULL,&tval) > 0) {
			char szRcv[MAX_MESSAGE_LEN] = {0};
			int len = recv(new_fd, szRcv, MAX_MESSAGE_LEN, 0);
			if (FT_FAILURE == __on_rcv_client_msg(new_fd, szRcv, len)) {
				continue;
			}
		} else {
			Info("time out! close socket fd[%d]", new_fd);
			close(new_fd);
			continue;
		}
		/****************************/

		pthread_t client;
		int client_ret;
		unsigned long fd = new_fd;
		client_ret = pthread_create(&client, NULL, (void*)clientThread, (void*)fd);
		if(client_ret!=0){
			Error("Create clientThread error!");
		}
		usleep(10);
	}
}


int ikcc_init_net_server(const int port)
{
	ikcc_init_tcp_server(port);
}


int ikcc_init_tcp_server(int port)
{	
	init_list(&s_dev_map_queue);

	create_timer(funct, 1000, 0);
	
	struct sockaddr_in my_addr; /* 本机地址信息 */ 
	unsigned int myport, lisnum;
	long svr_socketfd;
	myport = port==0?8888:port; 
	lisnum = 5; 
	if ((svr_socketfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        Error("init socket error!");
        perror("socket"); 
		return FT_FAILURE; 
	} 
	Info("socket port %d ok ",myport);
	my_addr.sin_family=PF_INET;
	my_addr.sin_port=htons(myport); 
	my_addr.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(my_addr.sin_zero), 0); 
	if (bind(svr_socketfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) { 
	    Error("bind error!");
		perror("bind");
		return FT_FAILURE;  
	} 
	Info("bind ok ");
	if (listen(svr_socketfd, lisnum) == -1) { 
        Error("listen error!");
		perror("listen"); 
		return FT_FAILURE; 
	}
	Info("listen ok ");

    //create send and rcv thread
	pthread_t listenid;
	int sendret, rcvret;
	sendret=pthread_create(&listenid,NULL,(void *)listenThread, (void*)svr_socketfd);
	if(sendret!=0) {
		Error("Create listenThread error!");
		return FT_FAILURE;
	}
	return FT_SUCCESS;
}

