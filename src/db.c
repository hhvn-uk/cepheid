#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "main.h"

int
vdbsetf(char *dir, char *group, char *key, char *fmt, va_list args) {
	va_list ap;
	char *str;
	int ret;

	va_copy(ap, args);
	str = vsmprintf(fmt, ap);
	va_end(ap);
	ret = dbset(dir, group, key, str);
	free(str);
	return ret;
}

int
dbsetf(char *dir, char *group, char *key, char *fmt, ...) {
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vdbsetf(dir, group, key, fmt, ap);
	va_end(ap);
	return ret;
}

int
dbsetint(char *dir, char *group, char *key, int val) {
	return dbsetf(dir, group, key, "%d", val);
}

int
dbsetfloat(char *dir, char *group, char *key, float val) {
	return dbsetf(dir, group, key, "%f", val);
}

int
vdbgetf(char *dir, char *group, char *key, char *fmt, va_list args) {
	va_list ap;
	char *str;
	int ret;

	str = dbget(dir, group, key);
	if (!str)
		return EOF;
	va_copy(ap, args);
	ret = vsscanf(str, fmt, ap);
	va_end(ap);
	free(str);
	return ret;
}

int
dbgetf(char *dir, char *group, char *key, char *fmt, ...) {
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vdbgetf(dir, group, key, fmt, ap);
	va_end(ap);
	return ret;
}

int
dbgetint(char *dir, char *group, char *key) {
	int ret;

	dbgetf(dir, group, key, "%d", &ret);
	return ret;
}

float
dbgetfloat(char *dir, char *group, char *key) {
	float ret;

	dbgetf(dir, group, key, "%f", &ret);
	return ret;
}

int
dbsettree(char *dir, Tree *t, Treesetter func) {
	char *path[TREEMAX];
	char group[PATH_MAX];
	char *sp;
	size_t sl, i, len;
	int depth;
	Tree *p;
	int ret = 0;

	for (p = NULL; tree_iter(t, TREEMAX, &p, &depth) != -1; ) {
next:
		path[depth] = p->name;

		if (p->data) {
			sl = sizeof(group);
			sp = group;
			for (i = 0; i <= depth; i++) {
				len = strlen(path[i]);
				if (len + (p->d ? strlen("index") : 0) >= sl) {
					ret = -1;
					warning("insufficient space to concatenate tree path\n");
					goto next;
				}

				memcpy(sp, path[i], len);
				if (i != depth || p->d)
					sp[len] = '/';
				else
					sp[len] = '\0';
				sp += len + 1;
				sl -= len + 1;
			}
			if (p->d) {
				len = strlen("index");
				memcpy(sp, "index", len + 1);
			}

			func(dir, group, depth, p);
		}
	}

	return ret;
}
