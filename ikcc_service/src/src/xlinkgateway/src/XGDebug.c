/*
 * XGDebug.c
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#include "XGDebug.h"
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

pthread_mutex_t mutext = PTHREAD_MUTEX_INITIALIZER;

#define LOCK  pthread_mutex_lock(&mutext)
#define UNLOCK  pthread_mutex_unlock(&mutext)

int g_log_lev = X_LOG_DEBUG;

const char *msg[] = { "debug", "info", "warning", "error", "max" };

void GetTime(char*pszTimeStr) {
	struct tm tSysTime = { 0 };
	struct timeval tTimeVal = { 0 };
	time_t tCurrentTime = { 0 };

	char szUsec[20] = { 0 };    // 微秒
	char szMsec[20] = { 0 };    // 毫秒

	if (pszTimeStr == NULL) {
		return;
	}

	tCurrentTime = time(NULL);
	localtime_r(&tCurrentTime, &tSysTime);   // localtime_r是线程安全的

	gettimeofday(&tTimeVal, NULL);
	sprintf(szUsec, "%06d", tTimeVal.tv_usec);  // 获取微秒
	strncpy(szMsec, szUsec, 3);                // 微秒的前3位为毫秒(1毫秒=1000微秒)

	sprintf(pszTimeStr, "%04d-%02d-%02d %02d:%02d:%02d.%3.3s", tSysTime.tm_year + 1900, tSysTime.tm_mon + 1, tSysTime.tm_mday, tSysTime.tm_hour, tSysTime.tm_min, tSysTime.tm_sec,
			szMsec);
}

int XGSetDebugLev(int lev) {
	if (lev > X_LOG_MAX) {
		return -1;
	}
	g_log_lev = lev;
	return 0;
}


static logcallback g_logcall = NULL;

int XGSetLogOutCallback(logcallback callback) {
	g_logcall = callback;
	return 0;
}
void XGDebug(unsigned int lev, const char *file, int line, const char *format, ...) {
	if (g_log_lev > lev) {
		return;
	}
	if (lev > X_LOG_MAX) {
		return;
	}
	LOCK;
	char buffer[256] = { 0x00 };
	GetTime(buffer);
	va_list args;
	va_start(args, format);
	if (g_logcall) {
		g_logcall(lev,file,line,format,args);
	} else {
		printf("[%s]%s【%d】【%s】", buffer, file, line, msg[lev]);
		vprintf(format, args);
	}
	va_end(args);
	UNLOCK;
}
