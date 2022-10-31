#!/bin/sh
#
# Test priv_drop.
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

export PATH
user="$(id -un)"
uid="$(id -u)"
cd -P "$script_dir" || exit


#
# Test
#

skipc=0
if [ "$uid" -eq 0 ]
then
	owner="$(owner "$src_dir")"
	if [ "$(id -u "$owner")" -eq 0 ]
	then
		warn -y "${bld-}$src_dir${rst_y-} owned by superuser."
		skipc=$((skipc + 1))
	else
		check -s1 -e'Operation not permitted' \
			runas "$owner" priv_drop "$owner"
	fi

	if ! regular="$(reguser 500 30000 1 30000)"
	then
		warn -y "no regular user found."
		skipc=$((skipc + 1))
	else
		uid="$(id -u "$regular")"
		gid="$(id -g "$regular")"
		
		check -s0 -o"euid=$uid egid=$gid ruid=$uid rgid=$gid" \
			priv_drop "$regular"
	fi	

	check -s1 -e'could resume superuser privileges' \
		priv_drop "$user"

	case $skipc in
		(0) warn -g "all tests passed." ;;
		(*) warn -y "${bld-}$skipc${rst_y-} tests skipped." ;;
	esac
else
	check -s1 -e'Operation not permitted' priv_drop "$user"
	warn -y "all non-superuser tests passed."
fi

