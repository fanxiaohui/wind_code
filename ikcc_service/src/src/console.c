#include "console.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "print.h"
#include "ikcc_common.h"
#include "shell_script.h"
#include "platform_type.h"


#define CMD_ARGS_SIZE 16
#define CMD_ARGS_LEN 64

void __custom(int argc, char argv[][CMD_ARGS_LEN]);
extern int ft_xlink_console(int argc, char argv[][CMD_ARGS_LEN]);

/**
* @ Description: 将字符串转化为数组,空格分隔
* @ arg: 入参字符串
* @ args: 出参字符串数组
* @ return: 字符串数组宽度
*/
int __char2args(char* arg, char args[][CMD_ARGS_LEN])
{
	char* p = arg;
	int args_size = 0;
	int sublen = 0;
	int len = 0;
	int local = 0;
	if (*p==0)
		return 0;
	while(*p != 10)			//换行
	{			
		if (*p != 32)		//
		{
			sublen++;
			len++;
			p++;
		}else{
			if (sublen > 0){		//read sub command
				if (sublen > CMD_ARGS_LEN-1){
					FT_MEMCPY(args[args_size++], arg+local, CMD_ARGS_LEN-1);
					Warn("cmd len overflow");
				}else{
					FT_MEMCPY(args[args_size++], arg+local, sublen);
				}
				if(args_size >= CMD_ARGS_SIZE)
				{
					return CMD_ARGS_SIZE;
				}
				len++;
				local = len;
				sublen=0;
				p++;
			}else{				//read blank space form begin
				len++;
				local = len;
				p++;
				continue;
			}
		}
	}
	FT_MEMCPY(args[args_size++], arg+local, sublen);
	return args_size;
}



void __command(char* arg)
{
	char argv[CMD_ARGS_SIZE][CMD_ARGS_LEN] = {0};
	int argc = __char2args(arg, argv);

	__custom(argc, argv);
	printf("#");
}

void* thread(void* arg){
	pthread_detach(pthread_self());
	char buf[1024] = {0};
	while(1){
		setbuf(stdin, NULL);
		char* s = fgets(buf, 1024, stdin);
		//Debug("buf: %p, buf1[%d]", buf, buf[0]);
		if (0 == buf[0]){
			//Error("buf data NULL!");
			break;
		}
		__command(buf);
		usleep(100000);
	}
}


/**
* @ Description: 初始化控制台命令行功能
* @ return: 0:success -1:faile
*/
int init_console()
{
	//pthread_t id;
	//int ret = pthread_create(&id, NULL, (void*)thread, NULL);
	//pthread_join(id, NULL);
	thread(NULL);
}

void __custom(int argc, char argv[][64])
{
	if (argc == 0)
		return;
	if (argv[1] != NULL && *argv[1] == '-'){
		FT_MEMCPY(argv[1], argv[1]+1, 63);
	}

	//ikcm command
	if(!FT_STRCMP(argv[0], "ikcc")){
		if(!FT_STRCMP(argv[1], "?") || !strcmp(argv[1], "h")|| !strcmp(argv[1], "help")){
			Info("-login (login to IKCC server)				\
					-logout (logout from IKCC server)			\
					-send [msg](send msg to IKCC server)		\
				");
		}else if(!FT_STRCMP(argv[1], "login")){
			
		}else if(!FT_STRCMP(argv[1], "send")){
			
		}else if(!FT_STRCMP(argv[1], "logout")){
			
		}else{
			Warn("unknow %s command!\n", argv[0]);
		}
	}else if (!FT_STRCMP(argv[0], "print")){
		if(!FT_STRCMP(argv[1], "?") || !strcmp(argv[1], "h")|| !strcmp(argv[1], "help")){
			Info("-set [level]				\
					-get 			\
				\n");
		}else if(!FT_STRCMP(argv[1], "get")){
			int i;
			for(i=0;i<=5;i++){
				if (get_verbose(i)){
					Info("level:%d", i);
					return;
				}
			}		
		}else if(!FT_STRCMP(argv[1], "set")){
			set_verbose(atoi(argv[2]));
		}else{
			Warn("unknow %s command!\n", argv[0]);
		}
	}else if (!FT_STRCMP(argv[0], "map")){
		if(!FT_STRCMP(argv[1], "?") || !strcmp(argv[1], "h")|| !strcmp(argv[1], "help")){
			
		}else if(!FT_STRCMP(argv[1], "dump")){
			dump_device_list();		
		}else{
			Warn("unknow %s command!\n", argv[0]);
		}
#ifdef OPENWRT
	}else if (!FT_STRCMP(argv[0], "shell")){
		if(!FT_STRCMP(argv[1], "?") || !strcmp(argv[1], "h")|| !strcmp(argv[1], "help")){
			Info("-updateipk (update ikcc ipk)				\
					-reboot (reboot system)			\
					-updatesystem (update system)		\
					-setwifi [item] [option] [value] (set wireless item value)	\
					-getwifi [item] [option] (get wireless item value)	\
				");
		}else if(!FT_STRCMP(argv[1], "updateipk")){
			ft_update_ipk();
		}else if(!FT_STRCMP(argv[1], "reboot")){
			ft_system_reboot();
		}else if(!FT_STRCMP(argv[1], "updatesystem")){
			//ft_update_system(13245);
		}else if(!FT_STRCMP(argv[1], "tmpssid")){
			ft_tmp_ssid();
		}else if(!FT_STRCMP(argv[1], "mac")){
			char mac[32] = {0};
			unsigned char mac_array[6] = {0};
			get_eth0_mac(mac, mac_array, 32);
			Info("mac[%s] mac_array[%02X:%02X:%02X:%02X:%02X:%02X]", mac, mac_array[0], mac_array[1], mac_array[2], mac_array[3], 
				mac_array[4], mac_array[5]);
		}else if(!FT_STRCMP(argv[1], "setlamp")){
			set_lamp_option(atoi(argv[2]), atoi(argv[3]));
		}else if(!FT_STRCMP(argv[1], "setrelay")){
			set_relay_option(atoi(argv[2]), atoi(argv[3]));
		}else{
			Warn("unknow %s command!\n", argv[0]);
		}
#endif /* OPENWRT */
	}
	else if (!FT_STRCMP(argv[0], "xlink")){
	    ft_xlink_console(argc, argv);
	}
	//other command
	//Trace("cmd over!");
}


