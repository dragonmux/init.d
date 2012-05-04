#include <stdio.h>
#include <unistd.h>

extern char **environ;
extern void source(const char *file);
extern int yylex();

int main(int argc, char **argv)
{
	char *var;
	int i;
	clearenv();
	if (argc > 1)
		source(argv[1]);
	else
		yylex();
	for (i = 0, var = environ[i]; var != NULL; i++, var = environ[i])
		printf("%s\n", var);
	return 0;
}
