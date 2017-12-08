
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <getopt.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h>

#include "uart.h"
#include "fj_pro.h"
#include "config.h"
#include "debug.h"

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

	debug(LOG_DEBUG, "SIGCHLD handler: Trying to reap a child\n");

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
	debug(LOG_NOTICE, "Handler for termination caught signal %d\n", s);

	debug(LOG_NOTICE, "Exiting...\n");
	exit(s == 0 ? 1 : 0);
}

static void timer_handler(int sig)
{
	if(sig == SIGALRM) {
		debug(LOG_NOTICE, "Get signal Alarm\n");
	}
}

static void usrsig_handler(int sig)
{
	if(sig == SIGUSR1) {
		debug(LOG_NOTICE, "Get signal USR1\n");
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

	debug(LOG_DEBUG, "Setting SIGCHLD handler to sigchld_handler()\n");
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
	debug(LOG_DEBUG, "Setting SIGPIPE  handler to SIG_IGN\n");
	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &sa, NULL) == -1) {
		debug(LOG_ERR, "sigaction(): %s", strerror(errno));
		exit(1);
	}

	debug(LOG_DEBUG, "Setting SIGTERM,SIGQUIT,SIGINT  handlers to termination_handler()\n");
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


int main(int argc, char **argv)
{
	int fd = 0;
	int ret = 0;
	if(argc >= 2)
		run_as_daemon();
	
	init_signals();
	
	while(1) {
		fd = init_com_dev(FJ_DEV, FJ_BPS, FJ_PAR, FJ_BLOCK);
		if(fd <= 0) {
			debug(LOG_ERR, "Pos dev open error:%d\n",errno);
			sleep(5);
			continue;
		}
		fj_recv_loop(fd,FJ_MIN_CHAR_INTERVAL);
		debug(LOG_INFO, "FJ Dev closed. reopen after 3s.\n");
		close(fd);
		fd = 0;
		sleep(3);
	}
	debug(LOG_INFO, "<======Process Exit\n");
	return 0;
}