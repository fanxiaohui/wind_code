#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void set_verbose(int verbose);
int get_verbose(int verbose);

extern int g_sdk_open;
// print message, controlled by messsage level
void LogDebug(const char* file, const char* function, int line, const char* fmt, ...);
void LogNotice(const char* file, const char* function, int line, const char* fmt, ...);
void LogInfo(const char* file, const char* function, int line, const char* fmt, ...);
void LogWarn(const char* file, const char* function, int line, const char* fmt, ...);
void LogError(const char* file, const char* function, int line, const char* fmt, ...);
void LogFatal(const char* file, const char* function, int line, const char* fmt, ...);

void init_syslog();


/// ��ʮ�����Ƶķ�ʽ��ӡָ�����ȵ�data����
void print_hexbuf(char* file, const char* func, const int line, const unsigned char * data, int len);

// ȥ��__FILE__�е�·����ֻҪ�ļ���

#ifdef __cplusplus
//#define __FILENAME__S (strrchr(__FILE__, '/'))
#define __FILENAME__S ("[ICKK] "+std::string(x).substr(std::string(x).find_last_of('/')+1)).c_str()
#else
#define filename(x) strrchr(x,'/')?strrchr(x,'/')+1:x
#define __FILENAME__S (filename(__FILE__))
#endif

#define Print_hex(msg, len) print_hexbuf(__FILENAME__S, __FUNCTION__, __LINE__, msg, len);

/// ������־��ӡ, ��ɫ
#define Debug(format, ...) LogDebug(__FILENAME__S, __FUNCTION__, __LINE__, format, ## __VA_ARGS__)

/// ��Ҫ��Ϣ��ӡ, ��ɫ
#define Info(format, ...)  LogInfo(__FILENAME__S, __FUNCTION__, __LINE__, format, ## __VA_ARGS__)

/// ������־��ӡ, ��ɫ
#define Notice(format, ...) LogNotice(__FILENAME__S, __FUNCTION__, __LINE__, format, ## __VA_ARGS__)

/// �澯��Ϣ��ӡ,��ɫ
#define Warn(format, ...)  LogWarn(__FILENAME__S, __FUNCTION__, __LINE__, format, ## __VA_ARGS__)

/// ������Ϣ��ӡ,��ɫ
#define Error(format, ...) LogError(__FILENAME__S, __FUNCTION__, __LINE__, format, ## __VA_ARGS__)

/// ������Ϣ��ӡ,��ɫ
#define Fatal(format, ...) LogFatal(__FILENAME__S, __FUNCTION__, __LINE__, format, ## __VA_ARGS__)

#define Tracepoint() Debug("")


#endif

