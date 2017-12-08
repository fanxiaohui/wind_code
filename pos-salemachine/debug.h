#ifndef _LEOM_DBG_H_
#define _LEOM_DBG_H_
#include <syslog.h>

#define debug syslog
//#define debug(xxx,...) _debug(__BASE_FILE__, __LINE__, xxx, __VA_ARGS__)

/** @internal */
void _debug(const char filename[], int line, int level, const char *format, ...);

#endif
