#include <stdio.h>
#include "leom_utils.h"

long get_current_time(char *timestr)
{
	time_t tt;
    time(&tt);

    if(timestr)
   	 snprintf(timestr,127,"%s",ctime(&tt));
   	
    return tt;
}

unsigned int calc_crc(unsigned char *u8Buf, unsigned int u16Len)
{
	unsigned char u8Index;
    unsigned int u16CRC = 0xFFFF;
    while(u16Len--)
	{
        u16CRC ^= *u8Buf++;
        for(u8Index = 0;u8Index < 8;u8Index++)
        {
            if(u16CRC & 0x0001)
                u16CRC = (u16CRC >> 1)^0xa001;
            else
                u16CRC = (u16CRC >> 1);
        }
    }
    return (u16CRC);
}
