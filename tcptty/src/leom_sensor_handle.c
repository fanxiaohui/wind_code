#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "leom_sensor_handle.h"
#include "leom_utils.h"
#include "leom_dbg.h"
#include "leom_uart.h"
#include "leoman.h"
#include "leom_pro.h"

#define SUCCESS	0x00
#define FAILED	0x01
#define DEVERR	0x0e

#define TRUE	1
#define FALSE 	0

extern st_glb_cfg glb_cfg;
extern volatile uchar deviceid[32];
extern volatile uchar devicename[32];

typedef struct _s_dev_stat {
	char id_valid;
	char name_valid;
	char is_start_ok;
	char is_stop_ok;
	char is_set_ok;
	char is_clear_ok;
	char is_dev_running;
	char is_dev_error;
	char is_task_ok;
	unsigned char dev_ver;
	unsigned char dev_type;
	unsigned char devid[32];
	unsigned char devname[32];
	unsigned char data[MAX_LEN];
} DEV_STAT;

DEV_STAT device;
volatile int com_rsp_flag = 0;

int self_dev_send_cmd(int fd, int cmd, int payload, int paylen);

/* return length had be written to file */
static int _write_file(char *path, char *buf, int len, char *mode)
{
	if(path == NULL || buf == NULL || len <= 0 || mode == NULL)
		return 0;

	FILE *fp = fopen(path,mode);
	if(!fp) {
		debug(LOG_ERR, "===== can NOT create file:%s\n",path);
		return 0;
	}
	int ret = fwrite(buf,1,len,fp);
	if(ret != len)
		debug(LOG_WARNING, "WARN! write file %s! actual size %d<%d\n",path,ret,len);
	fflush(fp);
	fclose(fp);

	return ret;
}

/*
 *NOTE:
 *For linkit7688 uart <--> socket
 */
unsigned char calc_crc(unsigned char *buf, unsigned int len)
{
	if(!buf || !len)
		return 0xEE;
	
	unsigned int crc = 0;
	int i = 0;
	for(i=0; i < len; i++) {
		crc += buf[i];
	}
	return crc & 0xff;
}

void dump_dev_status(DEV_STAT *dev)
{
	if(!dev)
		return;
	debug(LOG_DEBUG, "Device ID:%s",dev->devid);
	debug(LOG_DEBUG, "Device name:%s",dev->devname);
	debug(LOG_DEBUG, "Device Ver:%02x",dev->dev_ver);
	debug(LOG_DEBUG, "Device Start:%d",dev->is_start_ok);
	debug(LOG_DEBUG, "Device Stop:%d",dev->is_stop_ok);
	debug(LOG_DEBUG, "Device SET:%d",dev->is_set_ok);
	debug(LOG_DEBUG, "Device CLR:%d",dev->is_clear_ok);
	debug(LOG_DEBUG, "Device Err:%d",dev->is_dev_error);
	debug(LOG_DEBUG, "Device Run:%d",dev->is_dev_running);
	debug(LOG_DEBUG, "Device Task:%d",dev->is_task_ok);
}

void reinit_keysval(DEV_STAT *dev)
{
	if(!dev)
		return ;
	dev->is_start_ok = 0;
	dev->is_stop_ok = 0;
	dev->is_set_ok = 0;
	dev->is_clear_ok = 0;
	dev->is_dev_error = 0;
	dev->is_dev_running = 0;
	dev->is_task_ok = 0;

	memset(dev->data, 0, sizeof(dev->data));
}

#define self_read_id(fd)	self_dev_send_cmd(fd, E_RD_DEVID, NULL, 0)
#define self_read_name(fd)	self_dev_send_cmd(fd, E_RD_DEVNAME, NULL, 0)
#define self_read_ver(fd)	self_dev_send_cmd(fd, E_RD_PROVER, NULL, 0)


void loop_handle_sensor_data(int fd, int tt_ms)
{
	int socket = glb_cfg.glb_sock;
	
	static PRO *pro = NULL;
	if(!pro) {
		/* do not need to release */
		pro = (PRO*)malloc(sizeof(PRO));
		pro->payload = (uchar*)malloc(ONLINE_MAX_LEN);
	} else {
		//memset(pro, 0, sizeof(PRO)); //cause crash!
		memset(pro->payload, 0, (ONLINE_MAX_LEN));
	}
	
	int ret = 0;
	unsigned int getlen = 0, paylen = 0;
	unsigned char buffer[ONE_PKG_MAX_LEN] = {0};
	unsigned char *ptr = buffer;
	unsigned char get_cmd = 0;

	if(tt_ms <= 0)
		tt_ms = 500;

	debug(LOG_NOTICE, "--- Loop Recving Com data ...\n");
	while(1) {
		socket = glb_cfg.glb_sock;
		
		/* read Key info; then continue */
		if(0 == device.id_valid) {
			self_read_id(fd);
		} else if(0 == device.name_valid) {
			self_read_name(fd);
		} else if(0 == device.dev_ver) {
			self_read_ver(fd);
		} else {
			//OK
			;
		}
		
		/* read uart always */
		memset(buffer, 0, sizeof(buffer));
		ptr = buffer;
		getlen = 0;
		com_rsp_flag = 0;
		ret = get_one_package(fd, tt_ms, ptr, &getlen);

		switch(ret) {
			case COM_TIMEOUT:
				/* no data or not full data */
				if(getlen < MIN_LEN)
					// no cmd ,no payload,no crc
					;
				else
					// not full payload
					;
				continue;
			
			case COM_ERROR:
				debug(LOG_ERR, "==Com Error! maybe need to restart system!\n");
				return -1;
				
			case COM_OK:
				debug(LOG_DEBUG, "<--- Uart:Get One package from Com. pkglen = %u\n",getlen);
				com_rsp_flag = 1;
				pt_cond_notify();
				break;
			
			case COM_CLOSED:
				/* none block; return 0 is not closed */
				/* block; Com is closed if return 0 */
				continue;
			case COM_PROERR:
				/* Header right but len error 
				 * Maybe read Len error. Eg. only one byte.
				*/
				continue;
			default:
				debug(LOG_ERR, "=== Uart:get com Unknow ret %d!\n",ret);
				continue;
		}
		
		paylen = (ptr[2] << 8) + ptr[3];
		get_cmd = ptr[4];
		if(get_cmd % 2 == 0) {
			debug(LOG_NOTICE, "<--- Uart Get Response Cmd:0x%02x,paylen %u\n",get_cmd,paylen);
		} else {
			debug(LOG_NOTICE, "<--- Uart Get Request Cmd:0x%02x,paylen %u\n",get_cmd,paylen);
		}
		/* clear flag */
		reinit_keysval(&device);

		switch(get_cmd) {
			// uart response
			case E_RD_DEVID+1:
				if(ptr[5] == SUCCESS) {
					device.id_valid = 1;
					memcpy(device.devid, &ptr[6], 32);
					memcpy(deviceid, &ptr[6], 32);  //global deviceid in cmd.c
				} else {
					device.id_valid = 0;
				}
				break;
			case E_RD_DEVNAME+1:
				if(ptr[5] == SUCCESS) {
					device.name_valid = 1;
					memcpy(device.devname, &ptr[6], 32);
					memcpy(devicename, &ptr[6], 32);  //global deviceid in cmd.c
				} else {
					device.name_valid = 0;
				}
				break;
			case E_RD_PROVER+1:
				if(ptr[5] == SUCCESS) {
					device.dev_ver = ptr[6];
				} else {
					device.dev_ver = 0x00;
				}
				break;
			case E_DO_START+1:
				if(ptr[5] == SUCCESS) {
					device.is_start_ok = 1;
				} else {
					device.is_start_ok = 2;
				}
				break;
			case E_DO_STOP+1:
				if(ptr[5] == SUCCESS) {
					device.is_stop_ok = 1;
				} else {
					device.is_stop_ok = 2;
				}
				break;
			case E_DO_SETPARS+1:
				if(ptr[5] == SUCCESS) {
					device.is_set_ok = 1;
				} else {
					device.is_set_ok = 2;
				}
				break;
			case E_DO_DELPARS+1:
				if(ptr[5] == SUCCESS) {
					device.is_clear_ok = 1;
				} else {
					device.is_clear_ok = 2;
				}
				break;
			case E_DEV_ERROR:
				if(ptr[5] == DEVERR) {
					device.is_dev_error = 1;
				} else {
					device.is_dev_error = 2;
				}
				break;
			case E_DEV_RESULT:
				device.is_task_ok = 1;
				//need response
				ptr[4] = E_DEV_RESULT + 1;
				ptr[getlen-1] = calc_crc(&ptr[2], getlen-1);
				write(fd, ptr, getlen);
				break;
			case E_DEV_PROGRESS:
				device.is_dev_running = 1;
				break;
				
			//uart response
			case E_SYS_BAUDRATE+1:
				// 0x01c200 = 115200; 4B
				memcpy(device.data, &ptr[5], 4);
				break;
			default:
				debug(LOG_ERR, "==== Handle: Unknow uart CMD 0x%02x\n",get_cmd);
				continue;
		}
		
		if(socket <= 0) {
			debug(LOG_NOTICE, "remote socket disconnected or login failed! pls wait...\n");
			debug(LOG_DEBUG, "===Uart: Dump Device Status[NetFailed]:\n");
			dump_dev_status(&device);
			continue;
		}
		switch(get_cmd) {
			// uart cmd
			case E_DEV_ERROR:
				memcpy(device.data, &ptr[6], 32);
				send_taskerr_info(socket, pro, device.data);
				break;
				
			case E_DEV_RESULT:
				//device.is_task_ok = 1;
				memcpy(device.data, &ptr[5], 4);  //reserve
				
				//if(glb_cfg.rsp_cmd_type & SVR_NEED_TASK_RET)
					send_taskret_info(socket, pro, device.data);
				break;
				
			case E_DEV_PROGRESS:
				//device.is_dev_running = 1;
				memcpy(device.data, &ptr[5], 4); //2B countdown;2B percentge
				//if(glb_cfg.rsp_cmd_type & SVR_NEED_TASK_PERCT)
					send_taskperc_info(socket, pro, device.data);
				break;
			default:
				break;
		}
		debug(LOG_DEBUG, "===Uart: Dump Device Status[NetOk]:\n");
		dump_dev_status(&device);
	} /* end while(1) */
}

int leom_handle_sock_cmd(int cmd,unsigned char *payload, int length)
{
	int fd = 0;
	int ret = 0;
	if(glb_cfg.glb_tty > 0)
		fd = glb_cfg.glb_tty;
	else {
		debug(LOG_NOTICE, "tty dev error! pls wait...\n");
		return -1;
	}
	if(length <= 0)
		length = 0;
	
	unsigned char buf[ONE_PKG_MAX_LEN] = {0};
	unsigned char *ptr = buf;
	int len = 0;
	switch(cmd) {
		// uart response
		case E_RD_DEVID:
			len = 2;
			break;
		case E_RD_DEVNAME:
			len = 2;
			break;
		case E_RD_PROVER:
			len = 2;
			break;
		case E_DO_START:
			len = 2;
			break;
		case E_DO_STOP:
			len = 2;
			break;
		case E_DO_SETPARS:
			len = 2 + length;
			break;
		case E_DO_DELPARS:
			len = 2;
			break;
		default:
			debug(LOG_ERR,"==Com handle:Unknow cmd %02x\n",cmd);
			return 0;
	}
	*ptr++  = HEAD_H;
	*ptr++  = HEAD_L;
	*ptr++ = (len >> 8) & 0xff;
	*ptr++ = len & 0xff;
	*ptr++ = cmd;
	if(len < MIN_LEN || len > MAX_LEN) {
		debug(LOG_ERR, "===Com Handle: Package Len %d Error!\n",len);
		return 0;
	}
	if(len > MIN_LEN) {
		memcpy(ptr, payload, length);
		ptr += length;
	}
	*ptr = calc_crc(&buf[2], len+2); //include len-self

	int re_count = 0, exit_count = 0;
WT_AGAIN:
	if(exit_count >= 3) {
		debug(LOG_ERR, "===Can Not Get response from UART!\n");
		return 0;
	}
	if(com_write(fd, buf, len+4) <= 0)
		return -1;
	
	ret = len+4;
#if VOLATILE_VAR
	/* wait uart response, timeout 500ms */
	while(!com_rsp_flag && re_count++ < 5) {
		usleep(100*1000);
	}
	if(!com_rsp_flag) {
		/* send again */
		re_count = 0;
		exit_count++;
		goto WT_AGAIN;
	}
#else
	if(pt_cond_wait(500))
		return ret;
	else {
		/* send again */
		exit_count++;
		goto WT_AGAIN;
	}
#endif
	return ret;
}

int self_dev_send_cmd(int fd, int cmd, int payload, int paylen)
{
	int ret = 0;
	unsigned char buf[ONE_PKG_MAX_LEN] = {0};
	unsigned char *ptr = buf;
	int len = 0;
	switch(cmd) {
		// uart response
		case E_RD_DEVID:
			len = 2;
			break;
		case E_RD_DEVNAME:
			len = 2;
			break;
		case E_RD_PROVER:
			len = 2;
			break;
		default:
			debug(LOG_ERR,"==Com Self: Not Self cmd %02x\n",cmd);
			return 0;
	}
	*ptr++  = HEAD_H;
	*ptr++  = HEAD_L;
	*ptr++ = (len >> 8) & 0xff;
	*ptr++ = len & 0xff;
	*ptr++ = cmd;
	if(len < MIN_LEN || len > MAX_LEN) {
		debug(LOG_ERR, "===Com Self: Package Len %d Error!\n",len);
		return 0;
	}
	if(len > MIN_LEN) {
		memcpy(ptr, payload, paylen);
		ptr += paylen;
	}
	*ptr = calc_crc(&buf[2], len+2); //include len-self
	ret = len+4; //include header
	if(com_write(fd, buf, len+4) <= 0)
		return -1;

	return ret;
}

int leom_handle_sensor_data(unsigned char data[],int len)
{
	if(glb_cfg.glb_sock <= 0) {
		debug(LOG_NOTICE, "remote socket disconnected or login failed! pls wait...\n");
		return -1;
	}
	static PRO *pro = NULL;
	if(!pro) {
		/* do not need to release */
		pro = (PRO*)malloc(sizeof(PRO));
		pro->payload = (uchar*)malloc(ONLINE_MAX_LEN);
	} else {
		//memset(pro, 0, sizeof(PRO)); //cause crash!
		memset(pro->payload, 0, (ONLINE_MAX_LEN));
	}
	
	int socket = glb_cfg.glb_sock;
	
	/* for test */
	switch(data[0]) {
		case 0x00:
			// send to socket directly
			sock_send(socket, data, len);
			break;
		case 0x01:
			// up status
			send_status_info(socket, pro, 0x01, 0);
			break;
		default:
			break;
	}
	return 0;
}

