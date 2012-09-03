#include "functions.h"

/* Loosely based on the halt shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2012-05-03 22:00 +0000 (Thursday, 03 May 2012)
 */

int usage(char *name)
{
	printf("Usage: %s stop\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	int ret = 0;
	if (argc < 2)
		return usage(argv[0]);

	if (CHECK_ARG(1, "stop"))
	{
		runProcess(5, 0, NULL, NULL, "halt", "-d", "-f", "-i", "-p", NULL);
		return 0;
	}
	else
		return usage(argv[0]);
}
