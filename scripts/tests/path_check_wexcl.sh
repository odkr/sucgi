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
PATH="${TESTSDIR:-./build/tests}:$script_dir/../../build/tests:$PATH"

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
		abort "path_check_wexcl reports $bold$mode$reset" \
		      "as ${bold}exclusively writable$reset."
	chmod ugo= "$fname"
done

yes="u=w ugo="
for mode in $yes
do
	chown "$uid:$gid" "$fname"
	chmod "$mode" "$fname"
	path_check_wexcl "$uid" "$fname" "$TMPDIR" ||
		abort "path_check_wexcl reports $mode as" \
		      "${bold}not$reset exclusively writable."
	path_check_wexcl "$uid" "$fname" / &&
		abort "path_check_wexcl reports $bold$fname$reset" \
		      "as exclusively writable ${bold}w/o stop$reset."
	chmod ugo= "$fname"
done

path_check_wexcl 0 /bin/sh / ||
	abort "path_check_wexcl reports $bold/bin/sh$reset as" \
	      "${bold}not$reset exclusively writable."
