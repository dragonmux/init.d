#include "functions.h"

/* Loosely based on the mountfs shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2012-05-13 01:30 +0000 (Sunday, 13 May 2012)
 */

int usage(char *name)
{
	printf("Usage: %s {start|stop}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return usage(argv[0]);

	if (CHECK_ARG(1, "start"))
	{
		int ret;
		printf(WARNING "Remounting root file system in read-write mode..." NEWLINE);
		evaluateRetVal(runProcess(5, 0, NULL, NULL, "mount", "-n", "-o", "remount,rw", "/", NULL));
		// Remove fsck-related file system watermarks.
		unlink("/fastboot");
		unlink("/forcefsck");

		printf(INFO "Recording existing mounts in /etc/mtab..." NEWLINE);
		fclose(fopen("/etc/mtab", "w"));
		ret = runProcess(3, RUN_PROC_PASS_STDOUT, NULL, NULL, "mount", "-f", "/");
		ret |= runProcess(3, RUN_PROC_PASS_STDOUT, NULL, NULL, "mount", "-f", "/proc");
		ret |= runProcess(3, RUN_PROC_PASS_STDOUT, NULL, NULL, "mount", "-f", "/sys");
		evaluateRetVal(ret);

		/*
			This will mount all filesystems that do not have _netdev in
			their option list.  _netdev denotes a network filesystem.
		*/
		printf(INFO "Mounting remaining file systems..." NEWLINE);
		evaluateRetVal(runProcess(4, 0, NULL, NULL, "mount", "-a", "-O", "no_netdev"));
	}
	else if (CHECK_ARG(1, "stop"))
	{
		printf(INFO "Unmounting all other currently mounted file systems..." NEWLINE);
		return evaluateRetVal(runProcess(4, 0, NULL, NULL, "umount", "-a", "-d", "-r", NULL));
	}
	else
		return usage(argv[0]);
	return 0;
}
