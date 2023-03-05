#!/bin/sh
#
# Test path_check_wexcl.
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
# Configuration
#

eval $(main -C | grep -vE ^PATH=)


#
# Prelude
#

user="$(id -un)"
uid="$(id -u "$user")"

dir="$TMPDIR/dir"
mkdir "$dir"

shallow="$TMPDIR/file"
touch "$shallow"

deeper="$dir/file"
touch "$deeper"
chmod 600 "$deeper"

root="$(ids | awk '$1 == 0 {print $2; exit}')"
bin="$(cd -P /bin && pwd)"

long_fname="$(pad / $((MAX_FNAME_LEN - 1)) x r)"
err_fname="${long_fname}c"


#
# Main
#

if ! [ "${NDEBUG-}" ]
then
	check -s134 -e"*basedir == '/'" \
	      path_check_wexcl "$user" '' /
	check -s134 -e"*basedir == '/'" \
	      path_check_wexcl "$user" relpath /
	check -s134 -e"*basedir == '/'" \
	      path_check_wexcl "$user" . /

	check -s134 -e"*fname == '/'" \
	      path_check_wexcl "$user" / ''
	check -s134 -e"*fname == '/'" \
	      path_check_wexcl "$user" / relpath
	check -s134 -e"*fname == '/'" \
	      path_check_wexcl "$user" / .

	check -s1 \
	      path_check_wexcl "$user" "$long_fname" /
	check -s1 \
	      path_check_wexcl "$user" / "$long_fname"
	check -s134 -e 'strnlen(basedir, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN' \
	      path_check_wexcl "$user" "$err_fname" /
	check -s134 -e 'strnlen(fname, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN' \
	      path_check_wexcl "$user" / "$err_fname"
fi

no="u=r,g=w,o= u=r,g=,o=w u=rw,g=w,o= u=rw,g=,o=w u=rw,go=w u=rw,go=w"
for mode in $no
do
	chmod "$mode" "$shallow"
	check -s1 -e"$shallow is writable by users other than $user" \
		path_check_wexcl "$user" "$TMPDIR" "$shallow"

	chmod "$mode" "$dir"
	chmod u+x "$dir"
	check -s1 -e"$deeper is writable by users other than $user" \
		path_check_wexcl "$user" "$TMPDIR" "$deeper"
done


yes="u=rw,go= ugo=r"
for mode in $yes
do
	chmod go-w "$dir" "$shallow" "$deeper"

	chmod "$mode" "$shallow"
	check path_check_wexcl "$user" "$TMPDIR" "$shallow"
	[ "$uid" -ne 0 ] &&
		check -s1 -e"$deeper is writable by users other than $user" \
			path_check_wexcl "$user" / "$deeper"

	chmod "$mode" "$dir"
	chmod u+wx,go= "$dir"
	check path_check_wexcl "$user" "$TMPDIR" "$deeper"

	chmod g+w,o= "$deeper"
	check -s1 -e"$deeper is writable by users other than $user" \
		path_check_wexcl "$user" "$TMPDIR" "$deeper"

	chmod g=,o+w "$deeper"
	check -s1 -e"$deeper is writable by users other than $user" \
		path_check_wexcl "$user" "$TMPDIR" "$deeper"
done


check path_check_wexcl "$root" "$bin" "$bin/cat"


warn "all tests passed."
