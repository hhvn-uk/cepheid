#!/bin/sh

# GNU grep only
command -v ggrep >/dev/null && grep="ggrep" || grep="grep"

# Final grep is required for the return value to indicate existance
$grep --color=always -nE '[^fen](malloc|calloc|realloc|strdup)\(' src/*.c |
	$grep -v mem.c |
	$grep . && {
	tput setaf 160
	tput bold
	printf '>>> '
	tput sgr0
	printf 'allocations should be made with functions defined in src/mem.c\n'
	return 1
}

return 0
