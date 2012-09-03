#include "functions.h"

/* Loosely based on the swap shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2012-02-07 17:55 +0000 (Tuesday, 07 February 2012)
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
		printf(INFO "Activating all swap files/partitions.." NEWLINE);
		return evaluateRetVal(runProcess(2, 0, NULL, NULL, "swapon", "-a", NULL));
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(INFO "Deactivating all swap files/partitions.." NEWLINE);
		return evaluateRetVal(runProcess(2, 0, NULL, NULL, "swapoff", "-a", NULL));
	}
	else if (CHECK_ARG(1, "restart"))
	{
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "stop", NULL);
		usleep(50);
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "start", NULL);
	}
	else if (CHECK_ARG(1, "status"))
	{
		printf(INFO "Retrieving swap status..");
		echoOk();
		printf(NEWLINE);
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, "swapon", "-s", NULL);
	}
	else
		return usage(argv[0]);
	return 0;
}
