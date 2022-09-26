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
# Prelude
#

file="$TMPDIR/file"
touch "$file"
symlink="$TMPDIR/symlink"
ln -s "$TMPDIR" "$symlink"


#
# Main
#

# shellcheck disable=2154
file_safe_open "$file" ||
	abort "file_safe_open refuses to open $bold$file$reset."

# shellcheck disable=2154
file_safe_open "$TMPDIR/symlink/file" &&
	abort "file_safe_open does not refuse to open $bold$symlink$reset."

exit 0
