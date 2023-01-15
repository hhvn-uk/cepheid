STYLE = aurora
SAVEDIR = saves
RAYLIB	= -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
CFLAGS	= -Wno-switch
LDFLAGS	= $(RAYLIB) $(DBLIB)

# include dev/config.mk
