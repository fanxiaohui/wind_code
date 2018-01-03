#include <stdio.h>
#include <stdlib.h>
#include "hw_management.h"
#include "ds_management.h"
#include "ft_queue.h"
#include "ft_def.h"
#include "ikcc_lua_api.h"
#include "print.h"
#include "platform_type.h"
#include "./src/XGDebug.h"
#include <pthread.h>

unsigned char eth0_mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
int g_udp_svr_port = 26637;
int g_udp_listen_port = 5987;

int g_sdk_open = 0;

void xg_printf(unsigned int lev, const char *file, int line, const char *format, ...)
{
	//XG_PRINTF(lev, format);
}


extern int ft_xlink_gateway_init(unsigned char *mac);

void* thread1(int *arg)
{
	eth0_mac[5]++;
	ft_xlink_gateway_init(eth0_mac);
	return NULL;
}

int main(int argc ,char **argv)
{
	int i = 0;
	if(argc < 9) 
		return -1;
	g_udp_svr_port = atoi(argv[1]);
	g_udp_listen_port = atoi(argv[2]);
	i = 3;
	eth0_mac[0] = strtol(argv[i++],NULL,16);
	eth0_mac[1] = strtol(argv[i++],NULL,16);
	eth0_mac[2] = strtol(argv[i++],NULL,16);
	eth0_mac[3] = strtol(argv[i++],NULL,16);
	eth0_mac[4] = strtol(argv[i++],NULL,16);
	eth0_mac[5] = strtol(argv[i++],NULL,16);
	
	/*********** data service module ************/
	init_syslog();
	//char ss[18]={51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68};
	//Print_hex(ss,18);
	XGSetLogOutCallback(xg_printf);
	Info("***********************FOTILE IKCC START***************************");
	
	ft_xlink_gateway_init(eth0_mac);

//	pthread_t pid1 = NULL;
//	pthread_create(&pid1,NULL,(void *)thread1, NULL);
	
    /***********console command************/
    //init_console();		//此接口会阻塞
	while(1)
	{
		sleep(10);
	}
//	pthread_join(pid1,NULL);
	
	Info("core over!");
}
