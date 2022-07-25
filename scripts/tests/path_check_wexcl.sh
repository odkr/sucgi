#!/bin/sh
# Test if path_check_wexcl correctly identifies exclusive write access.
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

tmpdir tmp .
TMPDIR="$(realdir "$TMPDIR")" && [ "$TMPDIR" ] ||
	abort "failed to get canonical path of temporary directory."


#
# Main
#

umask 0777
fname="$TMPDIR/file"
touch "$fname"

uid="$(id -u)" && [ "$uid" ] ||
	abort "failed to get process' effective UID."
gid="$(id -g)" && [ "$gid" ] ||
	abort "failed to get process' effective GID."

no="g=w o=w ug=w uo=w go=w ugo=w"
for mode in $no
do
	chown "$uid:$gid" "$fname"
	chmod "$mode" "$fname"
	path_check_wexcl "$uid" "$fname" "$TMPDIR" &&
		abort "path_check_wexcl reports $mode as exclusively writable."
	chmod ugo= "$fname"
done

yes="u=w ugo="
for mode in $yes
do
	chown "$uid:$gid" "$fname"
	chmod "$mode" "$fname"
	path_check_wexcl "$uid" "$fname" "$TMPDIR" ||
		abort "path_check_wexcl reports $mode as not exclusively writable."
	path_check_wexcl "$uid" "$fname" / &&
		abort "path_check_wexcl reports $fname as exclusively writable w/o stop condition."
	chmod ugo= "$fname"
done

path_check_wexcl 0 /bin/sh / ||
	abort "path_check_wexcl reports /bin/sh as not exclusively writable."