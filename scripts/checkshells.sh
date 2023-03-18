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
shells='sh bash dash ksh mksh oksh posh yash zsh'

# Be quiet?
quiet=

# Be verbose?
verbose=


#
# Initialiation
#

set -Cefu
tools_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(dirname -- "$tools_dir")"
# shellcheck disable=2034
readonly tools_dir src_dir
# shellcheck disable=1091
. "$tools_dir/funcs.sh" || exit
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
while getopts Dc:hqv opt; do
	# shellcheck disable=2154
	case $opt in
	(h) exec cat <<EOF
$prog_name - run scripts with different shells

Usage:     $prog_name [SH ...]
           $prog_name -h

Operands:
    SH     A shell.

Options:
    -c CC  Use CC as C compiler.
    -q     Be more quiet.
    -v     Be verbose, but do not log runs.
    -h     Show this help screen.

Must be called from a directory with a makefile.
The makefile must provide the standard Autoconf targets.
EOF
	    ;;
	(D) set -x ;;
	(c) CC="$OPTARG" ;;
	(q) quiet=y ;;
	(v) verbose=y ;;
	(*) exit 1
	esac
done
shift $((OPTIND - 1))
unset opt

# shellcheck disable=2086
[ $# -eq 0 ] && set -- $shells

# Use a fast compiler, if one is available.
if ! [ "${CC-}" ]
then
	for cc in tcc clang
	do
		if command -v "$cc" >/dev/null
		then
			CC="$cc"
			break
		fi
	done
fi
[ "${CC-}" ] && export CC


#
# Main
#

cd -P "$src_dir" || exit
lock check.lock
# shellcheck disable=2034
cleanup="mrproper; ${cleanup-}"

failures=
for shell
do
	command -v "$shell" >/dev/null 2>&1 || continue

	warn -n 'checking %s ... ' "$shell"
	[ "$verbose" ] && echo >&2

	logfile="$TMPDIR/checkshell-$shell.log"
	if (
		[ "$verbose" ] || exec >"$logfile" 2>&1

		mrproper

		"$shell" ./configure -c'posix.env'

		for file in $files
		do
			[ -e "$file" ] && continue

			err '%s ./configure did not generate %s' \
			    "$shell" "$file"
		done

		find ./tests -name '*.sh' ! -name funcs.sh \
		     -exec make check SHELL="$shell" checks='{}' ';'
	)
	then
		[ "$verbose" ] && printf '%s\n' pass >&2
	else
		if ! [ "$verbose" ]
		then printf '%s\n' fail >&2
		elif [ -e "$logfile" ]
		then mv "$logfile" .
		fi
		failures="$failures $shell"
	fi
done

if [ "$failures" ]
then err -s70 'failures: %s' "${failures# }"
else warn -q 'all shells passed.'
fi
