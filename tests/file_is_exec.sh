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
oldtmp="${TMPDIR-}"
tmpdir chk


#
# Main
#

umask 0777

euid="$(id -u)" && [ "$euid" ] ||
	err "failed to get process' effective UID."
egid="$(id -g)" && [ "$egid" ] ||
	err "failed to get process' effective GID."

fname="$TMPDIR/file"
touch "$fname"
chown "$euid:$egid" "$fname"

no="ugo= uo=,g=x ug=,o=x"
for mode in $no
do
	warn "checking $mode with owner $euid and group $egid ..."
	chmod "$mode" "$fname"
	file_is_exec "$fname" &&
		err "reported as executable."
	chmod ugo= "$fname"
done

yes="u=x ug=x ugo=x"
for mode in $yes
do
	warn "checking $mode with owner $euid and group $egid ..."
	chmod "$mode" "$fname"
	file_is_exec "$fname" ||
		err "not reported as executable."
	chmod ugo= "$fname"
done

warn "checking /bin/sh as user $euid ..."
file_is_exec /bin/sh ||
	err "not reported as executable."

euid="$(id -u)" && [ "$euid" ] ||
	err "failed to get process' effective user ID."

if [ "$euid" -ne 0 ]
then
	# shellcheck disable=2154
	warn "${green}success.$reset"
	exit
fi

unalloc_uid="$(unallocid -u 1000 30000)" && [ "$unalloc_uid" ] ||
	err "failed to find an unallocated user ID."
unalloc_gid="$(unallocid -g 1000 30000)" && [ "$unalloc_gid" ] ||
	err "failed to find an unallocated group ID."

chown "$unalloc_uid:$unalloc_gid" "$fname"

warn "checking o= with owner $unalloc_uid and group $unalloc_gid ..."
chmod o= "$fname"
file_is_exec "$fname" &&
	err "reported as executable."

warn "checking o=x with owner $unalloc_uid and group $unalloc_gid ..."
chmod o=x "$fname"
file_is_exec "$fname" ||
	err "not reported as executable."

warn "checking u=x with owner $euid and group $unalloc_gid ..."
chown "$euid" "$fname"
chmod u=x,go= "$fname"
file_is_exec "$fname" ||
        err "not reported as executable."

warn "checking u=x with owner $unalloc_uid and group $egid ..."
chown "$unalloc_uid:$egid" "$fname"
file_is_exec "$fname" &&
        err "reported as executable."

warn "checking g=x with owner $euid and group $unalloc_gid ..."
chown "$euid:$unalloc_gid" "$fname"
chmod uo=,g=x "$fname"
file_is_exec "$fname" &&
        err "reported as executable."

warn "checking g=x with owner $unalloc_uid and group $egid ..."
chown "$unalloc_uid:$egid" "$fname"
file_is_exec "$fname" ||
        err "not reported as executable."

warn "checking g=x with owner $euid and group $egid ..."
chown "$euid:$egid" "$fname"
file_is_exec "$fname" &&
        err "reported as executable."

ruser="$(ls -ld "$script_dir" | cut -d ' ' -f4)"
if ! [ "$ruser" ] || [ "$ruser" = root ]
then
	ruser="$(regularuser)" && [ "$ruser" ] ||
		err "failed to get non-root user."
fi

ruid="$(id -u "$ruser")" && [ "$ruid" ] ||
	err "failed to get user ID of $ruser."
rgid="$(id -g "$ruid")" && [ "$rgid" ] ||
	err "failed to get primary group ID of $ruser."

TMPDIR="$oldtmp" runas "$ruid" "$rgid" "$0"
