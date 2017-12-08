#ifndef _UART_H_
#define _UART_H_

int init_com_dev(const char *com_dev, int speed, char parity, int none_block);
void clear_io_buff(int fd);


#endif
