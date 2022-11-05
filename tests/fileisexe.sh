#!/bin/sh
#
# Test if fileisexe correctly identifies executables.
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
script_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(cd -P "$script_dir/.." && pwd)"
tools_dir="$src_dir/tools"
readonly script_dir src_dir tools_dir
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit
tmpdir tmp "$src_dir"


#
# Prelude
#

umask 0777

uid="$(id -u)"
gid="$(id -g)"

fname="$TMPDIR/file"
printf '#!/bin/sh\n:\n' >"$fname"
chown "$uid:$gid" "$fname"


#
# Non-root tests.
#

no="ugo= uo=,g=x ug=,o=x"
for mode in $no
do
	warn "checking ${bld-}$mode${rst-} with" \
	     "owner ${bld-}$uid${rst-} and group ${bld-}$gid${rst-} ..."
	chmod "$mode" "$fname"
	fileisexe "$fname" &&
		err -s70 "reported as executable."
	chmod ugo= "$fname"
done

yes="u=x ug=x ugo=x"
for mode in $yes
do
	warn "checking ${bld-}$mode${rst-} with" \
	     "owner ${bld-}$uid${rst-} and group ${bld-}$gid${rst-} ..."
	chmod "$mode" "$fname"
	fileisexe "$fname" ||
		err -s70 "reported as not executable."
	chmod ugo= "$fname"
done

warn "checking ${bld-}/bin/sh${rst-} as user ${bld-}$uid${rst-} ..."
[ "$uid" -eq 0 ] || [ -x /bin/sh ] ||
	err "/bin/sh is not executable."
fileisexe /bin/sh ||
	err -s70 "not reported as executable."

uid="$(id -u)" && [ "$uid" ] ||
	err "failed to get process' effective user ID."

if [ "$uid" -ne 0 ]
then
	[ $(id -u $(ps -o user= $PPID)) -ne 0 ] &&
		warn -y "all non-superuser tests passed."
	exit 0
fi


#
# Root tests
#

unalloc_uid="$(unallocid -u 1000 30000)"
unalloc_gid="$(unallocid -g 1000 30000)"

chown "$unalloc_uid:$unalloc_gid" "$fname"

warn "checking ${bld-}o=${rst-} with" \
     "owner ${bld-}$unalloc_uid${rst-} and" \
     "group ${bld-}$unalloc_gid${rst-} ..."
chmod o= "$fname"
fileisexe "$fname" && err "reported as executable."

warn "checking ${bld-}o=x${rst-} with" \
     "owner ${bld-}$unalloc_uid${rst-} and" \
     "group ${bld-}$unalloc_gid${rst-} ..."
chmod o=x "$fname"
fileisexe "$fname" || err -s70 "not reported as executable."

warn "checking ${bld-}o=x${rst-} with" \
     "owner ${bld-}$uid${rst-} and group ${bld-}$unalloc_gid${rst-} ..."
chown "$uid" "$fname"
chmod u=x,go= "$fname"
fileisexe "$fname" || err -s70 "reported as not executable."

warn "checking ${bld-}u=x${rst-} with" \
     "owner ${bld-}$unalloc_uid${rst-} and group ${bld-}$gid${rst-} ..."
chown "$unalloc_uid:$gid" "$fname"
fileisexe "$fname" && err -s70 "reported as executable."

warn "checking ${bld-}g=x${rst-} with" \
     "owner ${bld-}$uid${rst-} and group ${bld-}$unalloc_gid${rst-} ..."
chown "$uid:$unalloc_gid" "$fname"
chmod uo=,g=x "$fname"
fileisexe "$fname" && err -s70 "reported as executable."

warn "checking ${bld-}g=x${rst-} with" \
     "owner ${bld-}$unalloc_uid${rst-} and group ${bld-}$gid${rst-} ..."
chown "$unalloc_uid:$gid" "$fname"
fileisexe "$fname" || err -s70 "reported as not executable."

warn "checking ${bld-}g=x${rst-} with" \
     "owner ${bld-}$uid${rst-} and group ${bld-}$gid${rst-} ..."
chown "$uid:$gid" "$fname"
fileisexe "$fname" && err -s70 "reported as executable."


#
# Run tests as non-root user
#

if ! owner="$(owner "$src_dir")" || [ "$(id -u "$owner")" -eq 0 ]
then
	warn -y "${bld-}$src_dir${rst_y-} owned by superuser, skipping tests."
	exit 1
fi

# shellcheck disable=2154
runas "$owner" "$script_dir/$prog_name"

warn -g "all tests passed."