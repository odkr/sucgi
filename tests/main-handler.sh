#!/bin/sh
#
# Test script handling.
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

eval "$(main-handler -C | grep -vE ^PATH=)"


#
# Setup
#

# User ID.
uid="$(id -u)"
[ "$uid" -eq 0 ] || err -s75 'not invoked by the superuser.'

# Search for a regular user.
reguser="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")"
[ "$reguser" ] || err -s75 'no regular user found.'

# Determine user directory.
case $USER_DIR in
(/*%s*) userdir="$(printf -- "$USER_DIR" "$reguser")" ;;
(*%s*)	err 'user directories within home directories are unsupported.' ;;
(*)	userdir="$USER_DIR/$reguser" ;;
esac

# Create a temporary directory.
tmpdir="${userdir%/$reguser*}"
case $tmpdir in
(/tmp/*)	: ;;
(*)		err 'temporary directory %s is outside of /tmp.' "$tmpdir"
esac
readonly tmpdir

catch=
mkdir -m 0755 "$tmpdir"
cleanup="rm -rf \"\$tmpdir\"; ${cleanup}"
catch=x
[ "$caught" ] && kill -"$caught" $$

TMPDIR="$tmpdir"
export TMPDIR

# Create the user directory.
mkdir -p "$userdir"
tmp="$(cd -P "$userdir" && pwd)" && [ "$tmp" ] || exit
userdir="$tmp"
readonly userdir
unset tmp

# Create a script w/o a suffix.
nosuffix="$userdir/script"
touch "$nosuffix"

# Create a script with an unknown suffix.
unknownsuffix="$userdir/script.nil"
touch "$unknownsuffix"

# Create a script for which an empty handler is registered.
emptyhandler="$userdir/script.empty"
touch "$emptyhandler"

# Create a script with a too long filename suffix.
suffix=$(pad . $MAX_SUFFIX_LEN x r)
hugesuffix="$userdir/script$suffix"
touch "$hugesuffix"
unset suffix

# Adapt ownership.
chown -R "$reguser" "$userdir"


#
# Tests
#

# Bad handler.
check -s1 -e"script $emptyhandler: bad handler." \
	PATH_TRANSLATED="$emptyhandler" main-handler

# Suffix is too long.
check -s1 -e"script $hugesuffix: filename suffix too long." \
	PATH_TRANSLATED="$hugesuffix" main-handler

# No known suffix.
for script in "$nosuffix" "$unknownsuffix"
do
	check -s1 -e"script $script: no handler found." \
		PATH_TRANSLATED="$script" main-handler
done


#
# Success
#

warn 'all tests passed.'
