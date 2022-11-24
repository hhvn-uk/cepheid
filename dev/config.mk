CFLAGS += -Wall -g3 -O0
CFLAGS += -DDEBUG
CFLAGS += -DCHECK_FRAME_MEM_FREE

all: tags checks
tags: $(SRC)
	ctags --exclude=data/*.h --exclude=data/icons/*.h -R .

checks: $(SRC)
	./dev/checkalloc.sh

test: all
	gdb ./$(BIN) -ex 'set confirm on' -ex run -ex bt -ex quit --args $(ARGS)
gdb: all
	gdb ./$(BIN) --args $(ARGS)

VALFILE = dev/valgrind.log
VALSUPP = dev/valgrind-suppress
memcheck: all
	@echo Outputting to $(VALFILE)
	valgrind --tool=memcheck --leak-check=full --suppressions=$(VALSUPP) --log-file=$(VALFILE) ./$(BIN) $(ARGS)

.PHONY: tags checks test gdb valgrind
