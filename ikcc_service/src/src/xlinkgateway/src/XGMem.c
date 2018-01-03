/*
 * XGMem.c
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#include <stdio.h>
#include <stdlib.h> //malloc
#include <string.h>
#include "XGMem.h"

unsigned int memcount = 0;

void *XGMemalloc(const char *file, int line, int size) {

	void *t = malloc(size);
	if (t != NULL) {
		memset(t,0,size);
		memcount++;
		printf("--------------------------------------------men count=%d\r\n", memcount);
	}
	return t;
}

void XGFree(const char *file, int line, void *p) {
	memcount--;
	printf("--------------------------------------------men count=%d\r\n", memcount);
	free(p);
}

