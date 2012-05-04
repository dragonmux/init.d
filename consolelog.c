#include "functions.h"

/* Loosely based on the consolelog shell script from the LFS bootscripts package.
 * Completely rewritten in C by Richard Mant - richard@richardmant.com
 *
 * Revision 0.1 - Richard Mant, 2012-05-04 00:00 +0000 (Friday, 04 May 2012)
 */

int usage(char *name)
{
	printf("Usage: %s {start|status}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return usage(argv[0]);

	setenv("LOGLEVEL", "7", 1);
	if (normalFileExists("/etc/sysconfig/console") == TRUE)
		source("/etc/sysconfig/console");

	if (CHECK_ARG(1, "start"))
	{
		char *loglevel = getenv("LOGLEVEL");
		int logLevel = strToInt(loglevel);
		if (logLevel > 0 && logLevel < 9)
		{
			printf(INFO "Setting the console log level to %s..." NEWLINE, loglevel);
			return evaluateRetVal(runProcess(3, 0, NULL, NULL, "dmesg", "-n", loglevel, NULL));
		}
		else
		{
			printf(FAILURE "Console log level '%s' is invalid" NEWLINE, loglevel);
			return echoFailure();
		}
	}
	else if (CHECK_ARG(1, "status"))
	{
		if (fileExists("/proc/sys/kernel/printk") == TRUE)
		{
			char *printk = readLine("/proc/sys/kernel/printk", 1);
			int level = strToInt(printk);
			printf(INFO "The current console log level is %d" NEWLINE, level);
		}
		else
		{
			printf(FAILURE "Can't read the current console log level" NEWLINE);
			return echoFailure();
		}
	}
	else
		return usage(argv[0]);
	return 0;
}
