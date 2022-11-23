#!/bin/sh

# GNU grep only
command -v ggrep >/dev/null && grep="ggrep" || grep="grep"

# Final grep is required for the return value to indicate
# existance of any calls to non error-checking allocators
! $grep -nE '[^fen](malloc|calloc|realloc|strdup)\(' src/*.c | $grep -v mem.c | $grep .
