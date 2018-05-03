/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210E-EVAL Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/

#define  APP_OS_PROBE_EN                         DEF_ENABLED
#define  APP_PROBE_COM_EN                        DEF_ENABLED

/*
*********************************************************************************************************
*                                              TASKS NAMES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define APP_TASK_SYS_STATUS_PRIO				(OS_LOWEST_PRIO - 4)
#define APP_TASK_START_PRIO				(OS_LOWEST_PRIO - 3)

#define  OS_TASK_TMR_PRIO                (OS_LOWEST_PRIO - 2)


#define  APP_TASK_CAN_PRIO    3
#define  APP_TASK_SysTest_PRIO 4
#define  APP_TASK_MDB_PRIO    5
#define  APP_TASK_MFUN_PRIO  6
#define  APP_TASK_CARD_PRIO    7
#define  APP_TASK_KEY_PRIO    8
#define  APP_TASK_APPFLASH_PRIO  9
//#define  APP_TASK_ETH_PRIO    6
//#define  APP_TASK_GPRS_PRIO   7
#define  APP_TASK_HAND_PRIO   10
#define  APP_TASK_LEDDS_PRIO  11
#define  APP_TASK_TIMES_PRIO  12
#define  APP_TASK_ERRLIST_PRIO  13
#define  APP_TASK_COMM_PRIO  14
/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define APP_TASK_SYS_STK_SIZE 512
#define APP_TASK_START_STK_SIZE 512

#define APP_TASK_MDB_STK_SIZE 512
#define APP_TASK_CAN_STK_SIZE 512
#define APP_TASK_KEY_STK_SIZE 512
#define APP_TASK_APPFLASH_STK_SIZE 512
#define APP_TASK_ETH_STK_SIZE 512
#define APP_TASK_GPRS_STK_SIZE 512
#define APP_TASK_HAND_STK_SIZE 512
#define APP_TASK_SysTest_STK_SIZE 512
#define APP_TASK_LEDDS_STK_SIZE 256
#define APP_TASK_TIMES_STK_SIZE 1024
#define APP_TASK_ERRLIST_STK_SIZE 512
#define APP_TASK_CARD_STK_SIZE 512
#define APP_TASK_COMM_STK_SIZE 512
#define APP_TASK_MFUN_STK_SIZE 2048
/*
*********************************************************************************************************
*                                                  LIB
*********************************************************************************************************
*/

extern void AppTaskSysStatus (void);


#endif
