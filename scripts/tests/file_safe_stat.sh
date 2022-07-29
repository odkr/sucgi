#!/bin/sh
# Test if file_safe_stat only stats safe files.
# shellcheck disable=1091,2015

#
# Initialisation
#

set -Cefu
readonly script_dir="${0%/*}"
# shellcheck disable=1091
. "$script_dir/../utils.sh" || exit
init || exit
PATH="${TESTSDIR:-./build/tests}:$script_dir/../../build/tests:$PATH"

tmpdir chk
TMPDIR="$(realdir "$TMPDIR")" && [ "$TMPDIR" ] ||
	abort "failed to get canonical path of temporary directory."


#
# Prelude
#

uid="$(id -u)" && [ "$uid" ] ||
	abort "failed to get process' effective UID."

file="$TMPDIR/file"
touch "$file"
symlink="$TMPDIR/symlink"
ln -s "$TMPDIR" "$symlink"


#
# Main
#

checkok "UID $uid" \
	file_safe_stat "$file"

checkerr "file_safe_stat: $symlink: Too many levels of symbolic links." \
	file_safe_stat "$symlink"

exit 0
