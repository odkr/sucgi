#!/bin/sh
# Utility functions for scripts.
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

# shellcheck disable=2015,2031

# Re-raise signal $1 if $catch is set.
# Otherwise set $caught to $1.
catch() {
	: "${1:?}"
	# shellcheck disable=2119
	clearln
	warn 'caught %s.' "$1"
	# shellcheck disable=2034
	caught="$1"
	[ "${catch-}" ] || return 0
	cleanup
	trap - "$1"
	kill -s "$1" "$$"
}

# Terminate all children, eval $cleanup and exit with status $?.
cleanup() {
	# shellcheck disable=2319
	_cleanup_retval=$?
	set +e
	trap '' EXIT HUP INT TERM
	# shellcheck disable=2046
	kill -- $(jobs -p 2>/dev/null) -$$ >/dev/null 2>&1
	wait
	eval "${cleanup-}"
	cleanup=
	return $_cleanup_retval
}

# Clear the current line of terminal $1.
# shellcheck disable=2120
clearln() (
	_clearln_fd="${1:-2}"
	[ -t "$_clearln_fd" ] && printf '\033[0K' >&"$_clearln_fd"
)

# Print a message to STDERR and exit with a non-zero status.
# Exit with status if -s STATUS is given.
err() {
	_err_status=2
	OPTIND=1 OPTARG='' _err_opt=''
	while getopts s: _err_opt
	do
		case $_err_opt in
		(s) _err_status="$OPTARG" ;;
		(*) exit 70
		esac
	done
	shift $((OPTIND - 1))

	warn "$@"

	exit "$_err_status"
}

# Enforce POSIX-compliance, register signals, and set global variables.
init() {
	# Root directory of the repository.
	: "${src_dir:?}"

	# shellcheck disable=2039,3040
	[ "${BASH_VERSION-}" ] && set -o posix
	[ "${ZSH_VERSION-}" ] && emulate sh 2>/dev/null
	export BIN_SH=xpg4 NULLCMD=: POSIXLY_CORRECT=x CLICOLOR_FORCE=

	# Environment variables.
	# shellcheck disable=2154
	PATH="$src_dir/tests:$src_dir/tools:$PATH"
	unset IFS

	# Signal handling.
	catch=x caught=
	trap 'catch HUP' HUP
	trap 'catch INT' INT
	trap 'catch TERM' TERM
	trap cleanup EXIT
	[ "$caught" ] && kill -s"$caught" "$$"

	# Permission mask.
	umask 022

	# Output control.
	quiet='' verbose=''

	# Programme name.
	prog_name="$(basename -- "$0")" || prog_name="$0"
	readonly prog_name
}

# Check if $needle matches any member of $@ using $op.
inlist() (
	# shellcheck disable=2034
	_inlist_op="${1:?}" _inlist_needle="${2?}"
	shift 2

	# shellcheck disable=2034
	for _inlist_straw
	do
		test "$_inlist_straw" "$_inlist_op" "$_inlist_needle" &&
		return
	done

	return 1
)

# Run $@ and redirect its output to a log file unless $verbose is set.
# -d DIR     Store log file in DIR if command exit with a non-zero status.
# -i STATUS  Do not store a log if the command exits with STATUS.
# -l FNAME   Store the log in FNAME.
# -u USER    Make USER the owner of FNAME.
# -g GROUP   Make GROUP the group of FNAME.
logged() (
	_logged_dir=.
	_logged_fname=''
	_logged_group=''
	_logged_user=''
	_logged_mask=0
	_logged_status=0

	if [ "$verbose" ]
	then
		"$@" || _logged_status=$?
		return $_logged_status
	fi

	OPTIND=1 OPTARG='' _logged_opt=''
	while getopts 'd:g:i:l:u:' _logged_opt
	do
		case $_logged_opt in
		(l) _logged_fname="${OPTARG:?}" ;;
		(d) _logged_dir="${OPTARG:?}" ;;
		(g) _logged_group="${OPTARG:?}" ;;
		(i) _logged_mask="$_logged_mask ${OPTARG:?}" ;;
		(u) _logged_user="${OPTARG:?}" ;;
		(*) return 1
		esac
	done
	shift $((OPTIND - 1))

	: "${TMPDIR:-/tmp}"
	: "${1:?}"
	: "${_logged_fname:="$(basename "$1").log"}"

	"$@" >>"$TMPDIR/$_logged_fname" 2>&1 || _logged_status=$?

	if inlist -eq "$_logged_status" $_logged_mask
	then
		rm -f "$TMPDIR/$_logged_fname"
	else
		if [ "$_logged_user" ]
		then
			: "${_logged_group:=$(id -gn "$_logged_user")}"
			chown "$_logged_user:$_logged_group" \
			      "$TMPDIR/$_logged_fname"
		fi

		mv "$TMPDIR/$_logged_fname" "$_logged_dir"
		warn '%s: exited with status %d.' \
		     "$*" "$_logged_status"
		warn 'see %s for details.' \
		     "$_logged_fname"
	fi

	return $_logged_status
)

# Print the login name of a file's owner.
owner() (
	ls -ld "${1:?}" | awk '{print $3}'
)

# Find a user with an UID from $1 to $2 who
# only belongs to groups with GIDs from $3 to $4.
reguser() (
	: "${1:?}" "${2:?}" "${3:?}" "${4:?}"

	uids | {
		while read -r _reguser_uid _reguser_logname
		do
			[ "$_reguser_uid" -le "$1" ] && continue
			[ "$_reguser_uid" -ge "$2" ] && continue

			case $_reguser_logname in (_*|*[!A-Za-z0-9_]*)
				continue
			esac

			_reguser_gids=$(id -G "$_reguser_logname")

			inlist -lt "$3" $_reguser_gids && continue
			inlist -gt "$4" $_reguser_gids && continue

			printf '%s\n' "$_reguser_logname"
			exit 0
		done
		exit 1
	}
)

# Move to the beginning of the previous line of terminal $1.
rewindln() (
	_rewindln_fd="${1:-2}"
	[ -t "$_rewindln_fd" ] && printf '\r\033[1A' >&"$_rewindln_fd"
)

# Create a directory with the filename $1-$$ in $dir,
# register it for deletion via $cleanup, and set it as $TMPDIR.
tmpdir() {
	# shellcheck disable=2031
	_tmpdir_prefix="${1:-tmp}" _tmpdir_dir="${2:-"${TMPDIR:-/tmp}"}"
	[ "${_tmpdir_tmpdir-}" ] && return

	# shellcheck disable=SC2016
	_tmpdir_real="$(cd -P "$_tmpdir_dir" && pwd)" ||
	err 'cd -P $_tmpdir_dir && pwd: exited with status %d.' $?
	readonly _tmpdir_tmpdir="$_tmpdir_real/$_tmpdir_prefix-$$"

	catch=
	mkdir -m 0755 "$_tmpdir_tmpdir" || exit
	cleanup="rm -rf \"\$_tmpdir_tmpdir\"; ${cleanup-}"
	catch=x
	[ "${caught-}" ] && kill -s"$caught" "$$"

	# shellcheck disable=2031
	export TMPDIR="$_tmpdir_tmpdir"
}

# Print $* to stderr.
# -n  Suppress the terminating LF.
# -q  Suppress output if $quiet is set.
# -v  Suppress output unless $verbose is set.
warn() (
	_warn_lf=y
	OPTIND=1 OPTARG='' _warn_opt=''
	while getopts 'nqv' _warn_opt
	do
		case $_warn_opt in
		(n) _warn_lf= ;;
		(q) [ "${quiet-}" ] && return 0 ;;
		(v) [ "${verbose-}" ] || return 0 ;;
		(*) return 1
		esac
	done
	shift $((OPTIND - 1))

	exec >&2
	printf '%s: ' "${prog_name:-$0}"
	# shellcheck disable=SC2059
	printf -- "$@"
	if [ "$_warn_lf" ]
	then echo
	fi
)
