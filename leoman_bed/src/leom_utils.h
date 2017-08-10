#ifndef _LEOM_UTILS_H_
#define _LEOM_UTILS_H_
#include <time.h>

long get_current_time(char *timestr);
unsigned int calc_crc(unsigned char *u8Buf, unsigned int u16Len);
#endif
