#include "functions.h"

/* Loosely based on the reboot shell script from the LFS bootscripts package.
 * Completely rewritten in C by Richard Mant - richard@richardmant.com
 *
 * Revision 0.1 - Richard Mant, 2012-05-04 00:35 +0000 (Friday, 04 May 2012)
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
		printf(WARNING "Restarting system..." NEWLINE);
		runProcess(4, 0, NULL, NULL, "reboot", "-d", "-f", "-i", NULL);
		return 0;
	}
	else
		return usage(argv[0]);
}
