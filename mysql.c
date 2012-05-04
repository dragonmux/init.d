#include "functions.h"

/* Loosely based on the mysql shell script from the LFS bootscripts package.
 * Completely rewritten in C by Richard Mant - richard@richardmant.com
 *
 * Revision 0.1 - Richard Mant, 2011-03-08 18:41 +0000 (Tuesday, 08 Mar 2011)
 */

#define PIDFILE_FORMAT "/srv/mysql/%s.pid"

int usage(char *name)
{
	printf("Usage: %s {start|stop|restart|status}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	char *pidFile, *hostName;
	int ret;
	if (argc < 2)
		return usage(argv[0]);

	hostName = getHostName();
	/* NOTE: Technically we're very naughty here in that we don't free() the
	 * memory returned as a result of this call.. */
	pidFile = toString(PIDFILE_FORMAT, hostName);
	free(hostName);
	if (CHECK_ARG(1, "start"))
	{
		uint8_t failure = FALSE;
		printf(SUCCESS "Starting MySQL daemon...");
		if (normalFileExists(pidFile) == TRUE)
		{
			uint32_t nPIDs;
			pid_t *PIDs;
			int ret = pidOfProc(&PIDs, &nPIDs, "mysqld", pidFile);
			if (nPIDs != 0 && ret == 0)
			{
				printf(WARNING " mysqld already running!" NEWLINE);
				echoWarning();
				return 0;
			}
			else
			{
				unlink(pidFile);
				if (fileExists(pidFile) == TRUE)
					failure = TRUE;
			}
		}
		if (failure == TRUE)
		{
			printf(NEWLINE);
			return echoFailure();
		}
		else
		{
			printf(NEWLINE);
			ret = runProcess(4, RUN_PROC_NO_WAIT, NULL, NULL, "/usr/bin/mysqld_safe", "-u", "mysql", NULL);
			return evaluateRetVal(ret);
		}
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(FAILURE "Stopping MySQL daemon...");
		if (fileExists(pidFile) == TRUE)
		{
			printf(NEWLINE);
			killProc("/usr/bin/mysqld", pidFile, -1, 20);
		}
		else
		{
			printf(WARNING " mysqld not running!" NEWLINE);
			echoWarning();
			if (normalFileExists(pidFile) == TRUE)
				unlink(pidFile);
		}
	}
	else if (CHECK_ARG(1, "restart"))
	{
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "stop", NULL);
		usleep(50);
		runProcess(2, RUN_PROC_PASS_STDOUT, NULL, NULL, argv[0], "start", NULL);
	}
	else if (CHECK_ARG(1, "status"))
		return statusProc("/usr/sbin/mysqld", NULL);
	else
		return usage(argv[0]);
	return 0;
}
