#!/bin/sh
#
# Test error.
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
# shellcheck disable=1091

#
# Initialisation
#

set -Cefu
tests_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(cd -P "$tests_dir/.." && pwd)"
# shellcheck disable=2034
readonly tests_dir src_dir
# shellcheck disable=1091
. "$tests_dir/funcs.sh" || exit
init || exit
tmpdir tmp .


#
# Globals
#

# Test result
result=0


#
# Build configuration
#

eval "$(main -C | grep -E ^NDEBUG=)"


#
# Assertions
#

if [ "${NDEBUG-}" ]
then
	trap 'catch ABRT' ABRT
	
	catch=
	check -s134 -e"*message" error ''
	catch=x

	case $caught in
	(''|ABRT) : ;;
	(*) kill -s"$caught" "$$"
	esac
fi

#
# Messages
#

for format in - foo bar baz %s foo%s bar%s baz%s
do
	for arg in - foo bar baz
	do
		case $format in
		(*%s*)	# shellcheck disable=2059
			message="$(printf -- "$format" "$arg")" ;;
		(*)	# shellcheck disable=2059
			message="$(printf -- "$format")" ;;
		esac
		check -s1 -e"$message" error "$format" "$arg" || result=70
	done
done


#
# Result
#

exit "$result"
