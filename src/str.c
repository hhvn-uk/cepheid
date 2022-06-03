#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

char *
vsmprintf(char *fmt, va_list args) {
	va_list ap;
	int size;
	char *ret;

	va_copy(ap, args);
	size = vsnprintf(NULL, 0, fmt, ap) + 1;
	va_end(ap);

	if (size == 0) /* -1 */
		return NULL;

	ret = malloc(size);
	if (!ret) return NULL;

	va_copy(ap, args);
	vsnprintf(ret, size, fmt, ap);
	va_end(ap);
	return ret;
}

char *
smprintf(char *fmt, ...) {
	va_list ap;
	char *ret;

	va_start(ap, fmt);
	ret = vsmprintf(fmt, ap);
	va_end(ap);
	return ret;
}

char *
vsbprintf(char *fmt, va_list args) {
	static char *ret = NULL;
	va_list ap;

	free(ret);
	va_copy(ap, args);
	ret = vsmprintf(fmt, args);
	va_end(ap);
	return ret;
}

char *
sbprintf(char *fmt, ...) {
	va_list ap;
	char *ret;

	va_start(ap, fmt);
	ret = vsbprintf(fmt, ap);
	va_end(ap);
	return ret;
}
