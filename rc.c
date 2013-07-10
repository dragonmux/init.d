#include "functions.h"
#include <dirent.h>

/* Loosely based on the rc shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2013-07-10 17:45 +0100 (Wednesday, 10 Jul 2013)
 */

#define RCPATH "%s/rc%s.d"

typedef struct bootScripts
{
	size_t count;
	char **paths;
} bootScripts;

typedef struct dirent dirent;

char *rcBase, *rcPath;

// TODO: Rewrite this properly!
void ensureTTYSane()
{
	runProcess(3, RUN_PROC_NO_STDOUT, NULL, NULL, "stty", "sane", NULL);
}

int usage(char *name)
{
	printf("Usage: %s <runlevel>\n", name);
	return 1;
}

int runlevelExists(char *level)
{
	char *path = toString(RCPATH, rcBase, level);
	if (!dirExists(path))
	{
		printf(WARNING "%s does not exist." NEWLINE, path);
		free(path);
		return FALSE;
	}
	rcPath = path;
	return TRUE;
}

int scriptSort(const void *a, const void *b)
{
	return strverscmp(*((char * const *)a), *((char * const *)b));
}

bootScripts *fetchScripts(char *level, char *type)
{
	dirent *scriptEnt;
	bootScripts *scripts = rcMalloc(sizeof(bootScripts));
	char *path = toString(RCPATH, rcBase, level);
	DIR *pathDir = opendir(path);
	size_t typeLen = strlen(type);

	scripts->count = 0;
	scripts->paths = NULL;

	while ((scriptEnt = readdir(pathDir)) != NULL)
	{
		if (strncmp(scriptEnt->d_name, type, typeLen) == 0)
		{
			size_t idx = scripts->count++;
			scripts->paths = rcRealloc(scripts->paths, scripts->count * sizeof(char *));
			scripts->paths[idx] = strdup(scriptEnt->d_name);
		}
	}

	closedir(pathDir);
	free(path);
	qsort(scripts->paths, scripts->count, sizeof(char *), scriptSort);
	return scripts;
}

void freeScripts(bootScripts *scripts)
{
	size_t i;
	for (i = 0; i < scripts->count; i++)
		free(scripts->paths[i]);
	free(scripts);
}

int checkScript(char *script)
{
	if (!fileSymlink(script))
	{
		printf(WARNING "%s is not a valid symlink." NEWLINE, script);
		echoWarning();
	}
	else if (!fileExecutable(script))
	{
		printf(WARNING "%s is not executable, skipping." NEWLINE, script);
		echoWarning();
	}
	else
		return TRUE;
	return FALSE;
}

void rcErrorMsg(char *script, int ret)
{
	printf(FAILURE "FAILURE:\n\nYou should not be reading this error message.\n\n"
		" It means that an unforceen error took place in %s, which exited with"
		" a return value of %d.\n"
		" If you're able to track this error down to a bug"
		" in one of the files provided by the CLFS book, please be so kind to"
		" inform us at clfs-dev@cross-lfs.org.\n", script, ret);
	printf(INFO "Press Enter to continue..." NEWLINE);
	readConsoleLine();
}

int main(int argc, char **argv)
{
	char *previous, *runlevel;

	if (argc != 2 || strcmp(argv[1], "") == 0)
		return usage(argv[0]);

	if (!fileExists("/etc/sysconfig/rc"))
	{
		printf("/etc/sysconfig/rc does not exist and must exist for %s to run\n", argv[0]);
		return 1;
	}
	source("/etc/sysconfig/rc");
	ensureTTYSane();

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	runlevel = argv[1];
	previous = getenv("PREVLEVEL");
	if (previous == NULL || strcmp(previous, "") == 0)
	{
		free(previous);
		previous = strdup("N");
	}
	rcBase = getenv("rc_base");

	if (!runlevelExists(runlevel))
		return 1;

	if (strcmp(previous, "N") != 0)
	{
		size_t i;
		bootScripts *scripts = fetchScripts(runlevel, "K");
		for (i = 0; i < scripts->count; i++)
		{
			char *script = toString("%s/%s", rcPath, scripts->paths[i]);
			if (checkScript(script))
			{
				int ret;
				if (strcmp(runlevel, "0") != 0 && strcmp(runlevel, "6") != 0)
				{
					// XXX:Replace the following check with a call to check previous and "sysinit" folders
					// for the existance of a file who's name starts with S and ends with scripts->paths[i] + 3.
					// This is to cater for the numerical value potentially being different.
					char *prevStart = toString(RCPATH "/S%s", rcBase, previous, scripts->paths[i] + 1);
					char *sysInitStart = toString(RCPATH "/S%s", rcBase, "sysinit", scripts->paths[i] + 1);
					if (!fileExists(prevStart) && !fileExists(sysInitStart))
					{
						printf(WARNING "WARNING:\n\n%s can't be executed because it was not started in the previous runlevel (%s)." NEWLINE,
							script, previous);
						free(sysInitStart);
						free(prevStart);
						free(script);
						continue;
					}
					free(sysInitStart);
					free(prevStart);
				}
				ret = runProcess(3, RUN_PROC_PASS_STDOUT, NULL, NULL, script, "stop", NULL);
				if (ret != 0)
					rcErrorMsg(script, ret);
			}
			free(script);
		}
		freeScripts(scripts);
	}

	{
		size_t i;
		bootScripts *scripts = fetchScripts(runlevel, "S");
		for (i = 0; i < scripts->count; i++)
		{
			char *script;
			if (strcmp(previous, "N") != 0)
			{
				// XXX:Replace the following check with a call to check previous and runlevel folders
				// for the existance of a file who's name starts with S and ends with scripts->paths[i] + 3.
				// This is to cater for the numerical value potentially being different.
				char *stop = toString(RCPATH "/K%s", rcBase, runlevel, scripts->paths[i] + 1);
				char *prevStart = toString(RCPATH, "/S%s", rcBase, runlevel, scripts->paths[i] + 1);
				if (!fileExists(prevStart) && !fileExists(stop))
				{
					free(prevStart);
					free(stop);
					continue;
				}
				free(prevStart);
				free(stop);
			}

			script = toString("%s/%s", rcPath, scripts->paths[i]);
			if (checkScript(script))
			{
				int ret;
				if (strcmp(runlevel, "0") == 0 || strcmp(runlevel, "6") == 0)
					ret = runProcess(3, RUN_PROC_PASS_STDOUT, NULL, NULL, script, "stop", NULL);
				else
					ret = runProcess(3, RUN_PROC_PASS_STDOUT, NULL, NULL, script, "start", NULL);
				if (ret != 0)
					rcErrorMsg(script, ret);
			}
		}
		freeScripts(scripts);
	}

	return 0;
	//return runProcess(3, RUN_PROC_PASS_STDOUT, NULL, NULL, "/etc/rc.d/init.d/rc.sh", argv[1], NULL);
}
