#ifndef _LEOMAN_H_
#define _LEOMAN_H_
#include <pthread.h>

#define SVR_NEED_TASK_RET	0x00000001
#define SVR_NEED_TASK_PERCT	0x00000010
#define SVR_NEED_DEV_STATUS	0x00000100


typedef struct _tty {
	int uart_speed;
	int time_out;
	char com_dev[16];
}TTY;

typedef struct _sock {
	char ip[32];
	int port;
	int tt_ms;
	int noneblock;
}SVR;

typedef struct _glb_cfg {
	int daemon;
	int debug_level;
	int log_syslog;
	int cp2sd_flag;
	int gen_new_flag;
	volatile int glb_sock;
	volatile int glb_tty;
	volatile int rsp_cmd_type;
	TTY sensor;
	TTY uicmd;
	SVR svr;
	pthread_mutex_t lock;// = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond;// = PTHREAD_COND_INITIALIZER;
} st_glb_cfg;

int pt_cond_wait(int tt_ms);
int pt_cond_notify();
#endif
