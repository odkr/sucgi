#!/bin/sh
# Test if file_safe_open only opens safe files.
# shellcheck disable=1091,2015

set -Cefu

dir="$(dirname "$0")" && [ "$dir" ]
cd -P "$dir" || exit
. ./utils.sh || exit
trap cleanup EXIT

CAUGHT=0
trap 'CAUGHT=1' HUP
trap 'CAUGHT=2' INT
trap 'CAUGHT=15' TERM

: "${TMPDIR:=.}"
OWD="$(pwd)" && [ "$OWD" ] ||
	abort "failed to save working directory."
cd -P "$TMPDIR" || exit
TMPDIR="$(pwd)" && [ "$TMPDIR" ] ||
	abort "failed to get canonical path of temporary directory."
cd "$OWD" || exit

readonly TMP="${TMPDIR:-.}/check-$$"
mkdir -m 0700 "$TMP" || exit
# shellcheck disable=2034
CLEANUP="[ \"${TMP-}\" ] && rm -rf \"\$TMP\""
export TMPDIR="$TMP"

trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

[ "$CAUGHT" -gt 0 ] && exit $((CAUGHT + 128))

umask 077
touch "$TMP/file"
ln -s "$TMP" "$TMP/symlink"

./file_safe_open "$TMP/file" ||
	abort "file_safe_open refuses to open $TMP/file."

./file_safe_open "$TMP/symlink/file" &&
	abort "file_safe_open does not refuse to open $TMP/symlink/file."

exit 0
