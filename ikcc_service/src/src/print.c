#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <syslog.h>
#include "platform_type.h"
#include "print.h"


typedef enum _info_level{
	IPRINT_DEBUG = 7,
	IPRINT_INFO = 6,
	IPRINT_NOTICE = 5,
	IPRINT_WARNING = 4,
	IPRINT_ERR = 3,
	IPRINT_ALERT = 1
}info_level;

static int print_level = IPRINT_DEBUG;

#define MAX_PRINT_SIZE 4096
#define MAX_DATA_SIZE 1024

#ifdef SYSLOG
	#define PrintInfo(lev, color, type, format)  \
		do {\
			char __printBuf[MAX_PRINT_SIZE] = { 0 }; \
			if (strlen(format) < MAX_PRINT_SIZE) { \
				va_list  __va; \
				va_start(__va, format); \
				vsnprintf(__printBuf, MAX_PRINT_SIZE, format, __va);\
				va_end(__va); \
				int __offset = strlen(__printBuf); \
				const char* pColor = strlen(color) > 0 ? "\033[0m" : ""; \
				snprintf(__printBuf+__offset, MAX_PRINT_SIZE-__offset, "%s", pColor); \
				syslog(lev, "%s %s", type, __printBuf);\
				__upload_syslog( __printBuf );\
			}\
		} while(0)
#else
#define PrintInfo(color, type, format)  \
	do {\
		char __printBuf[MAX_PRINT_SIZE] = { 0 }; \
		if (strlen(format) < MAX_PRINT_SIZE) { \
			struct tm *ptm;\
			time_t ts = time(NULL);\
			ptm = localtime(&ts);\
			snprintf(__printBuf, MAX_PRINT_SIZE, "%s%02d-%02d %02d:%02d:%02d|%s ", \
					 color, (ptm->tm_mon)+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, type); \
			size_t __offset = strlen(__printBuf); \
			va_list  __va; \
			va_start(__va, format); \
			vsnprintf(__printBuf+__offset, MAX_PRINT_SIZE-__offset, format, __va);\
			va_end(__va); \
			__offset = strlen(__printBuf); \
			const char* pColor = strlen(color) > 0 ? "\033[0m" : ""; \
			snprintf(__printBuf+__offset, MAX_PRINT_SIZE-__offset, "%s", pColor); \
			printf("%s", __printBuf); \
			printf("\n"); \
		}\
	} while(0)

#endif /* SYSLOG */

	void set_verbose(int iVerbose)
	{	print_level = iVerbose;	}
		
	int get_verbose(int iVerbose)
	{	return iVerbose <= print_level;	}

	void init_syslog()
	{
		openlog("ikcc", LOG_PID, LOG_LOCAL0);
	}

	void close_syslog()
	{
		closelog();
	}
	
	void __upload_syslog(char *printbuf)
	{
		int i;
		int value = 0;
		wchar_t wstr[MAX_DATA_SIZE]={0}; 
		unsigned char  buf[MAX_DATA_SIZE]; 
		if(g_sdk_open == 1)
			{
				buf[0] = 199;
				buf[1] = 9 << 4;
				
				mbstowcs(wstr,printbuf,MAX_DATA_SIZE*2);
				unsigned int *p = (wchar_t *)wstr;
				for (i = 0; i < (wcslen(wstr)-4); i++)
				{
					value = p[i];
					buf[3 + i] = value;
				}
				buf[2] = i;
				/*buf[i-1] = 0x20;
				buf[i] = 0x20;
				buf[i+1] = 0x20;
				buf[i+2] = 0x20;
				buf[i+3] = 0x20;*/
				XlinkUpdateDataPointTcp(0,buf,(i+3),0);
			}
		return;
	}
	
	// 打印调试日志
	void LogDebug(const char* file, const char* function, int line, const char* fmt, ...)
	{ 
		if(get_verbose(IPRINT_DEBUG))
		{
			char szTypeInfo[256] = { 0 };
			snprintf(szTypeInfo, sizeof(szTypeInfo), " %s::%s<%d>", file, function, line);
#ifdef SYSLOG
			PrintInfo(IPRINT_DEBUG, "\033[0;36m", szTypeInfo, fmt); 	//深绿
#else
			PrintInfo("\033[0;36m", szTypeInfo, fmt); 	//深绿
#endif /* SYSLOG */
		}
	}
	
	// 打印调试日志
	void LogNotice(const char* file, const char* function, int line, const char* fmt, ...)
	{ 
		if(get_verbose(IPRINT_NOTICE))
		{
			char szTypeInfo[256] = { 0 };
			snprintf(szTypeInfo, sizeof(szTypeInfo), " %s::%s<%d>", file, function, line);
#ifdef SYSLOG
			PrintInfo(IPRINT_NOTICE, "\033[0;37m", szTypeInfo, fmt); 	//白色
#else
			PrintInfo("\033[0;37m", szTypeInfo, fmt); 	//白色
#endif /* SYSLOG */
		}
	}
	
	// 打印重要信息    
	void LogInfo(const char* file, const char* function, int line, const char* fmt, ...)
	{
		if(get_verbose(IPRINT_INFO))
		{
			char szTypeInfo[256] = { 0 };
			snprintf(szTypeInfo, sizeof(szTypeInfo), " %s::%s<%d>", file, function, line);
#ifdef SYSLOG
			PrintInfo(IPRINT_INFO, "\033[0;32m", szTypeInfo, fmt);	//绿色
#else
			PrintInfo("\033[0;32m", szTypeInfo, fmt);	//绿色
#endif /* SYSLOG */
		}
	}
	
	// 打印告警信息
	void LogWarn(const char* file, const char* function, int line, const char* fmt, ...)
	{
		if(get_verbose(IPRINT_WARNING))
		{
			char szTypeInfo[256] = { 0 };
			snprintf(szTypeInfo, sizeof(szTypeInfo), " %s::%s<%d>", file, function, line);
#ifdef SYSLOG
			PrintInfo(IPRINT_WARNING, "\033[0;33m", szTypeInfo, fmt);	//黄色
#else
			PrintInfo("\033[0;33m", szTypeInfo, fmt);	//黄色
#endif /* SYSLOG */

		}
	}
	
	// 打印错误信息
	void LogError(const char* file, const char* function, int line, const char* fmt, ...)
	{
		if(get_verbose(IPRINT_ERR))
		{
			char szTypeInfo[256] = { 0 };
			snprintf(szTypeInfo, sizeof(szTypeInfo), " %s::%s<%d>", file, function, line);
#ifdef SYSLOG
			PrintInfo(IPRINT_ERR, "\033[0;31m", szTypeInfo, fmt);	//红色
#else
			PrintInfo("\033[0;31m", szTypeInfo, fmt);	//红色
#endif /* SYSLOG */
		}
	}
	
	// 打印致命信息
	void LogFatal(const char* file, const char* function, int line, const char* fmt, ...)
	{
		if(get_verbose(IPRINT_ALERT))
		{
			char szTypeInfo[256] = { 0 };
			snprintf(szTypeInfo, sizeof(szTypeInfo), " %s::%s<%d>", file, function, line);
#ifdef SYSLOG 
			PrintInfo(IPRINT_ALERT, "\033[0;35m", szTypeInfo, fmt);	//紫色
#else
			//PrintInfo("\033[0;35m", szTypeInfo, fmt);	//紫色
#endif /* SYSLOG */

		}
	}
	
#ifdef SYSLOG
	void print_hexbuf(char* file, const char* func, const int line, const unsigned char * data, int len)
	{		
	    int i, j;
		char tmp[128] = {0};
		char tmp1[128] = {0};
		char tmp2[128] = {0};
		if(get_verbose(IPRINT_DEBUG))
		{
			Info("Start %s::%s<%d> len[%d]++++++++++++++++++++++++++++++++++++", file, func, line, len);
			
			int lineNum = 16; /// 每行显示的字符数，默认16个
			int rows = len/lineNum; /// 整行数
			
			for( i = 0; i < rows; i++)
			{
				//Info("%04x		",i * lineNum); /// 打印行号
				
				int num = i*lineNum;
				//sprintf(tmp1, "%04x		", i * lineNum);
				for( j = 0; j < lineNum; j++) /// 打印本行的16进制数据
				{
					//Info("%02x ",data[num + j]);
					sprintf(tmp1, "%s%02x ", tmp1, data[num + j]);
				}
		
				for( j = 0; j < lineNum; j++) /// 打印本行的字符
				{
					if(data[num + j] <= 32 || data[num + j] >= 127) /// 将不可见字符作为空格输出
					{
						sprintf(tmp2, "%s ", tmp2);
					}
					else
					{
						sprintf(tmp2, "%s%c", tmp2, data[num+j]);
					}
				}
				sprintf(tmp, "%04x	%s	%s", i * lineNum, tmp1, tmp2);
				Info(tmp);
				FT_MEMSET(tmp,0,128);
				FT_MEMSET(tmp1,0,128);
				FT_MEMSET(tmp2,0,128);
			}
			
			int leftlen = len%lineNum;
			if(leftlen != 0)
			{
				//Info("\n%04x	 ",len-leftlen);
				for( j = 0; j < leftlen; j++)
				{
					//Info("%02x ",data[len - leftlen + j]);
					sprintf(tmp1, "%s%02x ", tmp1, data[len - leftlen + j]);
				}
				
				for( j = leftlen; j < lineNum; j++)
				{
					//Info("   ");
					sprintf(tmp1, "%s   ", tmp1);
				}
				
				//Info("	 ");
				for( j = 0; j < leftlen; j++)
				{
					if(data[len - leftlen + j] <= 32 || data[len - leftlen + j] >= 127)
					{
						sprintf(tmp2, "%s ", tmp2);
					}
					else
					{
						//Info("%c",data[len-leftlen + j]);
						sprintf(tmp2, "%s%c", tmp2, data[len-leftlen + j]);
					}
				}
				sprintf(tmp, "%04x	%s	%s", len-leftlen, tmp1, tmp2);
				Info(tmp);
			}
			
			Info("End PrintHexBuf ------------------------------------------");
		}
	}
#else
void print_hexbuf(char* file, const char* func, const int line, const unsigned char * data, int len)
{		
    int i, j;
	if(get_verbose(IPRINT_DEBUG))
	{
		Info("Start %s::%s<%d> len[%d]++++++++++++++++++++++++++++++++++++\n", file, func, line, len);
		
		int lineNum = 16; /// 每行显示的字符数，默认16个
		int rows = len/lineNum; /// 整行数
		
		for( i = 0; i < rows; i++)
		{
			Info("\n%04x	 ",i * lineNum); /// 打印行号
			int num = i*lineNum;
			
			for( j = 0; j < lineNum; j++) /// 打印本行的16进制数据
			{
				Info("%02x ",data[num + j]);
			}
	
			Info("	 ");
			for( j = 0; j < lineNum; j++) /// 打印本行的字符
			{
				if(data[num + j] <= 32 || data[num + j] >= 127) /// 将不可见字符作为空格输出
				{
					Info(" ");
				}
				else
				{
					Info("%c",data[num+j]);
				}
			}
		}
		
		int leftlen = len%lineNum;
		if(leftlen != 0)
		{
			Info("\n%04x	 ",len-leftlen);
			for( j = 0; j < leftlen; j++)
			{
				Info("%02x ",data[len - leftlen + j]);
			}
			
			for( j = leftlen; j < lineNum; j++)
			{
				Info("   ");
			}
			
			Info("	 ");
			for( j = 0; j < leftlen; j++)
			{
				if(data[len - leftlen + j] <= 32 || data[len - leftlen + j] >= 127)
				{
					Info(" ");
				}
				else
				{
					Info("%c",data[len-leftlen + j]);
				}
			}
		}
		
		Info("\nEnd PrintHexBuf ------------------------------------------\n");
	}
}
#endif

