#include "functions.h"

/* Loosely based on the rsyslog shell script from the LFS bootscripts package.
 * Completely rewritten in C by Richard Mant - richard@richardmant.com
 *
 * Revision 0.1 - Richard Mant, 2012-05-09 08:45 +0000 (Wednesday, 09 May 2012)
 */

int usage(char *name)
{
	printf("Usage: %s {start|stop|restart|status}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return usage(argv[0]);

	if (CHECK_ARG(1, "start"))
	{
		printf(SUCCESS "Starting system log daemon..." NEWLINE);
		loadProc("rsyslogd", NULL, 0, 0, 1, "-c3");
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(FAILURE "Stopping system log daemon..." NEWLINE);
		killProc("rsyslogd", NULL, -1, 0);
	}
	else if (CHECK_ARG(1, "restart"))
	{
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "stop", NULL);
		usleep(50);
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "start", NULL);
	}
	else if (CHECK_ARG(1, "status"))
		return statusProc("rsyslogd", NULL);
	else
		return usage(argv[0]);
	return 0;
}
