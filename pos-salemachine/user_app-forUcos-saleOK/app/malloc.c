/**************************************************************************
*                                                                         *
*   PROJECT     : ARM port for ucos                                       *
*                                                                         *
*   MODULE      : malloc.c                                                *
*                                                                         *
*   AUTHOR      : luo_huilian					 						  *
*                 URL  : http://www.bogodtech.com				    	  *
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


#include "config.h"



/* ********************************************************************* */
/* Local  Maric definitions */


/* MEMORY */


#define BlockNum 				8
#define BlockSize 				16


/* ********************************************************************* */
/* Global variable definitions */


OS_MEM 	*Malloc_Sixteen_p;
INT8U 	malloc_sixteen_addr[BlockNum][BlockSize];


/*
*********************************************************************************************
*                                       mem_init
*
* Description: Create PLUS Memory Pool.
*
* Arguments  : none.
*
* Return     : none.
*
* Author     : luo_huilian
*
* Date       : 2007-3-14
*
* Note(s)    :
*********************************************************************************************
*/

void mem_init(void)
{
    //	INT8U err;

    /* 创建第一个内存分区 16Byte/block */
    //memset((INT8U*)malloc_sixteen_addr , 0xFE , sizeof(malloc_sixteen_addr));

    //	Malloc_Sixteen_p = OSMemCreate(malloc_sixteen_addr, BlockNum, BlockSize, &err);
    /*
    if(err == OS_NO_ERR )
    	printf("Create_Memory_partition_successfully,Malloc_Sixteen_p\n");
    else
    	printf("Create_Memory_partition_error: %d\n",err);
    	*/
}

/*
*********************************************************************************************
*                                       mem_malloc
*
* Description: Malloc PLUS Memory Pool with SUSPEND.
*
* Arguments  : none.
*
* Return     : none.
*
* Author     : luo_huilian
*
* Date       : 2007-3-14
*
* Note(s)    :
*********************************************************************************************
*/

void *mem_malloc_sixteen(INT16U alloc_size)
{
    INT8U 		err;
    void 		*temp_ptr = (void *)0;

    OS_MEM_DATA mem_info;

    if(alloc_size < 16)
    {
        //	err= OSMemQuery(Malloc_Sixteen_p,&mem_info);
        //printf("mem_info value: %d\n",mem_info.OSNFree);
        if(mem_info.OSNFree >= 1)
        {
            temp_ptr = OSMemGet(Malloc_Sixteen_p, &err);
        }
        else
        {
            printf("Malloc_Sixteen_p is not free memory!\n");
        }
    }

    return temp_ptr;

}



/*
*********************************************************************************************
*                                       mem_free
*
* Description: It is free memory that is allocte by mem_malloc or mem_startup.
*
* Arguments  : none.
*
* Return     : none.
*
* Author     : luo_huilian
*
* Date       : 2007-3-14
*
* Note(s)    :
*********************************************************************************************
*/
void mem_free_sixteen(void *memory_ptr)
{
    INT8U	err;

    err = OSMemPut(Malloc_Sixteen_p, memory_ptr);

    if (err == OS_ERR_NONE)
        printf("free ok\n");
    else
        printf("mem_free_sixteen  is fail!\n");

}



/* ************************************end of file **************************************** */
