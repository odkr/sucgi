#!/bin/sh
#
# Test pathchkfmt
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

path_max="$(getconf PATH_MAX "$TMPDIR")"
[ "$path_max" -lt 0 ] && path_max=255

user="foo"
home="$TMPDIR/foo"
mkdir -p "$home"

wrong="$TMPDIR/wrong"
touch "$wrong"

long_format="$(mklongpath "$TMPDIR/%s" "$((path_max - 1))")"


#
# Main
#

check -s134 -e 'access(fname, F_OK) == 0' \
	pathchkfmt dummy dummy "$home" "$user"

check -s1 -e'expanded user directory is too long' \
	pathchkfmt "$home" "$long_format" "$home" "$user"

check -s1 -e 'realpath <no file!>: No such file or directory' \
	pathchkfmt "$home" '<no file!>' "$home" "$user"

check -s1 -e 'realpath <no file!>: No such file or directory' \
	pathchkfmt "$home" '%s' '<no file!>' "$user"

# shellcheck disable=2016
check -s1 -e 'realpath <no file!>: No such file or directory' \
	pathchkfmt "$home" '%1$s' '<no file!>' "$user"

# shellcheck disable=2016
check -s1 -e 'realpath <no file!>: No such file or directory' \
	pathchkfmt "$wrong" '%2$s' "$home" "<no file!>"

check -s1 -e 'does not match format' \
	pathchkfmt "$wrong" '%s' "$home" "$user"

# shellcheck disable=2016
check -s1 -e 'does not match format' \
	pathchkfmt "$wrong" '%1$s' "$home" "$user"

check -s1 -e 'does not match format' \
	pathchkfmt "$wrong" "$TMPDIR/%2\$s" "$home" "$user"

check pathchkfmt "$home" '%s' "$home" "$user"

# shellcheck disable=2016
check pathchkfmt "$home" '%1$s' "$home" "$user"

check pathchkfmt "$home" "$TMPDIR/%2\$s" "$home" "$user"

warn -g 'all tests passed.'
