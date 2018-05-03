/*

*--------------------------------------------------------------------------------------
****************************************************************************************
*/

#include "config.h"

/*
****************************************************************************************
* Private define:	TASK STACK SIZES
****************************************************************************************
*/


#define	OS_MFUN_TASK_STACK_SIZE	    256		    /* 主功能函数*/
#define	OS_VENDMACHINE_TASK_STACK_SIZE	    256		    /* 主功能函数*/
#define	OS_MDB_TASK_STACK_SIZE	    256		    /*MDB分析功能 */
#define	OS_MEM_TASK_STACK_SIZE	    256		    /*MEM存储功能 */
#define	OS_REALGEL_TASK_STACK_SIZE  256		    /*检测检查系统的参数能 */
#define	OS_LEDDOOR_TASK_STACK_SIZE  256		    /*LED系统的参数能 */
#define	OS_CARD1_TASK_STACK_SIZE     256		    /*读卡的参数能 */
#define	OS_CARD2_TASK_STACK_SIZE     256		    /*读卡的参数能 */
#define	OS_CARD3_TASK_STACK_SIZE     256		    /*读卡的参数能 */
#define	OS_CARD4_TASK_STACK_SIZE     256		    /*读卡的参数能 */
#define	OS_USB_TASK_STACK_SIZE      8000		    /*usb */

#define	OS_VTS_TASK_STACK_SIZE	    256		    /* VTS*/
#define	OS_POS_TASK_STACK_SIZE	    256		    /* POS*/
/*
****************************************************************************************
* Private variables:	TASK STACK ARRAY
****************************************************************************************
*/
OS_STK		VTSTaskStk[OS_VTS_TASK_STACK_SIZE];		/*vts*/
OS_STK		POSTaskStk[OS_POS_TASK_STACK_SIZE];		/*pos*/

OS_STK		VENDMACHINETaskStk[OS_VENDMACHINE_TASK_STACK_SIZE];		/*主任务任务 */
OS_STK		MEMTaskStk[OS_MEM_TASK_STACK_SIZE];		/*存储任务 */
OS_STK		MDBTaskStk[OS_MDB_TASK_STACK_SIZE];		/*MDB任务 */
OS_STK		REALGELTaskStk[OS_REALGEL_TASK_STACK_SIZE];		/*实时检测任务 */

OS_STK		LEDDOORTaskStk[OS_LEDDOOR_TASK_STACK_SIZE];		/*LED灯检测任务 */
OS_STK		CARD1TaskStk[OS_CARD1_TASK_STACK_SIZE];		/*读卡器检测任务 */
OS_STK		CARD2TaskStk[OS_CARD2_TASK_STACK_SIZE];		/*读卡器检测任务 */
OS_STK		CARD3TaskStk[OS_CARD3_TASK_STACK_SIZE];		/*读卡器检测任务 */
OS_STK		CARD4TaskStk[OS_CARD4_TASK_STACK_SIZE];		/*读卡器检测任务 */

OS_STK		USBTaskStk[OS_USB_TASK_STACK_SIZE];		/*USB读写U盘任务 */
/*
****************************************************************************************
* Public variables:	OS EVENT ECB
****************************************************************************************
*/



OS_EVENT *DOORQS;   //存储队列
OS_EVENT *ServerQS;   //存储队列

OS_EVENT *VEMMsgMbox;/*售货机传入的命令*/
OS_EVENT *VEM_ENQ_MsgMbox;/*售货机输入的查询命令*/

OS_EVENT *VEM_DEC_MsgMbox;/*售货机扣费请求命令*/

OS_EVENT *CARD1_TO_MIANMbox;/*从读卡器发送数据至主程序*/
OS_EVENT *MAIN1_SL;/*主程序选择商品*/
OS_EVENT *CARD1_DEC_MsgMbox;/*读卡器扣费信箱*/
OS_EVENT *CARD1_Updata_MsgMbox;/*读卡器卡更新信箱*/

OS_EVENT *CARD2_TO_MIANMbox;/*从读卡器发送数据至主程序*/
OS_EVENT *MAIN2_SL;/*主程序选择商品*/
OS_EVENT *CARD2_DEC_MsgMbox;/*读卡器扣费信箱*/
OS_EVENT *CARD2_Updata_MsgMbox;/*读卡器卡更新信箱*/


//OS_EVENT *CARD3_TO_MIANMbox;/*从读卡器发送数据至主程序*/
//OS_EVENT *MAIN3_SL;/*主程序选择商品*/
//OS_EVENT *CARD3_DEC_MsgMbox;/*读卡器扣费信箱*/
//OS_EVENT *CARD3_Updata_MsgMbox;/*读卡器卡更新信箱*/
//
//OS_EVENT *CARD4_TO_MIANMbox;/*从读卡器发送数据至主程序*/
//OS_EVENT *MAIN4_SL;/*主程序选择商品*/
//OS_EVENT *CARD4_DEC_MsgMbox;/*读卡器扣费信箱*/
//OS_EVENT *CARD4_Updata_MsgMbox;/*读卡器卡更新信箱*/

OS_EVENT *USB_Write_MsgMbox;/*USB写入信箱*/

OS_EVENT *Wait_VTS_MsgMbox; //等待售货机发生扣款指令
OS_EVENT *Wait_POS_MsgMbox; // 等待POS返回刷卡结果

OS_EVENT *Memmutex;/*FLASH读写互斥量*/

void *DOORQSQSArray[OSArr];
void *ServerQSQSArray[OSArr];
/****************************************************************************************
** 函数名称: create_os_task
** 功能描述: 创建系统中的大部分任务
** 参    数: None
** 返 回 值: None
** 作　  者: 罗辉联
** 日  　期: 2007年12月25日
**---------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_task(void)
{
    //
    OSTaskCreateExt(vts_recv_task, (void *)0, (OS_STK *)&VTSTaskStk[OS_VTS_TASK_STACK_SIZE - 1],
                    VTS_MFUN_TASK_PRO,
                    VTS_MFUN_TASK_PRO,
                    (OS_STK *)&VTSTaskStk[0],
                    OS_VTS_TASK_STACK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OSTaskCreateExt(pos_recv_task, (void *)0, (OS_STK *)&POSTaskStk[OS_POS_TASK_STACK_SIZE - 1],
                    POS_MFUN_TASK_PRO,
                    POS_MFUN_TASK_PRO,
                    (OS_STK *)&POSTaskStk[0],
                    OS_POS_TASK_STACK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
//    OSTaskCreateExt(VENDMACHINETask, (void *)0, (OS_STK *)&VENDMACHINETaskStk[OS_VENDMACHINE_TASK_STACK_SIZE - 1],
//                    OS_VENDMACHINE_TASK_PRIO,
//                    OS_VENDMACHINE_TASK_PRIO,
//                    (OS_STK *)&VENDMACHINETaskStk[0],
//                    OS_VENDMACHINE_TASK_STACK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);


//    OSTaskCreateExt(MDBTask, (void *)0, (OS_STK *)&MDBTaskStk[OS_MDB_TASK_STACK_SIZE - 1],
//                    OS_MDB_TASK_PRIO,
//                    OS_MDB_TASK_PRIO,
//                    (OS_STK *)&MDBTaskStk[0],
//                    OS_MDB_TASK_STACK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

//    OSTaskCreateExt(MEMtask, (void *)0, (OS_STK *)&MEMTaskStk[OS_MEM_TASK_STACK_SIZE - 1],
//                    OS_MEM_TASK_PRIO,
//                    OS_MEM_TASK_PRIO,
//                    (OS_STK *)&MEMTaskStk[0],
//                    OS_MEM_TASK_STACK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

//    OSTaskCreateExt(REALGELtask, (void *)0, (OS_STK *)&REALGELTaskStk[OS_REALGEL_TASK_STACK_SIZE - 1],
//                    OS_REALGEL_TASK_PRIO,
//                    OS_REALGEL_TASK_PRIO,
//                    (OS_STK *)&REALGELTaskStk[0],
//                    OS_REALGEL_TASK_STACK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

//    OSTaskCreateExt(LEDDOORtask, (void *)0, (OS_STK *)&LEDDOORTaskStk[OS_LEDDOOR_TASK_STACK_SIZE - 1],
//                    OS_LEDDOOR_TASK_PRIO,
//                    OS_LEDDOOR_TASK_PRIO,
//                    (OS_STK *)&LEDDOORTaskStk[0],
//                    OS_LEDDOOR_TASK_STACK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
//    OSTaskCreateExt(CARD1task, (void *)0, (OS_STK *)&CARD1TaskStk[OS_CARD1_TASK_STACK_SIZE - 1],
//                    OS_CARD1_TASK_PRIO,
//                    OS_CARD1_TASK_PRIO,
//                    (OS_STK *)&CARD1TaskStk[0],
//                    OS_CARD1_TASK_STACK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
//    OSTaskCreateExt(CARD2task, (void *)0, (OS_STK *)&CARD2TaskStk[OS_CARD2_TASK_STACK_SIZE - 1],
//                    OS_CARD2_TASK_PRIO,
//                    OS_CARD2_TASK_PRIO,
//                    (OS_STK *)&CARD2TaskStk[0],
//                    OS_CARD2_TASK_STACK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
//    OSTaskCreateExt(USBtask, (void *)0, (OS_STK *)&USBTaskStk[OS_USB_TASK_STACK_SIZE - 1],
//                    OS_USB_TASK_PRIO,
//                    OS_USB_TASK_PRIO,
//                    (OS_STK *)&USBTaskStk[0],
//                    OS_USB_TASK_STACK_SIZE,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if (OS_TASK_NAME_SIZE >= 16)
    OSTaskNameSet(OS_KPD_TASK_PRIO, (INT8U *)"kpd task", &err);
#endif

}
/****************************************************************************************
** 函数名称: create_task_status
** 功能描述: 设置特殊任务的初始状态
** 参    数: None
** 返 回 值: None
** 作　  者: 罗辉联
** 日  　期: 2007年12月25日
**---------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_task_status(void)
{


}

/****************************************************************************************
** 函数名称: create_os_semphore
** 功能描述: 创建系统中的大部分信号量
** 参    数: None
** 返 回 值: None
** 作　  者: 罗辉联
** 日  　期: 2007年12月25日
**---------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_semphore(void)
{
}

/****************************************************************************************
** 函数名称: create_os_mutex
** 功能描述: 创建系统中的大部分互斥量
** 参    数: None
** 返 回 值: None
** 作　  者: 罗辉联
** 日  　期: 2007年12月25日
**---------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_mutex(void)
{
    u8 err;
    Memmutex = OSMutexCreate(1, &err);


}
/****************************************************************************************
** 函数名称: create_os_mailbox
** 功能描述: 创建系统中的通用消息邮箱
** 参    数: None
** 返 回 值: None
** 作　  者: 罗辉联
** 日  　期: 2007年12月25日
**---------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_mailbox(void)
{
	Wait_VTS_MsgMbox = OSMboxCreate((void *)0);
	Wait_POS_MsgMbox = OSMboxCreate((void *)0);

    //USB_Write_MsgMbox = OSMboxCreate((void *)0);
}

/****************************************************************************************
** 函数名称: create_os_queue
** 功能描述: 创建系统中的通用消息队列
** 参    数: None
** 返 回 值: None
** 作　  者: 罗辉联
** 日  　期: 2007年12月25日
**---------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_queue(void)
{



}

/****************************************************************************************
** 函数名称: create_os_timer
** 功能描述: 创建OS软件定时器
** 参    数: None
** 返 回 值: None
** 作　  者: 罗辉联
** 日  　期: 2007年12月25日
**---------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_timer(void)
{
    /* Os软件定时器实体创建 */

}

/************************************** end of file ************************************/



