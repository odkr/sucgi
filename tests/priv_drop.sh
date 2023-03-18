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
readonly script_dir src_dir
# shellcheck disable=1091
. "$src_dir/scripts/funcs.sh" || exit
init || exit
tmpdir chk


#
# Build configuration
#

eval "$(main -C | grep -vE ^PATH=)"
: "${MIN_UID:?}" "${MAX_UID:?}" "${MIN_GID:?}" "${MAX_GID:?}"


#
# Setup
#

export PATH
user="$(id -un)" uid="$(id -u)"


#
# Assertions
#

if ! [ "${NDEBUG-}" ]
then
	root="$(ids | awk '$1 == 0 {print $2; exit}')"
	check -s134 -e'uid > 0' priv_drop "$root"
fi


#
# Test
#

result=0
if [ "$uid" -eq 0 ]
then
	reguser="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")" ||
		err -s75 "no regular user user found."

	uid="$(id -u "$reguser")"
	gid="$(id -g "$reguser")"

	check -s1 -e'Operation not permitted' \
		runas "$reguser" priv_drop "$reguser" || result=70
	check -s0 -o"euid=$uid egid=$gid ruid=$uid rgid=$gid" \
		priv_drop "$reguser" || result=70

	exit "$result"
else
	check -s1 -e'Operation not permitted' \
		priv_drop "$user" || result=70

	case $result in
	(0) err -s75 "skipped superuser tests." ;;
	(*) exit "$result"
	esac
fi

# Unreachable.
# shellcheck disable=2317
exit 69
