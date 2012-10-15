#include "functions.h"

/* Loosely based on the dbus shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2012-05-01 13:00 +0000 (Tueday, 01 May 2012)
 */

#define PIDFILE "/var/run/sshd.pid"

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
		if (normalFileExists("/etc/ssh/ssh_host_key") == FALSE)
		{
			int ret;
			printf(INFO "Generating /etc/ssh/ssh_host_key" NEWLINE);
			ret = evaluateRetVal(runProcess(7, 0, NULL, NULL, "ssh-keygen", "-t", "rsa1", "-f", "/etc/ssh/ssh_host_key", "-N", "", NULL));
			if (ret != 0)
				return ret;
		}
		if (normalFileExists("/etc/ssh/ssh_host_dsa_key") == FALSE)
		{
			int ret;
			printf(INFO "Generating /etc/ssh/ssh_host_dsa_key" NEWLINE);
			ret = evaluateRetVal(runProcess(7, 0, NULL, NULL, "ssh-keygen", "-t", "dsa", "-f", "/etc/ssh/ssh_host_dsa_key", "-N", "", NULL));
			if (ret != 0)
				return ret;
		}
		if (normalFileExists("/etc/ssh/ssh_host_rsa_key") == FALSE)
		{
			int ret;
			printf(INFO "Generating /etc/ssh/ssh_host_rsa_key" NEWLINE);
			ret = evaluateRetVal(runProcess(7, 0, NULL, NULL, "ssh-keygen", "-t", "rsa", "-f", "/etc/ssh/ssh_host_rsa_key", "-N", "", NULL));
			if (ret != 0)
				return ret;
		}
		printf(SUCCESS "Starting SSH Server..." NEWLINE);
		loadProc("/usr/sbin/sshd", PIDFILE, 0, 0, 0);
		usleep(50);
		{
			FILE *file;
			char *fileName, *PID;
			PID = readLine(PIDFILE, 1);
			if (PID == NULL)
			{
				printf(FAILURE "loadProc() failed to work! Please ignore the apparent success of the previous line." NEWLINE);
				return echoFailure();
			}
			fileName = toString("/proc/%s/oom_adj", PID);
			file = fopen(fileName, "w");
			fwrite("-16\n", 4, 1, file);
			fclose(file);
			free(fileName);
		}
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(FAILURE "Stopping SSH Server..." NEWLINE);
		killProc("/usr/sbin/sshd", PIDFILE, -1, 0);
	}
	else if (CHECK_ARG(1, "reload"))
	{
		printf(WARNING "Reloading SSH Server..." NEWLINE);
		reloadProc("/usr/sbin/sshd", PIDFILE);
	}
	else if (CHECK_ARG(1, "restart"))
	{
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "stop", NULL);
		usleep(100);
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "start", NULL);
	}
	else if (CHECK_ARG(1, "status"))
		return statusProc("/usr/sbin/sshd", PIDFILE);
	else
		return usage(argv[0]);
	return 0;
}
