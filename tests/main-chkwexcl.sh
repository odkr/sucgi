#!/bin/sh
#
# Test whether non-exclusively writable paths are rejected.
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
. "$tests_dir/lib.sh" || exit
init || exit
tmpdir chk


#
# Build configuration
#

eval $(main -C | grep -vE ^PATH=)


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

lock "$tests_dir/tmpdir.lock"

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

# Create a file in the user directory.
shallow="$userdir/file"
touch "$shallow"

# Create a file in a sub-directory.
subdir="$userdir/dir"
mkdir "$subdir"
deeper="$subdir/file"
touch "$deeper"

# Change ownership.
chown -R "$reguser" "$userdir"


#
# Not exclusively writable
#

no="u=r,g=w,o= u=r,g=,o=w u=rw,g=w,o= u=rw,g=,o=w u=rw,go=w u=rw,go=w"
for mode in $no
do
	chmod "$mode" "$shallow"
	check -s1 -e"script $shallow: writable by users other than $reguser" \
		PATH_TRANSLATED="$shallow" main

	chmod "$mode" "$subdir"
	chmod u+x "$subdir"
	check -s1 -e"script $deeper: writable by users other than $reguser" \
		PATH_TRANSLATED="$deeper" main
done


#
# Exclusively writable
#

yes="u=rw,go= ugo=r"
for mode in $yes
do
	chmod go-w "$subdir" "$shallow" "$deeper"

	chmod "$mode" "$shallow"
	check -s1 -e"script $shallow: no handler found." \
		PATH_TRANSLATED="$shallow" main

	chmod "$mode" "$subdir"
	chmod u+wx,go= "$subdir"
	check -s1 -e"script $deeper: no handler found." \
		PATH_TRANSLATED="$deeper" main

	chmod g+w,o= "$deeper"
	check -s1 -e"script $deeper: writable by users other than $reguser" \
		PATH_TRANSLATED="$deeper" main

	chmod g=,o+w "$deeper"
	check -s1 -e"script $deeper: writable by users other than $reguser" \
		PATH_TRANSLATED="$deeper" main
done


#
# Success
#

warn "all tests passed."
