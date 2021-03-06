/**
  ******************************************************************************
  * @file    Project/STM32F2xx_StdPeriph_Template/stm32f2xx_it.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_it.h"
#include "ucos_ii.h"
#include "config.h"
#include "VEMdefine.h"
#include "MFUN.h"
#include "UART.h"



//#include "stm32f2xx_hal_hcd.h"


/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
//void HardFault_Handler(void)
//{
//  /* Go to infinite loop when Hard Fault exception occurs */
//  while (1)
//  {
//  }
//}


__asm void wait()
{
    BX lr
}




void HardFaultException(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
        *((u32 *)0xE000ED0C) = 0x05fa0004;
    }

}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    wait();
    while (1)
    {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{

}

/******************************************************************************/
/*                 STM32F2xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f2xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
//void USB_LP_CAN_RX0_IRQHandler
extern u8 UseKey;
extern struct CANRxList *CANRxMsgHead;
extern struct CANRxList *CANRxMsgCurrent;

void CAN1_RX0_IRQHandler(void)
{

    OS_CPU_SR  cpu_sr;
    CanRxMsg RxMessage;

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();



    RxMessage.StdId = 0x00;
    RxMessage.ExtId = 0x00;
    RxMessage.IDE = 0;
    RxMessage.DLC = 0;
    RxMessage.FMI = 0;
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);


    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}


/*
ITStatus USART_GetITStatus(USART_TypeDef* USARTx, uint16_t USART_IT)  ： 
不仅会判断标志位是否置1，同时还会判断是否使能了相应的中断。
所以在串口中断函数中，如果要获取中断标志位，通常使用该函数。

FlagStatus USART_GetFlagStatus(USART_TypeDef* USARTx, uint16_t USART_FLAG)：
该函数只判断标志位。在没有使能相应的中断函数时，通常使用该函数来判断标志位是否置1
*/


/*CAN总线接收
D[0]:0~99 彩色LED状态 D[2]D[3],D[4]D[5],D[6]D[7] 为3组数据
D[0]：100~199 白色LED状态  D[2]，D[3],D[4]，D[5],D[6]，D[7] 为6组数据
D[1]:1 立即更新LED数据值LED板，0，不更新数据至LED板
D[0]:200 D[2]为数据 开锁
*/

void USB_LP_CAN_RX0_IRQHandler(void)
{
    //	u8 ret;


}

//vts recv timeout 100ms onechar
void TIM2_IRQHandler(void)
{
	OSIntEnter();
	
    if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET )
    {
		//debug("TIM2...\r\n");
		if(GLB_VTS_RX_STA > 0)
			GLB_VTS_RX_STA |= VTS_FIN_MASK; //VTS 接收超时(字符间隔>定时时间)，强制一帧结束
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update );  
		TIM_Cmd(TIM2, DISABLE);  //等待VTS中开启
    }
    TIM_ClearFlag(TIM2 , TIM_FLAG_Update);//必须清除中断标志位否则一直中断
	
	OSIntExit();
}

void TIM3_IRQHandler(void)
{
	static u8 flag = 0;
	OSIntEnter();
	
    if ( TIM_GetITStatus(TIM3 , TIM_IT_Update) != RESET )
    {
		//debug("TIM3...\r\n");
			flag = !flag;
			if(flag)
				GPIO_SetBits(GPIOB,GPIO_Pin_12);
			else
				GPIO_ResetBits(GPIOB,GPIO_Pin_12);

		TIM_ClearITPendingBit(TIM3, TIM_IT_Update );
		
    }
	 TIM_ClearFlag(TIM3, TIM_FLAG_Update); 
    //TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);//必须清除中断标志位否则一直中断
	
	OSIntExit();
}

void TIM4_IRQHandler(void)
{
	OSIntEnter();
	
	if ( TIM_GetITStatus(TIM4 , TIM_IT_Update) != RESET )
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update );
		//TIM_Cmd(TIM4, DISABLE);
	}
	TIM_ClearFlag(TIM4 , TIM_FLAG_Update);
	
	OSIntExit();
}

void delay_s(u8 se)
{
	u32 i, j;
    for(i = 0; i < 10000*se; i++)
    {
        for(j = 0; j < 1000; j++);
    }
}

void VTS_UART_IRQHandler(void)
{
//	u8 clear;
	u8 res;
	
	OSIntEnter();
	
	if(USART_GetFlagStatus(VTS_UART,USART_FLAG_ORE) == SET) //溢出
	{
		USART_ClearFlag(VTS_UART,USART_FLAG_ORE); //读SR
		USART_ReceiveData(VTS_UART); //读DR
		debug("----err-----VTS overrun!!!\r\n");
	}
	if(USART_GetFlagStatus(VTS_UART,USART_FLAG_PE) == SET) //溢出
	{
		USART_ClearFlag(VTS_UART,USART_FLAG_PE);
		USART_ReceiveData(VTS_UART);
		debug("----err-----VTS PE error!!!\r\n");
	}
	
	if(USART_GetITStatus(VTS_UART,USART_IT_RXNE) != RESET) //中断产生
	{
		
		USART_ClearITPendingBit(VTS_UART,USART_IT_RXNE); //清除中断标志.
		res = USART_ReceiveData(VTS_UART);
		//debug("----ok-----data 0x%x!!!\r\n",res);	//take some time will cause ORE	
		if((GLB_VTS_RX_STA & VTS_FIN_MASK) == 0 )
		{
			if(GLB_VTS_RX_STA < VTS_LEN_MAX)
			{
				TIM_SetCounter(TIM2, 0);  //重新计时
				if(GLB_VTS_RX_STA == 0) 	//收到第一个字节，使能定时器
				{
					 TIM_ClearFlag(TIM2, TIM_FLAG_Update); 
					 TIM_ClearITPendingBit(TIM2, TIM_IT_Update );
					TIM_Cmd(TIM2, ENABLE);
					
					if(res == ENQ || res == STX)
					{
						glb_vts_buffer[GLB_VTS_RX_STA++] = res;
					}
					else
					{
						// NOT start char; ingore...
					}
					
				}
				else
				{
					glb_vts_buffer[GLB_VTS_RX_STA++] = res;
				}
			}
			else 
			{
				GLB_VTS_RX_STA |= VTS_FIN_MASK; //达到最大接收数
			}
		} //buffer还未被处理完成,丢掉后续数据
	}
	
//	if(USART_GetFlagStatus(VTS_UART,USART_IT_IDLE) != RESET) //IDLE,one byte time;115200 onebyte=82.8us	
//	{
//		clear = VTS_UART->SR;
//		clear = VTS_UART->DR;
//		vts_recv_end = 1;
//		//debug("VTS IDLE, had read %d bytes\n",vts_inx);
//	}
	
	OSIntExit();
	
}

void POS_UART_IRQHandler(void)
{
	u8 res, retcode;
	u16 paylen = 0;

	OSIntEnter();
	
	if(USART_GetFlagStatus(POS_UART,USART_FLAG_ORE) == SET) //溢出
	{
		USART_ClearFlag(POS_UART,USART_FLAG_ORE); //读SR
		USART_ReceiveData(POS_UART); //读DR
		debug("---err-----POS overrun!!!\r\n");
	}
	
	if(USART_GetITStatus(POS_UART,USART_IT_RXNE) != RESET) //中断产生
	{
		USART_ClearITPendingBit(POS_UART,USART_IT_RXNE); //清除中断标志.
		res = USART_ReceiveData(POS_UART);		 
		if((GLB_POS_RX_STA & POS_FIN_MASK) == 0 )
		{
			if(GLB_POS_RX_STA < POS_LEN_MAX)
			{
				if(GLB_POS_RX_STA == 0) 	//收到第一个字节
				{

					if(res == PRO_START)
					{
						glb_pos_buffer[GLB_POS_RX_STA++] = res;
					}
					else
					{
						// NOT start char; ingore...
					}
				}
				else
				{
					glb_pos_buffer[GLB_POS_RX_STA++] = res;
					if(GLB_POS_RX_STA >= POS_MIN_RET_LEN)
					{
						//recv retcode.
						paylen = (glb_pos_buffer[5] << 8) + glb_pos_buffer[6];
						retcode = glb_pos_buffer[7];
						if(paylen >= 26 && retcode == POS_SUCCESS) {
							if(GLB_POS_RX_STA >= POS_OK_RET_LEN) {
								//decode ret OK. recv end.
								GLB_POS_RX_STA |= POS_FIN_MASK; //接受完毕
								POS_DEAL_RET_CODE = POS_SUCCESS;
							}
						}
						else if(paylen == 2 && retcode == DEAL_WAIT) 
						{
							if(GLB_POS_RX_STA >= POS_DEAL_WAIT_LEN) {
								//deal waiting. recv end.
								GLB_POS_RX_STA |= POS_FIN_MASK; //接受完毕
								POS_DEAL_RET_CODE = DEAL_WAIT;
								//debug("<-----pos deal waiting %d seconds\n",posret[8]);
								
							}
						} else if(paylen == 1 && retcode >= CRC_FAIL && retcode <= DEAL_TIMEOUT) {
							if(GLB_POS_RX_STA >= POS_NORMAL_LEN) {
								//paylen=1. recv end.
								GLB_POS_RX_STA |= POS_FIN_MASK; //接受完毕
								POS_DEAL_RET_CODE = retcode;
								//debug("<------ POS deal err,retcode=0x%x\n",retcode);
							}
						} else {
							//error paylen
							GLB_POS_RX_STA |= POS_FIN_MASK; //接受完毕
							POS_DEAL_RET_CODE = UNSUPPORT;
							//debug("<------ POS result err,paylen=0x%x,retcode=0x%x\n",paylen,retcode);
						}
					}
				}
			}
			else 
			{
				GLB_POS_RX_STA |= POS_FIN_MASK; //达到最大接收数
			}
		} //buffer还未被处理完成,丢掉后续数据
	}
	
	OSIntExit();
}

void DBG_UART_IRQHandler(void)
{
	u8 res;
	
	OSIntEnter();
	
	if(USART_GetFlagStatus(DBG_UART,USART_FLAG_ORE) == SET) //溢出
	{
		USART_ClearFlag(DBG_UART,USART_FLAG_ORE); //读SR
		USART_ReceiveData(DBG_UART); //读DR
		//debug("DBG overrun!!!\n");
	}
	
	if(USART_GetITStatus(DBG_UART,USART_IT_RXNE) != RESET) //中断产生
	{
		USART_ClearITPendingBit(DBG_UART,USART_IT_RXNE); //清除中断标志.
		res = USART_ReceiveData(DBG_UART);
		if(res == 'a')
		{
			USART_SendChar(DBG_UART,(res+1));
		}
		else if(res == 'b')
		{
			USART_SendChar(DBG_UART,(res+1));
		}
	}
	
	OSIntExit();
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
