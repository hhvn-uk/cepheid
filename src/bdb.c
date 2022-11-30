#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "main.h"

/* Bulk db interface
 *
 * Set: 't', key, val
 * Set: 'T', key, val[], n
 *
 * Get: 't', key, &val
 * Get: 'T', key, val[], n
 *
 * Where 't'/'T' is:
 *  's' - string (will allocate memory on get)
 *  'i' - int
 *  'f' - float
 *  'v' - vector
 *  'p' - polar coordinate
 *  'S' - array of strings (will allocate memory for elements on get)
 *  'I' - array of ints
 *  'F' - array of floats
 *
 * The functions themselves require a NULL value at the end,
 * but this is appended by a macro in main.h */

#define NUMBUF 64

void
_bdbset(char *dir, char *group, ...) {
	va_list ap;
	char type;
	char *key;
	char *array;
	char buf[NUMBUF];
	union {
		char *s;
		int i;
		float f;
		Vector v;
		Polar p;
		char **S;
		int *I;
		float *F;
	} v;
	int n, i;

	va_start(ap, group);

	for (;;) {
		type = (char)va_arg(ap, int);
		if (type == 0) return;
		key = va_arg(ap, char *);

		array = NULL;

		switch (type) {
		case 's':
			v.s = va_arg(ap, char *);
			dbset(dir, group, key, v.s);
			break;
		case 'i':
			v.i = va_arg(ap, int);
			dbsetf(dir, group, key, "%d", v.i);
			break;
		case 'f':
			v.f = (float)va_arg(ap, double);
			dbsetf(dir, group, key, "%f", v.f);
			break;
		case 'v':
			v.v = va_arg(ap, Vector);
			dbsetf(dir, group, key, "%f\t%f", v.v.x, v.v.y);
			break;
		case 'p':
			v.p = va_arg(ap, Polar);
			dbsetf(dir, group, key, "%f\t%f", v.p.r, v.p.theta);
			break;
		case 'S':
			v.S = va_arg(ap, char **);
			n = va_arg(ap, int);
			array = strjoin("\t", v.S, n);
			dbset(dir, group, key, array);
			free(array);
			break;
		case 'I':
			v.I = va_arg(ap, int *);
			n = va_arg(ap, int);
			for (i = 0, array = NULL; i < n; i++) {
				snprintf(buf, sizeof(buf), "%d", v.I[i]);
				strjoinl("\t", &array, buf);
			}
			dbset(dir, group, key, array);
			free(array);
			break;
		case 'F':
			v.F = va_arg(ap, float *);
			n = va_arg(ap, int);
			for (i = 0, array = NULL; i < n; i++) {
				snprintf(buf, sizeof(buf), "%f", v.F[i]);
				strjoinl("\t", &array, buf);
			}
			dbset(dir, group, key, array);
			free(array);
			break;
		default:
			error(1, "invalid bdb type: '%c' (%d)\n", type, (int)type);
		}
	}

	va_end(ap);
}

#define ARRAYMAX 64

void
_bdbget(char *dir, char *group, ...) {
	va_list ap;
	char type;
	char *key;
	char *str;
	char *list[ARRAYMAX];
	size_t len;
	union {
		char **s;
		int *i;
		float *f;
		Vector *v;
		Polar *p;
		char **S;
		int *I;
		float *F;
	} v;
	int i, n;

	va_start(ap, group);

	for (;;) {
		type = va_arg(ap, int);
		if (type == 0) return;
		key = va_arg(ap, char *);

		if (isupper(type)) {
			str = nstrdup(dbget(dir, group, key));
			len = strsplit(str, "\t", list, ARRAYMAX);
		}

		switch (type) {
		case 's':
			v.s = va_arg(ap, char **);
			*v.s = nstrdup(dbget(dir, group, key));
			break;
		case 'i':
			v.i = va_arg(ap, int *);
			dbgetf(dir, group, key, "%d", v.i);
			break;
		case 'f':
			v.f = va_arg(ap, float *);
			dbgetf(dir, group, key, "%f", v.f);
			break;
		case 'v':
			v.v = va_arg(ap, Vector *);
			dbgetf(dir, group, key, "%f\t%f",
					&(*v.v).x, &(*v.v).y);
			break;
		case 'p':
			v.p = va_arg(ap, Polar *);
			dbgetf(dir, group, key, "%f\t%f",
					&(*v.p).r, &(*v.p).theta);
			break;
		case 'S':
			v.S = va_arg(ap, char **);
			n = va_arg(ap, int);
			for (i = 0; i < n; i++)
				v.S[i] = i < len ? nstrdup(list[i]) : NULL;
			break;
		case 'I':
			v.I = va_arg(ap, int *);
			n = va_arg(ap, int);
			for (i = 0; i < n; i++)
				v.I[i] = i < len ? atoi(list[i]) : 0;
			break;
		case 'F':
			v.F = va_arg(ap, float *);
			n = va_arg(ap, int);
			for (i = 0; i < n; i++)
				v.F[i] = i < len ? strtof(list[i], NULL) : 0;
			break;
		default:
			error(1, "invalid bdb type: '%c'\n", type);
		}

		if (isupper(type))
			free(str);
	}
}
