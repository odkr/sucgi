#!/bin/sh
# Test if file_safe_stat only stats safe files.
# shellcheck disable=1091,2015

set -Cefu

dir="$(dirname "$0")" && [ "$dir" ]
cd -P "$dir" || exit
. ./utils.sh || exit
PATH="../../build/tests:$PATH"

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

readonly TMP="${TMPDIR:-.}/chk-$$.tmp"
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

file_safe_stat "$TMP/file" ||
	abort "file_safe_stat refuses to stat $TMP/file."

file_safe_stat "$TMP/symlink/file" &&
	abort "file_safe_stat does not refuse to stat $TMP/symlink/file."

exit 0
