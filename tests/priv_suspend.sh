#!/bin/sh
#
# Test priv_suspend.
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
tests_dir="$src_dir/tests"
readonly script_dir src_dir tests_dir
# shellcheck disable=1091
. "$src_dir/scripts/lib.sh" || exit
init || exit
tmpdir chk


#
# Build configuration
#

eval "$(main -C | grep -E '^(MIN|MAX)_(UID|GID)=')"
: "${MIN_UID:?}" "${MAX_UID:?}" "${MIN_GID:?}" "${MAX_GID:?}"


#
# Setup
#

export PATH
user="$(id -un)" uid="$(id -u)"


#
# Test
#

if [ "$uid" -eq 0 ]
then
	regular="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")" ||
		err -s75 'no regular user found.'

	uid="$(id -u "$regular")"
	gid="$(id -g "$regular")"

	check -s1 -o"euid=$uid egid=$gid ruid=$uid rgid=$gid" \
		runas "$regular" priv_suspend
	check -s1 -e"seteuid: Operation not permitted" \
		runas "$regular" priv_suspend
	check -o"euid=$uid egid=$gid ruid=$uid rgid=$gid" \
		runas -r "$regular" priv_suspend

	warn 'all tests passed.'
else
	uid="$(id -u)"
	gid="$(id -g)"

	check -s1 -o"euid=$uid egid=$gid ruid=$uid rgid=$gid"	priv_suspend
	check -s1 -e"seteuid: Operation not permitted"		priv_suspend

	warn 'all non-superuser tests passed.'
fi

