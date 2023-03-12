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
# Build configuration
#

eval $(main -C | grep -vE ^PATH=)


#
# Setup
#

user="$(id -un)"
uid="$(id -u "$user")"

basedir="$TMPDIR/dir"
mkdir "$basedir"

shallow="$TMPDIR/file"
touch "$shallow"

deeper="$basedir/file"
touch "$deeper"


#
# Assertions
#

if ! [ "${NDEBUG-}" ]
then
	long_fname="$(pad / $((MAX_FNAME_LEN - 1)) x r)"
	err_fname="${long_fname}x"

	check -s134 -e"*basedir == '/'"	path_check_wexcl "$user" '' /
	check -s134 -e"*basedir == '/'"	path_check_wexcl "$user" relpath /
	check -s134 -e"*basedir == '/'"	path_check_wexcl "$user" . /

	check -s134 -e"*fname == '/'" 	path_check_wexcl "$user" / ''
	check -s134 -e"*fname == '/'"	path_check_wexcl "$user" / relpath
	check -s134 -e"*fname == '/'"	path_check_wexcl "$user" / .

	check -s1 path_check_wexcl "$user" "$long_fname" /
	check -s1 path_check_wexcl "$user" / "$long_fname"

	check -s134 \
	      -e'strnlen(basedir, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN' \
	      path_check_wexcl "$user" "$err_fname" /
	check -s134 \
	      -e'strnlen(fname, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN' \
	      path_check_wexcl "$user" / "$err_fname"
fi


#
# Bad arguments
#

for pair in					\
	"/:/"					\
	"/foo:/bar"				\
	"/bar:/foo"				\
	"/foo:/foobar"				\
	"/foo:/"				\
	"/foo:/foo"				\
	"/home/jdoe:/srv/www/jdoe"		\
	"/srv/www:/home/jdoe/public_html"	\
	"/𝒇ȫǭ:/𝕓ắ𝚛"				\
	"/𝕓ắ𝚛:/𝒇ȫǭ"				\
	"/𝒇ȫǭ:/𝒇ȫǭ𝕓ắ𝚛"				\
	"/𝒇ȫǭ:/"				\
	"/𝒇ȫǭ:/𝒇ȫǭ"				\
	"/home/⒥𝑑𝓸𝖊:/srv/www/⒥𝑑𝓸𝖊"		\
	"/srv/www:/home/⒥𝑑𝓸𝖊/public_html"
do
	IFS=:
	set -- $pair
	unset IFS

	check -s1 -e"file $2: not within $1" \
		path_check_wexcl "$user" "$1" "$2"
done


#
# Non-existant files
#

check -s1 -e "stat $TMPDIR/<nosuchfile>: No such file or directory" \
	path_check_wexcl "$user" "$TMPDIR" "$TMPDIR/<nosuchfile>"


#
# Not exclusively writable
#

no="u=r,g=w,o= u=r,g=,o=w u=rw,g=w,o= u=rw,g=,o=w u=rw,go=w u=rw,go=w"
for mode in $no
do
	chmod "$mode" "$shallow"
	check -s1 -e"file $shallow: writable by users other than $user" \
		path_check_wexcl "$user" "$TMPDIR" "$shallow"

	chmod "$mode" "$basedir"
	chmod u+x "$basedir"
	check -s1 -e"file $deeper: writable by users other than $user" \
		path_check_wexcl "$user" "$TMPDIR" "$deeper"
done


#
# Exclusively writable
#

yes="u=rw,go= ugo=r"
for mode in $yes
do
	chmod go-w "$basedir" "$shallow" "$deeper"

	chmod "$mode" "$shallow"
	check path_check_wexcl "$user" "$TMPDIR" "$shallow"
	[ "$uid" -ne 0 ] &&
		check -s1 -e"file $deeper: writable by users other than $user" \
			path_check_wexcl "$user" / "$deeper"

	chmod "$mode" "$basedir"
	chmod u+wx,go= "$basedir"
	check path_check_wexcl "$user" "$TMPDIR" "$deeper"

	chmod g+w,o= "$deeper"
	check -s1 -e"file $deeper: writable by users other than $user" \
		path_check_wexcl "$user" "$TMPDIR" "$deeper"

	chmod g=,o+w "$deeper"
	check -s1 -e"file $deeper: writable by users other than $user" \
		path_check_wexcl "$user" "$TMPDIR" "$deeper"
done


#
# Success
#

warn "all tests passed."
