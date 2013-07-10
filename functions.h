#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define COLOUR(Code) "\x1B["Code"m"
#define NORMAL COLOUR("0;39")
#define SUCCESS COLOUR("1;32")
#define WARNING COLOUR("1;33")
#define FAILURE COLOUR("1;31")
#define INFO COLOUR("1;36")
#define BRACKET COLOUR("1;34")

#define CURS_UP "\x1B[1A\x1B[0G"
#define SET_COL "\x1B[%dG"

#define NEWLINE NORMAL "\x1B[1A\x1B[s\x1B[1B\n"
#define MOVE_END_PRINTED "\x1B[u"

#define CHECK_ARG(num, val) strcmp(argv[num], val) == 0

#define COL(val) val - 8
#define WCOL(val) val - 2

#define TRUE 1
#define FALSE 0

#define LOAD_FORCE				1
#define LOAD_USE_NICE			2

/* Let the program run with no stdout */
#define RUN_PROC_NO_STDOUT		0
/* Let the program run with stdout piped to a buffer returned through stdOut */
#define RUN_PROC_RET_STDOUT		1
/* Let the program run with stdout left alone */
#define RUN_PROC_PASS_STDOUT	2
#define RUN_PROC_STDOUT_MASK	(RUN_PROC_RET_STDOUT | RUN_PROC_PASS_STDOUT)
#define RUN_PROC_NO_WAIT		4
#define RUN_PROC_PUT_STDIN		8

#define STATUS_TYPE_SUCCESS		0
#define STATUS_TYPE_WARNING		1
#define STATUS_TYPE_ERROR		2

#define STATUS_NONE				0
#define STATUS_RUNNING			1
#define STATUS_NOT_RUNNING		2
#define STATUS_NOT_AVAILABLE	3

#define NSECS_IN_USEC			1000000
#define NSECS_IN_SEC			1000000000

typedef struct _WaitInfo
{
	pthread_mutex_t *waitMutex;
	pthread_cond_t *waitCondition;
	pid_t pid;
} WaitInfo;

extern void source(const char *file);
int vaRunProcess(uint32_t numParams, uint32_t flags, char **stdOut, char *stdIn, const char *cmd, va_list args);
int sysRunProcess(uint32_t flags, char **stdOut, char *stdIn, const char **argv);

int runProcess(uint32_t numParams, uint32_t flags, char **stdOut, char *stdIn, const char *cmd, ...)
{
	int ret;
	va_list args;
	va_start(args, cmd);
	ret = vaRunProcess(numParams, flags, stdOut, stdIn, cmd, args);
	va_end(args);
	return ret;
}

int vaRunProcess(uint32_t numParams, uint32_t flags, char **stdOut, char *stdIn, const char *cmd, va_list args)
{
	uint32_t i = 0;
	const char **cmdArgs = malloc(sizeof(char *) * (numParams + 1));
	if (cmdArgs == NULL)
	{
		fprintf(stderr, FAILURE "Error allocating memory for the command line arguments of the child process" NEWLINE);
		exit(1);
		return 1;
	}
	cmdArgs[0] = cmd;
	for (i = 1; i < numParams; i++)
		cmdArgs[i] = va_arg(args, const char *);
	cmdArgs[numParams] = NULL;
	return sysRunProcess(flags, stdOut, stdIn, cmdArgs);
}

int sysRunProcess(uint32_t flags, char **stdOut, char *stdIn, const char **argv)
{
	pid_t pid;
	int pio[2], ret, bytesToRead, poi[2];

	/* Sanity check stdOut's value if we're meant to fill it with the stdOut
	 * of the program.. */
	if ((flags & RUN_PROC_RET_STDOUT) != 0 && stdOut == NULL)
		return 1;
	if ((flags & RUN_PROC_PASS_STDOUT) == 0)
	{
		if (pipe(pio) < 0)
		{
			fprintf(stderr, FAILURE "Error setting up a pipe to launch a child process with" NEWLINE);
			return 1;
		}
	}
	if ((flags & RUN_PROC_PUT_STDIN) != 0)
	{
		if (pipe(poi) < 0)
		{
			fprintf(stderr, FAILURE "Error setting up a pipe to launch a child process with" NEWLINE);
			return 1;
		}
	}

	pid = fork();
	if (pid < 0)
	{
		fprintf(stderr, FAILURE "Error forking this process in order to launch a child process" NEWLINE);
		return 1;
	}
	else if (pid > 0)
	{
		int ret;
		if ((flags & RUN_PROC_PASS_STDOUT) == 0)
			close(pio[1]);
		if ((flags & RUN_PROC_PUT_STDIN) != 0)
		{
			close(poi[0]);
			write(poi[1], stdIn, strlen(stdIn));
			close(poi[1]);
		}
		if ((flags & RUN_PROC_NO_WAIT) == 0)
		{
			while (pid != waitpid(pid, &ret, 0));
			free(argv);
		}
		else
			ret = 0;
		if ((flags & RUN_PROC_STDOUT_MASK) == 0)
			close(pio[0]);
		/*if ((flags & RUN_PROC_PUT_STDIN) != 0)
			close(poi[1]);*/
		if ((flags & RUN_PROC_RET_STDOUT) == 0)
			return WEXITSTATUS(ret);
	}
	else
	{
		if ((flags & RUN_PROC_PASS_STDOUT) == 0)
		{
			dup2(pio[1], 1);
			close(pio[0]);
		}
		if ((flags & RUN_PROC_PUT_STDIN) != 0)
		{
			dup2(poi[0], 0);
			close(poi[1]);
		}
		if ((flags & RUN_PROC_NO_WAIT) != 0)
		{
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			setsid();
		}
		if (execvp(argv[0], (char **)argv) < 0)
		{
			fprintf(stderr, FAILURE "Error launching the child process after forking" NEWLINE);
			if ((flags & RUN_PROC_PASS_STDOUT) == 0)
				close(pio[1]);
			exit(2);
			return 2;
		}
		exit(0);
		return 0;
	}
	/* The following code is only reached if flags has RUN_PROC_RET_STDOUT */
	ret = ioctl(pio[0], FIONREAD, &bytesToRead);
	if (ret < -1)
	{
		/* TODO: Program vaRunProcess to be able to cope with ioctl not working */
	}
	else
	{
		int nRead = 0;
		char *retStdOut = (char *)malloc(bytesToRead + 1);
		if (retStdOut == NULL)
		{
			close(pio[0]);
			fprintf(stderr, "\nError allocating memory to hold the output of the child process\n");
			return 1;
		}
		do
		{
			ret = read(pio[0], retStdOut + nRead, bytesToRead - nRead);
			if (ret < 0)
			{
				perror(NULL);
				close(pio[0]);
				return 1;
			}
			else
				nRead += ret;
		}
		while (nRead < bytesToRead);
		close(pio[0]);
		retStdOut[bytesToRead] = 0;
		*stdOut = retStdOut;
	}
	return 0;
}

void *rcMalloc(size_t size)
{
	void *ret = malloc(size);
	if (ret == NULL)
	{
		printf(FAILURE "Memory allocation failure in bootscript controller!" NEWLINE);
		exit(1);
	}
	return ret;
}

void *rcRealloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);
	if (ret == NULL)
	{
		free(ptr);
		printf(FAILURE "Memory reallocation failure in bootscript controller!" NEWLINE);
		exit(1);
	}
	return ret;
}

char *toString(char *Fmt, ...)
{
	va_list args;
	char *Str;
	int lenStr;
	va_start(args, Fmt);
	lenStr = vsnprintf(NULL, 0, Fmt, args) + 1;
	va_end(args);
	Str = rcMalloc(lenStr);
	va_start(args, Fmt);
	vsprintf(Str, Fmt, args);
	va_end(args);
	return Str;
}

uint8_t isDigit(char x)
{
	if (x >= '0' && x <= '9')
		return TRUE;
	return FALSE;
}

int strToInt(const char *str)
{
	size_t i = 0;
	int32_t ret = 0;
	if (isDigit(str[0]) == FALSE)
		i = 1;
	for (; i < strlen(str); i++)
	{
		if (isDigit(str[i]) == FALSE)
			break;
		ret *= 10;
		ret += str[i] - '0';
	}
	if (str[0] == '-')
		ret = -ret;
	return ret;
}

int getColumns()
{
	struct winsize win;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &win);
	return (win.ws_col == 0 ? 80 : win.ws_col);
}

int echoOk()
{
	printf(CURS_UP SET_COL BRACKET "[" SUCCESS "  OK  " BRACKET "]" NEWLINE, COL(getColumns()));
	return 0;
}

int echoFailure()
{
	printf(CURS_UP SET_COL BRACKET "[" FAILURE " FAIL " BRACKET "]" NEWLINE, COL(getColumns()));
	return 1;
}

void echoWarning()
{
	printf(CURS_UP SET_COL BRACKET "[" WARNING " WARN " BRACKET "]" NEWLINE,
		COL(getColumns()));
}

void printStatus(uint32_t Status, uint32_t extraInfo)
{
	switch (Status)
	{
		case STATUS_TYPE_SUCCESS:
			echoOk();
			return;
		case STATUS_TYPE_WARNING:
			switch (extraInfo)
			{
				case STATUS_RUNNING:
					printf(CURS_UP MOVE_END_PRINTED WARNING "   Already running." NEWLINE);
					break;
				case STATUS_NOT_RUNNING:
					printf(CURS_UP MOVE_END_PRINTED WARNING "   Not running." NEWLINE);
					break;
				case STATUS_NOT_AVAILABLE:
					printf(CURS_UP MOVE_END_PRINTED WARNING "   Not available." NEWLINE);
					break;
			}
			echoWarning();
			return;
		case STATUS_TYPE_ERROR:
			echoFailure();
			return;
	}
}

int evaluateRetVal(int returnValue)
{
	if (returnValue == 0)
		return echoOk();
	else
		return echoFailure();
}

int fileExists(const char *file)
{
	struct stat statRet;
	if (stat(file, &statRet) == 0)
		return TRUE;
	return FALSE;
}

int normalFileExists(const char *file)
{
	struct stat statRet;
	if (stat(file, &statRet) == 0 && S_ISREG(statRet.st_mode))
		return TRUE;
	return FALSE;
}

int fileSymlink(const char *file)
{
	struct stat statRet;
	if (lstat(file, &statRet) == 0 && S_ISLNK(statRet.st_mode))
		return TRUE;
	return FALSE;
}

int fileExecutable(const char *file)
{
	if (access(file, X_OK) == 0)
		return TRUE;
	return FALSE;
}

int dirExists(const char *dir)
{
	struct stat statRet;
	if (stat(dir, &statRet) == 0 && S_ISDIR(statRet.st_mode))
		return TRUE;
	return FALSE;
}

void logWarningMsg(const char *Msg, ...)
{
	va_list args;
	va_start(args, Msg);
	vprintf(Msg, args);
	va_end(args);
	printf(SET_COL BRACKET "[" WARNING " WARN " BRACKET "]" NEWLINE, COL(getColumns()));
}

void logFailureMsg(const char *Msg, ...)
{
	va_list args;
	va_start(args, Msg);
	vprintf(Msg, args);
	va_end(args);
	printf(SET_COL BRACKET "[" FAILURE " FAIL " BRACKET "]" NEWLINE, COL(getColumns()));
}

char *readLine(const char *file, uint32_t lineNumber)
{
	uint32_t i;
	char c, *ret;
	FILE *f_in = fopen(file, "r");
	if (f_in == NULL)
		return NULL;
	for (i = 1; i < lineNumber; i++)
	{
		do
		{
			fread(&c, 1, 1, f_in);
			if (feof(f_in) != FALSE)
			{
				fclose(f_in);
				return NULL;
			}
		}
		while (c != '\n');
	}
	ret = malloc(0);
	i = 0;
	do
	{
		i++;
		fread(&c, 1, 1, f_in);
		ret = realloc(ret, i);
		ret[i - 1] = c;
		if (feof(f_in) != FALSE)
			break;
	}
	while (c != '\n');
	if (c == '\n')
		ret[i - 1] = 0;
	else
	{
		ret = realloc(ret, i + 1);
		ret[i] = 0;
	}
	fclose(f_in);
	return ret;
}

#define whiteSpace "\t \r\n"

/* FIXME: use of strtok()!! */
char **whitespaceTokenise(const char *pidStr, uint32_t *const numPIDs)
{
	char **ret, *currTok, *strtokPtr, *ptrStart;
	uint32_t nPIDret = 0;

	ptrStart = strdup(pidStr);
	for (strtokPtr = ptrStart; ; strtokPtr = NULL, nPIDret++)
	{
		currTok = strtok(strtokPtr, whiteSpace);
		if (currTok == NULL)
			break;
	}
	free(ptrStart);
	ret = malloc(sizeof(char *) * nPIDret);
	for (strtokPtr = (char *)pidStr, nPIDret = 0; ; strtokPtr = NULL, nPIDret++)
	{
		currTok = strtok(strtokPtr, whiteSpace);
		if (currTok == NULL)
			break;
		ret[nPIDret] = currTok;
	}

	*numPIDs = nPIDret;
	return ret;
}

#undef whiteSpace

void checkPIDs(pid_t **pPIDs, uint32_t *p_nPIDs)
{
	pid_t *retPIDs, *inPIDs;
	uint32_t i, ret_nPIDs, j;
	inPIDs = *pPIDs;
	ret_nPIDs = 0;
	for (i = 0; i < *p_nPIDs; i++)
	{
		/* skip over non-process PIDs */
		if (inPIDs[i] < 1)
			continue;
		/* This checks if the process is running and that it's not in any
		 *  of the ESRCH states listed on http://linux.die.net/man/2/kill */
		if (kill(inPIDs[i], 0) != 0)
			inPIDs[i] = 0;
		else
			ret_nPIDs++;
	}
	retPIDs = malloc(sizeof(pid_t) * ret_nPIDs);
	for (j = 0, i = 0; i < *p_nPIDs; i++)
	{
		if (inPIDs[i] > 0)
			retPIDs[j++] = inPIDs[i];
	}
	free(inPIDs);
	*pPIDs = retPIDs;
	*p_nPIDs = ret_nPIDs;
}

pid_t *pidArrayFromString(char *pidStr, uint32_t *nPIDs)
{
	uint32_t i, numPIDs;
	pid_t *pidArray;
	char **pids = whitespaceTokenise(pidStr, &numPIDs);
	pidArray = malloc(sizeof(pid_t) * numPIDs);
	for (i = 0; i < numPIDs; i++)
		pidArray[i] = (pid_t)strToInt(pids[i]);
	free(pidStr);
	free(pids);
	checkPIDs(&pidArray, &numPIDs);
	*nPIDs = numPIDs;
	return pidArray;
}

#define FILL_RETURN_VALUES() \
	if (p_pid != NULL && nPIDs != NULL) \
	{ \
		*p_pid = pidArray; \
		*nPIDs = numPIDs; \
	} \
	else \
		free(pidArray)

/* This rewrite completely drops support for the silent option/parameter */
int pidOfProc(pid_t **const p_pid, uint32_t *const nPIDs, const char *proc, const char *pidFile)
{
	uint32_t numPIDs;
	char *pidStr;
	pid_t *pidArray;
	if (pidFile != NULL)
	{
		if (euidaccess(pidFile, R_OK) != 0)
			return 2;
		pidStr = readLine(pidFile, 1);
		if (pidStr == NULL)
			/* No PIDs read, but the pidFile existed - Program is dead */
			return 1;

		pidArray = pidArrayFromString(pidStr, &numPIDs);
		FILL_RETURN_VALUES();
		if (numPIDs == 0)
			/* No PIDs read, but the pidFile existed - Program is dead */
			return 1;
		else
			/* Program was fine */
			return 0;
	}
	else
	{
		runProcess(3, RUN_PROC_RET_STDOUT, &pidStr, NULL, "pidof", "-x", proc, NULL);
		pidArray = pidArrayFromString(pidStr, &numPIDs);
		FILL_RETURN_VALUES();
		if (numPIDs == 0)
			return 2;
		else
			return 0;
	}
}

void printfPIDs(pid_t *PIDs, uint32_t nPIDs)
{
	uint32_t i;
	for (i = 0; i < nPIDs; i++)
	{
		printf("%ld", (long)PIDs[i]);
		if (i + 1 < nPIDs)
			printf(" ");
	}
}

#undef FILL_RETURN_VALUES

int statusProc(const char *proc, const char *pidFile)
{
	int ret;
	uint32_t nPIDs;
	pid_t *PIDs;
	const char *base = strrchr(proc, '/');
	if (base == NULL)
		base = proc;
	else
		base++;

	ret = pidOfProc(&PIDs, &nPIDs, proc, pidFile);
	if (nPIDs != 0)
	{
		printf(INFO "%s is running with Process ID(s) ", base);
		printfPIDs(PIDs, nPIDs);
		printf("." NEWLINE);
	}
	else
	{
		if (ret > 0)
			printf(WARNING "%s is not running but /var/run/%s.pid exists." NEWLINE, proc, base);
		else
			printf(INFO "%s is not running." NEWLINE, proc);
	}
	free(PIDs);

	return ret;
}

int reloadProc(const char *proc, const char *pidFile)
{
	uint32_t nPIDs, i;
	pid_t *PIDs;
	int ret;

	ret = pidOfProc(&PIDs, &nPIDs, proc, pidFile);
	if (nPIDs == 0 || ret > 0)
	{
		printf(WARNING "Process %s not running." NEWLINE, proc);
		echoWarning();
		return 0;
	}
	else
	{
		for (i = 0; i < nPIDs; i++)
			ret |= (kill(PIDs[i], SIGHUP) == 0 ? 0 : 1);
		return evaluateRetVal(ret);
	}
}

int loadProc(const char *proc, const char *pidFile, uint32_t nice, uint32_t flags, uint32_t numParams, ...)
{
	int ret;
	char *real_nice;
	const char **niceArgs;
	if ((flags & LOAD_FORCE) == 0)
	{
		int ret = pidOfProc(NULL, NULL, proc, pidFile);
		switch (ret)
		{
			case 0:
				logWarningMsg("Unable to continue: %s is running", proc);
				return 0;
			case 1:
				logWarningMsg("Unable to continue: %s exists", pidFile);
				return 0;
			case 2:
				break;
			default:
				logFailureMsg("Unknown error code from pidOfProc(): %d", ret);
				return 4;
		}
	}
	if ((flags & LOAD_USE_NICE) == 0)
		real_nice = toString("%d", 10);
	else
		real_nice = toString("%u", nice);
	{
		va_list args;
		uint32_t i;
		niceArgs = malloc(sizeof(char *) * (numParams + 5));
		niceArgs++;
		niceArgs[0] = "-n";
		niceArgs[1] = real_nice;
		niceArgs[2] = proc;
		va_start(args, numParams);
		for (i = 0; i < numParams; i++)
			niceArgs[i + 3] = va_arg(args, const char *);
		va_end(args);
		niceArgs[i + 3] = NULL;
		niceArgs--;
		niceArgs[0] = "nice";
	}
	ret = sysRunProcess(RUN_PROC_PASS_STDOUT, NULL, NULL, niceArgs);
	free(real_nice);
	evaluateRetVal(ret);
	return 0;
}

void *waitpidThread(void *p_waitInfo)
{
	WaitInfo *waitInfo = p_waitInfo;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	while (1)
	{
		if (kill(waitInfo->pid, 0) == 0)
			usleep(100000);
		else
			break;
	}
	//waitpid(waitInfo->pid, NULL, 0);

	pthread_mutex_lock(waitInfo->waitMutex);
	pthread_cond_signal(waitInfo->waitCondition);
	pthread_mutex_unlock(waitInfo->waitMutex);
	return 0;
}

void killPID(pid_t pid, uint32_t waitTime)
{
	WaitInfo *waitInfo;
	pthread_t waitThread;
	pthread_attr_t waitTAttrs;
	pthread_mutex_t *waitMutex;
	pthread_mutexattr_t waitMAttrs;
	pthread_cond_t *waitCondition;
	pthread_condattr_t waitCAttrs;
	struct timespec ts;
	struct timeval tv;

	waitMutex = malloc(sizeof(pthread_mutex_t));
	waitCondition = malloc(sizeof(pthread_cond_t));
	waitInfo = malloc(sizeof(WaitInfo));

	pthread_mutexattr_init(&waitMAttrs);
	pthread_mutexattr_settype(&waitMAttrs, PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutex_init(waitMutex, &waitMAttrs);
	pthread_mutexattr_destroy(&waitMAttrs);

	pthread_condattr_init(&waitCAttrs);
	pthread_cond_init(waitCondition, &waitCAttrs);
	pthread_condattr_destroy(&waitCAttrs);

	pthread_attr_init(&waitTAttrs);
	pthread_attr_setdetachstate(&waitTAttrs, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&waitTAttrs, PTHREAD_SCOPE_PROCESS);

	waitInfo->waitMutex = waitMutex;
	waitInfo->waitCondition = waitCondition;
	waitInfo->pid = pid;

	pthread_mutex_lock(waitMutex);

	gettimeofday(&tv, NULL);
	ts.tv_nsec = (tv.tv_usec * NSECS_IN_USEC);
	ts.tv_sec = tv.tv_sec + (ts.tv_nsec / NSECS_IN_SEC) + waitTime;
	ts.tv_nsec %= NSECS_IN_SEC;

	pthread_create(&waitThread, &waitTAttrs, waitpidThread, waitInfo);

	if (pthread_cond_timedwait(waitCondition, waitMutex, &ts) == ETIMEDOUT)
	{
		pthread_cancel(waitThread);
		pthread_join(waitThread, NULL);
	}

	pthread_mutex_unlock(waitMutex);
	pthread_cond_destroy(waitCondition);
	free(waitCondition);
	pthread_mutex_destroy(waitMutex);
	free(waitMutex);
	free(waitInfo);
	pthread_attr_destroy(&waitTAttrs);
}

/* Use signal = -1 to let the function use the default. */
int killProc(const char *proc, const char *pidFile, int signal, uint32_t _waitTime)
{
	uint32_t nPIDs, i, waitTime;
	pid_t *PIDs;
	int ret, real_signal = (signal == -1 ? SIGTERM : signal);

	ret = pidOfProc(&PIDs, &nPIDs, proc, pidFile);
	if (nPIDs == 0 || ret > 0)
	{
		printStatus(STATUS_TYPE_WARNING, STATUS_NOT_RUNNING);
		return 1;
	}

	waitTime = (_waitTime != 0 ? _waitTime : 3);
	for (i = 0; i < nPIDs; i++)
	{
		ret = (kill(PIDs[i], real_signal) == 0 ? 0 : 1);
		if (real_signal == SIGTERM || real_signal == SIGKILL)
		{
			killPID(PIDs[i], waitTime);

			if (kill(PIDs[i], 0) == 0)
				/* Yes, SIGKILL it */
				ret = (kill(PIDs[i], SIGKILL) == 0 ? 0 : 1);
		}
	}
	free(PIDs);

	if (nPIDs != 0 && (real_signal == SIGTERM || real_signal == SIGKILL))
	{
		ret = pidOfProc(NULL, NULL, proc, pidFile);
		if (ret != 0)
		{
			/* Program was terminated */
			if (normalFileExists(pidFile) == TRUE)
				unlink(pidFile);
			return echoOk();
		}
		else
		{
			/* Program is still running */
			echoFailure();
			return 2;
		}
	}
	else
		return evaluateRetVal(ret);
}

char *getHostName()
{
	uint32_t lenRet = 1;
	char *ret = malloc(lenRet);
	do
	{
		lenRet++;
		ret = realloc(ret, lenRet);
		gethostname(ret, lenRet);
		if (ret[lenRet - 2] == 0)
			return ret;
	}
	while (ret[lenRet - 2] != 0);
	free(ret);
	return NULL;
}

void writeNewline(const char *fileName)
{
	FILE *file = fopen(fileName, "wb");
	if (file == NULL)
		return;
	fwrite("\n", 1, 1, file);
	fclose(file);
}

void readConsoleLine()
{
	char c;
	do
	{
		fread(&c, 1, 1, stdin);
	}
	while (c != '\r' && c != '\n');
}
