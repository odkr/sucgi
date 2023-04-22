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
	__check_status=0 __check_stream=out
	OPTIND=1 OPTARG='' __check_opt=''

	while getopts 's:o:e:v' __check_opt
	do
		# shellcheck disable=2034
		case $__check_opt in
		(s) __check_status="${OPTARG:?}" ;;
		(o) __check_stream=out __check_pattern="${OPTARG:?}" ;;
		(e) __check_stream=err __check_pattern="${OPTARG:?}" ;;
		(*) return 2
		esac
	done
	shift $((OPTIND - 1))

	: "${1:?no command given}"

	__check_err=0 __check_retval=0

	case $__check_stream in
	(out) __check_output="$(env "$@" 2>/dev/null)" || __check_err=$? ;;
	(err) __check_output="$(env "$@" 2>&1 >/dev/null)" || __check_err=$? ;;
	esac

	if ! inlist -eq "$__check_err" "$__check_status" 141
	then
		warn '%s: exited with status %d.' "$*" "$__check_err"
		__check_retval=70
	fi

	if [ "$__check_stream" ]
	then
		IFS='
'
		__check_match=
		for __check_line in $__check_output
		do
			case $__check_line in (*$__check_pattern*)
				__check_match=x
				break
			esac
		done

		if ! [ "$__check_match" ]
		then
			__check_retval=70
			warn '%s printed to std%s:' "$*" "$__check_stream"
			for __check_line in $__check_output
			do warn '> %s' "$__check_line"
			done
		fi

		unset IFS
	fi

	return "$__check_retval"
)
