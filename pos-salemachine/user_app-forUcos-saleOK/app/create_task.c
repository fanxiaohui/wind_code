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


#define	OS_MFUN_TASK_STACK_SIZE	    256		    /* �����ܺ���*/
#define	OS_VENDMACHINE_TASK_STACK_SIZE	    256		    /* �����ܺ���*/
#define	OS_MDB_TASK_STACK_SIZE	    256		    /*MDB�������� */
#define	OS_MEM_TASK_STACK_SIZE	    256		    /*MEM�洢���� */
#define	OS_REALGEL_TASK_STACK_SIZE  256		    /*�����ϵͳ�Ĳ����� */
#define	OS_LEDDOOR_TASK_STACK_SIZE  256		    /*LEDϵͳ�Ĳ����� */
#define	OS_CARD1_TASK_STACK_SIZE     256		    /*�����Ĳ����� */
#define	OS_CARD2_TASK_STACK_SIZE     256		    /*�����Ĳ����� */
#define	OS_CARD3_TASK_STACK_SIZE     256		    /*�����Ĳ����� */
#define	OS_CARD4_TASK_STACK_SIZE     256		    /*�����Ĳ����� */
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

OS_STK		VENDMACHINETaskStk[OS_VENDMACHINE_TASK_STACK_SIZE];		/*���������� */
OS_STK		MEMTaskStk[OS_MEM_TASK_STACK_SIZE];		/*�洢���� */
OS_STK		MDBTaskStk[OS_MDB_TASK_STACK_SIZE];		/*MDB���� */
OS_STK		REALGELTaskStk[OS_REALGEL_TASK_STACK_SIZE];		/*ʵʱ������� */

OS_STK		LEDDOORTaskStk[OS_LEDDOOR_TASK_STACK_SIZE];		/*LED�Ƽ������ */
OS_STK		CARD1TaskStk[OS_CARD1_TASK_STACK_SIZE];		/*������������� */
OS_STK		CARD2TaskStk[OS_CARD2_TASK_STACK_SIZE];		/*������������� */
OS_STK		CARD3TaskStk[OS_CARD3_TASK_STACK_SIZE];		/*������������� */
OS_STK		CARD4TaskStk[OS_CARD4_TASK_STACK_SIZE];		/*������������� */

OS_STK		USBTaskStk[OS_USB_TASK_STACK_SIZE];		/*USB��дU������ */
/*
****************************************************************************************
* Public variables:	OS EVENT ECB
****************************************************************************************
*/



OS_EVENT *DOORQS;   //�洢����
OS_EVENT *ServerQS;   //�洢����

OS_EVENT *VEMMsgMbox;/*�ۻ������������*/
OS_EVENT *VEM_ENQ_MsgMbox;/*�ۻ�������Ĳ�ѯ����*/

OS_EVENT *VEM_DEC_MsgMbox;/*�ۻ����۷���������*/

OS_EVENT *CARD1_TO_MIANMbox;/*�Ӷ���������������������*/
OS_EVENT *MAIN1_SL;/*������ѡ����Ʒ*/
OS_EVENT *CARD1_DEC_MsgMbox;/*�������۷�����*/
OS_EVENT *CARD1_Updata_MsgMbox;/*����������������*/

OS_EVENT *CARD2_TO_MIANMbox;/*�Ӷ���������������������*/
OS_EVENT *MAIN2_SL;/*������ѡ����Ʒ*/
OS_EVENT *CARD2_DEC_MsgMbox;/*�������۷�����*/
OS_EVENT *CARD2_Updata_MsgMbox;/*����������������*/


//OS_EVENT *CARD3_TO_MIANMbox;/*�Ӷ���������������������*/
//OS_EVENT *MAIN3_SL;/*������ѡ����Ʒ*/
//OS_EVENT *CARD3_DEC_MsgMbox;/*�������۷�����*/
//OS_EVENT *CARD3_Updata_MsgMbox;/*����������������*/
//
//OS_EVENT *CARD4_TO_MIANMbox;/*�Ӷ���������������������*/
//OS_EVENT *MAIN4_SL;/*������ѡ����Ʒ*/
//OS_EVENT *CARD4_DEC_MsgMbox;/*�������۷�����*/
//OS_EVENT *CARD4_Updata_MsgMbox;/*����������������*/

OS_EVENT *USB_Write_MsgMbox;/*USBд������*/

OS_EVENT *Wait_VTS_MsgMbox; //�ȴ��ۻ��������ۿ�ָ��
OS_EVENT *Wait_POS_MsgMbox; // �ȴ�POS����ˢ�����

OS_EVENT *Memmutex;/*FLASH��д������*/

void *DOORQSQSArray[OSArr];
void *ServerQSQSArray[OSArr];
/****************************************************************************************
** ��������: create_os_task
** ��������: ����ϵͳ�еĴ󲿷�����
** ��    ��: None
** �� �� ֵ: None
** ����  ��: �޻���
** ��  ����: 2007��12��25��
**---------------------------------------------------------------------------------------
** �� �� ��:
** �ա�  ��:
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
** ��������: create_task_status
** ��������: ������������ĳ�ʼ״̬
** ��    ��: None
** �� �� ֵ: None
** ����  ��: �޻���
** ��  ����: 2007��12��25��
**---------------------------------------------------------------------------------------
** �� �� ��:
** �ա�  ��:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_task_status(void)
{


}

/****************************************************************************************
** ��������: create_os_semphore
** ��������: ����ϵͳ�еĴ󲿷��ź���
** ��    ��: None
** �� �� ֵ: None
** ����  ��: �޻���
** ��  ����: 2007��12��25��
**---------------------------------------------------------------------------------------
** �� �� ��:
** �ա�  ��:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_semphore(void)
{
}

/****************************************************************************************
** ��������: create_os_mutex
** ��������: ����ϵͳ�еĴ󲿷ֻ�����
** ��    ��: None
** �� �� ֵ: None
** ����  ��: �޻���
** ��  ����: 2007��12��25��
**---------------------------------------------------------------------------------------
** �� �� ��:
** �ա�  ��:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_mutex(void)
{
    u8 err;
    Memmutex = OSMutexCreate(1, &err);


}
/****************************************************************************************
** ��������: create_os_mailbox
** ��������: ����ϵͳ�е�ͨ����Ϣ����
** ��    ��: None
** �� �� ֵ: None
** ����  ��: �޻���
** ��  ����: 2007��12��25��
**---------------------------------------------------------------------------------------
** �� �� ��:
** �ա�  ��:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_mailbox(void)
{
	Wait_VTS_MsgMbox = OSMboxCreate((void *)0);
	Wait_POS_MsgMbox = OSMboxCreate((void *)0);

    //USB_Write_MsgMbox = OSMboxCreate((void *)0);
}

/****************************************************************************************
** ��������: create_os_queue
** ��������: ����ϵͳ�е�ͨ����Ϣ����
** ��    ��: None
** �� �� ֵ: None
** ����  ��: �޻���
** ��  ����: 2007��12��25��
**---------------------------------------------------------------------------------------
** �� �� ��:
** �ա�  ��:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_queue(void)
{



}

/****************************************************************************************
** ��������: create_os_timer
** ��������: ����OS�����ʱ��
** ��    ��: None
** �� �� ֵ: None
** ����  ��: �޻���
** ��  ����: 2007��12��25��
**---------------------------------------------------------------------------------------
** �� �� ��:
** �ա�  ��:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void create_os_timer(void)
{
    /* Os�����ʱ��ʵ�崴�� */

}

/************************************** end of file ************************************/



