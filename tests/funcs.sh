#!/bin/sh
# Utility functions for test scripts.
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
# Initialisation
#

: "${src_dir:?}"
. "$src_dir/scripts/funcs.sh"


#
# Functions
#

# Check if calling a programme produces a result.
# -s checks for an exit status (defaults to 0),
# -o STR check for STR on stdout, -e on stderr;
# -o and -e are mutually exclusive.
# FIXME: Doesn't work with posh.
check() (
	exstatus=0 stream=out
	OPTIND=1 OPTARG='' opt=''

	while getopts 's:o:e:v' opt
	do
		# shellcheck disable=2034
		case $opt in
		(s) exstatus="$OPTARG" ;;
		(o) stream=out pattern="$OPTARG" ;;
		(e) stream=err pattern="$OPTARG" ;;
		(*) return 2
		esac
	done
	shift $((OPTIND - 1))

	: "${1:?no command given}"
	: "${stdout:=/dev/null}"
	: "${stderr:=/dev/null}"

	err=0 retval=0

	case $stream in
	(out) output="$(env "$@" 2>/dev/null)" || err=$? ;;
	(err) output="$(env "$@" 2>&1 >/dev/null)" || err=$? ;;
	esac

	if [ "$err" -ne "$exstatus" ] && [ "$err" -ne 141 ]
	then
		warn '%s: exited with status %d.' "$*" "$err"
		retval=70
	fi

	if [ "$stream" ]
	then
		IFS='
'

		hit=
		for line in $output
		do
			case $line in (*$pattern*)
				hit=x
				break
			esac
		done

		if ! [ "$hit" ]
		then
			retval=70
			warn '%s printed to std%s:' "$*" "$stream"
			for line in $output
			do warn '> %s' "$line"
			done
		fi

		unset IFS
	fi

	return "$retval"
)
