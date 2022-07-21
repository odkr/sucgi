#!/bin/sh
# Test if file_is_exec correctly identifies executables.
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

readonly TMP="${TMPDIR:-.}/.check-$$"
mkdir -m 0700 "$TMP" || exit
# shellcheck disable=2034
CLEANUP="[ \"${TMP-}\" ] && rm -rf \"\$TMP\""
export TMPDIR="$TMP"

trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

[ "$CAUGHT" -gt 0 ] && exit $((CAUGHT + 128))

umask 0777
fname="$TMP/file"
touch "$fname"

no="ugo= ug=,o=x"
for mode in $no
do
	chmod "$mode" "$fname"
	./file_is_exec "$fname" &&
		abort "file_is_exec reports $mode as executable."
	chmod ugo= "$fname"
done

yes="ugo=x ug=x u=x g=x"
for mode in $yes
do
	chmod "$mode" "$fname"
	./file_is_exec "$fname" ||
		abort "file_is_exec does not report $mode as executable."
	chmod ugo= "$fname"
done

./file_is_exec /bin/sh ||
	abort "file_is_exec does not report /bin/sh as executable."
