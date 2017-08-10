#ifndef _LEOMAN_H_
#define _LEOMAN_H_

typedef struct _tty {
	int uart_speed;
	int time_out;
	char com_dev[16];
}TTY;

typedef volatile struct _glb_cfg {
	int daemon;
	int debug_level;
	int log_syslog;
	int cp2sd_flag;
	int gen_new_flag;
	int start_sensor_flag;
	TTY sensor;
	TTY uicmd;
} st_glb_cfg;

#endif
