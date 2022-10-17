#!/bin/sh
# Test if file_safe_open only opens safe files.
# shellcheck disable=1091,2015

#
# Initialisation
#

set -Cefu
script_dir="${0%/*}"
[ "$script_dir" = "$0" ] && script_dir=.
readonly script_dir
# shellcheck disable=1091
. "$script_dir/../tools/lib.sh" || exit
init || exit
tmpdir chk


#
# Main
#

file="$TMPDIR/file"
echo $$ > "$file"
symlink="$TMPDIR/symlink"
ln -s "$TMPDIR" "$symlink"

checkok $$ file_safe_open "$file" f

checkerr 'Not a directory' file_safe_open "$file" d

checkerr 'Too many levels of symbolic links' file_safe_open "$symlink" d

# shellcheck disable=2154
warn "${green}success.$reset"
