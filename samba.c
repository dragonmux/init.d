#include "functions.h"

/* Loosely based on the samba shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2012-10-10 04:00 +0000 (Tuesday, 10 October 2012)
 */

#define SMBDPIDFILE "/var/run/smbd.pid"
#define NMBDPIDFILE "/var/run/nmbd.pid"

int usage(char *name)
{
	printf("Usage: %s {start|stop|reload|restart|status}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return usage(argv[0]);

	if (CHECK_ARG(1, "start"))
	{
		int ret = 0;
		printf(SUCCESS "Starting nmbd..." NEWLINE);
		ret |= loadProc("/usr/sbin/nmbd", NMBDPIDFILE, 0, 0, 1, "-D");
		printf(SUCCESS "Starting smbd..." NEWLINE);
		ret |= loadProc("/usr/sbin/smbd", SMBDPIDFILE, 0, 0, 1, "-D");
		return ret;
	}
	else if (CHECK_ARG(1, "stop"))
	{
		int ret = 0;
		printf(FAILURE "Stopping nmbd..." NEWLINE);
		ret |= killProc("/usr/sbin/nmbd", NMBDPIDFILE, -1, 0);
		printf(FAILURE "Stopping smbd..." NEWLINE);
		ret |= killProc("/usr/sbin/smbd", SMBDPIDFILE, -1, 0);
		return ret;
	}
	else if (CHECK_ARG(1, "reload"))
	{
		int ret = 0;
		printf(WARNING "Reloading nmbd..." NEWLINE);
		ret |= reloadProc("/usr/sbin/nmbd", NMBDPIDFILE);
		printf(WARNING "Reloading smbd..." NEWLINE);
		ret |= reloadProc("/usr/sbin/smbd", SMBDPIDFILE);
		return ret;
	}
	else if (CHECK_ARG(1, "restart"))
	{
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "stop", NULL);
		usleep(50);
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "start", NULL);
		return 0;
	}
	else if (CHECK_ARG(1, "status"))
	{
		int ret = 0;
		ret |= statusProc("/usr/sbin/nmbd", NMBDPIDFILE);
		ret |= statusProc("/usr/sbin/smbd", SMBDPIDFILE);
		return ret;
	}
	else
		return usage(argv[0]);
}
