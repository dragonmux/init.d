%{

extern void source(const char *file);
char *var;

%}

%option never-interactive noyywrap

NL			\r\n
VARIABLE	^[A-Za-z][0-9A-Za-z_]+
WHITE		[ \t]*
VALUE		[/\.A-Za-z0-9_-]+
SQVALUE		'.*'
DQVALUE		\".*\"
%x			VAR
%x			VAR_VAL
%x			CLEAN

%%

{VARIABLE}				{ var = strdup(yytext); BEGIN(VAR); }
<VAR>{WHITE}={WHITE}	{ BEGIN(VAR_VAL); }
<VAR_VAL>{SQVALUE}		{ yytext[yyleng - 1] = 0; setenv(var, yytext + 1, 1); free(var); BEGIN(INITIAL); }
<VAR_VAL>{DQVALUE}		{ yytext[yyleng - 1] = 0; setenv(var, yytext + 1, 1); free(var); BEGIN(INITIAL); }
<VAR_VAL>{VALUE}		{ setenv(var, yytext, 1); free(var); BEGIN(INITIAL); }
[{NL}]|[^{NL}]			{ }

%%

void source(const char *file)
{
	FILE *stdIn = yyin;
	yyin = fopen(file, "r");
	yylex();
	fclose(yyin);
	yyin = stdIn;
}
