SRC = db.c dbtool.c
OBJ = $(SRC:.c=.o)
BIN = dbtool

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN) $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
