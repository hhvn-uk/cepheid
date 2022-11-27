#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include "main.h"

int
mkdirp(char *path) {
	char *dup;
	char *p[64];
	int create;
	int i, max;
	int ret = 0;

	dup = estrdup(path);
	max = strsplit(dup, "/", p, ELEMS(p));

	for (i = 0, create = 0; i < max; i++) {
		if (create || access(dup, F_OK) == -1) {
			if (mkdir(dup, 0777 & ~umask(0)) == -1) {
				ret = -1;
				goto end;
			}
			create = 1;
		}
		if (i + 1 != max)
			*(p[i + 1] - 1) = '/';
	}

end:
	free(dup);
	return ret;
}

int
rmdirp(char *dir) {
	struct dirent **dirent;
	struct stat st;
	char path[PATH_MAX];
	int ret = 0;
	int n, i;

	n = scandir(dir, &dirent, NULL, alphasort);
	if (n < 0) return 0;

	for (i = 0; i < n; i++) {
		snprintf(path, sizeof(path), "%s/%s", dir, dirent[i]->d_name);
		stat(path, &st);
		if (!streq(dirent[i]->d_name, ".") && !streq(dirent[i]->d_name, "..")) {
			if (S_ISDIR(st.st_mode)) {
				if (rmdirp(path) == -1)
					ret = -1;
			} else {
				if (unlink(path) == -1)
					ret = -1;
			}
		}
		free(dirent[i]);
	}
	free(dirent);

	if (rmdir(dir) == -1)
		ret = -1;

	return ret;
}
