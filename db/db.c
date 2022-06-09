#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include "db.h"

typedef struct dbGroup dbGroup;
typedef struct dbPair dbPair;

struct dbGroup {
	char *dir;
	char *name;
	char *path;
	dbPair *pairs;
};

struct dbPair {
	dbPair *prev;
	char *key;
	char *val;
	dbPair *next;
};

/* produced only by dirlist()
 * A linked list is much easier to append to than a dynamically sized array.
 * This is especially the case in a recursive function, like dirlist(),
 * where I would probably need to use static variables to keep track
 * of the size and how much has been used. It is preferable to not use static
 * variables in case I ever want to use multithreading.
 *
 * A possible alternative would be to have a dirlist_backend() function that
 * returns the files/dirs in a dir. The frontend would then append the files to
 * its return list, and append the dirs to another list: it keeps calling the
 * backend for each element in this list until none remain. */
struct Dirlist {
	char *path;
	struct Dirlist *next;
};

static int track(dbGroup *group);
static int untrack(dbGroup *group);
static dbGroup *gettracked(char *dir, char *group);

static int dirlist(struct Dirlist **list, char *dir, int (*filter)(void *data, char *path), void *fdata);

static dbGroup *dbinitgroup_p(char *dir, char *group);
static size_t dblistkeys_p(char ***ret, dbGroup *group, int (*filter)(void *data, char *path), void *fdata);
static dbGroup *dbgetgroup_p(char *dir, char *group, int init);
static dbPair *dbgetpair_p(dbGroup *group, char *key);
static int dbset_p(dbPair *pair, char *val);
static int dbdelgroup_p(dbGroup *group);
static void dbdelpair_p(dbGroup *group, dbPair *pair);
static int dbwritegroup_p(dbGroup *group);
static void dbfreegroup_p(dbGroup *group);

static dbGroup *tracked[MAXGROUPS] = {NULL};

/*
 * Tracking loaded groups
 */
static int
track(dbGroup *group) {
	int i;
	if (!group) return -1;
	for (i = 0; i < MAXGROUPS; i++) {
		if (!tracked[i]) {
			tracked[i] = group;
			return 0;
		}
	}
	return -1;
}

static int
untrack(dbGroup *group) {
	int i;
	if (!group) return -1;
	for (i = 0; i < MAXGROUPS; i++) {
		if (tracked[i] == group) {
			tracked[i] = NULL;
			return 0;
		}
	}
	return -1;
}

static dbGroup *
gettracked(char *dir, char *group) {
	int i;
	for (i = 0; i < MAXGROUPS; i++)
		if (tracked[i] &&
				strcmp(dir, tracked[i]->dir) == 0 &&
				strcmp(group, tracked[i]->name) == 0)
			return tracked[i];
	return NULL;
}

/*
 * Various struct/data/etc handling
 */
static int
appendpair(dbPair **pairs, char *key, char *val) {
	dbPair *p, *new;

	if (!pairs || !key) {
		errno = EINVAL;
		return -1;
	}

	new = malloc(sizeof(dbPair));
	if (!new) return -1; /* ENOMEM */
	new->key = strdup(key);
	new->val = val ? strdup(val) : NULL;
	if (!new->key || (val && !new->val)) {
		free(new->key);
		free(new->val);
		free(new);
		errno = ENOMEM;
		return -1;
	}

	if (!*pairs) {
		*pairs = new;
		new->prev = new->next = NULL;
	} else {
		for (p = *pairs; p && p->next; p = p->next);
		p->next = new;
		new->prev = p;
		new->next = NULL;
	}
	return 0;
}

char **
dblistdup(char **list, size_t len) {
	char **ret;
	int i;

	if (!len || !list) {
		errno = EINVAL;
		return NULL;
	}
	ret = malloc(len * sizeof(char *));
	if (!ret) return NULL;
	for (i = 0; i < len; i++) {
		*(ret + i) = *(list + i) ? strdup(*(list + i)) : NULL;
		if (*(list + i) && !*(ret + i)) {
			while (i--)
				free(*(ret + i));
			errno = ENOMEM;
			return NULL;
		}
	}
	return ret;
}

void
dblistfree(char **list, size_t len) {
	int i;

	if (!len || !list) return;
	for (i = 0; i < len; i++)
		free(*(list + i));
	free(list);
}

int
dblisteq(char **l1, size_t s1, char **l2, size_t s2) {
	int i;
	/* The order of these if statements are significant. */
	if (s1 != s2)
		return 0;
	if (l1 == l2) /* previous statement handles different sizes */
		return 1;
	if (!l1 || !l2) /* previous statement handles both being null */
		return 0;
	for (i = 0; i < s1; i++)
		if (strcmp(*(l1 + i), *(l2 + i)) != 0)
			return 0;
	return 1;
}

static void
freepair(dbPair *pair) {
	if (pair) {
		free(pair->key);
		free(pair->val);
		free(pair);
	}
}

/*
 * Init
 */
static dbGroup *
dbinitgroup_p(char *dir, char *group) {
	dbGroup *ret;

	if (!dir || !group) {
		errno = EINVAL;
		return NULL;
	}
	if ((ret = gettracked(dir, group)))
		return ret;
	ret = malloc(sizeof(dbGroup));
	if (!ret) return NULL;
	ret->path = malloc(strlen(dir) + strlen(group) + 2);
	ret->dir = strdup(dir);
	ret->name = strdup(group);
	ret->pairs = NULL;
	if (!ret->path || !ret->dir || !ret->name) {
		free(ret->path);
		free(ret->dir);
		free(ret->name);
		free(ret);
		errno = ENOMEM;
		return NULL;
	}
	sprintf(ret->path, "%s/%s", dir, group);
	track(ret);
	return ret;
}

static dbGroup *
dbloadgroup_p(char *dir, char *group) {
	char path[PATH_MAX];
	char buf[8192];
	char *key, *val;
	dbGroup *ret;
	FILE *fp;

	if (!dir || !group) {
		errno = EINVAL;
		return NULL;
	}
	if ((ret = gettracked(dir, group)))
		return ret;

	snprintf(path, sizeof(path), "%s/%s", dir, group);
	if (!(fp = fopen(path, "r")))
		return NULL; /* errno set by fopen */
	if (!(ret = dbinitgroup_p(dir, group)))
		return NULL;
	while (fgets(buf, sizeof(buf), fp)) {
		buf[strlen(buf) - 1] = '\0'; /* remove \n */
		key = strtok_r(buf, "\t", &val);
		appendpair(&ret->pairs, key, val);
	}
	return ret;
}

/*
 * List
 */
static int
dirlist(struct Dirlist **list, char *dir, int (*filter)(void *data, char *path), void *fdata) {
	struct dirent **dirent;
	struct stat st;
	struct Dirlist *p, *prev;
	char path[PATH_MAX];
	int n, i;

	/* Theoretically, a function could wrap around filter() so that it
	 * could be passed to scandir(), however, it would be difficult for that
	 * function to access filter() without global variables. Therefore,
	 * this function calls the filter on data returned by scandir(). */

	n = scandir(dir, &dirent, 0, alphasort);
	if (n < 0) {
		return -1; /* scandir sets errno */
	} else {
		for (i = 0; i < n; i++) {
			snprintf(path, sizeof(path), "%s/%s", dir, dirent[i]->d_name);
			if (strcmp(dirent[i]->d_name, "..") != 0 &&
					strcmp(dirent[i]->d_name, ".") != 0 &&
					stat(path, &st) != -1) {
				if (S_ISDIR(st.st_mode)) {
					dirlist(list, path, filter, fdata);
				} else if (!filter || filter(fdata, path)) {
					if (!(p = malloc(sizeof(struct Dirlist))) || !(p->path = strdup(path))) {
						free(p);
						for (prev = NULL; p; p = p->next) {
							free(p->path);
							free(prev);
							prev = p;
						}
						errno = ENOMEM;
						return -1;
					}
					p->next = *list;
					*list = p;
				}
			}
			free(dirent[i]);
		}
		free(dirent);
	}

	if (!*list) {
		errno = ENOENT;
		return -1;
	}

	return 0;
}

size_t
dblistgroups_f(char ***ret, char *dir, int (*filter)(void *data, char *path), void *fdata) {
	struct Dirlist *res = NULL, *p, *prev;
	size_t len, i;

	if (!ret || !dir) {
		errno = EINVAL;
		if (ret) *ret = NULL;
		return 0;
	}
	if (dirlist(&res, dir, filter, fdata) == -1) {
		*ret = NULL;
		return 0;
	}
	for (p = res, len = 0; p; p = p->next)
		len++;
	*ret = malloc(len * sizeof(char *));
	if (!*ret) {
		*ret = NULL;
		return 0; /* malloc sets errno */
	}
	for (i = 0, p = res, prev = NULL; p; p = p->next, i++) {
		*((*ret) + i) = strdup(p->path + strlen(dir) + 1);
		free(p->path);
		free(prev);
		prev = p;
	}
	return len;
}

size_t
dblistgroups(char ***ret, char *dir) {
	return dblistgroups_f(ret, dir, NULL, NULL);
}

static size_t
dblistkeys_p(char ***ret, dbGroup *group, int (*filter)(void *data, char *path), void *fdata) {
	size_t i = 0;
	size_t len;
	dbPair *p;

	if (!ret || !group) {
		errno = EINVAL;
		if (ret) *ret = NULL;
		return 0;
	}

	for (p = group->pairs, len = 0; p; p = p->next)
		if (!filter || filter(fdata, p->key))
			len++;

	*ret = malloc(len * sizeof(char *));
	if (!*ret) {
		*ret = NULL;
		return 0;
	}
	for (p = group->pairs; p && i < len; p = p->next)
		if (!filter || filter(fdata, p->key))
			*((*ret) + i++) = p->key ? strdup(p->key) : NULL;
	return len;
}

size_t
dblistkeys_f(char ***ret, char *dir, char *group, int (*filter)(void *data, char *path), void *fdata) {
	dbGroup *p;
	if (!(p = dbgetgroup_p(dir, group, 0))) {
		*ret = NULL;
		return 0;
	}
	return dblistkeys_p(ret, p, filter, fdata);
}

size_t
dblistkeys(char ***ret, char *dir, char *group) {
	return dblistkeys_f(ret, dir, group, NULL, NULL);
}

/*
 * Get
 */
static dbGroup *
dbgetgroup_p(char *dir, char *group, int init) {
	dbGroup *p;

	if (!dir || !group) {
		errno = EINVAL;
		return NULL;
	}
	if ((p = gettracked(dir, group)))
		return p;
	if ((p = dbloadgroup_p(dir, group)))
		return p;
	if (init)
		return dbinitgroup_p(dir, group);
	errno = ENOENT;
	return NULL;
}

static dbPair *
dbgetpair_p(dbGroup *group, char *key) {
	dbPair *p;

	if (!group || !key) {
		errno = EINVAL;
		return NULL;
	}
	for (p = group->pairs; p; p = p->next)
		if (strcmp(p->key, key) == 0)
			return p;
	errno = EINVAL;
	return NULL;
}

char *
dbget(char *dir, char *group, char *key) {
	dbGroup *gp;
	dbPair *pp;
	if (!(gp = dbgetgroup_p(dir, group, 0)) ||
			!(pp = dbgetpair_p(gp, key)))
		return NULL;
	return pp->val;
}

/*
 * Set
 */
static int
dbset_p(dbPair *pair, char *val) {
	if (!pair) {
		errno = EINVAL;
		return -1;
	}
	free(pair->val);
	pair->val = val ? strdup(val) : NULL;
	if (val && !pair->val) return -1; /* ENOMEM */
	return 0;
}

int
dbset(char *dir, char *group, char *key, char *val) {
	dbGroup *gp;
	dbPair *pp;
	if (!(gp = dbgetgroup_p(dir, group, 1)))
		return -1;
	if (!(pp = dbgetpair_p(gp, key)))
		return appendpair(&gp->pairs, key, val);
	return dbset_p(pp, val);
}

/*
 * Del
 */
static int
dbdelgroup_p(dbGroup *group) {
	int ret = 0, serrno;
	ret = unlink(group->path);
	serrno = errno;
	dbfreegroup_p(group);
	if (ret == -1)
		errno = serrno;
	return ret;
}

int
dbdelgroup(char *dir, char *group) {
	dbGroup *p;
	if (!(p = dbgetgroup_p(dir, group, 0)))
		return -1;
	return dbdelgroup_p(p);
}

static void
dbdelpair_p(dbGroup *group, dbPair *pair) {
	if (!pair) return;
	if (pair == group->pairs)
		group->pairs = pair->next;
	if (pair->next)
		pair->next->prev = pair->prev;
	if (pair->prev)
		pair->prev->next = pair->next;
	freepair(pair);
}

int
dbdelpair(char *dir, char *group, char *key) {
	dbGroup *gp;
	dbPair *pp;
	if (!(gp = dbgetgroup_p(dir, group, 0)) ||
			!(pp = dbgetpair_p(gp, key)))
		return -1;
	dbdelpair_p(gp, pp);
	return 0;
}

/*
 * Write
 */
static int
mkdirp(char *path, mode_t mode) {
	struct stat st;
	int serrno;
	char *p;

	path = strdup(path);
	if (!path)
		return -1; /* use strdup's errno */

	p = path;
	while (p) {
		p = strchr(p + 1, '/');
		if (p) {
			while (*(p + 1) == '/')
				p++;
			*p = '\0';
		}
		if (mkdir(path, mode) == -1) {
			serrno = errno;
			if (stat(path, &st) == -1) {
				errno = serrno;
				return -1;
			} else if (!S_ISDIR(st.st_mode)) {
				errno = ENOTDIR;
				return -1;
			}
		}
		if (p) *p = '/';
	}
	return 0;
}

static int
dbwritegroup_p(dbGroup *group) {
	FILE *fp;
	dbPair *p;

	if (!group || !group->path) {
		errno = EINVAL;
		return -1;
	}
	if (mkdirp(group->dir, 0755) == -1)
		return -1; /* errno set by mkdirp */
	if (!(fp = fopen(group->path, "w")))
		return -1; /* errno set by fopen */

	for (p = group->pairs; p; p = p->next)
		fprintf(fp, "%s\t%s\n", p->key, p->val);
	return 0;
}

int
dbwritegroup(char *dir, char *group) {
	dbGroup *p;
	if (!(p = dbgetgroup_p(dir, group, 0)))
		return -1;
	return dbwritegroup_p(p);
}

int
dbwrite(char *dir) {
	int ret = 0, i;

	if (!dir) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < MAXGROUPS; i++)
		if (tracked[i] && strcmp(dir, tracked[i]->dir) == 0)
			if (dbwritegroup_p(tracked[i]) == -1)
				ret = -1;
	return ret;
}

/*
 * Free
 */

void
dbfree(char *dir) {
	int i;

	for (i = 0; i < MAXGROUPS; i++)
		if (tracked[i] && strcmp(dir, tracked[i]->dir) == 0)
			dbfreegroup_p(tracked[i]);
}

static void
dbfreegroup_p(dbGroup *group) {
	dbPair *p, *prev;
	if (group) {
		untrack(group);
		prev = group->pairs;
		if (prev)
			p = prev->next;
		while (prev) {
			freepair(prev);
			prev = p;
			if (p) p = p->next;
		}
		free(group->dir);
		free(group->name);
		free(group->path);
		free(group);
	}
}

void
dbfreegroup(char *dir, char *group) {
	dbGroup *p;
	if ((p = dbgetgroup_p(dir, group, 0)))
		dbfreegroup_p(p);
}

void
dbcleanup(void) {
	int i;

	for (i = 0; i < MAXGROUPS; i++)
		if (tracked[i])
			dbfreegroup_p(tracked[i]);
}
