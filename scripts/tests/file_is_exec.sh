#!/bin/sh
# Test if file_is_exec correctly identifies executables.
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


#
# Main
#

umask 0777
fname="$TMPDIR/file"
touch "$fname"

no="ugo= ug=,o=x"
for mode in $no
do
	chmod "$mode" "$fname"
	file_is_exec "$fname" &&
		abort "file_is_exec reports $mode as executable."
	chmod ugo= "$fname"
done

yes="ugo=x ug=x u=x g=x"
for mode in $yes
do
	chmod "$mode" "$fname"
	file_is_exec "$fname" ||
		abort "file_is_exec does not report $mode as executable."
	chmod ugo= "$fname"
done

file_is_exec /bin/sh ||
	abort "file_is_exec does not report /bin/sh as executable."