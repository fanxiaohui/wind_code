#ifdef OPENWRT
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>        //for struct ifreq
#include <sys/ioctl.h>



#include "shell_script.h"
#include "print.h"
#include "ikcc_common.h"


//#include <unistd.h>





#define UPDATE_IPK "sh /bin/ikcc/update/update.sh"
#define UPDATE_SYSTEM "sh /bin/ikcc/update/sys_update.sh"
#define EASYLINK "sh /sbin/easylink.sh 60 &"
#define TMP_SSID "ifconfig|grep ra0||sh /sbin/tmp_ssid.sh 120 &"
//#define UPDATE_CMD "update_ipk"
//#define IKCC_GUARD_PORT 8181
#define UCI_CONFIG_FILE "/etc/config/wireless"

#if 0
#include <uci.h>
static struct uci_context * ctx = NULL; //定义一个UCI上下文的静态变量.
#endif

int ft_tmp_ssid()
{
	Debug("rcv temp ssid request!");
	system(TMP_SSID);
	Debug("rcv temp ssid request! end");
	return 0;
}


int ft_easylink()
{
	Debug("rcv easylink request!");
	system(EASYLINK);
	Debug("rcv easylink request! end");
	return 0;
}


int get_wireless_option(const char* item, const char* option, char* out_value)
{

}

void ft_system_reboot()
{
	system("reboot");
}

void ft_system_firstboot()
{
	system("firstboot");
}

int ft_update_ipk()
{
	Debug("rcv update ipk request!");
	system(UPDATE_IPK);
	Debug("rcv update ipk request!end");
	return 0;
}

int ft_update_system(char* md5,int Ver)
{
	Debug("rcv update system request!");
	char shell[100];
	memset(shell, 0, sizeof(shell)); 
	sprintf(shell,"sh /bin/ikcc/update/sys_update.sh %s %d",md5,Ver);
	system(shell);
	Debug("rcv update system request!end");
}

int set_lamp_option(enum_led_mode mode, enum_led_colour colour)
{
	char cmd[256]={0};
	char strcolour[32] = {0};
		switch(colour){
	case IKCC_LED_RED:
		FT_STRCPY(strcolour, "red");
		break;
	case IKCC_LED_BLUE:
		FT_STRCPY(strcolour, "blue");
		break;
	case IKCC_LED_GREEN:
		FT_STRCPY(strcolour, "green");
		break;
	default:
		Warn("colour[%d] error!", colour);
		return FT_FAILURE;
	}
	switch(mode){
	case IKCC_LED_OPEN:
		sprintf(cmd, "echo %d > /sys/class/leds/wrtnode:%s:net/brightness", 1, strcolour);
		break;
	case IKCC_LED_CLOSE:
		sprintf(cmd, "echo %d > /sys/class/leds/wrtnode:%s:net/brightness", 0, strcolour);
		break;
	case IKCC_LED_BLINK:
		sprintf(cmd, "echo %s > /sys/class/leds/wrtnode:%s:net/trigger", "timer", strcolour);
		break;
	case IKCC_LED_NOBLINK:
		sprintf(cmd, "echo %s > /sys/class/leds/wrtnode:%s:net/trigger", "none", strcolour);
		break;
	default:
		Warn("mode[%d] error!", mode);
		return FT_FAILURE;
	}

	Debug("rcv led control[%s]", cmd);
	system(cmd);
	return FT_SUCCESS;
}

int set_relay_option(int type, int open)
{
	if (open != 0 && open != 1){
		Warn("param error! open[%d]", open);
		return FT_FAILURE;
	}
	if (type <= 0){
		Warn("param error! type[%d]", type);
		return FT_FAILURE;
	}
	char cmd[256]={0};
	sprintf(cmd, "echo %d > /sys/class/leds/wrtnode:relay%d:net/brightness", open, type);
	Debug("rcv relay control[%s]", cmd);
	system(cmd);
}

int get_eth0_mac(char* mac, unsigned char* mac_array, int len_limit)
{
    struct ifreq ifreq;
    int sock;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket");
        return -1;
    }
    strcpy (ifreq.ifr_name, "eth0");

    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        perror ("ioctl");
        return -1;
    }
    memcpy(mac_array, ifreq.ifr_hwaddr.sa_data, 6);
	close(sock);
    return snprintf (mac, len_limit, "%02X:%02X:%02X:%02X:%02X:%02X", 
	(unsigned char) ifreq.ifr_hwaddr.sa_data[0],
	(unsigned char) ifreq.ifr_hwaddr.sa_data[1],
	(unsigned char) ifreq.ifr_hwaddr.sa_data[2],
	(unsigned char) ifreq.ifr_hwaddr.sa_data[3],
	(unsigned char) ifreq.ifr_hwaddr.sa_data[4],
	(unsigned char) ifreq.ifr_hwaddr.sa_data[5]);
}



/*
载入配置文件,并遍历Section. 
*/
#if 0
int load_config()
{
	struct uci_package * pkg = NULL;
	struct uci_element *e;
	char *tmp;
	const char *value;
	int result = FT_FAILURE;

	ctx = uci_alloc_context(); // 申请一个UCI上下文.
	if (UCI_OK != uci_load(ctx, UCI_CONFIG_FILE, &pkg)){
		goto cleanup; //如果打开UCI文件失败,则跳到末尾 清理 UCI 上下文.
	}

	//遍历UCI的每一个节
	uci_foreach_element(&pkg->sections, e)
	{
		struct uci_section *s = uci_to_section(e);

		Info("section s's type is %s.\n",s->type);

		if(!FT_STRCMP("wifi-iface",s->type)) //type of this section
		{
			Info("this seciton is a wifi-iface.\n");
			if (NULL != (value = uci_lookup_option_string(ctx, s, "device")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's device is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "network")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's network is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "mode")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's mode is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "ssid")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's ssid is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "encryption")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's encryption is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "key")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's key is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "disabled")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's disabled is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "ApCliEnable")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's ApCliEnable is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "ApCliAuthMode")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's ApCliAuthMode is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "ApCliEncrypType")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's ApCliEncrypType is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "ApCliSsid")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's ApCliAuthMode is %s.\n",s->e.name,value);
			}
			if (NULL != (value = uci_lookup_option_string(ctx, s, "ApCliPassWord")))
			{
				tmp = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。
				Info("%s's ApCliPassWord is %s.\n",s->e.name,value);
			}
		}
        // 如果您不确定是 string类型 可以先使用 uci_lookup_option() 函数得到Option 然后再判断.
        // Option 的类型有 UCI_TYPE_STRING 和 UCI_TYPE_LIST 两种.

		result = FT_SUCCESS;
	}
	uci_unload(ctx, pkg); // 释放 pkg 
	cleanup:
	uci_free_context(ctx);
	ctx = NULL;
	return result;
}

int set_wireless_option(const char *sec,const char *name,const char *value)  
{  
	if (NULL == name || NULL == value)  
	{  
		return 0;  
	}  
  
	struct uci_context * ctx = uci_alloc_context();  
	if (NULL == ctx)  
		Info("setWirelessCfgValue uci_alloc_context error\n");  
      
	int ret = 0;    
	struct uci_ptr ptr;  
	memset(&ptr, 0, sizeof(ptr));  
	char uci_option[128];  
	memset(uci_option, 0, sizeof(uci_option));  
	snprintf(uci_option, sizeof(uci_option)-1, "wireless.%s.%s",sec,name);  
	Info("uci_lookup_ptr %s\n", uci_option);  
	if(UCI_OK != uci_lookup_ptr(ctx, &ptr, uci_option, true))  
        {  
		uci_perror(ctx, "no found!\n");  
		return 0;  
        }  
	Info("uci_lookup_ptr ok: %s.%s.%s\n",   
	ptr.package, ptr.section, ptr.option);  
	ptr.value = value;   
      
	Info("change cfg: %s.%s.%s, value(%s)\n",ptr.package, ptr.section, ptr.option, ptr.value);  
	ret = uci_set(ctx,&ptr);  
	Info("uci_set ret(%d)\n", ret);  
	if (0 == ret)
	{  
		ret = uci_commit(ctx, &ptr.p, false);  
 		Info("uci_commit ret(%d)\n", ret);  
	}  
	uci_unload(ctx,ptr.p);  
	uci_free_context(ctx);  
      
	Info("set wireless cfg successed! network restart!%s: %s: %s\n",sec,name,value);  
	system("nr"); 
	return 0;  
} 
#endif



#endif /* OPENWRT */

int udp_server(){
#if 0

	struct sockaddr_in addr;
    int sock;

    if ( (sock=socket(AF_INET, SOCK_DGRAM, 0)) <0)
    {
        Error("socket create fail!");
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(IKCC_GUARD_PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char buff[512];
    int len = sizeof(addr);
	if (1)
    {
        strcpy(buff, UPDATE_CMD);
        int n;
        n = sendto(sock, buff, strlen(buff), 0, (struct sockaddr *)&addr, sizeof(addr));
        if (n < 0)
        {
            Warn("sendto error!");
            close(sock);
            return 0;
        }
        n = recvfrom(sock, buff, 512, 0, (struct sockaddr *)&addr, &len);
        if (n>0)
        {
            buff[n] = 0;
            Info("received: %s", buff);
        }
        else if (n==0)
        {
            Warn("server closed");
            close(sock);
            return 0;
        }
        else if (n == -1)
        {
            Warn("recvfrom error!");
            close(sock);
            return 0;
        }
    }
    
#endif
}

