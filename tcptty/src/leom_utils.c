#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define LOCKFILE "/var/run/tcptty.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int already_running(void)
{
	int		fd;
	char	buf[16];

	fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
	if (fd < 0) {
		syslog(LOG_ERR, "can't open %s: %s", LOCKFILE, strerror(errno));
		exit(1);
	}
	if (lockfile(fd) < 0) {
		if (errno == EACCES || errno == EAGAIN) {
			close(fd);
			return(1);
		}
		syslog(LOG_ERR, "can't lock %s: %s", LOCKFILE, strerror(errno));
		exit(1);
	}
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf)+1);
	return(0);
}

int lockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return(fcntl(fd, F_SETLK, &fl));
}

void sleep_intp_s(unsigned int secs)
{
	struct timeval tval;
	tval.tv_sec = secs;
	tval.tv_usec = 0;
	select(0,NULL,NULL,NULL,&tval);
}

long get_current_time(char *timestr)
{
	time_t tt;
    time(&tt);

    if(timestr)
   	 snprintf(timestr,127,"%s",ctime(&tt));
   	
    return tt;
}

int get_iface_mac(const char ifname[], char *mac)
{
	int r, s;
	struct ifreq ifr;
	char *hwaddr;

	strcpy(ifr.ifr_name, ifname);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == s) {
		syslog(LOG_ERR, "get_iface_mac socket: %s\n", strerror(errno));
		return 1;
	}

	r = ioctl(s, SIOCGIFHWADDR, &ifr);
	if (r == -1) {
		syslog(LOG_ERR, "get_iface_mac ioctl(SIOCGIFHWADDR): %s\n", strerror(errno));
		close(s);
		return 1;
	}

	hwaddr = ifr.ifr_hwaddr.sa_data;
	close(s);
	/*
	snprintf(mac, 13, "%02X%02X%02X%02X%02X%02X",
			 hwaddr[0] & 0xFF,
			 hwaddr[1] & 0xFF,
			 hwaddr[2] & 0xFF,
			 hwaddr[3] & 0xFF,
			 hwaddr[4] & 0xFF,
			 hwaddr[5] & 0xFF
			); */
	mac[0] = hwaddr[0] & 0xFF;
	mac[1] = hwaddr[1] & 0xFF;
	mac[2] = hwaddr[2] & 0xFF;
	mac[3] = hwaddr[3] & 0xFF;
	mac[4] = hwaddr[4] & 0xFF;
	mac[5] = hwaddr[5] & 0xFF;
	return 0;
}

unsigned int calc_crc_1(unsigned char *u8Buf, unsigned int u16Len)
{
	unsigned char u8Index;
    unsigned int u16CRC = 0xFFFF;
    while(u16Len--)
	{
        u16CRC ^= *u8Buf++;
        for(u8Index = 0;u8Index < 8;u8Index++)
        {
            if(u16CRC & 0x0001)
                u16CRC = (u16CRC >> 1)^0xa001;
            else
                u16CRC = (u16CRC >> 1);
        }
    }
    return (u16CRC);
}
