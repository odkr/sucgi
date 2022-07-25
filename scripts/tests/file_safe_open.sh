#!/bin/sh
# Test if file_safe_open only opens safe files.
# shellcheck disable=1091,2015

#
# Initialisation
#

set -Cefu
readonly script_dir="${0%/*}"
# shellcheck disable=1091
. "$script_dir/../utils.sh" || exit
init || exit
PATH="${TESTSDIR:-./build/tests}:$PATH"

tmpdir chk
TMPDIR="$(realdir "$TMPDIR")" && [ "$TMPDIR" ] ||
	abort "failed to get canonical path of temporary directory."


#
# Main
#

touch "$TMP/file"
ln -s "$TMP" "$TMP/symlink"

file_safe_open "$TMP/file" ||
	abort "file_safe_open refuses to open $TMP/file."

file_safe_open "$TMP/symlink/file" &&
	abort "file_safe_open does not refuse to open $TMP/symlink/file."

exit 0
