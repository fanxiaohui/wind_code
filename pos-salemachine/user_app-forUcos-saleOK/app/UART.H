#ifndef UART_H
#define UART_H
#include "stdarg.h"

void USART_SendString(USART_TypeDef *USARTx, u8 *String, u8 Len);
void USART_SendChar(USART_TypeDef *USARTx, u8 Data);
void vts_uart_send_string( u8 *String, u8 Len);
void pos_uart_send_string( u8 *String, u8 Len);
void vts_uart_send_char(u8 Data);
void _debug(char* fmt,...);
#endif
