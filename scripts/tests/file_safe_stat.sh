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
# Main
#

touch "$TMPDIR/file"
ln -s "$TMPDIR" "$TMPDIR/symlink"

file_safe_stat "$TMPDIR/file" ||
	abort "file_safe_stat refuses to stat $TMPDIR/file."

file_safe_stat "$TMPDIR/symlink/file" &&
	abort "file_safe_stat does not refuse to stat $TMPDIR/symlink/file."

exit 0
