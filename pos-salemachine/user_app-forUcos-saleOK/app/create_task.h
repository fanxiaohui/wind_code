/*
***************************************************************************************
*
*                       博格达科技有限公司
*
*               (c) Copyright 2006-2008, hui lian. luo, china, zj
*                            All Rights Reserved
*
*                           http://www.bogodtech.com
*
*--------------文件信息-----------------------------------------------------------------
* 文 件 名: create_task.h
* 创 建 人: 罗辉联
* 创建日期: 2007年11月10日
* 描    述: 任务管理文件
* 技术顾问: 楼东武(副教授)  浙江大学信电系
*
*---------- 版本信息-------------------------------------------------------------------
* 版    本: V1.0
*
*--------------------------------------------------------------------------------------
****************************************************************************************
*/

#ifndef CREATE_TASK_H
#define CREATE_TASK_H


/*
****************************************************************************************
* Public define:	TASK PRIORITIES
****************************************************************************************
*/
#define VTS_MFUN_TASK_PRO	3
#define POS_MFUN_TASK_PRO	4

#define OS_VENDMACHINE_TASK_PRIO		5
//#define OS_MFUN_TASK_PRIO  	    4
//#define OS_MDB_TASK_PRIO		3
#define OS_MEM_TASK_PRIO		6
#define OS_REALGEL_TASK_PRIO	7
#define OS_LEDDOOR_TASK_PRIO	8
#define OS_CARD1_TASK_PRIO		9
#define OS_CARD2_TASK_PRIO		10
#define OS_CARD3_TASK_PRIO		11
#define OS_CARD4_TASK_PRIO		12

#define OS_USB_TASK_PRIO		13



/*
****************************************************************************************
* Public variables:	OS EVENT ECB
****************************************************************************************
*/



extern OS_EVENT *DOORQS;   //存储队列
extern OS_EVENT *ServerQS;   //存储队列

extern OS_EVENT *VEMMsgMbox;/*售货机传入的命令*/
extern OS_EVENT *VEM_ENQ_MsgMbox;/*售货机输入的查询命令*/
extern OS_EVENT *VEM_DEC_MsgMbox;/*售货机扣费请求命令*/

//extern OS_EVENT *CARD_TO_MIANMbox;/*从读卡器发送数据至主程序*/
//extern OS_EVENT *MAIN_SL;/*主程序选择商品*/
//extern OS_EVENT *CARD_DEC_MsgMbox;/*读卡器扣费信箱*/
//extern OS_EVENT *CARD_Updata_MsgMbox;/*读卡器卡更新信箱*/

extern OS_EVENT *CARD1_TO_MIANMbox;/*从读卡器发送数据至主程序*/
extern OS_EVENT *MAIN1_SL;/*主程序选择商品*/
extern OS_EVENT *CARD1_DEC_MsgMbox;/*读卡器扣费信箱*/
extern OS_EVENT *CARD1_Updata_MsgMbox;/*读卡器卡更新信箱*/

extern OS_EVENT *CARD2_TO_MIANMbox;/*从读卡器发送数据至主程序*/
extern OS_EVENT *MAIN2_SL;/*主程序选择商品*/
extern OS_EVENT *CARD2_DEC_MsgMbox;/*读卡器扣费信箱*/
extern OS_EVENT *CARD2_Updata_MsgMbox;/*读卡器卡更新信箱*/


extern OS_EVENT *CARD3_TO_MIANMbox;/*从读卡器发送数据至主程序*/
extern OS_EVENT *MAIN3_SL;/*主程序选择商品*/
extern OS_EVENT *CARD3_DEC_MsgMbox;/*读卡器扣费信箱*/
extern OS_EVENT *CARD3_Updata_MsgMbox;/*读卡器卡更新信箱*/

extern OS_EVENT *CARD4_TO_MIANMbox;/*从读卡器发送数据至主程序*/
extern OS_EVENT *MAIN4_SL;/*主程序选择商品*/
extern OS_EVENT *CARD4_DEC_MsgMbox;/*读卡器扣费信箱*/
extern OS_EVENT *CARD4_Updata_MsgMbox;/*读卡器卡更新信箱*/



extern OS_EVENT *USB_Write_MsgMbox;/*USB写入信箱*/
extern OS_EVENT *Wait_VTS_MsgMbox; //等待刷卡
extern OS_EVENT *Wait_POS_MsgMbox; //等待刷卡

extern void *SAVEQSArray[OSArr];
extern void *FANQSArray[OSArr];
extern void *ServerQSArray[OSArr];
extern OS_EVENT *Memmutex;/*FLASH读写互斥量*/
/*
****************************************************************************************
* Public function
****************************************************************************************
*/

void create_os_task(void);

void create_task_status(void);

void create_os_semphore(void);

void create_os_mutex(void);

void create_os_mailbox(void);

void create_os_queue(void);

void create_os_timer(void);

#endif


/*********************** http://www.bogodtech.com *******End of file **********************/



