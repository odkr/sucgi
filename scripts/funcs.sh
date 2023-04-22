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

# Re-raise $signal if $catch is set.
# Otherwise set $caught to $signal.
catch() {
	signal="${1:?}"
	# shellcheck disable=2119
	clearln
	warn 'caught %s.' "$signal"
	# shellcheck disable=2034
	caught="$signal"
	[ "${catch-}" ] || return 0
	cleanup
	trap - "$signal"
	kill -s "$signal" "$$"
}

# Terminate all children, eval $cleanup and exit with status $?.
cleanup() {
	# shellcheck disable=2319
	retval=$?
	set +e
	trap '' EXIT HUP INT TERM
	# shellcheck disable=2046
	kill -- $(jobs -p 2>/dev/null) -$$ >/dev/null 2>&1
	wait
	eval "${cleanup-}"
	cleanup=
	return $retval
}

# Clear the current line of terminal $fd.
# shellcheck disable=2120
clearln() (
	fd="${1:-2}"
	[ -t "$fd" ] && printf '\033[0K' >&"$fd"
)

# Print a message to STDERR and exit with a non-zero status.
# -s sets a different status.
err() {
	exstatus=2
	OPTIND=1 OPTARG='' opt=''
	while getopts s: opt
	do
		case $opt in
		(s) exstatus="$OPTARG" ;;
		(*) exit 70
		esac
	done
	shift $((OPTIND - 1))

	warn "$@"

	exit "$exstatus"
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
	op="${1:?}" needle="${2?}"
	shift 2

	# shellcheck disable=2034
	for straw
	do eval "[ \"\$needle\" $op \"\$straw\" ]" && return
	done

	return 1
)

# Find a user whose UID is between $minuid and $maxuid and who
# only belongs to groups with GIDs between $mingid and $maxgid.
reguser() (
	minuid="${1:?}" maxuid="${2:?}" mingid="${3:?}" maxgid="${4:?}"
	pipe="${TMPDIR:-/tmp}/uids-$$.fifo" retval=0 reguser=
	mkfifo -m 700 "$pipe"

	uids >"$pipe" & uids=$!
	while read -r uid user
	do
		case $user in (_*|*[!A-Za-z0-9_]*)
			continue
		esac

		[ "$uid" -lt "$minuid" ] && continue
		[ "$uid" -gt "$maxuid" ] && continue

		for gid in $(id -G "$user")
		do
			[ "$gid" -lt "$mingid" ] && continue 2
			[ "$gid" -gt "$maxgid" ] && continue 2
		done

		reguser="$user"
		break
	done <"$pipe"

	wait $uids || retval=$?
	rm -f "$pipe"
	[ "$retval" -eq 0 ] && [ "$reguser" ] || return 1

	printf '%s\n' "$reguser"
)

# Move to the beginning of the previous line of terminal $fd.
rewindln() (
	fd="${1:-2}"
	[ -t "$fd" ] && printf '\r\033[1A' >&"$fd"
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
# -n suppresses the terminating LF.
# -q suppresses output if $quiet is set.
# -v suppresses output unless $verbose is set.
warn() (
	lf=y
	OPTIND=1 OPTARG='' opt=''
	while getopts 'nqv' opt
	do
		case $opt in
		(n) lf= ;;
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
	if [ "$lf" ]
	then echo
	fi
)
