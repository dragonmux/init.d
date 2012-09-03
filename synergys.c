#include "functions.h"

/* Loosely based on the shell scriptd from the LFS bootscripts package.
 * Written in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2011-09-27 12:05 +0001 (Tuesday, 27 Sept 2011)
 */

int usage(char *name)
{
	printf("Usage: %s {start|stop|restart|status}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	int ret;
	if (argc < 2)
		return usage(argv[0]);

	if (CHECK_ARG(1, "start"))
	{
		printf(SUCCESS "Starting Synergy server..." NEWLINE);
		ret = runProcess(8, 0, NULL, NULL, "/usr/bin/synergys", "-a", "192.168.10.4", "--display", ":0", "-l", "/var/log/synergys.log", NULL);
		return evaluateRetVal(ret);
	}
	else if (CHECK_ARG(1, "restart"))
	{
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "stop");
		usleep(50);
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "start");
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(FAILURE "Stopping Synergy server..." NEWLINE);
		ret = killProc("/usr/bin/synergys", NULL, -1, 0);
		return evaluateRetVal(ret);
	}
	else if (CHECK_ARG(1, "status"))
		return statusProc("/usr/bin/synergys", NULL);
	else
		return usage(argv[0]);
}
