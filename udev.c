#include "functions.h"

/* Loosely based on the udev shell script from the LFS bootscripts package.
 * Completely rewritten in C by Richard Mant - richard@richardmant.com
 *
 * Revision 0.1 - Richard Mant, 2012-05-01 09:30 +0000 (Tuesday, 1 May 2012)
 */

int usage(char *name)
{
	printf("Usage: %s {start|stop|restart|status|reload|force-reload}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	int ret = 0;
	if (argc < 2)
		return usage(argv[0]);

	if (CHECK_ARG(1, "start"))
	{
		int ret;
		printf(INFO "Creating /dev in tmpfs..." NEWLINE);
		ret = evaluateRetVal(runProcess(8, 0, NULL, NULL, "mount", "-n", "-t", "tmpfs", "-o", "mode=0755", "udev", "/dev", NULL));
		if (ret != 0)
			return ret;
		printf(INFO "Copying static entries..." NEWLINE);
		ret = system("cp --preserve=all --recursive --remove-destination /lib/udev/devices/* /dev");
		if (ret >= 0)
			ret = evaluateRetVal(ret);
		else
			echoFailure();
		if (ret != 0)
			return ret;
		printf(INFO "Setting Permissons on /dev/shm..." NEWLINE);
		ret = (chmod("/dev/shm", ACCESSPERMS | S_ISVTX) != 0);
		if (ret != 0)
			return ret;
		writeNewline("/sys/kernel/uevent_helper");
		printf(SUCCESS "Starting udevd..." NEWLINE);
		ret = evaluateRetVal(runProcess(2, 0, NULL, NULL, "/sbin/udevd", "--daemon", NULL));
		if (ret != 0)
			return ret;
		printf(INFO "Performing Coldplugging..." NEWLINE);
		mkdir("/dev/.udev/queue", (/*S_IRWXUGO*/ACCESSPERMS & ~umask(0)) | S_IWUSR | S_IXUSR);
		ret = 0;
		ret |= evaluateRetVal(runProcess(2, 0, NULL, NULL, "/sbin/udevadm", "trigger", NULL));
		ret |= evaluateRetVal(runProcess(2, 0, NULL, NULL, "/sbin/udevadm", "settle", NULL));
		if (ret != 0)
			return ret;
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(FAILURE "Stopping udevd..." NEWLINE);
		killProc("/sbin/udev", NULL, -1, 0);
	}
	else if (CHECK_ARG(1, "restart"))
	{
	}
	else if (CHECK_ARG(1, "status"))
		return statusProc("/sbin/udevd", NULL);
	else
		return usage(argv[0]);
}
