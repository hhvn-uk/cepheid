#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include "main.h"

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
nstrdup(char *str) {
	if (!str) {
		errno = EINVAL;
		return NULL;
	}
	return strdup(str);
}

char *
strkmdist(float km) {
	static char *ret = NULL;

	free(ret);

	if (km > GIGA)
		ret = smprintf("%.2fb km", km / GIGA);
	else if (km > MEGA)
		ret = smprintf("%.2fm km", km / MEGA);
	else if (km > KILO)
		ret = smprintf("%.2fk km", km / KILO);
	else
		ret = smprintf("%.2f km", km);

	return ret;
}

char *
strlightdist(float km) {
	static char *ret = NULL;
	float ls = km * KILO / C_MS;

	free(ret);

	if (ls > YEAR_LEN)
		ret = smprintf("%.2f ly", ls / YEAR_LEN);
	else if (ls > DAY_LEN)
		ret = smprintf("%.2f ld", ls / DAY_LEN);
	else if (ls > HOUR_LEN)
		ret = smprintf("%.2f lh", ls / HOUR_LEN);
	else if (ls > MIN_LEN)
		ret = smprintf("%.2f lm", ls / MIN_LEN);
	else
		ret = smprintf("%.2f ls", ls);

	return ret;
}

int
streq(char *s1, char *s2) {
	if (s1 == s2)
		return 1;
	else if (!s1 || !s2)
		return 0;
	else if (strcmp(s1, s2) == 0)
		return 1;
	else
		return 0;
}

int
strprefix(char *str, char *prefix) {
	if (str == prefix)
		return 1;
	else if (!str || !prefix)
		return 0;
	else
		return strncmp(str, prefix, strlen(prefix)) == 0;
}

char *
strsuffix(char *str, char *suffix) {
	size_t len, slen;

	if (str == suffix)
		return str;
	else if (!str || !suffix)
		return NULL;
	else if ((len = strlen(str)) < (slen = strlen(suffix)))
		return NULL;

	if (strcmp(str + len - slen, suffix) == 0)
		return str + len - slen;
	else
		return NULL;
}

int
strlistpos(char *str, char **list, size_t len) {
	int i;

	for (i = 0; i < INT_MAX && i < len; i++)
		if (streq(str, *(list + i)))
			return i;
	return -1;
}

float
strnum(char *str) {
	if (!str) return 0;
	return strtof(str, NULL);
}
