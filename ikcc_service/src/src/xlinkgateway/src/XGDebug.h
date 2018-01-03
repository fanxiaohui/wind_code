/*
 * XGDebug.h
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#ifndef XGDEBUG_H_
#define XGDEBUG_H_

#ifndef _FILE_AND_LINE_
#define _FILE_AND_LINE_  __FILE__,__LINE__
#endif

typedef void (*logcallback)(unsigned int lev, const char *file, int line, const char *format, ...);

enum E_LOG_LEV {
	X_LOG_DEBUG, X_LOG_INFO, X_LOG_WARNING, X_LOG_ERROR, X_LOG_MAX,
};


#define  XGDEBUG(x,y...)  XGDebug(X_LOG_DEBUG,_FILE_AND_LINE_,x,##y)
#define  XGINFO(x,y...)  XGDebug(X_LOG_INFO,_FILE_AND_LINE_,x,##y)
#define  XGWARNING(x,y...)  XGDebug(X_LOG_WARNING,_FILE_AND_LINE_,x,##y)
#define  XGERROR(x,y...)  XGDebug(X_LOG_ERROR,_FILE_AND_LINE_,x,##y)
#define  XGMAX(x,y...)  XGDebug(X_LOG_MAX,_FILE_AND_LINE_,x,##y)



extern void XGDebug(unsigned int lev, const char *file, int line, const char *format, ...);
extern int XGSetDebugLev(int lev);
extern int XGSetLogOutCallback(logcallback callback);



#endif /* XGDEBUG_H_ */

