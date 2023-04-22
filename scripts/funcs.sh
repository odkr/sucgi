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
	__cleanup_retval=$?
	set +e
	trap '' EXIT HUP INT TERM
	# shellcheck disable=2046
	kill -- $(jobs -p 2>/dev/null) -$$ >/dev/null 2>&1
	wait
	eval "${cleanup-}"
	cleanup=
	return $__cleanup_retval
}

# Clear the current line of terminal $1.
# shellcheck disable=2120
clearln() (
	__clearln_fd="${1:-2}"
	[ -t "$__clearln_fd" ] && printf '\033[0K' >&"$__clearln_fd"
)

# Print a message to STDERR and exit with a non-zero status.
# Exit with status if -s STATUS is given.
err() {
	__err_status=2
	OPTIND=1 OPTARG='' __err_opt=''
	while getopts s: __err_opt
	do
		case $__err_opt in
		(s) __err_status="$OPTARG" ;;
		(*) exit 70
		esac
	done
	shift $((OPTIND - 1))

	warn "$@"

	exit "$__err_status"
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
	__inlist_op="${1:?}" __inlist_needle="${2?}"
	shift 2

	# shellcheck disable=2034
	for __inlist_straw
	do
		test "$__inlist_straw" "$__inlist_op" "$__inlist_needle" &&
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
	__logged_dir=.
	__logged_fname=''
	__logged_group=''
	__logged_user=''
	__logged_mask=0
	__logged_status=0

	if [ "$verbose" ]
	then
		"$@" || __logged_status=$?
		return $__logged_status
	fi

	OPTIND=1 OPTARG='' __logged_opt=''
	while getopts 'd:g:i:l:u:' __logged_opt
	do
		case $__logged_opt in
		(l) __logged_fname="${OPTARG:?}" ;;
		(d) __logged_dir="${OPTARG:?}" ;;
		(g) __logged_group="${OPTARG:?}" ;;
		(i) __logged_mask="$__logged_mask ${OPTARG:?}" ;;
		(u) __logged_user="${OPTARG:?}" ;;
		(*) return 1
		esac
	done
	shift $((OPTIND - 1))

	: "${TMPDIR:-/tmp}"
	: "${1:?}"
	: "${__logged_fname:="$(basename "$1").log"}"

	"$@" >>"$TMPDIR/$__logged_fname" 2>&1 || __logged_status=$?

	if inlist -eq "$__logged_status" $__logged_mask
	then
		rm -f "$TMPDIR/$__logged_fname"
	else
		if [ "$__logged_user" ]
		then
			: "${__logged_group:=$(id -gn "$__logged_user")}"
			chown "$__logged_user:$__logged_group" \
			      "$TMPDIR/$__logged_fname"
		fi

		mv "$TMPDIR/$__logged_fname" "$__logged_dir"
		warn '%s: exited with status %d.' \
		     "$*" "$__logged_status"
		warn 'see %s for details.' \
		     "$__logged_fname"
	fi

	return $__logged_status
)

# Print the login name of a file's owner.
owner() (
	ls -ld "${1:?}" |
	awk '{print $3}'
)

# Find a user with an UID from $1 to $2 who
# only belongs to groups with GIDs from $3 to $4.
reguser() (
	__reguser_minuid="${1:?}"
	__reguser_maxuid="${2:?}"
	__reguser_mingid="${3:?}"
	__reguser_maxgid="${4:?}"
	__reguser_pipe="${TMPDIR:-/tmp}/uids-$$.fifo"
	__reguser_retval=0
	__reguser_user=

	mkfifo -m 700 "$__reguser_pipe"
	uids >"$__reguser_pipe" & __reguser_uids=$!

	while read -r __reguser_uid __reguser_logname
	do
		case $__reguser_logname in (_*|*[!A-Za-z0-9_]*)
			continue
		esac

		[ "$__reguser_uid" -le "$__reguser_minuid" ] && continue
		[ "$__reguser_uid" -ge "$__reguser_maxuid" ] && continue

		__reguser_gids=$(id -G "$__reguser_logname")

		inlist -lt "$__reguser_mingid" $__reguser_gids && continue
		inlist -gt "$__reguser_maxgid" $__reguser_gids && continue

		__reguser_user="$__reguser_logname"
		break
	done <"$__reguser_pipe"

	wait $__reguser_uids	|| __reguser_retval="$?"
	rm -f "$__reguser_pipe"	|| __reguser_retval="$?"

	[ "$__reguser_retval" -eq 0 ] && [ "$__reguser_user" ] || return 1

	printf '%s\n' "$__reguser_user"
)

# Move to the beginning of the previous line of terminal $1.
rewindln() (
	__rewindln_fd="${1:-2}"
	[ -t "$__rewindln_fd" ] && printf '\r\033[1A' >&"$__rewindln_fd"
)

# Create a directory with the filename $prefix-$$ in $dir,
# register it for deletion via $cleanup, and set it as $TMPDIR.
tmpdir() {
	[ "${__tmpdir_tmpdir-}" ] && return
	# shellcheck disable=2031
	__tmpdir_prefix="${1:-tmp}" __tmpdir_dir="${2:-"${TMPDIR:-/tmp}"}"
	# shellcheck disable=SC2016
	__tmpdir_real="$(cd -P "$__tmpdir_dir" && pwd)" ||
		err 'cd -P $__tmpdir_dir && pwd: exited with status %d.' $?
	readonly __tmpdir_tmpdir="$__tmpdir_real/$__tmpdir_prefix-$$"
	catch=
	mkdir -m 0755 "$__tmpdir_tmpdir" || exit
	cleanup="rm -rf \"\$__tmpdir_tmpdir\"; ${cleanup-}"
	catch=x
	[ "${caught-}" ] && kill -s"$caught" "$$"
	# shellcheck disable=2031
	export TMPDIR="$__tmpdir_tmpdir"
}

# Print $* to stderr.
# -n  Suppress the terminating LF.
# -q  Suppress output if $quiet is set.
# -v  Suppress output unless $verbose is set.
warn() (
	__warn_lf=y
	OPTIND=1 OPTARG='' __warn_opt=''
	while getopts 'nqv' __warn_opt
	do
		case $__warn_opt in
		(n) __warn_lf= ;;
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
	if [ "$__warn_lf" ]
	then echo
	fi
)
