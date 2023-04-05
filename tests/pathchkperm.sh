#!/bin/sh
#
# Test pathchkperm.
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
readonly script_dir src_dir
# shellcheck disable=1091
. "$src_dir/scripts/funcs.sh" || exit
init || exit
tmpdir chk
result=0


#
# Build configuration
#

eval "$(main -C | grep -vE ^PATH=)"


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

	check -s134 -e"*basedir == '/'"	\
		pathchkperm "$user" '' /			|| result=70
	check -s134 -e"*basedir == '/'"	\
		pathchkperm "$user" relpath /		|| result=70
	check -s134 -e"*basedir == '/'"	\
		pathchkperm "$user" . /			|| result=70

	check -s134 -e"*fname == '/'" \
		pathchkperm "$user" / ''			|| result=70
	check -s134 -e"*fname == '/'" \
		pathchkperm "$user" / relpath		|| result=70
	check -s134 -e"*fname == '/'" \
		pathchkperm "$user" / .			|| result=70

	check -s134 \
	      -e'strnlen(basedir, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN' \
	      pathchkperm "$user" "$err_fname" /		|| result=70
	check -s134 \
	      -e'strnlen(fname, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN' \
	      pathchkperm "$user" / "$err_fname"		|| result=70

	check -s1 pathchkperm "$user" "$long_fname" /	|| result=70
	check -s1 pathchkperm "$user" / "$long_fname"	|| result=70
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
	      pathchkperm "$user" "$1" "$2" || result=70
done


#
# Non-existant files
#

check -s1 -e "stat $TMPDIR/<nosuchfile>: No such file or directory" \
      pathchkperm "$user" "$TMPDIR" "$TMPDIR/<nosuchfile>" || result=70


#
# Not exclusively writable
#

no="u=r,g=w,o= u=r,g=,o=w u=rw,g=w,o= u=rw,g=,o=w u=rw,go=w u=rw,go=w"
for mode in $no
do
	chmod "$mode" "$shallow"
	check -s1 -e"file $shallow: writable by non-owner" \
	      pathchkperm "$user" "$TMPDIR" "$shallow"	|| result=70

	chmod "$mode" "$basedir"
	chmod u+x "$basedir"
	check -s1 -e"file $deeper: writable by non-owner" \
	      pathchkperm "$user" "$TMPDIR" "$deeper"	|| result=70
done


#
# Exclusively writable
#

yes="u=rw,go= ugo=r"
for mode in $yes
do
	chmod go-w "$basedir" "$shallow" "$deeper"

	chmod "$mode" "$shallow"
	check pathchkperm "$user" "$TMPDIR" "$shallow"	|| result=70
	if [ "$uid" -ne 0 ]
	then
		check -s1 -e"file $deeper: writable by non-owner" \
		      pathchkperm "$user" / "$deeper"	|| result=70
	fi

	chmod "$mode" "$basedir"
	chmod u+wx,go= "$basedir"
	check pathchkperm "$user" "$TMPDIR" "$deeper"	|| result=70

	chmod g+w,o= "$deeper"
	check -s1 -e"file $deeper: writable by non-owner" \
	      pathchkperm "$user" "$TMPDIR" "$deeper"	|| result=70

	chmod g=,o+w "$deeper"
	check -s1 -e"file $deeper: writable by non-owner" \
	      pathchkperm "$user" "$TMPDIR" "$deeper"	|| result=70
done


#
# Result
#

exit "$result"
