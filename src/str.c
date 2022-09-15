#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <wchar.h>
#include <errno.h>
#include <math.h>
#include "main.h"

#define FMEMMAX 1024

static void *fmem[FMEMMAX] = {NULL};
static int fmemi = 0;

/* allocate for a frame */
void *
falloc(size_t size) {
	if (fmemi + 1 == FMEMMAX) {
		errno = ENOMEM;
		return NULL;
	}
	return (fmem[fmemi++] = malloc(size));
}

void
ffree(void) {
	while (fmemi--) {
		free(fmem[fmemi]);
		fmem[fmemi] = NULL;
	}
	fmemi = 0;
}

char *
vsfprintf(char *fmt, va_list ap) {
	if (fmemi + 1 == FMEMMAX) {
		errno = ENOMEM;
		return NULL;
	}
	return (fmem[fmemi++] = vsmprintf(fmt, ap));
}

char *
sfprintf(char *fmt, ...) {
	va_list ap;
	char *ret;

	va_start(ap, fmt);
	ret = vsfprintf(fmt, ap);
	va_end(ap);
	return ret;
}

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
strkm(float km) {
	if (km > GIGA)
		return sfprintf("%.2fb km", km / GIGA);
	else if (km > MEGA)
		return sfprintf("%.2fm km", km / MEGA);
	else if (km > KILO)
		return sfprintf("%.2fk km", km / KILO);
	else
		return sfprintf("%.2f km", km);
}

char *
strly(float km) {
	float ls = km * KILO / C_MS;

	if (ls > YEAR_LEN)
		return sfprintf("%.2f ly", ls / YEAR_LEN);
	else if (ls > DAY_LEN)
		return sfprintf("%.2f ld", ls / DAY_LEN);
	else if (ls > HOUR_LEN)
		return sfprintf("%.2f lh", ls / HOUR_LEN);
	else if (ls > MIN_LEN)
		return sfprintf("%.2f lm", ls / MIN_LEN);
	else
		return sfprintf("%.2f ls", ls);
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

void
edittrunc(wchar_t *str, int *len, int *cur) {
	*len = *cur = 0;
	str[0] = '\0';
}

void
editrm(wchar_t *str, int *len, int *cur) {
	wmemmove(str + *cur - 1, str + *cur, *len - *cur);
	str[--(*len)] = '\0';
	(*cur)--;
}

void
editins(wchar_t *str, int *len, int *cur, int size, wchar_t c) {
	if (*len >= size)
		return;
	wmemmove(str + *cur + 1, str + *cur, *len - *cur);
	str[*cur] = c;
	str[++(*len)] = '\0';
	(*cur)++;
}
