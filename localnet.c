#include "functions.h"

/* Loosely based on the localnet shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2012-05-11 17:55 +0000 (Friday, 11 May 2012)
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

	if (normalFileExists("/etc/sysconfig/network") == TRUE)
		source("/etc/sysconfig/network");

	if (CHECK_ARG(1, "start"))
	{
		int ret;
		printf(INFO "Bringing up the loopback interface..." NEWLINE);
		ret = runProcess(8, RUN_PROC_PASS_STDOUT, NULL, NULL, "ip", "addr", "add", "127.0.0.1/8", "label", "lo", "dev", "lo", NULL);
		ret |= runProcess(5, RUN_PROC_PASS_STDOUT, NULL, NULL, "ip", "link", "set", "lo", "up", NULL);
		evaluateRetVal(ret);
		printf(INFO "Setting hostname to %s..." NEWLINE, getenv("HOSTNAME"));
		evaluateRetVal(runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, "hostname", getenv("HOSTNAME"), NULL));
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(INFO "Takeing down the loopback interface..." NEWLINE);
		return evaluateRetVal(runProcess(5, RUN_PROC_PASS_STDOUT, NULL, NULL, "ip", "link", "set", "lo", "down", NULL));
	}
	else if (CHECK_ARG(1, "restart"))
	{
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "stop", NULL);
		usleep(50);
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "start", NULL);
	}
	else if (CHECK_ARG(1, "status"))
	{
		char *hostname;
		runProcess(1, RUN_PROC_RET_STDOUT, &hostname, NULL, "hostname", NULL);
		printf(INFO "Hostname is: %s" NORMAL, hostname);
		fflush(stdout);
		runProcess(4, RUN_PROC_PASS_STDOUT, NULL, NULL, "ip", "link", "show", "lo", NULL);
	}
	else
		return usage(argv[0]);
	return 0;
}
