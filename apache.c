#include "functions.h"

/* Loosely based on the apache shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2011-03-07 22:23 +0000 (Monday, 07 Mar 2011)
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
		printf(SUCCESS "Starting Apache daemon..." NEWLINE);
		return evaluateRetVal(runProcess(4, 0, NULL, NULL, "/usr/sbin/apachectl", "-k", "start", NULL));
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(FAILURE "Stopping Apache daemon..." NEWLINE);
		return evaluateRetVal(runProcess(4, 0, NULL, NULL, "/usr/sbin/apachectl", "-k", "stop", NULL));
	}
	else if (CHECK_ARG(1, "restart"))
	{
		printf(WARNING "Restarting Apache daemon..." NEWLINE);
		return evaluateRetVal(runProcess(4, 0, NULL, NULL, "/usr/sbin/apachectl", "-k", "restart", NULL));
	}
	else if (CHECK_ARG(1, "status"))
		return statusProc("/usr/sbin/httpd", NULL);
	else
		return usage(argv[0]);
}
