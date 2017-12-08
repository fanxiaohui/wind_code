
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

void debug_buf(unsigned char *buffer, int length)
{
	int i = 0;
	char strbuf[512] = "";
	char *ptr = strbuf;
	if(length > 150 || length < 1) {
		debug(LOG_DEBUG,"===debug_buf length %d is invalid\n",length);
		return;
	}
	for(i=0; i < length; i++) {
		sprintf(ptr,"%02x ", buffer[i]);
		ptr += 3;
	}
	debug(LOG_DEBUG,"%s\n",strbuf);
}

