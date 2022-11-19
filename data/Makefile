FONTS	= $(shell find . -name "*.ttf")
ICONS	= $(shell find icons -name "*.png")
OBJ	= $(FONTS:.ttf=.o) $(ICONS:.png=.o) splash.o
HEADERS	= $(OBJ:.o=.h)

all: $(OBJ) $(HEADERS) dirs.c

sol: worlds.tsv worlds-parse.awk
	rm -rf sol
	./worlds-parse.awk < $<
	printf "main\tSol\n" > sol/index
	printf "x\t0\n" >> sol/index
	printf "y\t0\n" >> sol/index

dirs.c: dirs.h
dirs.h: sol
	./dir2h.sh sol

# Thanks Jonathan
worlds.tsv:
	curl https://planet4589.org/space/gcat/tsv/worlds/worlds.tsv | \
		sed 's/ *\t/\t/g' | \
		./worlds-order.awk > $@

VAR = `echo $< | sed 's~.*/~~;s/\./_/g'`
SVAR = $(VAR)_size
.ttf.c .png.c:
	@echo "Creating $@ from $<"
	@echo "#include <stddef.h>" > $@
	@echo "unsigned char $(VAR)[] = {" >> $@
	@xxd -i < $< >> $@
	@echo "};" >> $@
	@echo "size_t $(SVAR) = sizeof($(VAR));" >> $@

.ttf.h .png.h:
	@echo "Creating $@"
	@echo "extern unsigned char $(VAR)[];" > $@
	@echo "extern size_t $(SVAR);" >> $@

clean:
	rm -rf $(HEADERS) $(OBJ) sol

.SUFFIXES: .c .o .h .ttf .png
