FONTS	= $(shell find . -name "*.ttf")
HEADERS	= $(FONTS:.ttf=.h)

all: $(HEADERS) sol

sol: worlds.tsv
	rm -rf sol
	./worlds-parse.awk < $<
	printf "main\tsol\n" > sol/index

# Thanks Jonathan
worlds.tsv:
	curl https://planet4589.org/space/gcat/tsv/worlds/worlds.tsv | \
		sed 's/ *\t/\t/g' > $@

.ttf.h:
	xxd -i < $< > $@

clean:
	rm -rf $(HEADERS) sol

.SUFFIXES: .h .ttf
