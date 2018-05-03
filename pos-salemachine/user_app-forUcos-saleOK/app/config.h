/*

*
*--------------------------------------------------------------------------------------
****************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CONFIG_H
#define __CONFIG_H

#define TESTMODE 0
/* Includes ------------------------------------------------------------------*/
#define DATACLEAR  TRUE
#define SOFTVER   611
#define OSArr      4
#define CountKey 100 /*连续按键*/

#define DEFAULESYSMODE 2/*默认的模式,2 58门机器*/



#define HFJCANID 0XE0

#include "stm32f2xx.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "stm32f2xx_conf.h"
#include "ucos_ii.h"
#include "mfun.h"
#include "create_task.h"
#include "malloc.h"
#include <string.h>
#include "uart.h"
#include "STM32_FLASHRW.H"
#include "spi_flash.h"
#include "system_config.h"

#define LED1FLASH  GPIO_WriteBit(GPIOB,GPIO_Pin_12,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_12)))
#define LED2FLASH  GPIO_WriteBit(GPIOB,GPIO_Pin_13,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_13)))
#define LED3FLASH  GPIO_WriteBit(GPIOB,GPIO_Pin_14,(BitAction)(1-GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_14)))

#define SETLED1(x)  GPIO_WriteBit(GPIOB,GPIO_Pin_12,(BitAction)x)
#define SETLED2(x)  GPIO_WriteBit(GPIOB,GPIO_Pin_13,(BitAction)x)
#define SETLED3(x)  GPIO_WriteBit(GPIOB,GPIO_Pin_14,(BitAction)x)

#define LED4OFF()	SETLED2(1)
#define LED4ON()	SETLED2(0)

#define LED3OFF()	SETLED3(1)
#define LED3ON()	SETLED3(0)


#define MAXLEDBARLEN  40

#define LEDWHITE   0X08
#define LEDBLUE   0X04
#define LEDGREEN  0X01
#define LEDRED    0X02
#define LEDCLEAR   0XF8/*彩色关闭*/
#define LEDSET     0X07/*颜色全部打开*/
#define LEDSD   	0	 /*全部关闭*/

#define TRUE 1	 /*真定义*/
#define FALSE 0	 /*假定义*/

#define SUCCESS 3  /*成功*/
#define FAIL    2  /*失败*/


#define CARD_UART USART6
#define LED_UART UART5
#define LCD_UART UART4
#define EVM_UART USART3
#define MDB_UART USART2
#define PC_UART  USART1


#define CARD_UART_IRQHandler USART6_IRQHandler
#define LED_UART_IRQHandler UART5_IRQHandler
#define LCD_UART_IRQHandler UART4_IRQHandler
#define EVM_UART_IRQHandler USART3_IRQHandler
#define MDB_UART_IRQHandler USART2_IRQHandler
#define PC_UART_IRQHandler  USART1_IRQHandler

#define CARD_UART_TESTDATA 0x51
#define LED_UART_TESTDATA 0X52
#define LCD_UART_TESTDATA 0x53
#define EVM_UART_TESTDATA 0x54
#define MDB_UART_TESTDATA 0x55
#define PC_UART_TESTDATA  0x56


//add by wind
#define DBG_UART		USART1
#define VTS_UART		UART4
#define POS_UART		USART6

#define DBG_UART_IRQHandler USART1_IRQHandler
#define VTS_UART_IRQHandler UART4_IRQHandler
#define POS_UART_IRQHandler USART6_IRQHandler

#define DEBUG2TX1		1
//#undef DEBUG2TX1

#ifdef DEBUG2TX1
#define debug(xxx,...)	_debug(xxx, ## __VA_ARGS__)
#else
#define debug(xxx,...) {}
#endif

#endif /* __MAIN_H */

/*********************** http://www.bogodtech.com *******End of file **********************/
