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

static int
dbtree_concat(char *path[TREEMAX], size_t depth, char *group, int index) {
	size_t sl = PATH_MAX;
	size_t i, len;
	char *sp = group;

	for (i = 0; i <= depth; i++) {
		len = strlen(path[i]);
		if (len + (index ? strlen("index") : 0) >= sl) {
			warning("insufficient space to concatenate tree path\n");
			return -1;
		}

		memcpy(sp, path[i], len);
		if (i != depth || index)
			sp[len] = '/';
		else
			sp[len] = '\0';
		sp += len + 1;
		sl -= len + 1;
	}
	if (index) {
		len = strlen("index");
		memcpy(sp, "index", len + 1);
	}

	return 0;
}

int
dbsettree(char *dir, Tree *t, Treesetter func) {
	char *path[TREEMAX];
	char group[PATH_MAX];
	int depth;
	Tree *p;
	int ret = 0;

	for (p = NULL; tree_iter(t, TREEMAX, &p, &depth) != -1; ) {
		path[depth] = p->name;

		if (p->data) {
			if (dbtree_concat(path, depth, group, p->d != NULL) == -1) {
				ret = -1;
				continue;
			}
			func(dir, group, p->name, depth, p);
		}
	}

	return ret;
}

int
dbgettree(char *dir, Tree *r, Treegetter func) {
	char *ppath, *path;
	char *psplit[TREEMAX], *split[TREEMAX];
	char **groups;
	char group[PATH_MAX];
	char *name;
	size_t glen, pslen, slen;
	size_t common, up;
	int index;
	int i, j;
	Tree *t, *p;
	int ret = 0;

	glen = dblistgroups(&groups, dir);
	slen = 0;
	memset(&split, 0, sizeof(split));

	for (i = 0, ppath = path = NULL, t = r; i < glen; i++) {
		free(ppath);
		ppath = path;
		path = strdup(groups[i]);

		memcpy(&psplit, split, sizeof(split));
		pslen = slen;
		slen = strsplit(path, "/", split, TREEMAX);

		if (streq(split[slen - 1], "index")) {
			index = 1;
			slen--;
		} else {
			index = 0;
		}

		common = strlistcmp(psplit, pslen, split, slen);
		up = pslen - common;

		if (up && (float)up <= (float)((float)pslen / 2.0f)) {
			for (j = 0; j < up; j++)
				t = t->u;
			j = common;
		} else if (common == pslen) {
			j = common;
		} else {
			t = r;
			j = 0;
		}

		for (; j < slen; j++) {
nchild:
			name = split[j];

			for (p = t->d; p; p = p->n) {
				if (streq(p->name, name)) {
					t = p;
					goto nchild;
				}
			}

			/* Since name is a part of split[], it will be
			 * corrupted later. The Treegetter function is
			 * therefore responsible for allocating the name
			 * and returning it */
			t = tree_add_child(t, name, 0, NULL, NULL);
			if (dbtree_concat(split, j, group, index || j != slen - 1) == -1) {
				ret = -1;
				continue;
			}
			if (!t->data)
				t->name = func(dir, group, name, j + 1, t);
		}
	}

	return ret;
}
