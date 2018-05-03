/*
***************************************************************************************
*
*                       �����Ƽ����޹�˾
*
*               (c) Copyright 2006-2008, hui lian. luo, china, zj
*                            All Rights Reserved
*
*                           http://www.bogodtech.com
*
*--------------�ļ���Ϣ-----------------------------------------------------------------
* �� �� ��: create_task.h
* �� �� ��: �޻���
* ��������: 2007��11��10��
* ��    ��: ��������ļ�
* ��������: ¥����(������)  �㽭��ѧ�ŵ�ϵ
*
*---------- �汾��Ϣ-------------------------------------------------------------------
* ��    ��: V1.0
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



extern OS_EVENT *DOORQS;   //�洢����
extern OS_EVENT *ServerQS;   //�洢����

extern OS_EVENT *VEMMsgMbox;/*�ۻ������������*/
extern OS_EVENT *VEM_ENQ_MsgMbox;/*�ۻ�������Ĳ�ѯ����*/
extern OS_EVENT *VEM_DEC_MsgMbox;/*�ۻ����۷���������*/

//extern OS_EVENT *CARD_TO_MIANMbox;/*�Ӷ���������������������*/
//extern OS_EVENT *MAIN_SL;/*������ѡ����Ʒ*/
//extern OS_EVENT *CARD_DEC_MsgMbox;/*�������۷�����*/
//extern OS_EVENT *CARD_Updata_MsgMbox;/*����������������*/

extern OS_EVENT *CARD1_TO_MIANMbox;/*�Ӷ���������������������*/
extern OS_EVENT *MAIN1_SL;/*������ѡ����Ʒ*/
extern OS_EVENT *CARD1_DEC_MsgMbox;/*�������۷�����*/
extern OS_EVENT *CARD1_Updata_MsgMbox;/*����������������*/

extern OS_EVENT *CARD2_TO_MIANMbox;/*�Ӷ���������������������*/
extern OS_EVENT *MAIN2_SL;/*������ѡ����Ʒ*/
extern OS_EVENT *CARD2_DEC_MsgMbox;/*�������۷�����*/
extern OS_EVENT *CARD2_Updata_MsgMbox;/*����������������*/


extern OS_EVENT *CARD3_TO_MIANMbox;/*�Ӷ���������������������*/
extern OS_EVENT *MAIN3_SL;/*������ѡ����Ʒ*/
extern OS_EVENT *CARD3_DEC_MsgMbox;/*�������۷�����*/
extern OS_EVENT *CARD3_Updata_MsgMbox;/*����������������*/

extern OS_EVENT *CARD4_TO_MIANMbox;/*�Ӷ���������������������*/
extern OS_EVENT *MAIN4_SL;/*������ѡ����Ʒ*/
extern OS_EVENT *CARD4_DEC_MsgMbox;/*�������۷�����*/
extern OS_EVENT *CARD4_Updata_MsgMbox;/*����������������*/



extern OS_EVENT *USB_Write_MsgMbox;/*USBд������*/
extern OS_EVENT *Wait_VTS_MsgMbox; //�ȴ�ˢ��
extern OS_EVENT *Wait_POS_MsgMbox; //�ȴ�ˢ��

extern void *SAVEQSArray[OSArr];
extern void *FANQSArray[OSArr];
extern void *ServerQSArray[OSArr];
extern OS_EVENT *Memmutex;/*FLASH��д������*/
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



