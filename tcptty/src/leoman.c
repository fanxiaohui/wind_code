#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
/* for wait() */
#include <sys/wait.h>
#include <getopt.h>
#include <sys/types.h> 
#include <sys/time.h> 
#include <sys/stat.h>

#include "leom_utils.h"
#include "leom_dbg.h"
#include "leom_uart.h"
#include "leom_pro.h"
#include "leoman.h"

#define WT_SDCARD_PRIOD	(60)  //seconds
#define _VERSION_ "V1.0"

extern volatile uchar deviceid[32];
extern volatile uchar devicename[32];
extern volatile uchar devicemac[6];

#if 0
static char* const SHORT_OPTIONS = "p:l:i:o:b:s:t:d:hvS:D:";
static const struct option LONG_OPTIONS[] =
{
    {"host", 1, NULL, 'H'},
    {"port",1, NULL, 'p'},      
    {"src", 1, NULL, 'f'},    
    {"user", 1, NULL, 'u'},  
    {"item", 1, NULL, 'i'},  
    {"preference", 1, NULL, 's'},  
    {"table", 1, NULL, 't'},   
    {"dbname", 1, NULL, 'd'},   
    {"help", 0, NULL, 'h'},  
    {"version", 0, NULL, 'v'},  
    {"symbol", 1, NULL, 'S'},
    {"delimiter", 1, NULL, 'D'},
    {0, 0, 0, 0},      
};
#endif

st_glb_cfg glb_cfg;

/**@internal
 * @brief Handles SIGCHLD signals to avoid zombie processes
 *
 * When a child process exits, it causes a SIGCHLD to be sent to the
 * parent process. This handler catches it and reaps the child process so it
 * can exit. Otherwise we'd get zombie processes.
 */
static void sigchld_handler(int s)
{
	int	status;
	pid_t rc;

	debug(LOG_DEBUG, "SIGCHLD handler: Trying to reap a child");

	rc = waitpid(-1, &status, WNOHANG | WUNTRACED);

	if(rc == -1) {
		if(errno == ECHILD) {
			debug(LOG_DEBUG, "SIGCHLD handler: waitpid(): No child exists now.");
		} else {
			debug(LOG_ERR, "SIGCHLD handler: Error reaping child (waitpid() returned -1): %s", strerror(errno));
		}
		return;
	}

	if(WIFEXITED(status)) {
		debug(LOG_DEBUG, "SIGCHLD handler: Process PID %d exited normally, status %d", (int)rc, WEXITSTATUS(status));
		return;
	}

	if(WIFSIGNALED(status)) {
		debug(LOG_DEBUG, "SIGCHLD handler: Process PID %d exited due to signal %d", (int)rc, WTERMSIG(status));
		return;
	}

	debug(LOG_DEBUG, "SIGCHLD handler: Process PID %d changed state, status %d not exited, ignoring", (int)rc, status);
}

static void termination_handler(int s)
{
	debug(LOG_NOTICE, "Handler for termination caught signal %d", s);

	debug(LOG_NOTICE, "Exiting...");
	exit(s == 0 ? 1 : 0);
}

static void timer_handler(int sig)
{
	if(sig == SIGALRM) {
		debug(LOG_NOTICE, "Get signal Alarm\n");
		glb_cfg.cp2sd_flag = 1;
		alarm(WT_SDCARD_PRIOD);
	}
}

static void usrsig_handler(int sig)
{
	if(sig == SIGUSR1) {
		debug(LOG_NOTICE, "Get signal USR1\n");
		glb_cfg.gen_new_flag = 1;
	} else if(sig == SIGUSR2) {
		debug(LOG_NOTICE, "Get signal USR2\n");
	}
}

/** @internal
 * Registers all the signal handlers
 */
static void init_signals(void)
{
	struct sigaction sa;

	debug(LOG_DEBUG, "Setting SIGCHLD handler to sigchld_handler()");
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		debug(LOG_ERR, "sigaction(): %s", strerror(errno));
		exit(1);
	}

	debug(LOG_DEBUG, "Register signal ALARM\n");
	sa.sa_handler = timer_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGALRM, &sa, NULL) == -1) {
		debug(LOG_ERR, "sigaction(): %s", strerror(errno));
		exit(1);
	}

	debug(LOG_DEBUG, "Register signal USR1 and USR2\n");
	sa.sa_handler = usrsig_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		debug(LOG_ERR, "sigaction(): %s", strerror(errno));
		exit(1);
	}
	if (sigaction(SIGUSR2, &sa, NULL) == -1) {
		debug(LOG_ERR, "sigaction(): %s", strerror(errno));
		exit(1);
	}
	
	/* Trap SIGPIPE */
	debug(LOG_DEBUG, "Setting SIGPIPE  handler to SIG_IGN");
	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &sa, NULL) == -1) {
		debug(LOG_ERR, "sigaction(): %s", strerror(errno));
		exit(1);
	}

	debug(LOG_DEBUG, "Setting SIGTERM,SIGQUIT,SIGINT  handlers to termination_handler()");
	sa.sa_handler = termination_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	/* Trap SIGTERM */
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		debug(LOG_ERR, "sigaction(): %s", strerror(errno));
		exit(1);
	}

	/* Trap SIGQUIT */
	if (sigaction(SIGQUIT, &sa, NULL) == -1) {
		debug(LOG_ERR, "sigaction(): %s", strerror(errno));
		exit(1);
	}

	/* Trap SIGINT */
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		debug(LOG_ERR, "sigaction(): %s", strerror(errno));
		exit(1);
	}
}


/** @internal
 * @brief Print usage
 *
 * Prints usage, called when nodogsplash is run with -h or with an unknown option
 */
static void
usage(void)
{
	printf("Usage: leoman_bed [options]\n");
	printf("\n");
	printf("  -D            Run as Daemon\n");
	printf("  -S            Log to syslog\n");
	printf("  -l <level>    Debug level\n");
	printf("  -d <device>   Sensor Uart device\n");
	printf("  -s <speed>    Sensor Uart Speed\n");
	printf("  -t <ms>       Sensor uart timeout\n");
	printf("  -i <ip>       Server  IP Address\n");
	printf("  -p <port>     Server  TCP Port\n");
	printf("  -o <ms>       Recv TCP timeout\n");
	printf("  -b <1:0>      1:connent none block; 0: block\n");
	printf("  -h            Print usage\n");
	printf("  -v            Print version information\n");
	printf("\n");
}

void cmd_parser(int argc , char **argv , struct _glb_cfg *cfg)
{
	int c = 0;
	if(argc < 7) {
		usage();
		exit(1);
	}
	while (-1 != (c = getopt(argc, argv, "DSvhd:s:t:l:i:p:o:b:"))) {
		switch(c) {
		case 'h':
			usage();
			exit(1);
			break;

		case 'D':
			cfg->daemon = 1;
			break;
		case 'S':
			cfg->log_syslog = 1;
			break;
		// for sensor tty setting
		case 'd':
			if (optarg) {
				strncpy(cfg->sensor.com_dev,optarg,sizeof(cfg->sensor.com_dev)-1);
			}
			break;

		case 's':
			if (optarg && atoi(optarg) > 0) {
				cfg->sensor.uart_speed = atoi(optarg);
			}
			break;
			
		case 't':
			if (optarg && atoi(optarg) > 0) {
				cfg->sensor.time_out = atoi(optarg);
			}
			break;
		//for socket setting
		case 'i':
			if (optarg) {
				strncpy(cfg->svr.ip,optarg,sizeof(cfg->svr.ip)-1);
			}
			break;

		case 'p':
			if (optarg && atoi(optarg) > 0) {
				cfg->svr.port = atoi(optarg);
			}
			break;
			
		case 'o':
			if (optarg && atoi(optarg) > 0) {
				cfg->svr.tt_ms = atoi(optarg);
			}
			break;
		case 'b':
			if (optarg && atoi(optarg) > 0) {
				cfg->svr.noneblock = atoi(optarg);
			}
			break;
		//for debug 
		case 'l':
			if (optarg && atoi(optarg) > 0) {
				cfg->debug_level = atoi(optarg);
			}
			break;

		case 'v':
			printf("Leoman Version:%s\n", _VERSION_);
			exit(0);
			break;
		default:
			printf("Err Parameters.\n");
			usage();
			exit(1);
			break;
		}
	}
}

void run_as_daemon()
{
	int pid;
	int i;
	pid = fork();
	if(pid < 0)    
	    exit(1);
	else if( pid > 0)
	    exit(0);
	    
	setsid();
	pid = fork();
	if(pid > 0)
	    exit(0);
	else if( pid < 0)
	    exit(1);

	for(i = 3; i < 1024; i++)
		close(i);
	chdir("/");
	umask(0);
}

void get_abstime_wait(int microseconds, struct timespec *abstime)
{
	struct timeval tv;
	long long absmsec;
	gettimeofday(&tv, NULL);
	absmsec = tv.tv_sec * 1000ll + tv.tv_usec / 1000ll;
	absmsec += microseconds;

	abstime->tv_sec = absmsec / 1000ll;
	abstime->tv_nsec = absmsec % 1000ll * 1000000ll;
}

int pt_cond_wait(int tt_ms)
{
	int ret = 0;
	struct timespec ts;
//	struct timeval now;
//	gettimeofday(&now, NULL);
//	ts.tv_sec = now.tv_sec + 1;
//	ts.tv_nsec = (now.tv_usec + tt_ms*1000)*1000; // time has bug!
	get_abstime_wait(tt_ms,&ts);
	pthread_mutex_lock(&(glb_cfg.lock));
	ret = pthread_cond_timedwait(&(glb_cfg.cond), &(glb_cfg.lock), &ts);
	debug(LOG_DEBUG, "Pt cond ret %d,errno %d,%d\n",ret,errno,ETIMEDOUT);
	if(ret == ETIMEDOUT)
		ret = 0;
	else
		ret = 1;
	pthread_mutex_unlock (&(glb_cfg.lock));
	
	return ret;
}

int pt_cond_notify()
{
	pthread_mutex_lock(&(glb_cfg.lock));
	pthread_cond_signal(&(glb_cfg.cond));
	pthread_mutex_unlock(&(glb_cfg.lock));
	return 0;
}

void* handle_uart(void *arg)
{
	int fd = 0;

AGAIN:
	glb_cfg.glb_tty = -1;
	char *device = glb_cfg.sensor.com_dev;
	int devspeed = glb_cfg.sensor.uart_speed;
	fd = init_com_dev(device, devspeed,'N',0);
	debug(LOG_NOTICE,"Sensor thread init %s ret %d\n", device, fd);
	if(fd < 0) {
		sleep_intp_s(30);
		goto AGAIN;
	}
	glb_cfg.glb_tty = fd;

	loop_handle_sensor_data(fd, glb_cfg.sensor.time_out);
	if(fd > 0) close(fd);
	glb_cfg.glb_tty = -1;
	sleep_intp_s(3);
	goto AGAIN;
	
	return NULL;
}

int is_wifi_connected()
{
#define NET_OK	"/tmp/sta_ok"
	unlink(NET_OK);
	system("/sbin/ap_client | grep -qi ok && /bin/touch /tmp/sta_ok");
	if(access(NET_OK,F_OK) == 0)
		return 1;
	
	return 0;
}

void* handle_socket_machine(void *arg)
{
	int fd = 0;
	get_iface_mac("eth0",devicemac);
	while(1) {
		if(! is_wifi_connected()) {
			debug(LOG_NOTICE, "tcp Waiting WIFI connected...\n");
			sleep_intp_s(5);
			continue;
		}
		if(! check_valid_id_name()) {
			debug(LOG_NOTICE, "--- waitting deviceid and devicename from uart...\n");
			sleep_intp_s(5);
			continue;
		}
		fd = init_connect(glb_cfg.svr.ip, glb_cfg.svr.port, glb_cfg.svr.noneblock);
		debug(LOG_NOTICE,"Remote Socket thread init %s:%d ret %d\n",glb_cfg.svr.ip, glb_cfg.svr.port, fd);
		if(fd < 0) {
			sleep_intp_s(15);
			continue;
		}
		loop_socket_handle(fd, glb_cfg.svr.tt_ms);
		
		if(fd > 0) close(fd);
		sleep_intp_s(5);
	}
	
	return NULL;
}


void out_bgorlit()
{
	union {
		int interg;
		char a[4];
	}*p,u;
	p = &u;
	p->interg = 0x12345678;
	printf("%p: %p %p %p %p\n",u.a,&u.a[0],&u.a[1],&u.a[2],&u.a[3]);
	printf("%02x %02x %02x %02x\n",u.a[0],u.a[1],u.a[2],u.a[3]);
	if(u.a[0] == 0x12 && u.a[3] == 0x78)
		printf("BIG !!\n");
	else if(u.a[0] == 0x78 && u.a[3] == 0x12)
		printf("LIT !!\n");
}

int main(int argc, char  **argv)
{	
	out_bgorlit();
	test_hex2pro();
	test_pro2hex();
	
	cmd_parser(argc,argv,&glb_cfg);
	if(glb_cfg.daemon)
		run_as_daemon();
	
	/*
	 * Make sure only one copy of the daemon is running.
	 */
	if (already_running()) {
		debug(LOG_ERR, "daemon already running\n");
		exit(1);
	}
	
	pthread_mutex_init(&(glb_cfg.lock), NULL);
	pthread_cond_init(&(glb_cfg.cond), NULL);
//	init_log(argv[0]);
	
	init_signals();
	alarm(WT_SDCARD_PRIOD);

	pthread_t sorpid;
	pthread_create(&sorpid,NULL,handle_uart,NULL);
	
	pthread_t cmdpid;
	pthread_create(&cmdpid,NULL,handle_socket_machine,NULL);

	
	pthread_join(sorpid,NULL);
	pthread_join(cmdpid,NULL);
	
	debug(LOG_NOTICE,"<==========Process exit!\n");
//	close_log();
	return 0;
}
