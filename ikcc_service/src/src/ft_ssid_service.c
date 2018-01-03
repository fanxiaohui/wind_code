#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "print.h"
#include "shell_script.h"
#include "ft_queue.h"
#include "ft_def.h"
#include "ikcc_lua_api.h"

#include "tool/ft_set.h"
#include "platform_type.h"

static st_maxdata_set ssid_fdset;				//native socket fd set


void thread_ssid_msgio(void)
{
	int ret = pthread_detach(pthread_self());
	Debug("pthread_self():%lu, ret[%d]", pthread_self(), ret);
	char mac[32] = {0};
	char ssid[14] = {"FotileAP_"};
	int retssid = 0;
	while (1) 
	{
		fd_set rset, wset;
		FD_ZERO(&rset);
		//FD_ZERO(&wset);
		int i, maxfd, rcvfd;
		maxfd = set_get_max(&ssid_fdset);
		for(i=0;i<set_get_elem_num(&ssid_fdset);i++)
		{
			FD_SET(set_at(&ssid_fdset, i),&rset);
		}
		//FD_SET(socket_fd,&wset);
		struct timeval tval;
		tval.tv_sec = 0;
		tval.tv_usec = 10000;
		if(select(maxfd+1, &rset, NULL, NULL,&tval) > 0)
		{
			int i;
			for(i=0;i<set_get_elem_num(&ssid_fdset);i++)
			{
				if(FD_ISSET(set_at(&ssid_fdset, i), &rset))
				{
					rcvfd = set_at(&ssid_fdset, i);
					break;
				}
			}
			
			char szRcv[MAX_MESSAGE_LEN] = {0};
			int len = 0;
			int lens =0;
			unsigned char mac_arr[6] = {0};		//not use

			if (retssid == 0){
				retssid = get_eth0_mac(mac, mac_arr, 32);
				ssid[9] = mac[12];
				ssid[10] = mac[13];
				ssid[11] = mac[15];
				ssid[12] = mac[16];
				Debug("first time get ssid!");
			}else{

			}
			int ret = recv(rcvfd, szRcv, MAX_MESSAGE_LEN, 0);
			//Info("ret[%d]%s", ret, szRcv);
			if (ret > 0) 
			{
				cJSON *root1 = cJSON_Parse(szRcv);
				cJSON *root2 = cJSON_CreateObject();
				if(strcmp(cJSON_GetObjectItem(root1,"type")->valuestring,"reqssid") == 0)
				{
					cJSON_AddItemToObject(root2, "type", cJSON_CreateString("reqssid"));
					cJSON_AddNumberToObject(root2, "seq", cJSON_GetObjectItem(root1,"seq")->valueint);
					if(retssid > 0)
					{
						cJSON_AddNumberToObject(root2, "respcode", 0);
						cJSON_AddStringToObject(root2, "respmsg", ssid);
					}
					else
					{
						cJSON_AddNumberToObject(root2, "respcode", -1);
						cJSON_AddStringToObject(root2,"respmsg",  "error:get ssid failed!");
					}
					char *out = cJSON_Print(root2);
					//Info("out[%s]", out);
					lens = strlen(out);
					send(rcvfd, out , lens ,0);
					//Info("%s",rendered);
					cJSON_Delete(root1);
					cJSON_Delete(root2);
					free(out);
				}else{
					Warn("param error!");
				}
			} 
			else
			{
				//Warn("[fd:%d]rcv error! close socket", rcvfd);
				set_erase(rcvfd, &ssid_fdset);
				close(rcvfd);
			}
		}
	}
}

void listenSSIDThread(void* arg)
{
	int ret = pthread_detach(pthread_self());
	long svr_socketfd = (long)arg;
	unsigned int sin_size = sizeof(struct sockaddr_in);
	struct sockaddr_in their_addr; /* 客户地址信息 */ 
	int new_fd = 0;
	while (1)
	{
		//Info("ssid_fdset size[%d]wait connect...", set_get_elem_num(&ssid_fdset));
		//Info("ssid_fdset size[%d]wait connect...", set_get_elem_num(&ssid_fdset));
		if ((new_fd = accept(svr_socketfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
		{ 
			Error("accept error!");
			//Info("Error:accept error!");
			perror("accept!\n"); 
			continue;
		} 
		//Info("sockfd[%d]got connection from %s fd[%d] check user...", svr_socketfd, inet_ntoa(their_addr.sin_addr), new_fd);
		//Info("sockfd[%d]got connection from %s fd[%d] check user...", svr_socketfd, inet_ntoa(their_addr.sin_addr), new_fd);
		set_insert(new_fd, &ssid_fdset);
	}
}

int ikcc_init_ssid_tcp_server(int port)
{	
	struct sockaddr_in my_addr; /* 本机地址信息 */ 
	unsigned int myport, lisnum;
	long svr_socketfd;
	myport = port==0?23576:port; 
	lisnum = 5; 
	if ((svr_socketfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        Error("init socket error!");
        //Info("Error:init socket error!");
        perror("socket"); 
		return FT_FAILURE; 
	} 
	Info("socket port %d ok ",myport);
	//Info("socket port %d ok",myport);
	my_addr.sin_family=PF_INET;
	my_addr.sin_port=htons(myport); 
	my_addr.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(my_addr.sin_zero), 0); 
	if (bind(svr_socketfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) { 
		Error("bind error!");
		//Info("Error:bind error!");
		perror("bind");
		return FT_FAILURE;  
	} 
	Info("bind ok ");
	//Info("bind ok");
	if (listen(svr_socketfd, lisnum) == -1) { 
		Error("listen error!");
		//Info("Error:listen error!");
		perror("listen"); 
		return FT_FAILURE; 
	}
	Info("listen ok ");
	//Info("listen ok");
	pthread_t listenid, ioid;
	int listenret, ioret;

	//create send and rcv thread
	int sendret, rcvret;
	sendret=pthread_create(&listenid,NULL,(void *)listenSSIDThread, (void*)svr_socketfd);
	if(sendret!=0) {
		Error("Create listenThread error!");
		//Info("Error:Create listenThread error!");
		return FT_FAILURE;
	}
	ioret = pthread_create(&ioid,NULL,(void *)thread_ssid_msgio,NULL);
	if(ioret!=0) {
		Error("Create nativeListenThread error!");
		return FT_FAILURE;
	}
	return FT_SUCCESS;
}

