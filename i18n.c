#include "functions.h"

/* Loosely based on the i18n shell script from the LFS bootscripts package.
 * Completely rewritten in C by Rachel Mant - rachel@rachelmant.com
 *
 * Revision 0.1 - Rachel Mant, 2012-01-24 17:50 +0000 (Tuesday, 08 Mar 2011)
 */

#define FONTTRANS	"-m %s"
#define TTY_FORMAT	"/dev/tty%u"

int usage(char *name)
{
	printf("Usage: %s {start}\n", name);
	return 1;
}

#define RUNPROC(nArgs, mode, stdoutBuf, stdinBuf, ...) \
	ret |= runProcess(nArgs, mode, stdoutBuf, stdinBuf, __VA_ARGS__ NULL)

#define FONT_SETTER(nArgs, ...) \
	RUNPROC(nArgs, RUN_PROC_NO_STDOUT, NULL, NULL, "setfont", __VA_ARGS__ "-C", tty, )
#define DUMPKEYS(nArgs, buffer, ...) \
	RUNPROC(nArgs, RUN_PROC_RET_STDOUT, &buffer, NULL, "dumpkeys", __VA_ARGS__)
#define LOADKEYS(buffer) \
	RUNPROC(2, RUN_PROC_PUT_STDIN, NULL, buffer, "loadkeys", "--unicode", )

int main(int argc, char **argv)
{
	int ret;
	if (argc < 2)
		return usage(argv[0]);

	if (fileExists("/etc/sysconfig/i18n"))
		source("/etc/sysconfig/i18n");

	if (CHECK_ARG(1, "start"))
	{
		char *dumpkeysCharset, *ttyOp, *ttyEncoding;
		uint8_t i, failure = FALSE;
		int ret = 0;

		printf(INFO "Setting Font..." NEWLINE);
		{
			char *font, *fontTrans = NULL;
			if (getenv("FONT_TRANSLATION") != NULL)
				fontTrans = toString(FONTTRANS, getenv("FONT_TRANSLATION"));
			font = getenv("FONT");
			for (i = 0; i < 12; i++)
			{
				char *tty = toString(TTY_FORMAT, i + 1);
				if (font == NULL && fontTrans == NULL)
					FONT_SETTER(4, );
				else if (font == NULL)
					FONT_SETTER(5, fontTrans, );
				else if (fontTrans == NULL)
					FONT_SETTER(5, font, );
				else
					FONT_SETTER(6, fontTrans, font, );
				free(tty);
			}
			free(fontTrans);
		}
		evaluateRetVal(ret);

		printf(INFO "Setting keymap to UTF-8..." NEWLINE);
		dumpkeysCharset = getenv("DUMPKEYS_CHARSET");
		{
			char *unicode = getenv("UNICODE");
			if (unicode != NULL && strcasecmp(unicode, "yes") == 0)
			{
				char *dumpkeys;
				if (dumpkeysCharset == NULL || strlen(dumpkeysCharset) == 0)
					DUMPKEYS(1, dumpkeys, );
				else
					DUMPKEYS(3, dumpkeys, "-c", dumpkeysCharset, );
				LOADKEYS(dumpkeys);
				ttyOp = "\033%G";
				ttyEncoding = "Unicode";
			}
			else
			{
				ttyOp = "\033(K";
				ttyEncoding = "ASCII";
			}
		}
		evaluateRetVal(ret);

		printf(INFO "Enabling Multibyte input..." NEWLINE);
		evaluateRetVal(runProcess(2, RUN_PROC_NO_STDOUT, NULL, NULL, "kbd_mode", "-u", NULL));

		printf(INFO, "Setting up keymaps..." NEWLINE);
		{
			char *windowsKeyboard = getenv("WINDOWS_KEYBOARD");
			/*if (windowsKeyboard == NULL || strlen(windowsKeyboard) == 0)
				LOADKEYS_Q(*/
			//loadkeys -q ${WINDOW_KEYMAP} ${KEYMAP} ${EXTENDED_KEYMAPS}
		}
		evaluateRetVal(ret);

		printf(INFO "Setting encoding to %s.." NEWLINE, ttyEncoding);
		for (i = 0; i < 12; i++)
		{
			char *tty = toString(TTY_FORMAT, i + 1);
			FILE *fTTY = fopen(tty, "w");
			if (fTTY == NULL)
			{
				ret = 1;
				break;
			}
			ret |= (fwrite(ttyOp, 1, strlen(ttyOp), fTTY) != strlen(ttyOp));
			ret |= (fclose(fTTY) != 0);
			if (ret != 0)
				break;
		}
		evaluateRetVal(ret);
	}
	else
		return usage(argv[0]);
	return 0;
}
