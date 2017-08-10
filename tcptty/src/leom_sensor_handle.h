#ifndef _LEOM_SENSOR_HANDLE_H
#define _LEOM_SENSOR_HANDLE_H
#include "leom_uart.h"

void loop_handle_sensor_data(int fd, int tt_ms);
int leom_handle_sensor_data(unsigned char data[], int len);
void leom_pre_check_tail_for_report();

/* for socket handle */
enum {
	E_D_RESERVE,
	E_D_ULTRA_CLR,
	E_D_CENTRIFUAL
};

enum {
	E_RESERVE = 0x00,
	
	E_RD_DEVID = 0x02,
	E_RD_DEVNAME = 0x04,
	E_RD_PROVER	 = 0x06,
	
	E_DO_START = 0x20,
	E_DO_STOP  = 0x22,
	E_DO_SETPARS = 0x24,
	E_DO_DELPARS = 0x26,
	
	E_DEV_ERROR  = 0x80,
	E_DEV_RESULT = 0x82,
	E_DEV_PROGRESS = 0x84,
	
	E_SYS_BAUDRATE = 0xFE,
	
	E_MAX
};

/* Will use pthread_mutex and pthread_cond;
 * this functions should be call at different thread
 */
#define dev_start(pay,len)	leom_handle_sock_cmd(E_DO_START,pay,len)
#define dev_stop(pay,len)	leom_handle_sock_cmd(E_DO_STOP,pay,len)
#define dev_reset(pay,len)	{leom_handle_sock_cmd(E_DO_STOP,pay,len); \
							usleep(100*1000); \
							leom_handle_sock_cmd(E_DO_DELPARS,pay,len);}
#define dev_setpar(pay,len)	leom_handle_sock_cmd(E_DO_SETPARS,pay,len)
#define dev_clrpar(pay,len)	leom_handle_sock_cmd(E_DO_DELPARS,pay,len)

#define dev_getid(pay,len)	leom_handle_sock_cmd(E_RD_DEVID,pay,len)
#define dev_getname(pay,len)	leom_handle_sock_cmd(E_RD_DEVNAME,pay,len)
#define dev_getver(pay,len)	leom_handle_sock_cmd(E_RD_PROVER,pay,len)
/* End */

#endif
