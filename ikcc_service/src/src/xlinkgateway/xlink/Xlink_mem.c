/*
 * Xlink_mem.c
 *
 *  Created on: 2016-11-24
 *      Author: john
 */

#include "Xlink_mem.h"

#include <stdlib.h>

static int memecount = 0;

XLINK_FUNC void *XlinkMemMalloc(char *file, int line, unsigned int size) {
	void *ret = malloc(size);
	if (ret != NULL) {
		memset(ret, 0, size);
		memecount++;
		//printf("---------------------------------------------memcount=%d", memecount);
	}

	return ret;
}

XLINK_FUNC void XlinkMemFree(char *file, int line, void *p) {
	free(p);
	p = NULL;
	memecount--;
	//printf("---------------------------------------------memcount=%d", memecount);
}

