#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <time.h>
#include "leom_dbg.h"

#define MAX_UART_WRITE_LEN	(128)
#define LEOM_CLUA_NAME	"lmclua"
#define PREFIX_FOR_LUA  (extern "C")

/*
 * only one paramter ,is lua_State*
 * return , the count of result for LUA
*/

static unsigned int calc_crc_s(unsigned char *u8Buf, unsigned int u16Len)
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

static void handle2uart_respmsg(const char *msg,int len,int fd)
{
	/*coming data
	 tostr:55a31607e103019000000020000000158d72b8658d72d84
	 0x55 0xa3 0x16 ...
	*/
}

static int leomclua_writeuart(lua_State* L)
{
	
	int fd = 0, ret = 0;
	unsigned char high = 0x00;
	unsigned char low = 0x00;
	int crc = 0xFFFF;
	//char sndbuf[MAX_UART_WRITE_LEN] = {0};
	const char *device = luaL_checkstring(L, 1);
	const unsigned char *msg = luaL_checkstring(L, 2);
    unsigned int wlen = luaL_optint(L, 3, 0);
	unsigned char buf[128] = {0};
	memcpy(buf,msg,wlen);
	crc = calc_crc_s(msg,wlen);
	low = (unsigned char)(crc&0x00ff);
	high = (unsigned char)((crc&0xff00) >> 8);
	buf[wlen++] = high;
	buf[wlen++] = low;
	
	if(wlen) {
		fd = open(device,O_RDWR);
		if(fd) {
			ret = write(fd,buf,wlen);
			close(fd);
		}
	}
	lua_pushnumber(L,ret);
    return 1;
}

static int get_time_from_utc(lua_State* L)
{
	struct tm *p = NULL;
	time_t tt = (time_t)luaL_optint(L,1,0);
	//p = gmtime(&tt); notimezone
	if(tt == 0) {
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		return 6;
	}
	
	p = localtime(&tt);
	if(!p) {
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		lua_pushnumber(L,0);
		return 6;
	}
	
	//printf("\n=======%d-%d-%d-%d-%d-%d\n",1900+p->tm_year,1+p->tm_mon,
	//		p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	lua_pushnumber(L,1900+p->tm_year);
	lua_pushnumber(L,1+p->tm_mon);
	lua_pushnumber(L,p->tm_mday);
	lua_pushnumber(L,p->tm_hour);
	lua_pushnumber(L,p->tm_min);
	lua_pushnumber(L,p->tm_sec);
	return 6;
}

static int log2system(lua_State* L)
{
	const char *msg = luaL_checkstring(L, 1);
	debug(LOG_DEBUG,"%s\n",msg);
	return 0;
}

static luaL_Reg leom_clualibs[] = {
	/* string for function called by Lua, function name in .c file */
    {"writeuart", leomclua_writeuart},
	{"time_from_utc",get_time_from_utc},
	{"log2sys",log2system},
	/* must be end by NULL,NULL */
    {NULL, NULL}
}; 

int luaopen_leomclualib(lua_State* L) 
{
    const char *libname = LEOM_CLUA_NAME;
    luaL_register(L,libname,leom_clualibs);
	return 0;
}
