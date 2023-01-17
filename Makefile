DATADIR	= data
DBDIR	= db
DBLIB	= $(DBDIR)/db.o
DBTOOL	= $(DBDIR)/dbtool
SRCDIR	= src
SRC	= $(shell find $(SRCDIR) -name "*.c") styles/$(STYLE).c data/dirs.c
OBJ	= $(SRC:.c=.o) $(shell find $(DATADIR) -name "*.o" | grep -v 'dirs\.o')
BIN	= cepheid
RAYLIB	= -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
LDFLAGS	= $(RAYLIB) $(DBLIB)

include config.mk

all: db $(BIN)
.testing:
	touch .testing
src/main.o: .testing
src/data.o: data

.c.o:
	$(CC) $(CFLAGS) -D"SAVEDIR=\"$(SAVEDIR)\"" -c $< -o $@

$(OBJ): src/struct.h
$(BIN): $(OBJ) $(DBLIB)
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJ)

clean: db-clean
	rm -f $(BIN) $(OBJ)

db:
	@echo $(DBDIR): make $(DBLIB)
	+@cd $(DBDIR); make CFLAGS="$(CFLAGS)" `basename $(DBLIB)`
db-clean:
	@echo $(DBDIR): make clean
	+@cd $(DBDIR); make clean
dbtool:
	@echo $(DBDIR): make $(DBTOOL)
	+@cd $(DBDIR); make CFLAGS="$(CFLAGS)" dbtool

data:
	@echo $(DATADIR): make
	+@cd $(DATADIR); make
data-clean:
	@echo $(DATADIR): make clean
	+@cd $(DATADIR); make clean

# ignore generated headers
sloccount:
	sloccount $(SRC) $(DBDIR)

.PHONY: all clean db db-clean dbtool data data-clean
