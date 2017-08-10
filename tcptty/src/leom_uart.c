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
		fd = open(com_dev, O_RDWR|O_NOCTTY|O_NONBLOCK);
	else
		fd = open(com_dev, O_RDWR|O_NOCTTY);
	if (fd < 0) {
		debug(LOG_ERR,"Open Comdev '%s' Error! %d-%s\n",com_dev,errno,strerror(errno));
		return -1;
	}
	
	if(setport(fd,speed,8,1,parity) != 0)
		return -1;
	return fd;
}

void clear_io_buff(int fd)
{
    /* clear read & write buffer */
    tcflush(fd,TCIOFLUSH);
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
int tt_read(int fd, unsigned char *buf, int len, unsigned int timeout)
{
    int ret;
    fd_set readfds;
    struct timeval tv;
	unsigned char *ptr = buf;
	int leftsize = len;
	if(fd <= 0 || !buf || len <= 0)
		return -1;
	
	while(1) {
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000)*1000;
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		ret = select(fd+1,&readfds,NULL,NULL,&tv);
		if(ret == 0) {
			if(leftsize == 0)
				return len;
			else
				return 0;
		} else if(ret < 0) {
			if(errno == EINTR || errno == EAGAIN)
				continue;
			else {
				debug(LOG_ERR, "==== com select error!%d\n",errno);
				return -1;
			}
				
		} else {
			if(leftsize) {
				if((ret=read(fd, ptr , leftsize)) <= 0) {
					debug(LOG_ERR, "==== com read error!%d\n",errno);
					return -1;
				}
				ptr += ret;
				leftsize -= ret;
				if(leftsize == 0)
					return len;
				
				continue;
			}
			// can not to run here.
		}
	}
}
#endif

#define READ_CHAR(fd,ptr,tt)	if((ret=tt_read(fd,ptr,1,tt)) < 0) {return COM_ERROR;} else if(ret!=1) break;
#define READ_PRO_HEAD(fd,ptr,tt)	if((ret=tt_read(fd,ptr,HEAD_SIZE,tt)) < 0) {return COM_ERROR;} else if(ret!=HEAD_SIZE) break;
#define READ_PRO_LEN(fd,ptr,tt)		READ_PRO_HEAD(fd,ptr,tt)
#define READ_PAY_LEN(fd,ptr,len,tt)		if((ret=tt_read(fd,ptr,len,tt)) < 0) {return COM_ERROR;} else if(ret!=len) break;

comret get_one_package(int fd, int timeout, unsigned char *buffer, unsigned int *getlen)
{
	int ret = 0;
	unsigned int paylen = 0;
	unsigned char ch;
	unsigned char *ptr = buffer;
	if(timeout <= 0)
    	timeout = 500;

	while(1) {
		READ_CHAR(fd, ptr, timeout);
		if(ptr[0] == HEAD_H) {
			ptr++;
		FIND_L:
			READ_CHAR(fd, ptr, timeout);
			if(ptr[0] == HEAD_L)
				ptr++;
			else if (ptr[0] == HEAD_H)
				goto FIND_L;
			else break;
		} else {
			break;
		}
		/*
		READ_PRO_HEAD(fd, ptr, timeout);
		if(ptr[0] != HEAD_H || ptr[1] != HEAD_L) {
			debug(LOG_DEBUG, "=== Not header, continue to find...\n");
			//ptr = buffer;
			//memset(ptr, 0, HEAD_SIZE);
			break;
		}
		ptr += HEAD_SIZE;
		*/
		READ_PRO_LEN(fd, ptr, timeout);
		paylen = (ptr[0] << 8) + ptr[1];
		debug(LOG_DEBUG, "===Need Com %d len %02x-%02x,int %d\n",ret,ptr[0],ptr[1], paylen);
		if(paylen > MAX_LEN || paylen < MIN_LEN) {
			debug(LOG_ERR, "###ProData Err:Com data len 0x%02x error! To clear buffer.\n",paylen);
			clear_io_buff(fd);
			//memset(sen_out, 0, sizeof(sen_out));
			return COM_PROERR;
		}
		ptr += LEN_SIZE;
		
		READ_PAY_LEN(fd, ptr, paylen, timeout);
		if(getlen)
			*getlen = paylen+4;
		
		return COM_OK;
	}
	return COM_TIMEOUT;
}

comret loop_get_sensor_data_1(int fd, int timeout)
{
	int ret = 0;
	unsigned char sen_out[1024];
	
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
			//leom_pre_check_tail_for_report();
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
		ret = read(fd,sen_out,sizeof(sen_out));
		//debug(LOG_DEBUG, "goto handle sensor data\n");
		if(ret > 0)
			leom_handle_sensor_data(sen_out,ret);
		else
			return COM_ERROR;
		memset(sen_out,0,sizeof(sen_out));
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

int com_write(int fd, unsigned char *buf, int len)
{
	if(fd <= 0 || !buf || len <=0 )
		return -1;
	int ret = 0;
	ret = write(fd, buf, len);
	if(ret != len) {
		debug(LOG_ERR, "== Com Write error %d\n",errno);
		return -1;
	}
	return ret;
}
