#!/bin/sh
# Test if file_is_wexcl correctly identifies exclusive write access.
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
	# shellcheck disable=2154
	file_is_wexcl "$uid" "$fname" &&
		abort "file_is_wexcl reports $bold$mode$reset" \
		      "as ${bold}exclusively writable$reset."
	chmod ugo= "$fname"
done

yes="u=w ugo="
for mode in $yes
do
	chown "$uid:$gid" "$fname"
	chmod "$mode" "$fname"
	# shellcheck disable=2154
	file_is_wexcl "$uid" "$fname" ||
		abort "file_is_wexcl reports $bold$mode$reset" \
		      "as ${bold}not$reset exclusively writable."
	chmod ugo= "$fname"
done

file_is_wexcl 0 /bin/sh ||
	abort "file_is_wexcl reports /bin/sh as not exclusively writable."
