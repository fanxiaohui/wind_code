/**
* @file shell_script.h
* @brief 
* @version 1.1
* @author cairj
* @date 2016/12/09
*/
#ifndef _SHELL_SCRIPT_H_
#define _SHELL_SCRIPT_H_


typedef enum _enum_led_mode {
	IKCC_LED_OPEN 		=	0,
	IKCC_LED_CLOSE 		=	1,
	IKCC_LED_BLINK 		=	2,
	IKCC_LED_NOBLINK	=	3,
	IKCC_LED_MODE_NUM
} enum_led_mode;

typedef enum _enum_led_colour {
	IKCC_LED_RED 	=	0,
	IKCC_LED_BLUE 	=	1,
	IKCC_LED_GREEN	=	2,
	IKCC_LED_COLOUR_NUM
} enum_led_colour;


int ft_update_ipk();
int ft_tmp_ssid();
int ft_easylink();
void ft_system_reboot();
void ft_system_firstboot();

int ft_set_update_url(const char* url);
int get_wireless_option(const char* item, const char* option, char* out_value);
int set_wireless_option(const char* item, const char* option, const char* value);
int set_lamp_option(enum_led_mode mode, enum_led_colour colour);
int set_relay_option(int type, int open);
int get_eth0_mac(char* mac, unsigned char* mac_array, int len_limit);
//set_wireless_option("wifi-iface[0]", "disabled", "1")
int ft_update_system(char* md5,int Ver);
int ft_update_ipk();

#endif /* SHELL_SCRIPT_H */


