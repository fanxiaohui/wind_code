#ifndef _LEOM_UTILS_H_
#define _LEOM_UTILS_H_

int already_running(void);
void sleep_intp_s(unsigned int secs);
long get_current_time(char *timestr);
int get_iface_mac(const char ifname[], char *mac);
unsigned int calc_crc_1(unsigned char *u8Buf, unsigned int u16Len);
#endif
