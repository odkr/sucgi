#!/bin/sh
#
# Test environment sanitisation.
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
# Build configuration
#

eval "$(main -C | grep -E '^MAX_(NVARS|VARNAME_LEN|VAR_LEN)=')"


#
# Tests
#

# Too many variables.
i=0 vars=
while [ "$i" -le "$MAX_NVARS" ]
do
	i=$((i + 1))
	vars="$vars var$i="
done

check -s1 -e'too many environment variables.'	$vars main

# Variable name has maximum length.
varname="$(pad v $((MAX_VARNAME_LEN - 1)))"
check -s1 -e"discarding \$$varname"		"$varname=" main

# Variable name is too long.
varname="$(pad v $MAX_VARNAME_LEN)"
var="$varname=foo"
check -s1 -e"variable \$$var: name too long."	"$var" main

# Variable has maximum length.
var="$(pad var= $((MAX_VAR_LEN - 1)) x r)"
check -s1 -e"discarding \$${var%=*}"		"$var" main

# Variable is too long.
var="$(pad var= $MAX_VAR_LEN x r)"
check -s1 -e"variable \$$var: too long."	"$var" main

# Variable name is illegal.
for var in 'foo =' '0foo=' '$(echo foo)=' '`echo foo`='
do
	check -s1 -e"variable \$$var: bad name." \
		badenv "$var" "$tests_dir/main"
done

# Variable is not of the form <key>=<value>.
for var in 'foo' '0foo' 'foo '
do
	check -s1 -e"variable \$$var: malformed." \
		badenv -n1 "$var" "$tests_dir/main"
done

# Not a CGI variable.
check -s1 -e'discarding $foo'			foo=foo main


#
# Success
#

warn 'all tests passed.'
