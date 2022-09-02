#!/bin/sh

p="wdir_"
d="$1"
h="$d.h"
c="$d.c"

rm -f $h $c

cat > $c << EOF
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include "${h}"

int
${p}${d}(char *dir) {
	char path[PATH_MAX];
	int fd;

	if (mkdir(dir, 0755) == -1)
		return -1;
EOF

for f in $d/*; do
	b=$(basename "$f")
	v="${p}${d}_$(echo "$b" | tr " ()!'-" 'abcdef')"

	echo "unsigned char $v[] = {" >> $h
	xxd -i < $f >> $h
	echo "};" >> $h

	cat >> $c << EOF
	snprintf(path, PATH_MAX, "%s/$b", dir);
	fd = open(path, O_WRONLY|O_CREAT, 0644);
	if (fd == -1)
		return -1;
	write(fd, $v, sizeof($v));
	close(fd);
EOF
done

cat >> $c << EOF
	return 0;
}
EOF
