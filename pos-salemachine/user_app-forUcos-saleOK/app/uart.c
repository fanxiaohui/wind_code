/****************************Copyright (c)*********************************************
**
**               (c) Copyright 2006-2008, hui lian. luo, china, zj. hz
**                            All Rights Reserved
**
**							  ������Ӣ������Ϣ�������޹�˾
**                            http://www.embedinfo.com
**                            �����Ƽ����޹�˾
**                            http://www.bogodtech.com
**
**--------------�ļ���Ϣ--------------------------------------------------------------
** �� �� ��: uart.c
** �� �� ��: �޻���
** ��������: 2007��12��28��
** ��    ��: uart����ʵ�岿��
** ��������: ¥����(������)  �㽭��ѧ�ŵ�ϵ
**
**---------- �汾��Ϣ------------------------------------------------------------------
** ��    ��: V1.0
** ˵    ��: uart ��غ�����Ҫ��UART����
**
**-------------------------------------------------------------------------------------
**************************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "config.h"
#include "UART.H"
#include "stdarg.h"
/* Private typedef -----------------------------------------------------------*/
typedef char *uart_va_list;
/* Private define ------------------------------------------------------------*/
//#define uart_sizeof_int(n)   		((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
//#define uart_va_start(ap,v)  		(ap = (uart_va_list)&v +uart_sizeof_int(v))
//#define uart_va_arg(ap,t)    		(*(t *)((ap += uart_sizeof_int(t)) - uart_sizeof_int(t)))
//#define uart_va_end(ap)      		(ap = (uart_va_list)0)
//#define UART_SEND_BYTE(ch)			USART_SendChar(USART1, (u8*)ch)
u8 DBG_BUFFER[256];
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public  variables ---------------------------------------------------------*/

/****************************************************************************************
** ��������: UART_onfiguration
** ��������: UART1����
** ��    ��: None
** �� �� ֵ: None
** ����  ��: �޻���
** ��  ����: 2008��1��7��
**---------------------------------------------------------------------------------------
** �� �� ��:
** �ա�  ��:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void UART_onfiguration (void)
{
    //  GPIO_InitTypeDef GPIO_InitStructure;
    //  USART_InitTypeDef USART_InitStructure;
    //
    //  /* Enable USART1 clocks */
    //  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    //   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB1Periph_USART3 |RCC_APB1Periph_UART4 |RCC_APB1Periph_UART5  , ENABLE);
    //  /* Configure USART1_Tx as alternate function push-pull */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    //  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    //  GPIO_Init(GPIOA, &GPIO_InitStructure);
    //
    //  /* Configure USART2_Tx as alternate function push-pull */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    //  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    //  GPIO_Init(GPIOA, &GPIO_InitStructure);
    //
    //   /* Configure USART3_Tx as alternate function push-pull */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    //  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    //  GPIO_Init(GPIOB, &GPIO_InitStructure);
    //
    //     /* Configure USART4_Tx as alternate function push-pull */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    //  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    //  GPIO_Init(GPIOC, &GPIO_InitStructure);
    //
    //      /* Configure USART5_Tx as alternate function push-pull */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    //  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    //  GPIO_Init(GPIOC, &GPIO_InitStructure);
    //
    //  /* Configure USART1_Rx as input floating */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    //  GPIO_Init(GPIOA, &GPIO_InitStructure);
    //
    //  /* Configure USART2_Rx as input floating */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    //  GPIO_Init(GPIOA, &GPIO_InitStructure);
    //
    //  /* Configure USART3_Rx as input floating */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    //  GPIO_Init(GPIOB, &GPIO_InitStructure);
    //
    //  /* Configure USART4_Rx as input floating */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    //  GPIO_Init(GPIOC, &GPIO_InitStructure);
    //
    //   /* Configure USART5_Rx as input floating */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    //  GPIO_Init(GPIOD, &GPIO_InitStructure);
    //
    //  /* USARTx configuration ------------------------------------------------------*/
    //  USART_InitStructure.USART_BaudRate = 9600;
    //  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    //  USART_InitStructure.USART_StopBits = USART_StopBits_1;
    //  USART_InitStructure.USART_Parity = USART_Parity_No ;
    //  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    //  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    //
    //
    //  /* Configure the USARTx */
    //
    //	USART_Init(UART4, &USART_InitStructure);
    //	USART_Init(UART5, &USART_InitStructure);
    //
    //
    //	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    //	USART_Init(USART3, &USART_InitStructure);
    //
    //		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    //	USART_InitStructure.USART_BaudRate =19200;
    //	USART_Init(USART2, &USART_InitStructure);
    //
    //	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    //	USART_InitStructure.USART_BaudRate = 57600;
    //	USART_Init(USART1, &USART_InitStructure);
    //  /* Enable the USART Receive interrupt: this interrupt is generated when the
    //   USART1 receive data register is not empty */
    //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    //USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    //  /* Enable the USARTx */
    //USART_Cmd(USART1, ENABLE);
    //USART_Cmd(USART2, ENABLE);
    //USART_Cmd(USART3, ENABLE);
    //USART_Cmd(UART4, ENABLE);
    //USART_Cmd(UART5, ENABLE);
}

/****************************************************************************************
** ��������: USART_SendChar
** ��������: UART1 �����ַ�
** ��    ��: USARTx: ���ں�,  Data: ����������
** �� �� ֵ: None
** ����  ��: �޻���
** ��  ����: 2008��1��7��
**---------------------------------------------------------------------------------------
** �� �� ��:
** �ա�  ��:
**--------------------------------------------------------------------------------------
****************************************************************************************/
void USART_SendChar(USART_TypeDef *USARTx, u8 Data)
{
    /* Transmit Data */
    USARTx->DR = (Data & (u16)0x01FF);

    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

void USART_SendString(USART_TypeDef *USARTx, u8 *String, u8 Len)
{
    u8 i;
    u8 *Data = String;
    for(i = 0; i < Len; i++)
    {
        USART_SendChar(USARTx, *Data);
        Data++;
    }
}

void vts_uart_send_char(u8 Data)
{
	/* Transmit Data */
    USART_SendChar(VTS_UART, Data);
}

void vts_uart_send_string( u8 *String, u8 Len)
{
	u8 i;
    u8 *Data = String;
    for(i = 0; i < Len; i++)
    {
        USART_SendChar(VTS_UART, *Data);
        Data++;
    }
}

void pos_uart_send_string( u8 *String, u8 Len)
{
	u8 i;
    u8 *Data = String;
    for(i = 0; i < Len; i++)
    {
        USART_SendChar(POS_UART, *Data);
        Data++;
    }
}

void debug_char(u8 ch)
{
	while(USART_GetFlagStatus(DBG_UART,USART_FLAG_TC)==RESET);
		USART_SendData(DBG_UART, ch);
}

void _debug(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)DBG_BUFFER,fmt,ap);
	va_end(ap);
	i=strlen((const char*)DBG_BUFFER);	
	for(j=0;j<i;j++)							
	{
	  while(USART_GetFlagStatus(DBG_UART,USART_FLAG_TC)==RESET);
		USART_SendData(DBG_UART,DBG_BUFFER[j]);
		DBG_BUFFER[j] = '\0';
	} 
}

/****************************** http://www.bogodtech.com *******END OF FILE******************/
