#ifndef _LEOM_SENSOR_HANDLE_H
#define _LEOM_SENSOR_HANDLE_H
#include "leom_uart.h"

int leom_handle_sensor_data(unsigned char data[][MAX_INX+1]);
void leom_pre_check_tail_for_report();
int leom_handle_ui_cmd(unsigned char *cmdbuf, int length);
#endif
