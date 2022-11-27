#include <check.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include "test.h"
#include "../src/main.h"

int
test_run(void) {
	int ret, i;

	printf("\033[1mvvv Tests vvv\033[0m\n");

	for (ret = i = 0; i < TESTS; i++)
		ret += tests[i]();

	printf("\033[1m^^^ Tests ^^^\033[0m\n");

	return !!ret;
}
