#include <time.h>
#include "maths.h"
#include "main.h"

void
timespec_diff(struct timespec *t1, struct timespec *t2, struct timespec *diff) {
	struct timespec *tmp;

	if (t1->tv_sec < t2->tv_sec ||
			(t1->tv_sec == t2->tv_sec && t1->tv_nsec < t2->tv_nsec)) {
		tmp = t2;
		t2 = t1;
		t1 = tmp;
	}

	diff->tv_sec = t1->tv_sec - t2->tv_sec;
	diff->tv_nsec = t1->tv_nsec - t2->tv_nsec;

	if (diff->tv_nsec < 0) {
		diff->tv_sec--;
		diff->tv_nsec += NANO;
	}
}
