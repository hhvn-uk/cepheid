DATADIR	= data
DBDIR	= db
DBLIB	= $(DBDIR)/db.o
DBTOOL	= $(DBDIR)/dbtool
SRCDIR	= src
SRC	= $(shell find $(SRCDIR) -name "*.c")
OBJ	= $(SRC:.c=.o)
BIN	= game
RAYLIB	= -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
LDFLAGS	= $(RAYLIB) $(DBLIB)
CFLAGS	= -g3 -O0

all: db data $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJ)

clean: db-clean
	rm -f $(BIN) $(OBJ)

db:
	@echo $(DBDIR): make $(DBLIB)
	@cd $(DBDIR); make `basename $(DBLIB)`
db-clean:
	@echo $(DBDIR): make clean
	@cd $(DBDIR); make clean
dbtool:
	@echo $(DBDIR): make $(DBTOOL)
	@cd $(DBDIR); make dbtool

data:
	@echo $(DATADIR): make
	@cd $(DATADIR); make
data-clean:
	@echo $(DATADIR): make clean
	@cd $(DATADIR); make clean

# ignore generated headers
sloccount:
	sloccount $(SRC) $(DBDIR)

.PHONY: all db db-clean dbtool data data-clean
