TESTS	= $(shell find . -type f -name "*.test")
TESTC	= $(TESTS:.test=.c)
CHECKS	= `echo $(TESTS) | tr ' ' '\n' | sed 's/^.\//test_/;s/\.test$$//'`

all: list.c test.h $(TESTC)
$(TESTC): mktest.awk Makefile

list.c: test.h
	printf '#include "test.h"\n\n' > $@
	printf 'int (*tests[TESTS])(void) = {\n' >> $@
	for f in $(CHECKS); do \
		printf '\t%s,\n' $$f >> $@; \
		done
	printf '};\n' >> $@

test.h: $(TESTS)
	printf '%s' "$(CHECKS)" | sed 's/^/int /;s/$$/(void);/' > $@
	printf '\n#define TESTS %d\n' $$(echo "$(CHECKS)" | wc -l) >> $@
	printf '\nextern int (*tests[TESTS])(void);\n' >> $@

clean:
	rm -f `find . -name "*.c" -o -name "*.h" -o -name "*.o" | grep -v main\.c`

.test.c:
	./mktest.awk -v "test=$*" < $< > $@

.SUFFIXES: .test .c
