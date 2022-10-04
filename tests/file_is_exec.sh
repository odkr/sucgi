#!/bin/sh
# Test if file_is_exec correctly identifies executables.
# shellcheck disable=1091,2015

#
# Initialisation
#

set -Cefu
script_dir="${0%/*}"
[ "$script_dir" = "$0" ] && script_dir=.
readonly script_dir
# shellcheck disable=1091
. "$script_dir/../tools/lib.sh" || exit
init || exit
tmpdir chk


#
# Main
#

umask 0777

euid="$(id -u)" && [ "$euid" ] ||
	abort "failed to get process' effective UID."
egid="$(id -g)" && [ "$egid" ] ||
	abort "failed to get process' effective GID."

fname="$TMPDIR/file"
touch "$fname"
chown "$euid:$egid" "$fname"

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


#
# Interlude
#

euid="$(id -u)" && [ "$euid" ] ||
	abort "failed to get process' effective user ID."

[ "$euid" -ne 0 ] && exit

eval "$(unallocids)" && [ "$unalloc_uid" ] && [ "$unalloc_gid" ] ||
	abort "failed to find an unallocated UID and GID."

chown "$unalloc_uid:$unalloc_gid" "$fname"

chmod o= "$fname"
file_is_exec "$fname" &&
	abort "file_is_exec reports $mode as executable."

chmod o=x "$fname"
file_is_exec "$fname" ||
	abort "file_is_exec does not report $mode as executable."
