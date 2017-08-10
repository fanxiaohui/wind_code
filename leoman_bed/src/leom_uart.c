#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <linux/fs.h>
#include <errno.h>
#include <termio.h>
#include <fcntl.h>
#include <assert.h>

#include "leom_uart.h"
#include "leom_dbg.h"
#include "leom_sensor_handle.h"

static int setport(int fd, int baud, int databits, int stopbits, char parity)
{
	
	int    baudrate;
	struct termios newtio;   
	
	switch (baud)
	{
		case 300:
			baudrate = B300;
			break;
		case 600:
			baudrate = B600;
			break;
		case 1200:
			baudrate = B1200;
			break;
		case 2400:
			baudrate = B2400;
			break;
		case 4800:
			baudrate = B4800;
			break;
		case 9600:
			baudrate = B9600;
			break;
		case 19200:
			baudrate = B19200;
			break;
		case 38400:
			baudrate = B38400;
			break;
		case 57600:
			baudrate = B57600;
			break;
		case 115200:
			baudrate = B115200;
			break;
		default :
			baudrate = B115200;  
			break;
	}
	
	tcgetattr(fd, &newtio);     
	bzero(&newtio, sizeof(newtio));    
	
	//must be sed firstly!
	newtio.c_cflag |= (CLOCAL | CREAD);
	newtio.c_cflag &= ~CSIZE; 
	
	switch (databits)
	{   
		case 7:  
			newtio.c_cflag |= CS7; 
			break;
		case 8:     
			newtio.c_cflag |= CS8; 
			break;   
		default:    
			newtio.c_cflag |= CS8;
			break;    
	}
	
	switch (parity) 
	{   
		case 'n':
		case 'N':    
			newtio.c_cflag &= ~PARENB;   
			newtio.c_iflag &= ~INPCK;    
			break;  
		case 'o':   
		case 'O':     
			newtio.c_cflag |= (PARODD | PARENB); 
			newtio.c_iflag |= INPCK;            
			break;  
		case 'e':  
		case 'E':   
			newtio.c_cflag |= PARENB;     
			newtio.c_cflag &= ~PARODD;      
			newtio.c_iflag |= INPCK;       
			break;
		case 'S': 
		case 's':  
			newtio.c_cflag &= ~PARENB;
			newtio.c_cflag &= ~CSTOPB;
			break;  
		default:   
			newtio.c_cflag &= ~PARENB;   
			newtio.c_iflag &= ~INPCK;     
			break;   
	} 
	
	switch (stopbits)
	{   
		case 1:    
			newtio.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			newtio.c_cflag |= CSTOPB;  
			break;
		default:  
			newtio.c_cflag &= ~CSTOPB;  
			break;  
	} 

	newtio.c_cc[VTIME] = 0;    
	newtio.c_cc[VMIN] = 1; 
#if 1
	newtio.c_oflag &= ~OPOST;
    newtio.c_oflag &= ~(ONLCR | OCRNL);
    newtio.c_iflag &= ~(ICRNL | INLCR);
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
#endif
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw mode
	cfsetispeed(&newtio, baudrate);   
	cfsetospeed(&newtio, baudrate);   
	tcflush(fd, TCIFLUSH); 
	
	if (tcsetattr(fd,TCSANOW, &newtio) != 0)   
	{ 
		debug(LOG_ERR,"tcsetattr-error ,%d-%s\n",errno,strerror(errno));
		return -1;  
	}  
	
	return 0;
}

/*
 * open Com Dev
 * default 8 databits  1 stopbit
 * return 0 ok,otherwise failed
 */
int init_com_dev(const char *com_dev, int speed, char parity, int none_block)
{
	int fd = -1;
	if(NULL == com_dev) {
		debug(LOG_ERR,"com dev name is NULL！\n");
		return -1;
	}
	//	const char *com_dev = "/dev/ttySAC2";
	if(none_block)
		fd = open(com_dev, O_RDWR|O_EXCL|O_NOCTTY|O_NONBLOCK);
	else
		fd = open(com_dev, O_RDWR|O_EXCL|O_NOCTTY);
	if (fd < 0) {
		debug(LOG_ERR,"Open Comdev '%s' Error! %d-%s\n",com_dev,errno,strerror(errno));
		return -1;
	}
	
	if(setport(fd,speed,8,1,parity) != 0)
		return -1;
	return fd;
}

#if 0
int tt_read(int fd, void *buf, int len, unsigned int timeout)
{
    int ret;
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = timeout/1000;
    tv.tv_usec = (timeout%1000)*1000;
    FD_ZERO(&readfds);
    FD_SET(fd,&readfds);
    ret = select(fd+1,&readfds,NULL,NULL,&tv);
	switch(ret) {
		case -1:
			printf("Select Error,%d:%s!\n",errno,strerror(errno));
			if(errno == EINTR) {
				printf("###### select interrupt by signal #####\n");
				return INTP;
			} else 
				return ERROR;
			break;
		case 0:
			printf("tt_read select timeout!\n");
			return TIMEOUT;
			break;
		default:
			return read(fd,buf,len);
			break;
	}
}
#else
int tt_read(int fd, void *buf, int len, unsigned int timeout)
{
    int ret;
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = timeout/1000;
    tv.tv_usec = (timeout%1000)*1000;
    FD_ZERO(&readfds);
    FD_SET(fd,&readfds);
    ret = select(fd+1,&readfds,NULL,NULL,&tv);
    if(ret <= 0) {
        if(ret == 0) {
            errno = ETIME; /* time out */
         //   printf("tt_read timeout!\n");
        }
        if(errno == EINTR) {
            printf("###### select interrupt by signal #####\n");
        }
        return -1;
    }
    return read(fd,buf,len);
}
#endif

comret loop_get_sensor_data(int fd, int timeout)
{
	unsigned char ch = 0;
	int ret = 0, ch_num = 0, index = 0, stflag = 0;
	unsigned char sen_out[MAX_NUM][MAX_INX+1];
	
    fd_set readfds;
    struct timeval tv;
    if(timeout <= 0)
    	timeout = 1000;
    
	memset(sen_out,0,sizeof(sen_out));
	
	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000)*1000;
		ret = select(fd+1,&readfds,NULL,NULL,&tv);
		if(ret == 0) {
			leom_pre_check_tail_for_report();
			continue;
		} else if(ret < 0) {
			if(errno == EINTR) {
				debug(LOG_WARNING, "###### sensor get_char interrupt by signal #####\n");
				continue;
			} else
				debug(LOG_ERR, "sensor get_char error,%d:%s\n",errno,strerror(errno));

 			return COM_ERROR;  // error or closed
		}
		//run here! must be OK!
		ret = read(fd,&ch,1);
		//debug(LOG_DEBUG,"getchar %d,0x%x\n",ch,ch);
		if(ch == MAGIC) {
			stflag = 1;
			ch_num = 0;
			
			sen_out[index][ch_num] = ch;
			ch_num++;
			continue;
		}
		if(stflag) {
			if(ch_num < MAX_INX) {
				sen_out[index][ch_num] = ch;
				ch_num++;
			}
			if(ch_num == MAX_INX) {
				sen_out[index][MAX_INX] = 0;
				stflag = 0;
				if(index++ >= MAX_NUM - 1) {
					index = 0;
					debug(LOG_DEBUG, "goto handle sensor data\n");
					leom_handle_sensor_data(sen_out);
					memset(sen_out,0,sizeof(sen_out));
				}
			}
		}
	}
	return COM_OK;
}


comret loop_get_ctrl_cmd(int fd, int timeout)
{
	unsigned char ch = 0;
	int ret = 0, ch_num = 0, stflag = 0;
	unsigned char uicmd[MAX_UI_CMD_LEN] = {0};
	
    fd_set readfds;
    struct timeval tv;
    if(timeout <= 0)
    	timeout = 8;
	
	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000)*1000;
		ret = select(fd+1,&readfds,NULL,NULL,&tv);
		if(ret == 0) {
			//one package ending!
			// restart
			stflag = ch_num = 0;
			continue;
		} else if(ret < 0) {
			if(errno == EINTR) {
				debug(LOG_WARNING, "######cmd get_char interrupt by signal #####\n");
				continue;
			} else
				debug(LOG_ERR, "cmd get_char error,%d:%s\n",errno,strerror(errno));

 			return COM_ERROR;  // error or closed
		}
		//run here! must be OK ?
		ret = read(fd,&ch,1);
		if(ret != 1) {
			debug(LOG_ERR, "cmd get_char error,%d:%s\n",errno,strerror(errno));
			return COM_ERROR;
		}
		
		if(ch == MAGIC_CMD && stflag != 1) {
			stflag = 1;
			ch_num = 0;
			memset(uicmd,0,sizeof(uicmd));
			uicmd[ch_num] = ch;
			ch_num++;
			continue;
		}
		if(stflag) {
			if(ch_num < MAX_UI_CMD_LEN) {
				uicmd[ch_num] = ch;
				ch_num++;
				if(ch_num == uicmd[1]) {
					//get all data, to end
					leom_handle_ui_cmd(uicmd,ch_num);
					// restart
					stflag = ch_num = 0;
					memset(uicmd,0,sizeof(uicmd));
				}
			} else {
				debug(LOG_ERR, "######cmd length is too large, > 128!#####\n");
				//restart
				stflag = ch_num = 0;
				memset(uicmd,0,sizeof(uicmd));
			}
		}
	}
	return COM_OK;
}


/*
* read com bytes
* read len: nbytes
* timeout : ms
*/
int tt_read_nbys(int fd, void *buf, int nbytes, unsigned int timeout)
{
    int nleft;
    int nread;

    nleft = nbytes;
    while(nleft > 0) {
        if((nread = tt_read(fd,buf,nleft,timeout)) < 0) {
            if(nleft == nbytes)
                return -1; /* error, return -1 */
            else
                break; /* error, return amount read so far */
        } else if(nread == 0) {
            printf("Read EOF!\n");
            break;
        }
        //qDebug("tt read ret=%d,%s\n",nread,buf);
        nleft -= nread;
        buf += nread;
    }
    return (nbytes - nleft); /* return bytes had read , >= 0 */
}

int wait_response(int fd,unsigned char *resp_buf, int wait_length, int timeout)
{
    int ret = 0;
    ret = tt_read_nbys(fd,resp_buf,wait_length,timeout);
    if(ret != wait_length) {
        printf("tt_read_nbys error! %d\n",ret);
        return -1;
    } else  {
        return 0;
    }
}

void clear_io_buff(int fd)
{
    /* clear read & write buffer */
    tcflush(fd,TCIOFLUSH);
}
