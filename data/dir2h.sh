#!/bin/sh

p="wdir_"
h="dirs.h"
c="dirs.c"

cat > $h << EOF
struct WDir {
	char *dir;
	char *name;
	unsigned char *data;
	size_t size;
};

EOF

for d in $*; do
	for f in $d/*; do
		b=$(basename "$f")
		v="${p}${d}_$(echo "$b" | tr " ()!'-" 'abcdef')"

		echo "static unsigned char $v[] = {" >> $h
		xxd -i < $f >> $h
		echo "};" >> $h
	done
done

echo "static struct WDir wdirs[] = {" >> $h

for d in $*; do
	for f in $d/*; do
		b=$(basename "$f")
		v="${p}${d}_$(echo "$b" | tr " ()!'-" 'abcdef')"

		printf "\t{ \"%s\", \"%s\", %s, sizeof(%s) },\n" \
			"$d" "$b" "$v" "$v" >> $h
	done
done

echo "};" >> $h
