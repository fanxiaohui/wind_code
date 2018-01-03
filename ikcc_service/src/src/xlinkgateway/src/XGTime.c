/*
 * XGTime.c
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "XGTime.h"

XGTimeMs XGGetLocalTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void XGSleepMs(int ms) {
	usleep(ms * 1000);
}
