#!/bin/sh
#
# Test if fileiswex correctly identifies exclusive write access.
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
tmpdir chk


#
# Prelude
#

umask 0777
user="$(id -un)"
group="$(id -gn)"
fname="$TMPDIR/file"
touch "$fname"


#
# Main
#

for mode in g=w o=w ug=w uo=w go=w ugo=w
do
	warn "checking ${bld-}$mode${rst-} ..."
	chown "$user:$group" "$fname"
	chmod "$mode" "$fname"
	# shellcheck disable=2154
	fileiswex "$user" "$fname" &&
		err -s70 "reported as exclusively writable."
	chmod ugo= "$fname"
done

for mode in u=w ugo=
do
	warn "checking ${bld-}$mode${rst-} ..."
	chown "$user:$group" "$fname"
	chmod "$mode" "$fname"
	# shellcheck disable=2154
	fileiswex "$user" "$fname" ||
		err -s70 "reported as not exclusively writable."
	chmod ugo= "$fname"
done

owner="$(owner /bin/sh)"
warn "checking whether ${bld-}/bin/sh${rst-} is" \
     "exclusively writable by ${bld-}$owner${rst-} ..."
fileiswex "$owner" /bin/sh ||
	err -s70 "reported as not exclusively writable."

warn -g "all tests passed."
