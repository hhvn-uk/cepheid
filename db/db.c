#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include "db.h"

typedef struct DB DB;
typedef struct Group Group;
typedef struct Pair Pair;

struct DB {
	DB *prev;
	char *dir;
	struct Group **tracked;
	size_t tl;
	int changes;
	DB *next;
};

struct Group {
	struct DB *db;
	char *dir;
	char *name;
	char *path;
	int changes;
	Pair *pairs;
};

struct Pair {
	Pair *prev;
	char *key;
	char *val;
	Pair *next;
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
 * its return list, and append the dirs to a dir list: it keeps calling the
 * backend for each element in the dir list until none remain, and the return
 * list is filled. */
struct Dirlist {
	char *path;
	struct Dirlist *next;
};

static int subdir(char *d1, char *d2);
static DB *getdb(char *db);
static DB *getdbfor(char *path);

static int track(Group *group);
static int untrack(Group *group);
static Group *gettracked(char *dir, char *group);

static int dirlist(struct Dirlist **list, char *dir, int (*filter)(void *data, char *path), void *fdata);

static Group *initgroup(char *dir, char *group);
static size_t dblistkeys_p(char ***ret, Group *group, int (*filter)(void *data, char *path), void *fdata);
static Group *getgroup(char *dir, char *group, int init);
static Pair *getpair(Group *group, char *key);
static int dbset_p(Pair *pair, char *val);
static int dbdelgroup_p(Group *group);
static void dbdelpair_p(Group *group, Pair *pair);
static int dbwritegroup_p(Group *group);
static void dbfreegroup_p(Group *group);

static DB *dbs;

/*
 * DB
 */
static int
subdir(char *d1, char *d2) {
	if (!d1 || !d2) return 0;
	if (!((strlen(d1) > strlen(d2) && d1[strlen(d2)] == '/') ||
				(strlen(d2) > strlen(d1) && d2[strlen(d1)] == '/')))
		return 0;
	else if (strncmp(d1, d2, strlen(d1)) == 0 || strncmp(d2, d1, strlen(d2)) == 0)
		return 1;
	else
		return 0;
}

int
dbdeclare(char *dir) {
	DB *db;

	if (!dir) {
		errno = EINVAL;
		return -1;
	}

	db = malloc(sizeof(DB));
	if (!db) return -1;
	db->dir = strdup(dir);
	if (!db->dir) return -1;

	db->tl = 10;
	db->tracked = malloc(db->tl * sizeof(Group *));
	if (!db->tracked) return -1;
	memset(db->tracked, 0, db->tl * sizeof(Group *));
	db->changes = 0;

	if (!dbs) {
		db->prev = db->next = NULL;
		dbs = db;
	} else {
		db->prev = NULL;
		db->next = dbs;
		dbs = db;
	}

	return 0;
}

static DB *
getdb(char *db) {
	DB *p;

	for (p = dbs; p; p = p->next)
		if (strcmp(p->dir, db) == 0)
			return p;
	return NULL;
}

static DB *
getdbfor(char *path) {
	DB *p;

	for (p = dbs; p; p = p->next)
		if (strcmp(p->dir, path) == 0 || subdir(p->dir, path))
			return p;
	return NULL;
}

/*
 * Tracking loaded groups
 */
#define TL_INCR 10

static int
track(Group *group) {
	DB *db = group->db;
	void *r;
	int i;

	if (!group) return -1;
	for (i = 0; i < db->tl; i++) {
		if (!db->tracked[i]) {
			db->tracked[i] = group;
			return 0;
		}
	}

	r = realloc(db->tracked, (db->tl + TL_INCR) * sizeof(Group *));
	if (!r) return -1;
	db->tl += TL_INCR;
	db->tracked = r;
	memset(db->tracked + i, 0, TL_INCR * sizeof(Group *));
	db->tracked[i] = group;
	return 0;
}

static int
untrack(Group *group) {
	int i;
	if (!group) return -1;
	for (i = 0; i < group->db->tl; i++) {
		if (group->db->tracked[i] == group) {
			group->db->tracked[i] = NULL;
			return 0;
		}
	}
	return -1;
}

static Group *
gettracked(char *dir, char *group) {
	DB *db;
	int i;

	if (!(db = getdbfor(dir)))
		return NULL;
	for (i = 0; i < db->tl; i++)
		if (db->tracked[i] &&
				strcmp(dir, db->tracked[i]->dir) == 0 &&
				strcmp(group, db->tracked[i]->name) == 0)
			return db->tracked[i];
	return NULL;
}

/*
 * Various struct/data/etc handling
 */
static int
appendpair(Pair **pairs, char *key, char *val) {
	Pair *p, *new;

	if (!pairs || !key) {
		errno = EINVAL;
		return -1;
	}

	new = malloc(sizeof(Pair));
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
freepair(Pair *pair) {
	if (pair) {
		free(pair->key);
		free(pair->val);
		free(pair);
	}
}

/*
 * Init
 */
static Group *
initgroup(char *dir, char *group) {
	Group *ret;

	if (!dir || !group) {
		errno = EINVAL;
		return NULL;
	}
	if ((ret = gettracked(dir, group)))
		return ret;
	ret = malloc(sizeof(Group));
	if (!ret) return NULL;
	ret->path = malloc(strlen(dir) + strlen(group) + 2);
	ret->db = getdbfor(dir);
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
	ret->changes = 0;
	track(ret);
	return ret;
}

static Group *
loadgroup(char *dir, char *group) {
	char path[PATH_MAX];
	char buf[8192];
	char *key, *val;
	Group *ret;
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
	if (!(ret = initgroup(dir, group))) {
		fclose(fp);
		return NULL;
	}
	while (fgets(buf, sizeof(buf), fp)) {
		buf[strlen(buf) - 1] = '\0'; /* remove \n */
		key = strtok_r(buf, "\t", &val);
		appendpair(&ret->pairs, key, val);
	}
	fclose(fp);
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
					p = malloc(sizeof(struct Dirlist));
					if (p) p->path = strdup(path);
					if (!p || !p->path) {
						if (p)
							free(p->path);
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
	struct Dirlist *res = NULL, *p, *next;
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
	for (i = 0, p = res, next = NULL; p; p = next, i++) {
		*((*ret) + i) = strdup(p->path + strlen(dir) + 1);
		next = p->next;
		free(p->path);
		free(p);
	}
	return len;
}

size_t
dblistgroups(char ***ret, char *dir) {
	return dblistgroups_f(ret, dir, NULL, NULL);
}

static size_t
dblistkeys_p(char ***ret, Group *group, int (*filter)(void *data, char *path), void *fdata) {
	size_t i = 0;
	size_t len;
	Pair *p;

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
	Group *p;
	if (!(p = getgroup(dir, group, 0))) {
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
static Group *
getgroup(char *dir, char *group, int init) {
	Group *p;

	if (!dir || !group) {
		errno = EINVAL;
		return NULL;
	}
	if ((p = gettracked(dir, group)))
		return p;
	if ((p = loadgroup(dir, group)))
		return p;
	if (init)
		return initgroup(dir, group);
	errno = ENOENT;
	return NULL;
}

static Pair *
getpair(Group *group, char *key) {
	Pair *p;

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
	Group *gp;
	Pair *pp;
	if (!(gp = getgroup(dir, group, 0)) ||
			!(pp = getpair(gp, key)))
		return NULL;
	return pp->val;
}

/*
 * Set
 */
static int
dbset_p(Pair *pair, char *val) {
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
	Group *gp;
	Pair *pp;
	int ret;

	if (!(gp = getgroup(dir, group, 1)))
		return -1;
	if (!(pp = getpair(gp, key)))
		ret = appendpair(&gp->pairs, key, val);
	else
		ret = dbset_p(pp, val);

	if (ret != -1) {
		gp->changes++;
		gp->db->changes++;
	}
	return ret;
}

/*
 * Del
 */
static int
dbdelgroup_p(Group *group) {
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
	Group *p;
	if (!(p = getgroup(dir, group, 0)))
		return -1;
	return dbdelgroup_p(p);
}

static void
dbdelpair_p(Group *group, Pair *pair) {
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
	Group *gp;
	Pair *pp;
	if (!(gp = getgroup(dir, group, 0)) ||
			!(pp = getpair(gp, key)))
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
dbwritegroup_p(Group *group) {
	char *dir, *path;
	FILE *fp;
	Pair *p;
	int serrno;

	if (!group || !group->path) {
		errno = EINVAL;
		return -1;
	}

	/* This isn't a guarantee that the information is
	 * actually there, so for now always write */
	/* if (!group->changes) */
	/* 	return 0; */

	group->db->changes -= group->changes;
	group->changes = 0;

	path = strdup(group->path);
	if (!path) return -1;
	dir = dirname(path);
	if (mkdirp(dir, 0755) == -1) {
		serrno = errno;
		free(path);
		errno = serrno;
		return -1;
	}
	free(path);

	if (!(fp = fopen(group->path, "w")))
		return -1;

	for (p = group->pairs; p; p = p->next)
		fprintf(fp, "%s\t%s\n", p->key, p->val);
	fclose(fp);
	return 0;
}

int
dbwritegroup(char *dir, char *group) {
	Group *p;
	if (!(p = getgroup(dir, group, 0)))
		return -1;
	return dbwritegroup_p(p);
}

int
dbwrite(char *db) {
	int ret, i;
	DB *p;

	if (!db) {
		errno = EINVAL;
		return -1;
	}

	if (!(p = getdb(db))) {
		errno = ENOENT;
		return -1;
	}

	for (i = 0, ret = 0; i < p->tl; i++)
		if (p->tracked[i])
			if (dbwritegroup_p(p->tracked[i]) == -1)
				ret = -1;
	return ret;
}

/*
 * Changes
 */
int
dbchangesgroup(char *dir, char *group) {
	Group *p;
	if ((p = getgroup(dir, group, 0)))
		return p->changes;
	return 0;
}

int
dbchanges(char *db) {
	DB *p;
	if ((p = getdb(db)))
		return p->changes;
	return 0;
}

/*
 * Free
 */

void
dbfree(char *db) {
	DB *p;
	int i;

	if (!(p = getdb(db)))
		return;
	for (i = 0; i < p->tl; i++)
		if (p->tracked[i])
			dbfreegroup_p(p->tracked[i]);
	free(p->tracked[i]);
	if (p == dbs)
		dbs = p->next;
	if (p->next)
		p->next->prev = p->prev;
	if (p->prev)
		p->prev->next = p->next;
	free(p->dir);
	free(p);
}

static void
dbfreegroup_p(Group *group) {
	Pair *p, *prev;
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
	Group *p;
	if ((p = getgroup(dir, group, 0)))
		dbfreegroup_p(p);
}

void
dbcleanup(void) {
	DB *p, *prev;
	int i;

	prev = dbs;
	if (prev) p = prev->next;
	while (prev) {
		for (i = 0; i < prev->tl; i++)
			if (prev->tracked[i])
				dbfreegroup_p(prev->tracked[i]);
		free(prev->dir);
		free(prev);
		prev = p;
		if (p) p = p->next;
	}
	dbs = NULL;
}
