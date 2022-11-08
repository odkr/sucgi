#!/bin/sh
#
# Test userdirres.
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

# Load the build configuration
eval "$(main -C | grep -E '^MAX_FNAME=')"

# Get the systems maximum path length in $TMPDIR.
path_max="$(getconf PATH_MAX "$TMPDIR")"
[ "$path_max" -lt 0 ] && path_max=255

# Get user data.
user="$(id -un)"
eval home="~$user"
# shellcheck disable=2154
homebase="$(dirname "$home")"

# Create a path that is as long as the system and suCGI permit.
if [ "$path_max" -lt 0 ]
	then max="$MAX_FNAME"
	else max="$path_max"
fi
long_path="$(mklongpath "$TMPDIR" "$((max - 1))")"
long_format="$(mklongpath "$TMPDIR/%s" "$((max - 1))")"

ln -s '<no file!>' "$TMPDIR/$user"

# Create a path that is longer than the system permits.
huge_path="$(mklongpath "$TMPDIR" "$path_max")"
# shellcheck disable=2016
dirwalk "$TMPDIR" "$huge_path" 'mkdir "$fname"'

# Create a shortcut to the path that is longer than the system permits.
# shellcheck disable=2016
huge_path_link="$TMPDIR/$(dirwalk "$TMPDIR" "$huge_path" \
	'ln -s "$fname" p.d && printf p.d/' \
	'ln -s "$fname" p.f && printf p.f\\n')"

# Create a path that is longer than the suCGI permits.
huge_str="$(mklongpath "$TMPDIR" "$MAX_FNAME")"
# shellcheck disable=2016
dirwalk "$TMPDIR" "$huge_str" 'mkdir "$fname"'

# Create a shortcut to the path that is longer than suCGI permits.
# shellcheck disable=2016
huge_str_link="$TMPDIR/$(dirwalk "$TMPDIR" "$huge_str" \
	'ln -s "$fname" p.d && printf p.d/' \
	'ln -s "$fname" p.f && printf p.f\\n')"


#
# Main
#

check -s134 -e "*s != '\0'" \
	userdirres "" "$user"

check -s1 -e"expanded user directory is too long" \
	userdirres "$long_path" "$user"

check -s1 -e"expanded user directory is too long" \
	userdirres "$long_format" "$user"

check -s1 -e"realpath $TMPDIR/$user: No such file or directory" \
	userdirres "$TMPDIR" "$user"

check -s1 -e"realpath $TMPDIR/$user: No such file or directory" \
	userdirres "$TMPDIR/%s" "$user"

check -s1 -e"too long" \
	userdirres "$huge_path_link" "$user"

check -s1 -e"too long" \
	userdirres "$huge_path_link/%s" "$user"

check -s1 -e"too long" \
	userdirres "$huge_str_link" "$user"

check -s1 -e"too long" \
	userdirres "$huge_str_link/%s" "$user"

check -o"$home" userdirres . "$user"

check -o"$home" userdirres "$homebase" "$user"

check -o"$home" userdirres "$homebase/%s" "$user"

warn -g 'all tests passed.'
