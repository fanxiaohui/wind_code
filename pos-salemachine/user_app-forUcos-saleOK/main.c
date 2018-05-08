

#include "config.h"

void SystemPartInit(void);
static void AppTaskStart (void *pdata);

void App_Init(void);


/*����ϵͳ��ض���*/
OS_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];

void delay()
{
	u32 i, j;
    for(i = 0; i < 10000; i++)
    {
        for(j = 0; j < 1000; j++);
    }
}

void led_blink(u16 led,u8 count)
{
	int i = 0;
	for(i=0;i<count;i++)
	{
		GPIO_SetBits(GPIOB,led);  //OFF
		delay();
		GPIO_ResetBits(GPIOB,led); //ON
		delay();
	}
}

void led4_blink(u8 count)
{
	led_blink(GPIO_Pin_13,count);
}

void led2_blink(u8 count)
{
	led_blink(GPIO_Pin_12,count);
}

void led3_blink(u8 count)
{
	led_blink(GPIO_Pin_14,count);
}


/*����������������￪ʼִ��*/
int main (void)
{
    u8 err;
    u32 i, j;
    for(i = 0; i < 10000; i++)
    {
        for(j = 0; j < 1000; j++);
    }

    System_configAll();/*Ӳ���㣬�弶����*/

    OSInit();
	
    SystemPartInit();
    App_Init();
    OS_CPU_SysTickInit(); /*���ϵͳʱ�ӳ�ʼ��*/


    OSTaskCreateExt(AppTaskStart,
                    (void *)0,
                    (OS_STK *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1],
                    APP_TASK_START_PRIO,
                    APP_TASK_START_PRIO,
                    (OS_STK *)&AppTaskStartStk[0],
                    APP_TASK_START_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_TASK_START_PRIO, "Start Task", &err);
#endif

    OSStart();

    while(1);
}


void AppTaskStart (void *pdata)
{
    create_task_status();		/* ���ó�ʼ��״̬��Ҫ��������� */

    create_os_semphore();		/* ����Ӧ���еĴ�����ź��� 	*/

    create_os_mutex();			/* ����Ӧ���еĴ���������� 	*/

    create_os_mailbox();		/* ����Ӧ���еĴ����������Ϣ 	*/

    create_os_queue();

    create_os_timer();
    
    create_os_task(); 			/* ����Ӧ���еĴ�������� 		*/
	
//  #if OS_TASK_STAT_EN > 0
//		OSStatInit();
//  #endif
	
	// enable intterupt
	OpenPeriph();

	//system heartbeat led; gpio_12
	TIM_Cmd(TIM3, ENABLE);
	//TIM_Cmd(TIM4, ENABLE);
	
	_debug("start V7 POS 20180508-0940............\r\n");

    while(1)
    {
        OSTaskSuspend(OS_PRIO_SELF);	/* �����ʼ������ 		*/
    }
}
/*ϵͳ������ʼ��*/
void SystemPartInit(void)
{

}

/*ϵͳ��ʼ���У���������*/
void App_Init(void)
{
	led2_blink(3); //gpio12
}




