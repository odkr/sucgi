#!/bin/sh
#
# Test validation of script owner.
#
# Copyright 2023 Odin Kroeger.
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
# shellcheck disable=1091,2015,2016,2034

#
# Initialisation
#

set -Cefu
script_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(cd -P "$script_dir/.." && pwd)"
tests_dir="$src_dir/tests"
readonly script_dir src_dir tests_dir
# shellcheck disable=1091
. "$tests_dir/lib.sh" || exit
init || exit
tmpdir


#
# Build configuration
#

eval "$(main -C | grep -vE ^PATH=)"


#
# Setup
#

uid="$(id -u)"
[ "$uid" -eq 0 ] || err -s75 'not invoked by the superuser.'

# Create a script.
PATH_TRANSLATED="$TMPDIR/script.sh"
touch "$PATH_TRANSLATED"
export PATH_TRANSLATED

# Find an unallocated user ID.
unallocuid="$(unallocid $MIN_UID $MAX_UID)"

# Store a list of all user IDs.
uids="$TMPDIR/uids.list"
ids >"$uids"

# Find a user with an ID < $MIN_UID
lowuid="$(awk -vmax="$MIN_UID" '0 < $1 && $1 < max {print $2; exit}' "$uids")"

# Find a user with an ID > $MAX_UID
highuid="$(awk -vmin="$MAX_UID" '$1 > min {print $2; exit}' "$uids")"


#
# Tests
#

# Owned by non-existing user.
chown "$unallocuid" "$PATH_TRANSLATED"
check -s1 -e"$PATH_TRANSLATED has no owner." main

# Owned by a privileged user
for uid in 0 "$lowuid" "$highuid"
do
	[ "$uid" ] || continue
	chown "$uid" "$PATH_TRANSLATED"
	check -s1 -e"$PATH_TRANSLATED is owned by privileged user" main
done


#
# Success
#

warn 'all tests passed.'
