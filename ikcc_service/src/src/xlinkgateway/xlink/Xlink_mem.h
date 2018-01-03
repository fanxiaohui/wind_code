/*
 * Xlink_mem.h
 *
 *  Created on: 2016-11-24
 *      Author: john
 */

#ifndef XLINK_MEM_H_
#define XLINK_MEM_H_

#include "Xlink_Head_Adaptation.h"

#define _FILE_AND_LINE_  __FILE__,__LINE__

extern XLINK_FUNC void *XlinkMemMalloc(char *file,int line,unsigned int size);
extern XLINK_FUNC void XlinkMemFree(char *file,int line,void *p);

#endif /* XLINK_MEM_H_ */
