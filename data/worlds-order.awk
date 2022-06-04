#!/bin/awk -f

BEGIN {
	FS = "\t"
	i = 0
}

/^#/

!/^#/ {
	name = $2
	parent = $20

	# Parents should appear before children
	# Does not work for moonmoons, but there aren't any of those in Sol.. right?
	if (parent == "-" || parent == "Sol" || parent == "Sun") {
		print
		next
	} else {
		children[i++] = $0
	}
}

END {
	for (c in children)
		print children[c]
}
