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
#include <sys/stat.h>

#include "leom_utils.h"
#include "leom_dbg.h"
#include "leom_uart.h"
#include "leoman.h"

#define WT_SDCARD_PRIOD	(60)  //seconds
#define _VERSION_ "V2.0"

static char* const SHORT_OPTIONS = "h:p:f:u:i:s:t:d:hvS:D:";
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
	printf("  -u <device>   Cmd  Uart device\n");
	printf("  -p <speed>    Cmd  Uart Speed\n");
	printf("  -o <ms>       Cmd uart timeout\n");
	printf("  -h            Print usage\n");
	printf("  -v            Print version information\n");
	printf("\n");
}

void cmd_parser(int argc , char **argv , struct _glb_cfg *cfg)
{
	int c = 0;
	while (-1 != (c = getopt(argc, argv, "DSvhd:s:t:l:u:p:o:"))) {
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
		//for ui cmd tty setting
		case 'u':
			if (optarg) {
				strncpy(cfg->uicmd.com_dev,optarg,sizeof(cfg->uicmd.com_dev)-1);
			}
			break;

		case 'p':
			if (optarg && atoi(optarg) > 0) {
				cfg->uicmd.uart_speed = atoi(optarg);
			}
			break;
			
		case 'o':
			if (optarg && atoi(optarg) > 0) {
				cfg->uicmd.time_out = atoi(optarg);
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
			exit(0);
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

void* handle_sensor(void *arg)
{
	int fd = 0;
AGAIN:
	fd = init_com_dev(glb_cfg.sensor.com_dev,glb_cfg.sensor.uart_speed,'N',0);
	debug(LOG_NOTICE,"Sensor thread init %s ret %d\n",glb_cfg.sensor.com_dev,fd);
	if(fd < 0) {
		sleep(30);
		goto AGAIN;
	}
	loop_get_sensor_data(fd,glb_cfg.sensor.time_out);
	
	if(fd > 0) close(fd);
	sleep(5);
	goto AGAIN;
	
	return NULL;
}

void* handle_uicmd(void *arg)
{
	int fd = 0;
AGAIN:
	fd = init_com_dev(glb_cfg.uicmd.com_dev,glb_cfg.uicmd.uart_speed,'N',0);
	//fd = init_com_dev("/dev/ttyS2",glb_cfg.uart_speed,'N',0);
	debug(LOG_NOTICE,"Cmd thread init %s ret %d\n",glb_cfg.uicmd.com_dev,fd);
	if(fd < 0) {
		sleep(30);
		goto AGAIN;
	}
	loop_get_ctrl_cmd(fd,glb_cfg.uicmd.time_out);
	
	if(fd > 0) close(fd);
	sleep(5);
	goto AGAIN;
	
	return NULL;
}

int main(int argc, char  **argv)
{
	cmd_parser(argc,argv,&glb_cfg);
	if(glb_cfg.daemon)
		run_as_daemon();

	init_signals();
	alarm(WT_SDCARD_PRIOD);

	pthread_t sorpid;
	pthread_create(&sorpid,NULL,handle_sensor,NULL);
	
	pthread_t cmdpid;
	pthread_create(&cmdpid,NULL,handle_uicmd,NULL);
	
	pthread_join(sorpid,NULL);
	pthread_join(cmdpid,NULL);
	
	debug(LOG_NOTICE,"<==========Process exit!\n");
	return 0;
}
