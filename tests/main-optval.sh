#!/bin/sh
#
# Test option handling.
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


#
# Tests
#

# Help.
check -o'Print this help screen.'	main -h

# Version.
check -o'suCGI v'			main -V

# Build configuration.
sed -n 's/#define \([[:alnum:]_]*\).*/\1/p' "$src_dir/config.h"	|
sort -u								|
while read -r var
do
	[ "$var" = CONFIG_H ] && continue
	check -o"$var" main -C
done

# Usage message.
for args in -X -XX -x --x - -- '-h -C' '-hC' '-h -V' '-hC'
do
	check -s1 -e'usage: sucgi' main $args
done

check -s1 -e'usage: sucgi' main ''


#
# Success
#

warn 'all tests passed.'
