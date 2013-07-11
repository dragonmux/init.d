#include "functions.h"
#include <dirent.h>

/* Loosely based on the rc shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2013-07-10 17:45 +0100 (Wednesday, 10 Jul 2013)
 * Revision 0.2 - Rachel Mant, 2013-07-11 17:11 +0100 (Thursday, 11 Jul 2013)
 */

#define RCPATH "%s/rc%s.d"

typedef struct bootScripts
{
	size_t count;
	char **paths;
} bootScripts;

typedef struct dirent dirent;

char *rcBase, *rcPath;
static pthread_mutex_t consoleMutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
pthread_attr_t threadAttr;
pthread_barrier_t startBarrier, endBarrier;
static const char *action;

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
			scripts->paths[idx] = strdup(scriptEnt->d_name + 1);
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

int scriptExists(char *level, char *type, char *script)
{
	dirent *scriptEnt;
	int found = FALSE;
	char *path = toString(RCPATH, rcBase, level);
	DIR *pathDir = opendir(path);
	size_t typeLen = strlen(type);

	while ((scriptEnt = readdir(pathDir)) != NULL && !found)
	{
		if (strncmp(scriptEnt->d_name, type, typeLen) == 0 &&
			strcmp(scriptEnt->d_name + typeLen + 2, script) == 0)
			found = TRUE;
	}

	closedir(pathDir);
	free(path);
	return found;
}

int checkScript(const char *script)
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

void rcErrorMsg(const char *script, int ret)
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

void runScript(const char *action, const char *script)
{
	char *stdOut = NULL;
	int ret = runProcess(3, RUN_PROC_RET_STDOUT, &stdOut, NULL, script, action, NULL);
	pthread_mutex_lock(&consoleMutex);
	fputs(stdOut, stdout);
	if (ret != 0)
		rcErrorMsg(script, ret);
	pthread_mutex_unlock(&consoleMutex);
}

static void *stopScriptThread(void *pScript)
{
	const char *script = (const char *)pScript;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	// Wait for main() to finish spawning threads
	pthread_barrier_wait(&startBarrier);
	// Execute the script
	runScript("stop", script);
	// Re-synchronise to let main() know it can continue..
	pthread_barrier_wait(&endBarrier);
	return pScript;
}

static void *runScriptThread(void *pScript)
{
	const char *script = (const char *)pScript;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	// Wait for main() to finish spawning threads
	pthread_barrier_wait(&startBarrier);
	// Execute the script
	runScript(action, script);
	// Re-synchronise to let main() know it can continue..
	pthread_barrier_wait(&endBarrier);
	return pScript;
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

	pthread_attr_init(&threadAttr);
	pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

	if (strcmp(previous, "N") != 0)
	{
		size_t i, runListLen = 0;
		char **runList = NULL;
		bootScripts *scripts = fetchScripts(runlevel, "K");
		for (i = 0; i < scripts->count; i++)
		{
			char *script = toString("%s/K%s", rcPath, scripts->paths[i]);
			if (checkScript(script))
			{
				if (strcmp(runlevel, "0") != 0 && strcmp(runlevel, "6") != 0)
				{
					if (!scriptExists(previous, "S", scripts->paths[i] + 2) &&
						!scriptExists("sysinit", "S", scripts->paths[i] + 2))
					{
						printf(WARNING "WARNING:\n\n%s can't be executed because it was not started in the previous runlevel (%s)." NEWLINE,
							script, previous);
						free(script);
						continue;
					}
				}

				if ((i + 1) < scripts->count && strncmp(scripts->paths[i], scripts->paths[i + 1], 2) == 0)
				{
					size_t j = runListLen++;
					runList = rcRealloc(runList, runListLen * sizeof(char *));
					runList[j] = strdup(script);
				}
				else
				{
					if (runListLen == 0)
						runScript("stop", script);
					else
					{
						size_t j = runListLen++;
						runList = rcRealloc(runList, runListLen * sizeof(char *));
						runList[j] = strdup(script);

						pthread_barrier_init(&startBarrier, NULL, runListLen);
						pthread_barrier_init(&endBarrier, NULL, runListLen + 1);

						for (j = 0; j < runListLen; j++)
						{
							pthread_t scriptThread;
							pthread_create(&scriptThread, &threadAttr, stopScriptThread, runList[j]);
						}

						pthread_barrier_wait(&endBarrier);
						pthread_barrier_destroy(&startBarrier);
						pthread_barrier_destroy(&endBarrier);

						for (j = 0; j < runListLen; j++)
							free(runList[j]);
						free(runList);
						runList = NULL;
						runListLen = 0;
					}
				}
			}
			free(script);
		}
		freeScripts(scripts);
	}

	{
		size_t i, runListLen = 0;
		char **runList = NULL;
		bootScripts *scripts = fetchScripts(runlevel, "S");

		if (strcmp(runlevel, "0") == 0 || strcmp(runlevel, "6") == 0)
			action = "stop";
		else
			action = "start";

		for (i = 0; i < scripts->count; i++)
		{
			char *script;
			if (strcmp(previous, "N") != 0)
			{
				if (scriptExists(previous, "S", scripts->paths[i] + 2) &&
					!scriptExists(runlevel, "K", scripts->paths[i] + 2))
					continue;
			}

			script = toString("%s/S%s", rcPath, scripts->paths[i]);
			if (checkScript(script))
			{
				if ((i + 1) < scripts->count && strncmp(scripts->paths[i], scripts->paths[i + 1], 2) == 0)
				{
					size_t j = runListLen++;
					runList = rcRealloc(runList, runListLen * sizeof(char *));
					runList[j] = strdup(script);
				}
				else
				{
					if (runListLen == 0)
						runScript(action, script);
					else
					{
						size_t j = runListLen++;
						runList = rcRealloc(runList, runListLen * sizeof(char *));
						runList[j] = strdup(script);

						pthread_barrier_init(&startBarrier, NULL, runListLen);
						pthread_barrier_init(&endBarrier, NULL, runListLen + 1);

						for (j = 0; j < runListLen; j++)
						{
							pthread_t scriptThread;
							pthread_create(&scriptThread, &threadAttr, runScriptThread, runList[j]);
						}

						pthread_barrier_wait(&endBarrier);
						pthread_barrier_destroy(&startBarrier);
						pthread_barrier_destroy(&endBarrier);

						for (j = 0; j < runListLen; j++)
							free(runList[j]);
						free(runList);
						runList = NULL;
						runListLen = 0;
					}
				}
			}
			free(script);
		}
		freeScripts(scripts);
	}

	return 0;
}
