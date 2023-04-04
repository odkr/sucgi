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
tests_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(cd -P "$tests_dir/.." && pwd)"
readonly src_dir tests_dir
# shellcheck disable=1091
. "$src_dir/scripts/funcs.sh" || exit
init || exit
tmpdir chk
cd "$tests_dir" || exit


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

result=0

if [ "$uid" -eq 0 ]
then
	reguser="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")" ||
		err -s75 'no regular user found.'

	runas "$reguser" main -C >/dev/null 2>&1 ||
		err -s75 '%s cannot execute main.' "$reguser"

	reguid="$(id -u "$reguser")"
	reggid="$(id -g "$reguser")"

	check -s1 -o"euid=$reguid egid=$reggid ruid=$reguid rgid=$reggid" \
		runas "$reguser" "$tests_dir/priv_suspend" || result=70
	check -s1 -e'Operation not permitted' \
		runas "$reguser" "$tests_dir/priv_suspend" || result=70
	check -o"euid=$reguid egid=$reggid ruid=$reguid rgid=$reggid" \
		runas -r "$reguser" "$tests_dir/priv_suspend" || result=70

	exit "$result"
else
	uid="$(id -u)"
	gid="$(id -g)"

	check -s1 -o"euid=$uid egid=$gid ruid=$uid rgid=$gid" \
		priv_suspend || result=70
	check -s1 -e'Operation not permitted' \
		priv_suspend || result=70

	case $result in
	(0) err -s75 'skipped superuser tests.' ;;
	(*) exit "$result"
	esac
fi

# Unreachable.
# shellcheck disable=2317
exit 69
