#!/bin/sh
#
# Test priv_drop.
#
# Copyright 2022 Odin Kroeger.
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

eval "$(main -C | grep -E '^(MIN|MAX)_(UID|GID)=')"
: "${MIN_UID:?}" "${MAX_UID:?}" "${MIN_GID:?}" "${MAX_GID:?}"

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
		warn "$src_dir owned by superuser."
		skipc=$((skipc + 1))
	else
		check -s1 -e'Operation not permitted' \
			runas "$owner" priv_drop "$owner"
	fi

	if ! regular="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")"
	then
		warn "no regular user found."
		skipc=$((skipc + 1))
	else
		uid="$(id -u "$regular")"
		gid="$(id -g "$regular")"

		check -s0 -o"euid=$uid egid=$gid ruid=$uid rgid=$gid" \
			priv_drop "$regular"
	fi

	check -s134 -e'uid > 0' \
		priv_drop "$user"

	[ $skipc -eq 0 ] && err -s0 'all tests passed.'

	err -s75 '%d tests skipped.' "$skipc"
else
	check -s1 -e'Operation not permitted' priv_drop "$user"
	warn "all non-superuser tests passed."
fi

