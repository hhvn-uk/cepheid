CFLAGS += -Wall -g3 -O0
CFLAGS += -DDEBUG
CFLAGS += -DCHECK_FRAME_MEM_FREE

all: tags checks
tags: $(SRC)
	ctags --exclude=data/*.h --exclude=data/icons/*.h -R .

checks: $(SRC)
	./dev/checkalloc.sh

test: all
	gdb ./$(BIN) -ex 'set confirm on' -ex run -ex bt -ex quit
gdb: all
	gdb ./$(BIN)

.PHONY: tags checks test gdb
