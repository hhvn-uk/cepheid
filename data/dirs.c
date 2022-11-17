#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include "dirs.h"

#define LEN(array) (sizeof(array)/sizeof(array[0]))

int
dirs_write(char *dir, char *to) {
	char path[PATH_MAX];
	int fd;
	size_t i;

	if (mkdir(to, 0755) == -1)
		return -1;

	for (i = 0; i < LEN(wdirs); i++) {
		if (strcmp(wdirs[i].dir, dir) == 0) {
			snprintf(path, PATH_MAX, "%s/%s",
					to, wdirs[i].name);
			fd = open(path, O_WRONLY|O_CREAT, 0644);
			if (fd == -1)
				return -1;
			write(fd, wdirs[i].data, wdirs[i].size);
			close(fd);
		}
	}

	return 0;
}
