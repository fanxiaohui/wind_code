#ifndef _LEOM_UART_H_
#define _LEOM_UART_H_

#define MAX_NUM	60
#define MAX_UI_CMD_LEN	128
#define MIN_UI_CMD_LEN  5
typedef enum _errinfo
{
	COM_INTP = -3,
	COM_TIMEOUT = -2,
	COM_ERROR = -1,
	COM_OK = 0,
	COM_CLOSED = 1
} comret;

typedef enum _sen_inx
{
	HEADER = 0,
	HEART_I,
	HEART_F,
	BREATH_I,
	BREATH_F,
	MOVING,
	ON_BED,
	GATEO_H,
	GATEO_L,
	GATEM,
	MAX_INX
} SEN_INX;

typedef struct _obj {
	int heart_f;
	int breath_f;
	int is_moving;
	int is_on_bed;
	int gateo;
	int gatem;
} st_obj;

#define ERR_ABORT	assert

#define MAGIC		0xff
#define MAGIC_CMD	0x55

#define TAIL			"\r\n"
#define DEF_FCC		'?'
#define FCC_CALC_LEN		9
#define COM_DATA_LEN		9

#define IN
#define OUT

int init_com_dev(const char *com_dev, int speed, char parity, int none_block);
comret loop_get_sensor_data(int fd, int timeout);
comret loop_get_ctrl_cmd(int fd, int timeout);
#endif
