#!/bin/sh
# Run checks using different compilers.
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

# shellcheck disable=2015


#
# Constants
#

# Files that ./configure should create.
readonly files='config.status makefile build.h compat.h'


#
# Defaults
#

# Shells to test.
readonly shells='sh bash dash ksh mksh oksh posh sash yash zsh'

# Be quiet?
quiet=

# Store a log in the current working directory?
storelogs=y


#
# Initialiation
#

set -Cefu
tools_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(dirname -- "$tools_dir")"
# shellcheck disable=2034
readonly tools_dir src_dir
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit
tmpdir


#
# Functions
#

# make distclean if the makefile exists.
mrproper() {
	if [ -e makefile ]
	then make distclean >/dev/null 2>&1
	fi
}


#
# Options
#


OPTIND=1 OPTARG='' opt=''
# shellcheck disable=2034
while getopts hlq opt; do
	# shellcheck disable=2154
	case $opt in
	(h) exec cat <<EOF
$prog_name - run scripts with different shells

Usage:  $prog_name [SH ...]
        $prog_name -h

Operands:
    SH  A shell.

Options:
    -l  Do not log failed runs to the current working directory.
    -q  Be more quiet.
    -h  Show this help screen.

Must be called from a directory with a makefile.
The makefile must provide the standard Autoconf targets.
EOF
	    ;;
	(q) quiet=y ;;
	(l) storelogs= ;;
	(*) exit 1
	esac
done
shift $((OPTIND - 1))
unset opt

[ $# -eq 0 ] && set -- $shells


#
# Main
#

cd -P "$src_dir" || exit

# shellcheck disable=2034
cleanup="mrproper; ${cleanup-}"

failures=
for shell
do
	command -v "$shell" >/dev/null 2>&1 || continue

	warn -n 'checking %s ... ' "$shell"

	logfile="$TMPDIR/checkshell-$shell.log"
	if (
		if [ "$storelogs" ]
		then exec >"$logfile" 2>&1
		else exec >/dev/null 2>&1
		fi

		mrproper

		"$shell" ./configure

		for file in $files
		do
			[ -e "$file" ] && continue

			err '%s ./configure did not generate %s' \
			    "$shell" "$file"
		done

		find ./tests -name '*.sh' ! -name lib.sh \
		     -exec make check SHELL="$shell" checks='{}' ';'
	)
	then
		printf '%s\n' pass >&2
	else
		printf '%s\n' fail >&2
		[ "$storelogs"] && [ -e "$logfile" ] && mv "$logfile" .
		failures="$failures $shell"
	fi
done

if [ "$failures" ]
then err -s70 'failures: %s' "${failures# }"
else warn -q 'all tests passed.'
fi
