#!/bin/sh

#
# Utility functions for test scripts.
#
# Copyright 2022 Odin Kroeger.
#
# This file is part of suCGI.
#
# suCGI is free software: you can redistribute it and/or modify it
# under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# suCGI is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
# Public License for more details.
#
# You should have received a copy of the GNU Affero General Public
# License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
#

#
# Initialisation
#

. "${src_dir:?}/scripts/libutil.sh"


#
# Functions
#

# Check if a programme behaves as expected.
# -s STATUS   Check for exit status STATUS (default: 0).
# -o STR      Check for STR on stdout.
# -e STR      Check for STR on stderr.
# -o and -e are mutually exclusive.
# FIXME: Doesn't work with posh.
check() (
	_check_status=0 _check_stream=out
	OPTIND=1 OPTARG='' _check_opt=''

	while getopts 's:o:e:v' _check_opt
	do
		# shellcheck disable=2034
		case $_check_opt in
		(s) _check_status="${OPTARG:?}" ;;
		(o) _check_stream=out _check_pattern="${OPTARG:?}" ;;
		(e) _check_stream=err _check_pattern="${OPTARG:?}" ;;
		(*) return 2
		esac
	done
	shift $((OPTIND - 1))

	: "${1:?no command given}"

	_check_err=0 _check_retval=0

	case $_check_stream in
	(out) _check_output="$(env "$@" 2>/dev/null)"     || _check_err=$? ;;
	(err) _check_output="$(env "$@" 2>&1 >/dev/null)" || _check_err=$? ;;
	esac

	if	! inlist -eq "$_check_err" "$_check_status" 141
	then
		warn '%s: exited with status %d.' "$*" "$_check_err"
		_check_retval=70
	fi

	if	[ "$_check_stream" ] &&
		! printf '%s\n' "$_check_output" | grep -Fq "$_check_pattern"
	then
		_check_retval=70
		warn '%s printed to std%s:' "$*" "$_check_stream"
		printf '%s\n' "$_check_output" >&2
	fi

	return "$_check_retval"
)
