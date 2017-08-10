#ifndef _LEOM_UART_H_
#define _LEOM_UART_H_

typedef enum _errinfo
{
	COM_TIMEOUT = -2,
	COM_ERROR = -1,
	COM_OK = 0,
	COM_CLOSED = 1,
	COM_PROERR = 2
} comret;


#define ERR_ABORT	assert

#define HEAD_H		0xaa
#define HEAD_L		0x55
#define MAX_LEN		256
#define MIN_LEN		2
#define HEAD_SIZE	2
#define LEN_SIZE	HEAD_SIZE
#define ONE_PKG_MAX_LEN		(MAX_LEN+6)

#define IN
#define OUT

int init_com_dev(const char *com_dev, int speed, char parity, int none_block);
int com_write(int fd, unsigned char *buf, int len);
comret get_one_package(int fd, int timeout, unsigned char *buffer, unsigned int *getlen);
comret loop_get_sensor_data_1(int fd, int timeout);
comret loop_get_ctrl_cmd(int fd, int timeout);
#endif
