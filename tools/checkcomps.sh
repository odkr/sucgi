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

# Compilers to test.
readonly compilers='
	gcc-12 gcc-11 gcc-10 gcc-9 gcc-8 gcc musl-gcc
	clang-16 clang-15 clang-14 clang-13 clang-12
	clang-11 clang-10 clang-9 clang
	tcc icc c99 cc
'

# Build configurations to test.
readonly configs='prod.env devel.env speed.env'


#
# Initialiation
#

set -Ceu
tools_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(dirname -- "$tools_dir")"
# shellcheck disable=2034
readonly tools_dir src_dir
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit


#
# Options
#

OPTIND=1 OPTARG='' opt=''
# shellcheck disable=2034
while getopts h opt; do
	# shellcheck disable=2154
	case $opt in
	(h) exec cat <<EOF
$prog_name - run checks using different C compilers

Usage:  $prog_name
        $prog_name -h

Options:
    -h  Show this help screen.

Must be called from a directory with a makefile.
The makefile must provide the standard Autoconf targets.
EOF
	    ;;
	(*) exit 1
	esac
done
shift $((OPTIND - 1))
unset opt


#
# Main
#

cd -P "$src_dir" || exit

# shellcheck disable=2034
cleanup="[ -e makefile ] && make distclean"
for cc in $compilers
do
	command -v "$cc" >/dev/null || continue

	[ -e makefile ] && make distclean
	for template in *.m4
	do
		case $template in
		('*.m4')   break ;;
		('lib.m4') continue ;;
		esac

		m4 -D__CC="$cc" "$template" >"${template%.m4}"
	done

	make clean all check ||
	err '%s w/o configuration failed.' "$cc"

	for config in $configs
	do
		[ -e "$config" ] || continue

		[ -e makefile ] && make distclean
		CC="$cc" ./configure -fqc"$config"

		make clean all check ||
		err '%s with configuration %s failed.' "$cc" "$config"
	done
done

warn 'all compilers passed.'
