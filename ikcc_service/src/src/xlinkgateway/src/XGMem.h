/*
 * XGMem.h
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#ifndef XGMEM_H_
#define XGMEM_H_


#include <stdio.h>

#ifndef _FILE_AND_LINE_
#define _FILE_AND_LINE_  __FILE__,__LINE__
#endif

#define  XGMemFactory(x) XGMemalloc(_FILE_AND_LINE_,(x))
#define  XGMemRelease(x) XGFree(_FILE_AND_LINE_,(x))

extern void *XGMemalloc(const char *file,int line,int size);
extern void XGFree(const char *file,int line,void *p);

#endif /* XGMEM_H_ */
