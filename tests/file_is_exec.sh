#!/bin/sh
#
# Test if file_is_exec correctly identifies executables.
#
# Copyright 2022 Odin Kroeger
#
# This file is part of suCGI.
#
# suCGI is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# suCGI is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with suCGI. If not, see <https://www.gnu.org/licenses>.
#
# shellcheck disable=1091,2015

#
# Initialisation
#

set -Cefu
readonly script_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
readonly src_dir="$(cd -P "$script_dir/.." && pwd)"
readonly tools_dir="$src_dir/tools"
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit
oldtmp="${TMPDIR-}"
tmpdir chk


#
# Prelude
#

umask 0777

euid="$(id -u)"
egid="$(id -g)"

fname="$TMPDIR/file"
printf '#!/bin/sh\n:\n' >"$fname"
chown "$euid:$egid" "$fname"


#
# Non-root tests.
#

no="ugo= uo=,g=x ug=,o=x"
for mode in $no
do
	warn "checking ${bld-}$mode${rst-} with" \
	     "owner ${bld-}$euid${rst-} and group ${bld-}$egid${rst-} ..."
	chmod "$mode" "$fname"
	[ "$euid" -ne 0 ] && [ -x "$fname" ] &&
		err "$fname is executable."
	file_is_exec "$fname" &&
		err "reported as executable."
	chmod ugo= "$fname"
done

yes="u=x ug=x ugo=x"
for mode in $yes
do
	warn "checking ${bld-}$mode${rst-} with" \
	     "owner ${bld-}$euid${rst-} and group ${bld-}$egid${rst-} ..."
	chmod "$mode" "$fname"
	[ "$euid" -eq 0 ] || [ -x "$fname" ] || 
		err "$fname is not executable."
	file_is_exec "$fname" ||
		err "reported as not executable."
	chmod ugo= "$fname"
done

warn "checking ${bld-}/bin/sh${rst-} as user ${bld-}$euid${rst-} ..."
[ "$euid" -eq 0 ] || [ -x /bin/sh ] || 
	err "/bin/sh is not executable."
file_is_exec /bin/sh ||
	err "not reported as executable."

euid="$(id -u)" && [ "$euid" ] ||
	err "failed to get process' effective user ID."

if [ "$euid" -ne 0 ]
then
	warn -g "all tests passed."
	exit
fi


#
# Root tests
#

unalloc_uid="$(findid -nu 1000 30000)"
unalloc_gid="$(findid -ng 1000 30000)"

chown "$unalloc_uid:$unalloc_gid" "$fname"

warn "checking ${bld-}o=${rst-} with" \
     "owner ${bld-}$unalloc_uid${rst-} and" \
     "group ${bld-}$unalloc_gid${rst-} ..."
chmod o= "$fname"
file_is_exec "$fname" && err "reported as executable."

warn "checking ${bld-}o=x${rst-} with" \
     "owner ${bld-}$unalloc_uid${rst-} and" \
     "group ${bld-}$unalloc_gid${rst-} ..."
chmod o=x "$fname"
file_is_exec "$fname" || err "not reported as executable."

warn "checking ${bld-}o=x${rst-} with" \
     "owner ${bld-}$euid${rst-} and group ${bld-}$unalloc_gid${rst-} ..."
chown "$euid" "$fname"
chmod u=x,go= "$fname"
file_is_exec "$fname" || err "reported as not executable."

warn "checking ${bld-}u=x${rst-} with" \
     "owner ${bld-}$unalloc_uid${rst-} and group ${bld-}$egid${rst-} ..."
chown "$unalloc_uid:$egid" "$fname"
file_is_exec "$fname" && err "reported as executable."

warn "checking ${bld-}g=x${rst-} with" \
     "owner ${bld-}$euid${rst-} and group ${bld-}$unalloc_gid${rst-} ..."
chown "$euid:$unalloc_gid" "$fname"
chmod uo=,g=x "$fname"
file_is_exec "$fname" && err "reported as executable."

warn "checking ${bld-}g=x${rst-} with" \
     "owner ${bld-}$unalloc_uid${rst-} and group ${bld-}$egid${rst-} ..."
chown "$unalloc_uid:$egid" "$fname"
file_is_exec "$fname" || err "reported as not executable."

warn "checking ${bld-}g=x${rst-} with" \
     "owner ${bld-}$euid${rst-} and group ${bld-}$egid${rst-} ..."
chown "$euid:$egid" "$fname"
file_is_exec "$fname" && err "reported as executable."


#
# Run tests as non-root user
#

regular="$(owner "$0")"
! [ "$regular" ] || [ "$regular" = root ] && regular=$(regularuser)
! [ "$regular" ] && warn -y "could not run all tests."

uid="$(id -u "$regular")"
gid="$(id -g "$regular")"

TMPDIR="$oldtmp" runas "$uid" "$gid" "$script_dir/$prog_name"
