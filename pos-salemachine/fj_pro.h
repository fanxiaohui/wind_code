#ifndef _FJ_PRO_H_
#define _FJ_PRO_H_

#define STX		0x02
#define ETX		0x03
#define ENQ		0x05
#define ACK		0x06
#define NAK		0x15

#define FJ_MIN_CHAR_INTERVAL	100 //ms
#define FJ_NORMAL_CMD_LEN		5
#define FJ_CW_CMD_LEN			10

enum _fj_code {
	PD = 0,
	PW,
	CR,
	cr,
	CP,
	UR,
	CW,
	CE,
	LR,
	NS,
	SL,
	END
};

int fj_recv_loop(int fd, int timeout);

#endif
