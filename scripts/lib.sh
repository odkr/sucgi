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


#
# Functions
#

# Print a message to STDERR and exit with a non-zero status.
# -s sets a different status.
err() {
	rc=2
	OPTIND=1 OPTARG='' opt=''
	while getopts s: opt
	do
		case $opt in
		(s) rc="$OPTARG" ;;
		(*) exit 70
		esac
	done
	shift $((OPTIND - 1))

	warn "$@"

	exit "$rc"
}

# Exit with status $signo + 128 if $catch is set.
# Otherwise set $caught to $signo.
catch() {
	signo="${1:?}"
	sig="$(kill -l "$signo")"
	printf '%*s\r' 80 >&2
	warn 'caught %s.' "$sig"
	[ "${catch-}" ] && exit "$((signo + 128))"
	# shellcheck disable=2034
	caught="$signo"
}

# Terminate all children, eval $cleanup and exit with status $?.
cleanup() {
	rc=$?
	set +e
	trap '' EXIT HUP INT TERM
	# shellcheck disable=2046
	kill -- $(jobs -p 2>/dev/null) -$$ >/dev/null 2>&1
	wait
	eval "${cleanup-}"
	exit "$rc"
}

# Register signals, set variables, and enable colours.
init() {
	# Root directory of the repository.
	: "${src_dir:?}"

	# shellcheck disable=2039
	[ "${BASH_VERSION-}" ] && set -o posix
	[ "${ZSH_VERSION-}" ] && emulate sh 2>/dev/null
	export BIN_SH=xpg4 NULLCMD=: POSIXLY_CORRECT=x CLICOLOR_FORCE=

	# Environment variables.
	# shellcheck disable=2154
	PATH="$src_dir/tests:$src_dir/tools:$PATH"
	unset IFS

	# Signal handling.
	catch=x caught=
	trap 'catch 1' HUP
	trap 'catch 2' INT
	trap 'catch 15' TERM
	trap cleanup EXIT
	[ "$caught" ] && exit "$((caught + 128))"

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

# FIXME: Use a umask of 0 instead, and then remove this.
# Print the owner of $fname.
owner() (
	fname="${1:?}"
	pipe="${TMPDIR:-/tmp}/owner-$$.fifo" rc=0
	mkfifo -m 0700 "$pipe"
	ls -ld "$fname" >"$pipe" & ls=$!
	awk '{print $3}' <"$pipe" || rc=$?
	wait $ls                  || rc=$?
	rm -f "$pipe"
	return $rc
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
	mkdir -m 0700 "$__tmpdir_tmpdir" || exit
	cleanup="rm -rf \"\$__tmpdir_tmpdir\"; ${cleanup-}"
	catch=x
	[ "${caught-}" ] && exit $((caught + 128))
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
	[ "$lf" ] && echo

	return 0
)
