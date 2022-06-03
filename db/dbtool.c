#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "db.h"

void
fail(char *func) {
	perror(func);
	dbcleanup();
	exit(EXIT_FAILURE);
}

void
usage(char *name) {
	char *base;
	base = basename(name);
	fprintf(stderr, "usage: %s <dir>\n", base);
	fprintf(stderr, "       %s <dir> dump [group]\n", base);
	fprintf(stderr, "       %s <dir> get <group> <key>\n", base);
	fprintf(stderr, "       %s <dir> set <group> <key> [val]\n", base);
	fprintf(stderr, "       %s <dir> del <group> [key]\n", base);
}

void
listpairs(char *dir, char *group) {
	char **list;
	size_t len, i;

	len = dblistkeys(&list, dir, group);
	if (!list)
		fail("dblistkeys()");
	for (i = 0; i < len; i++)
		printf("%s=%s\n", *(list + i), dbget(dir, group, *(list + i)));
	dblistfree(list, len);
}

int
main(int argc, char *argv[]) {
	char *p;
	char **list;
	size_t len, i;

	if (argc == 2) {
		len = dblistgroups(&list, argv[1]);
		if (!list)
			fail("dblistgroups()");
		for (i = 0; i < len; i++)
			printf("%s\n", *(list + i));
	} else if (argc == 3 && strcmp(argv[2], "dump") == 0) {
		len = dblistgroups(&list, argv[1]);
		if (!list)
			fail("dblistgroups()");
		for (i = 0; i < len; i++) {
			printf("%s:\n", *(list + i));
			listpairs(argv[1], *(list + i));
			if (i != len - 1)
				printf("\n");
		}
	} else if (argc == 4 && strcmp(argv[2], "dump") == 0) {
		listpairs(argv[1], argv[3]);
	} else if (argc == 4 && strcmp(argv[2], "get") == 0) {
		len = dblistkeys(&list, argv[1], argv[3]);
		if (!list)
			fail("dblistkeys()");
		for (i = 0; i < len; i++)
			printf("%s\n", *(list + i));
		dblistfree(list, len);
	} else if (argc == 5 && strcmp(argv[2], "get") == 0) {
		if (!(p = dbget(argv[1], argv[3], argv[4])))
			fail("dbget()");
		printf("%s\n", p);
	} else if (argc == 5 && strcmp(argv[2], "set") == 0) {
		if (dbset(argv[1], argv[3], argv[4], NULL) == -1)
			fail("dbset()");
		dbwrite(argv[1]);
	} else if (argc == 6 && strcmp(argv[2], "set") == 0) {
		if (dbset(argv[1], argv[3], argv[4], argv[5]) == -1)
			fail("dbset()");
		dbwrite(argv[1]);
	} else if (argc == 4 && strcmp(argv[2], "del") == 0) {
		if (dbdelgroup(argv[1], argv[3]) == -1)
			fail("dbdelgroup()");
		dbwrite(argv[1]);
	} else if (argc == 5 && strcmp(argv[2], "del") == 0) {
		if (dbdelpair(argv[1], argv[3], argv[4]) == -1)
			fail("dbdelpair()");
		dbwrite(argv[1]);
	} else {
		usage(argv[0]);
	}

	dbcleanup();
	return EXIT_SUCCESS;
}
