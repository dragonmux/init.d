#include "functions.h"

/* Loosely based on the setclock shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2012-05-12 02:15 +0000 (Saturday, 12 May 2012)
 */

int usage(char *name)
{
	printf("Usage: %s {start|stop}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	char *clockParams;
	int numParams;

	if (argc < 2)
		return usage(argv[0]);

	if (normalFileExists("/etc/sysconfig/clock") == TRUE)
		source("/etc/sysconfig/clock");

	{
		char *isUTC = getenv("UTC");
		if (isUTC == NULL || strcasecmp(isUTC, "yes") == 0 ||
			strcasecmp(isUTC, "true") == 0 || strcasecmp(isUTC, "1") == 0)
			clockParams = "--utc";
		else if (strcasecmp(isUTC, "no") == 0 || strcasecmp(isUTC, "false") == 0 ||
			strcasecmp(isUTC, "0") == 0)
			clockParams = "--localtime";
		else
			clockParams = NULL;
		numParams = 2 + (clockParams == NULL ? 0 : 1);
	}

	if (CHECK_ARG(1, "start"))
	{
		printf(INFO "Setting system clock..." NEWLINE);
		return evaluateRetVal(runProcess(numParams, 0, NULL, NULL, "hwclock", "--hctosys", clockParams, NULL));
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(INFO "Setting hardware clock..." NEWLINE);
		return evaluateRetVal(runProcess(numParams, 0, NULL, NULL, "hwclock", "--systohc", clockParams, NULL));
	}
	else
		return usage(argv[0]);
	return 0;
}
