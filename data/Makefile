FONTS	= $(shell find . -name "*.ttf")
ICONS	= $(shell find icons -name "*.png")
HEADERS	= $(FONTS:.ttf=.h) $(ICONS:.png=.h)

all: $(HEADERS) dirs.c

sol: worlds.tsv worlds-parse.awk
	rm -rf sol
	./worlds-parse.awk < $<
	printf "main\tSol\n" > sol/index
	printf "x\t0\n" >> sol/index
	printf "y\t0\n" >> sol/index

dirs.c: sol
	./dir2h.sh sol

# Thanks Jonathan
worlds.tsv:
	curl https://planet4589.org/space/gcat/tsv/worlds/worlds.tsv | \
		sed 's/ *\t/\t/g' | \
		./worlds-order.awk > $@

.ttf.h:
	xxd -i < $< > $@
.png.h:
	xxd -i < $< > $@

clean:
	rm -rf $(HEADERS) sol

.SUFFIXES: .h .ttf .png
