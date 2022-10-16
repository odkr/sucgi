#!/bin/sh
# Test if file_is_wexcl correctly identifies exclusive write access.
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
fname="$TMPDIR/file"
touch "$fname"

euid="$(id -u)" && [ "$euid" ] ||
	err "failed to get process' effective UID."
egid="$(id -g)" && [ "$egid" ] ||
	err "failed to get process' effective GID."

for mode in g=w o=w ug=w uo=w go=w ugo=w
do
	warn "checking $mode ..."
	chown "$euid:$egid" "$fname"
	chmod "$mode" "$fname"
	# shellcheck disable=2154
	file_is_wexcl "$euid" "$fname" &&
		err "reported as exclusively writable."
	chmod ugo= "$fname"
done

for mode in u=w ugo=
do
	warn "checking $mode ..."
	chown "$euid:$egid" "$fname"
	chmod "$mode" "$fname"
	# shellcheck disable=2154
	file_is_wexcl "$euid" "$fname" ||
		err "reported as not exclusively writable."
	chmod ugo= "$fname"
done

warn "checking whether /bin/sh is exclusively writable by root ..."
file_is_wexcl 0 /bin/sh ||
	err "reported as not exclusively writable."

warn "${green}success.$reset"
