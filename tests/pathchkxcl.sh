#!/bin/sh
#
# Test if pathchkxcl correctly identifies exclusive write access.
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

user="$(id -un)"
uid="$(id -u "$user")"

shallow="$TMPDIR/file"
touch "$shallow"

dir="$TMPDIR/dir"
mkdir "$dir"

deeper="$dir/file"
touch "$deeper"
chmod 600 "$deeper"

root="$(id -un 0)"
bin="$(cd -P /bin && pwd)"


#
# Main
#

no="u=r,g=w,o= u=r,g=,o=w u=rw,g=w,o= u=rw,g=,o=w u=rw,go=w u=rw,go=w"
for mode in $no
do
	chmod "$mode" "$shallow"
	check -s1 -e"$shallow is writable by users other than $user" \
		pathchkxcl "$user" "$shallow" "$TMPDIR"

	chmod "$mode" "$dir"
	chmod u+x "$dir"
	check -s1 -e"$dir is writable by users other than $user" \
		pathchkxcl "$user" "$deeper" "$TMPDIR"
done

yes="u=rw,go= ugo=r"
for mode in $yes
do
	chmod "$mode" "$shallow"
	check -o"$shallow" pathchkxcl "$user" "$shallow" "$TMPDIR"

	[ "$uid" -ne 0 ] &&
		check -s1 -e"/ is writable by users other than $user" \
			pathchkxcl "$user" "$deeper" /

	chmod "$mode" "$dir"
	chmod u+wx,go= "$dir"
	check -o"$deeper" pathchkxcl "$user" "$deeper" "$TMPDIR"

	chmod g+w,o= "$dir"
	check -s1 -e"$dir is writable by users other than $user" \
		pathchkxcl "$user" "$deeper" "$TMPDIR"

	chmod g=,o+w "$dir"
	check -s1 -e"$dir is writable by users other than $user" \
		pathchkxcl "$user" "$deeper" "$TMPDIR"
done

check -o"$bin/cat" pathchkxcl "$root" "$bin/cat" "$bin"

warn -g "all tests passed."
