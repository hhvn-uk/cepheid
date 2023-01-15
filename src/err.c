#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "main.h"

/* see main.h for macros */

#define BOLD "\033[1m"
#define FG "\033[38;5;%dm"
#define STANDOUT BOLD FG
#define NORMAL "\033[0m"

void
_err(int code, char *type, char *file, int line, const char *func, char *fmt, ...) {
	va_list ap;
	int col = (code != CODE_WARN ? 160 : 166);

	fprintf(stderr, STANDOUT ">>>" NORMAL " At %s:%d in %s\n"
			STANDOUT ">>>" NORMAL " %s: ", col, file, line, func, col, type);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

#ifdef DEBUG
	raise(SIGTRAP);
#else
	if (code == CODE_ASSERT)
		raise(SIGABRT);
	if (code >= 0)
		exit(code);
#endif /* DEBUG */
}
