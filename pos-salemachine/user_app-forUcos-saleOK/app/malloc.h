/**************************************************************************
*                                                                         *
*   PROJECT     : ARM port for nucleus plus                               *
*                                                                         *
*   MODULE      : malloc.h                                                *
*                                                                         *
*   AUTHOR      : luo_huilian					 						  *
*                 URL  : http://www.bogodtech.com							  *
*                 EMAIL: armgcc@foxmail.com                               *
*                                                                         *
*   PROCESSOR   : STR710 (32 bit ARM7TDI RISC core )                      *
*                                                                         *
*   IDE         : SDT 2.51 & ADS 1.2                                      *
*                                                                         *
*   DESCRIPTION :                                                         *
*   This is the Framework module. Creates an operating infrastructure.    *
*                                                                         *
**************************************************************************/



#ifndef __MALLOC_H__
#define __MALLOC_H__


#ifdef __cplusplus
extern "C" {
#endif


/* ********************************************************************* */
/* Module configuration */


/* ********************************************************************* */
/* Interface macro & data definition */

/* ********************************************************************* */
/* Interface function definition */

void mem_init(void);

void *mem_malloc_sixteen(INT16U alloc_size);

void mem_free_sixteen(void *memory_ptr);

/* ********************************************************************* */

#ifdef __cplusplus
}
#endif

#endif /*__MALLOC_H__*/
