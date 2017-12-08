#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


int debug_level = 7;

/** @internal
Do not use directly, use the debug macro */
void
_debug(const char filename[], int line, int level, const char *format, ...)
{
	char buf[28];
	va_list vlist;
	time_t ts;
	sigset_t block_chld;

	time(&ts);

	if (debug_level >= level) {
		sigemptyset(&block_chld);
		sigaddset(&block_chld, SIGCHLD);
		sigprocmask(SIG_BLOCK, &block_chld, NULL);

		if (1) {
			openlog("salepos", LOG_PID|LOG_NOWAIT, LOG_DAEMON);
			va_start(vlist, format);
			vsyslog(level, format, vlist);
			va_end(vlist);
			closelog();
		} else {
			if (level <= LOG_WARNING) {
				fprintf(stderr, "[%d][%.24s][%u](%s:%d) ", level, ctime_r(&ts, buf), getpid(),
						filename, line);
				va_start(vlist, format);
				vfprintf(stderr, format, vlist);
				va_end(vlist);
				fputc('\n', stderr);
			} else {
				fprintf(stdout, "[%d][%.24s][%u](%s:%d) ", level, ctime_r(&ts, buf), getpid(),
						filename, line);
				va_start(vlist, format);
				vfprintf(stdout, format, vlist);
				va_end(vlist);
				fputc('\n', stdout);
				fflush(stdout);
			}
		}

		sigprocmask(SIG_UNBLOCK, &block_chld, NULL);
	}
}