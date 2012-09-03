#include "functions.h"

/* Loosely based on the dbus shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2011-03-07 23:25 +0000 (Monday, 07 Mar 2011)
 */

#define PIDFILE "/var/run/dbus/pid"
#define SOCKET "/var/run/dbus/system_bus_socket"

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
		printf(SUCCESS "Starting the D-Bus Message Bus Daemon..." NEWLINE);
		runProcess(2, 0, NULL, NULL, "/usr/bin/dbus-uuidgen", "--ensure", NULL);
		loadProc("/usr/bin/dbus-daemon", PIDFILE, 0, 0, 1, "--system");
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(FAILURE "Stopping the D-Bus Message Bus Daemon..." NEWLINE);
		ret = killProc("/usr/bin/dbus-daemon", PIDFILE, -1, 0);
		if (ret == 0)
			unlink(SOCKET);
	}
	else if (CHECK_ARG(1, "restart"))
	{
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "stop", NULL);
		usleep(50);
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "start", NULL);
	}
	else if (CHECK_ARG(1, "status"))
		return statusProc("/usr/bin/dbus-daemon", PIDFILE);
	else
		return usage(argv[0]);
	return 0;
}
