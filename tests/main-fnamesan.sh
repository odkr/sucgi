#!/bin/sh
#
# Test sanitisation of $PATH_TRANSLATED.
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
# Functions
#

# Delete every segment of $path up to $stop. Should ignore PATH_MAX.
rmtree() (
	path="${1:?}" stop="${2:?}"

	while true
	do
		case $path in
		($stop/*)	: ;;
		(*)		break ;;
		esac

		dirwalk "$stop" "$path" : 'rm -rf "$fname"'

		path="$(dirname "$path")"
	done
)


#
# Build configuration
#

eval "$(main -C | grep -vE ^PATH=)"


#
# Setup
#

# User ID.
uid="$(id -u)"

# Create a path that is as long as suCGI permits.
varname='PATH_TRANSLATED='
maxlen="$((MAX_VAR_LEN - ${#varname}))"
[ "$maxlen" -gt "$MAX_FNAME_LEN" ] && maxlen="$MAX_FNAME_LEN"
readonly longpath="$(mklongpath "$TMPDIR" "$((maxlen - 1))")"
mkdir -p "$(dirname -- "$longpath")"
touch "$longpath"
unset varname maxlen

# Create a path that is longer than suCGI permits.
readonly hugepath="$(mklongpath "$TMPDIR" "$MAX_FNAME_LEN")"
catch=
dirwalk "$TMPDIR" "$hugepath" 'mkdir "$fname"' 'touch "$fname"'
cleanup="rmtree \"$hugepath\" \"$TMPDIR\"; ${cleanup-}"
catch=x
[ "$caught" ] && kill "-$caught" $$

# Create a link to that path.
catch=
readonly hugelink="$TMPDIR/$(dirwalk "$TMPDIR" "$hugepath" \
	'ln -s "$fname" s.d && printf s.d/' \
	'ln -s "$fname" s.f && printf s.f\\n')"
cleanup="rmtree \"$hugelink\" \"$TMPDIR\"; ${cleanup-}"
catch=x
[ "$caught" ] && kill "-$caught" $$

# Create a file of the wrong type.
wrong="$TMPDIR/wrong"
mkdir "$wrong"

# Create a valid script.
script="$TMPDIR/valid.php"
touch "$script"


#
# Tests
#

# PATH_TRANSLATED is undefined.
check -s1 -e'$PATH_TRANSLATED not set.'	\
	main

# $PATH_TRANSLATED is empty.
check -s1 -e'$PATH_TRANSLATED is empty.' \
	PATH_TRANSLATED= main

# $PATH_TRANSLATED is too long.
check -s1 -e'long' \
	PATH_TRANSLATED="$hugepath" main

# Path to script is too long after having been resolved.
check -s1 -e'long' \
	PATH_TRANSLATED="$hugelink" main

# Script is of the wrong type.
check -s1 -e"$wrong is not a regular file." \
	PATH_TRANSLATED="$wrong" main

# Script does not exist.
check -s1 -e"realpath $TMPDIR/<nosuchfile>: No such file or directory." \
	PATH_TRANSLATED="$TMPDIR/<nosuchfile>" main

# PATH_TRANSLATED is valid.
case $uid in
(0) err="$script is owned by privileged user root." ;;
(*) err='seteuid: Operation not permitted.' ;;
esac
check -s1 -e"$err" PATH_TRANSLATED="$script" main

# $PATH_TRANSLATED is valid despite its long name.
case $uid in
(0) err="$longpath is owned by privileged user root." ;;
(*) err='seteuid: Operation not permitted.' ;;
esac
check -s1 -e"$err" PATH_TRANSLATED="$longpath" main


#
# Success
#

warn 'all tests passed.'
