CFLAGS += -Wall -g3 -Og
CFLAGS += -DDEBUG
CFLAGS += -DCHECK_FRAME_MEM_FREE
ARGS =

all: tags checks .git/hooks/pre-commit
tags: $(SRC)
	ctags --exclude=data/*.h --exclude=data/icons/*.h -R .

.git/hooks/pre-commit: dev/pre-commit
	cp $< $@

checks: $(SRC)
	./dev/checkalloc.sh

# All objects in src/ that react to TEST should have .testing as a dependency.
# Updating it before and after running will:
#  - Compile them with TEST if they weren't already
#  - Compile them without TEST next time it should be compiled normally
test:
	touch .testing
	cd tests && make
	make test-run RUNNER=run-basic
	rm -rf test.*
	touch .testing

test-gdb:
	touch .testing
	cd tests && make
	CK_FORK=no make test-run RUNNER=gdb

clean: test-clean
test-clean:
	cd tests && make clean

# Since the 'test' target will create *.c files in tests/, those can't be added
# to $(SRC), as shell macros are evaluated before the target is run. Hence,
# this target is required.
test-run:
	make $(RUNNER) CFLAGS="$(CFLAGS) -DTEST" LDFLAGS="$(LDFLAGS) -lcheck" SRC="$(SRC) $(shell find tests -type f -name "*.c")"

run: all
	gdb -ex 'set confirm on' -ex run -ex bt -ex quit --args ./$(BIN) $(ARGS)
run-basic: all
	./$(BIN) $(ARGS)
gdb: all
	gdb --args ./$(BIN) $(ARGS)

VALFILE = dev/valgrind.log
VALSUPP = dev/valgrind-suppress
memcheck: all
	@echo Outputting to $(VALFILE)
	valgrind --tool=memcheck --leak-check=full --suppressions=$(VALSUPP) --log-file=$(VALFILE) ./$(BIN) $(ARGS)

.PHONY: tags checks test gdb valgrind
