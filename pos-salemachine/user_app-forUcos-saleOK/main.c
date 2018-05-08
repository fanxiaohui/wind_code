

#include "config.h"

void SystemPartInit(void);
static void AppTaskStart (void *pdata);

void App_Init(void);


/*操作系统相关定义*/
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


/*主函数，程序从这里开始执行*/
int main (void)
{
    u8 err;
    u32 i, j;
    for(i = 0; i < 10000; i++)
    {
        for(j = 0; j < 1000; j++);
    }

    System_configAll();/*硬件层，板级驱动*/

    OSInit();
	
    SystemPartInit();
    App_Init();
    OS_CPU_SysTickInit(); /*软件系统时钟初始化*/


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
    create_task_status();		/* 设置初始化状态需要挂起的任务 */

    create_os_semphore();		/* 创建应用中的大多数信号量 	*/

    create_os_mutex();			/* 创建应用中的大多数互斥量 	*/

    create_os_mailbox();		/* 创建应用中的大多数邮箱消息 	*/

    create_os_queue();

    create_os_timer();
    
    create_os_task(); 			/* 创建应用中的大多数任务 		*/
	
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
        OSTaskSuspend(OS_PRIO_SELF);	/* 挂起初始化任务 		*/
    }
}
/*系统参数初始化*/
void SystemPartInit(void)
{

}

/*系统开始运行，设置数据*/
void App_Init(void)
{
	led2_blink(3); //gpio12
}




