#include <stdio.h>
#include <stdlib.h>
#include "hw_management.h"
#include "ds_management.h"
#include "ft_queue.h"
#include "ft_def.h"
#include "ikcc_lua_api.h"
#include "print.h"
#include "platform_type.h"
#include "./xlinkgateway/src/XGDebug.h"

int g_sdk_open = 0;
void xg_printf(unsigned int lev, const char *file, int line, const char *format, ...)
{
	//XG_PRINTF(lev, format);
}


extern int ft_xlink_gateway_init(void);

int main(){
	/*********** data service module ************/
	init_syslog();
	//char ss[18]={51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68};
	//Print_hex(ss,18);
	XGSetLogOutCallback(xg_printf);
	Info("***********************FOTILE IKCC START***************************");
	Info("***********************FOTILE IKCC START***************************");
	Info("***********************FOTILE IKCC START***************************");
#if 0	
	ds_init();
	/*********** hardware service module ************/
	hw_init();

	/*********** message transfer ************/
	msg_transfer_init();

	/*********** net socket ************/
    ikcc_init_net_server(8888);

	/*********** native socket ************/
    ikcc_init_native_socket("/tmp/native_socket");

	/*********** lua script ************/
#endif
#ifdef LUA_SCRIPT
	ikcc_init_ssid_tcp_server(23576);

	//lua_init_lua();		//此功能会加载lua cjson库 需要安装到板子上
	//cjson_test();
#endif /* LUA_SCRIPT */


	ft_xlink_gateway_init();

    /***********console command************/
    //init_console();		//此接口会阻塞
	while(1)
	{
		sleep(10);
	}
	Info("core over!");
}
