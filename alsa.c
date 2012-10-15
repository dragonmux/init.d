#include "functions.h"

/* Loosely based on the alsa shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2012-04-30 17:30 +0000 (Monday, 30 Apr 2012)
 */

int usage(char *name)
{
	printf("Usage: %s stop\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return usage(argv[0]);

	if (CHECK_ARG(1, "stop"))
	{
		printf(INFO "Stopping ALSA...	Saving volumes..." NEWLINE);
		return loadProc("/usr/sbin/alsactl", NULL, 0, 0, 1, "store");
	}
	else
		return usage(argv[0]);
}
