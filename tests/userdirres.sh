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

path_max="$(getconf PATH_MAX "$TMPDIR")"
[ "$path_max" -lt 0 ] && path_max=255

user="$(id -un)"
eval home="~$user"
# shellcheck disable=2154
homebase="$(dirname "$home")"

long_path="$(mklongpath "$TMPDIR" "$((path_max - 1))")"
long_format="$(mklongpath "$TMPDIR/%s" "$((path_max - 1))")"

ln -s '<no file!>' "$TMPDIR/$user"



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

# FIXME
# resolved thingy is too long
# once for /foo/%s and once for /foo


check -o"$home" userdirres . "$user"

check -o"$home" userdirres "$homebase" "$user"

check -o"$home" userdirres "$homebase/%s" "$user"

warn -g 'all tests passed.'
