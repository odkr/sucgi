#!/bin/sh
#
# Test if path_check_wexcl correctly identifies exclusive write access.
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
readonly script_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
readonly src_dir="$(cd -P "$script_dir/.." && pwd)"
readonly tools_dir="$src_dir/tools"
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit
tmpdir tmp .


#
# Prelude 
#

euid="$(id -u)"
egid="$(id -g)"
shallow="$TMPDIR/file"
touch "$shallow"
dir="$TMPDIR/dir"
mkdir "$dir"
deeper="$dir/file"
touch "$deeper"
chmod 600 "$deeper"


#
# Main
#

no="u=r,g=w,o= u=r,g=,o=w u=rw,g=w,o= u=rw,g=,o=w u=rw,go=w u=rw,go=w"
for mode in $no
do
	chown "$euid:$egid" "$shallow"
	chmod "$mode" "$shallow"
	checkerr "$shallow is writable by user IDs other than $euid" \
		path_check_wexcl "$euid" "$TMPDIR" "$shallow"

	chown "$euid:$egid" "$dir"
	chmod "$mode" "$dir"
	chmod u+x "$dir"
	checkerr "$dir is writable by user IDs other than $euid" \
		path_check_wexcl "$euid" "$TMPDIR" "$deeper"
done

yes="u=rw,go= ugo=r"
for mode in $yes
do
	chown "$euid:$egid" "$shallow"
	chmod "$mode" "$shallow"
	checkok "$shallow" \
		path_check_wexcl "$euid" "$TMPDIR" "$shallow"

	[ "$euid" -ne 0 ] &&
		checkerr "/ is writable by user IDs other than $euid" \
			path_check_wexcl "$euid" / "$deeper"

	chown "$euid:$egid" "$shallow"
	chmod "$mode" "$dir"
	chmod u+wx,go= "$dir"
	checkok "$deeper" \
		path_check_wexcl "$euid" "$TMPDIR" "$deeper"

	chmod g+w,o= "$dir"
	checkerr "$dir is writable by user IDs other than $euid" \
		path_check_wexcl "$euid" "$TMPDIR" "$deeper"

	chmod g=,o+w "$dir"
	checkerr "$dir is writable by user IDs other than $euid" \
		path_check_wexcl "$euid" "$TMPDIR" "$deeper"
done

warn -g "all tests passed."
