#!/bin/sh
# Test if path_check_wexcl correctly identifies exclusive write access.
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
tmpdir tmp .


#
# Prelude 
#

fname="$TMPDIR/file"
touch "$fname"
dir="$TMPDIR/dir"
mkdir "$dir"
dirf="$dir/file"
touch "$dirf"
chmod 600 "$dirf"

euid="$(id -u)" && [ "$euid" ] ||
	err "failed to get process' effective UID."
egid="$(id -g)" && [ "$egid" ] ||
	err "failed to get process' effective GID."


#
# Main
#

no="u=r,g=w,o= u=r,g=,o=w u=rw,g=w,o= u=rw,g=,o=w u=rw,go=w u=rw,go=w"
for mode in $no
do
	chown "$euid:$egid" "$fname"
	chmod "$mode" "$fname"
	checkerr "$fname: writable by UIDs other than $euid" \
		path_check_wexcl "$euid" "$TMPDIR" "$fname"

	chown "$euid:$egid" "$dir"
	chmod "$mode" "$dir"
	chmod u+x "$dir"
	checkerr "$dir: writable by UIDs other than $euid" \
		path_check_wexcl "$euid" "$TMPDIR" "$dirf"
done

yes="u=rw,go= ugo=r"
for mode in $yes
do
	chown "$euid:$egid" "$fname"
	chmod "$mode" "$fname"
	checkok "$fname" \
		path_check_wexcl "$euid" "$TMPDIR" "$fname"

	[ "$euid" -ne 0 ] &&
		checkerr "/: writable by UIDs other than $euid" \
			path_check_wexcl "$euid" / "$dirf"

	chown "$euid:$egid" "$fname"
	chmod "$mode" "$dir"
	chmod u+wx,go= "$dir"
	checkok "$dirf" \
		path_check_wexcl "$euid" "$TMPDIR" "$dirf"

	chmod g+w,o= "$dir"
	checkerr "$dir: writable by UIDs other than $euid" \
		path_check_wexcl "$euid" "$TMPDIR" "$dirf"

	chmod g=,o+w "$dir"
	checkerr "$dir: writable by UIDs other than $euid" \
		path_check_wexcl "$euid" "$TMPDIR" "$dirf"
done

