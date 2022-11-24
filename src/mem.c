#include <stdlib.h>
#include <string.h>
#include "main.h"

#define NOMEM() error(2, "out of memory\n")
#define MCHECK(x) (x != NULL ? (void)0 : NOMEM())
#define FMEMMAX 1024

#ifdef CUSTOM_FREE
#undef free
#endif /* CUSTOM_FREE */


static void *fmem[FMEMMAX] = {NULL};
static int fmemi = 0;

/* allocate for a frame */
void *
falloc(size_t size) {
	if (fmemi + 1 == FMEMMAX)
		NOMEM();
	return (fmem[fmemi++] = emalloc(size));
}

void
ffree(void) {
	while (fmemi--) {
		free(fmem[fmemi]);
		fmem[fmemi] = NULL;
	}
	fmemi = 0;
}

char *
fstrdup(char *str) {
	char *ret;

	if (!str) return NULL;

	ret = falloc(strlen(str) + 1);
	strcpy(ret, str);
	return ret;
}

void *
emalloc(size_t size) {
	void *ret = malloc(size);
	MCHECK(ret);
	return ret;
}

void *
estrdup(char *str) {
	void *ret = strdup(str);
	MCHECK(ret);
	return ret;
}

void *
ecalloc(size_t nmemb, size_t size) {
	void *ret = calloc(nmemb, size);
	MCHECK(ret);
	return ret;
}

void *
erealloc(void *ptr, size_t size) {
	void *ret = realloc(ptr, size);
	MCHECK(ret);
	return ret;
}

void
_free(void *mem, char *file, int line, const char *func) {
#ifdef CHECK_FRAME_MEM_FREE
	int i;
	for (i = fmemi - 1; i >= 0; i--)
		if (fmem[i] == mem)
			_err(2, "error", file, line, func,
				"attempting to free memory allocated for a frame\n");
#endif /* CHECK_FRAME_MEM_FREE */
	free(mem);
}
