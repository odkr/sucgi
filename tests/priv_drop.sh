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
euid="$(id -u)"
cd -P "$script_dir" || exit


#
# Test
#

if [ "$euid" -eq 0 ]
then
	owner="$(owner priv_drop)"
	regular="$(regularuser)"

	[ "$owner" ] && [ "$owner" != root ] && (
		uid="$(id -u "$owner")"
		gid="$(id -g "$owner")"
		checkerr 'Operation not permitted' \
			runas "$uid" "$gid" priv_drop "$regular"
	)

	[ "$regular" ] && [ "$regular" != root ] && (
		uid="$(id -u "$regular")"
		gid="$(id -g "$regular")"
		
		checkok "euid=$uid egid=$gid ruid=$uid rgid=$gid" \
			priv_drop "$regular"
	)

	checkerr 'could resume superuser privileges' \
		priv_drop "$user"
else
	checkerr 'Operation not permitted' priv_drop "$user"
fi

warn -g "all tests passed."

