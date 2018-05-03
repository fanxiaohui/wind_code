/*
***************************************************************************************
*
*               (c) Copyright 2006-2008, hui lian. luo, china, zj. hz
*                            All Rights Reserved
*
*							 深圳市英蓓特信息技术有限公司
*                            http://www.embedinfo.com
*                            博格达科技有限公司
*                            http://www.bogodtech.com
*
*--------------文件信息-----------------------------------------------------------------
* 文 件 名: core_task.c
* 创 建 人: 罗辉联(wyuyun@hotmail.com, lhlzjut@hotmail.com)
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


#include "config.h"

/* Private define --------------------------------------------------------------------*/
/* Private  variables ----------------------------------------------------------------*/
/* Public  variables ----------------------------------------------------------------*/
/* 此表为0--F的字模 */

u8 const DisTab[16] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,
                        0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
                      };

extern u8 RxBuffer[RxBufferSize];

/****************************************************************************************
** 函数名称: snr_task_core
** 功能描述: 脉冲传感器信号任务处理
** 参    数: *pdata
** 返 回 值: None
** 作　  者: 罗辉联
** 日  　期: 2008年1月7日
**---------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void snr_task_core(void *pdata)
{
    INT8U err = 0;
    INT8U cnt = 0;

    pdata = pdata;

    while(1)
    {
        OSTimeDlyHMSM(0, 0, 0, 400);

        SPI_SendByte(DisTab[err]);
        err = err + 1;

        if(err == 16)
        {
            err = 0;

        }

        Diode_Show(cnt);

        cnt = cnt + 1;
        if(cnt  == 9)
        {
            cnt = 0;
            //	Buzzer_On();
        }
    }
}


/****************************************************************************************
** 函数名称: com_task_core
** 功能描述: 串口接收字符任务处理入口
** 参    数: *pdata
** 返 回 值: None
** 作　  者: 罗辉联
** 日  　期: 2008年1月7日
**---------------------------------------------------------------------------------------
** 修 改 人:
** 日　  期:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void com_task_core(void *pdata)
{
    INT8U err = 0;
    char a = 0;
    pdata = pdata;

    while(1)
    {
        OSSemPend(ComSem, 0, &err);

        /* UART收到字符处理实体 */

        if(RxBuffer[0] == 'a')
        {
            printf("You already send 'a' to uart1 form pc!\n");
        }
        else if(RxBuffer[0] == 'b')
        {
            printf("You already send 'b' to uart1 form pc!\n");
        }
        else if(RxBuffer[0] == 'v')
        {
            printf("STM3201 SDK ver1.0 by armgcc@foxmail! \n");
        }
        if(a)
        {
            a = ~a;
            GPIO_SetBits(GPIOF, GPIO_Pin_7);
        }
        else
        {
            a = ~a;
            GPIO_ResetBits(GPIOF, GPIO_Pin_7);
        }


        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    }

}
void TEST_task_core(void *pdata)

{
    char a = 0, b = 0;
    pdata = pdata;

    while(1)

    {

        OSTimeDlyHMSM(0, 0, 0, 500);
        //	USART_SendData(USART1, b++);
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
        if(a)
        {
            a = ~a;
            GPIO_SetBits(GPIOF, GPIO_Pin_6);
        }
        else
        {
            a = ~a;
            GPIO_ResetBits(GPIOF, GPIO_Pin_6);
        }
    }

}

/************************************** end of file *************************************/



