#include "functions.h"

/* Loosely based on the checkfs shell script from the LFS bootscripts package.
 * Completely rewritten in C by Richard Mant - richard@richardmant.com
 *
 * Revision 0.1 - Richard Mant, 2012-05-01 21:40 +0000 (Tuesday, 1 May 2012)
 */

int usage(char *name)
{
	printf("Usage: %s {start}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	int ret = 0;
	if (argc < 2)
		return usage(argv[0]);

	if (CHECK_ARG(1, "start"))
	{
		const char *options;
		if (normalFileExists("/fastboot") == TRUE)
		{
			printf(INFO "/fastboot found, will not perform file system checks as requested." NEWLINE);
			return echoOk();
		}

		printf(INFO "Mounting root file system in read-only mode..." NEWLINE);
		ret = evaluateRetVal(runProcess(5, 0, NULL, NULL, "mount", "-n", "-o", "remount,ro", "/", NULL));
		if (ret != 0)
		{
			printf(FAILURE "FAILURE:\n\nCannot check root filesystem because it could not be mounted"
				" in read-only mode.\n\nAfter you press Enter, this system will be halted and powered off."
				INFO "\n\nPress enter to continue..." NEWLINE);
			readConsoleLine();
			ret = runProcess(2, 0, NULL, NULL, "/etc/rc.d/init.d/halt", "stop", NULL);
		}

		if (normalFileExists("/forcefsck") == TRUE)
		{
			printf(INFO "/forcefsck found, forcing file system checks as requested." NEWLINE);
			options = "-f";
		}
		else
			options = NULL;
		printf(INFO "Checking file systems..." NEWLINE);
		ret = runProcess(5 + (options != NULL ? 1 : 0), 0, NULL, NULL, "fsck", "-a", "-A", "-C", "-T", options, NULL);
		if (ret == 0)
			echoOk();
		else if (ret == 1)
		{
			echoWarning();
			printf(WARNING "WARNING:\n\nFile system errors were found and have been corrected."
				" You may want to double-check that everything was fixed properly." NEWLINE);
		}
		else if (ret == 2 || ret == 3)
		{
			echoWarning();
			printf(WARNING "WARNING:\n\nFile system errors were found and have been been"
				" corrected, but the nature of the errors require this system to be"
				" rebooted.\n\nAfter you press enter, this system will be rebooted"
				INFO "\n\nPress Enter to continue..." NEWLINE);
			readConsoleLine();
			runProcess(2, 0, NULL, NULL, "reboot", "-f", NULL);
			return 0;
		}
		else if (ret > 3 && ret < 16)
		{
			echoFailure();
			printf(FAILURE "FAILURE:\n\nFile system errors were encountered that could not be"
				" fixed automatically.  This system cannot continue to boot and will"
				" therefore be halted until those errors are fixed manually by a"
				" System Administrator.\n\nAfter you press Enter, this system will be"
				" halted and powered off." INFO "\n\nPress Enter to continue..." NEWLINE);
			readConsoleLine();
			ret = runProcess(2, 0, NULL, NULL, "/etc/rc.d/init.d/halt", "stop", NULL);
		}
		else if (ret >= 16)
		{
			echoFailure();
			printf(FAILURE "FAILURE:\n\nUnexpected Failure running fsck.  Exited with error"
				" code: ${error_value}." NEWLINE);
			return ret;
		}
	}
	else
		return usage(argv[0]);
	return 0;
}
