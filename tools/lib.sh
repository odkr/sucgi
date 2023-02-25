#!/bin/sh
# Utility functions for scripts and the test suite.
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
	warn 'caught %s.' "$sig"
	[ "${catch-}" ] && exit "$((signo + 128))"
	# shellcheck disable=2034
	caught="$signo"
}

# Check if calling a programme produces a result.
# -s checks for an exit status (defaults to 0),
# -o STR for STR on stdout, -e STR for STR on stderr.
check() (
	status=0 stream=''
	OPTIND=1 OPTARG='' opt=''
	while getopts 's:o:e:v' opt
	do
		# shellcheck disable=2034
		case $opt in
		(s) status="$OPTARG" ;;
		(o) stream=out pattern="$OPTARG" ;;
		(e) stream=err pattern="$OPTARG" ;;
		(*) return 2
		esac
	done
	shift $((OPTIND - 1))

	if [ "$stream" ]
	then
		# shellcheck disable=2030
		: "${TMPDIR:=/tmp}"

		tmpfile="$TMPDIR/std$stream-$$.log"
		eval "std$stream=\"\$tmpfile\""
	fi

	: "${@?no command given}"
	: "${stdout:=/dev/null}"
	: "${stderr:=/dev/null}"

	warn -q 'checking %s ...' "$*"

	err=0 rc=0

	# shellcheck disable=2154
	env "$@" >"$stdout" 2>"$stderr" || err=$?

	if [ "$stream" ]
	then
		if ! grep -Fq "$pattern" <"$tmpfile"
		then
			warn 'expected output on std%s:' "$stream"
			warn '> %s' "$pattern"
			warn 'actual output:'
			while read -r line
			do
				warn '> %s' "$line"
			done <"$tmpfile"
			rc=70
		fi

		rm -f "$tmpfile"
	fi


	if [ "$err" -ne "$status" ] && [ "$err" -ne 141 ]
	then
		warn '%s exited with status %d.' "$1" "$err"
		rc=70
	fi

	return "$rc"
)

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

# Clear the current line of terminal $fd.
clearln() (
	fd="${1:-2}"
	[ -t "$fd" ] && printf '\033[0K' >&"$fd"
)

# Starting with, but excluding, $dirname run $dircmd for every path segment
# of $fname. If $fcmd is given, run $dircmd for every path segment up to,
# but excluding $fname, and run $fcmd for $fname.
dirwalk() (
	dirname="${1:?}" fname="${2:?}" dircmd="${3:?}" fcmd="${4:-"$3"}"

	IFS=/
	# shellcheck disable=2086
	set -- ${fname#"${dirname%/}/"}
	unset IFS
	cd "$dirname" || exit

	while [ "$#" -gt 0 ]
	do
		fname="$1"
		shift
		[ "$fname" ] || continue

		case $# in
		(0)
			eval "$fcmd"
			return
			;;
		(*)
			eval "$dircmd"
	 		dirname="${dirname%/}/$fname"
	 		cd "$fname" || return
			;;
		esac
	done
)

# Register signals, set variables, and enable colours.
init() {
	# Environment variables.
	: "${src_dir:?}"
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
	do
		eval "[ \"\$needle\" $op \"\$straw\" ]" && return
	done

	return 1
)

# Create a path that is $len characters long in $basepath.
mklongpath() (
	basepath="${1:?}" len="${2:?}" max=99999
	namemax="$(getconf NAME_MAX "$basepath" 2>/dev/null)" || namemax=14
	dirs=$((len / namemax + 1))
	dirlen=$((len / dirs))

	dir="$basepath" target=$((len - dirlen - 1))
	while [ ${#dir} -le $target ]
	do
		i=0
		while [ $i -lt "$max" ]
		do
			seg="$(pad "$i" "$dirlen")"
			if ! [ -e "$dir/$seg" ]
			then
				dir="$dir/$seg"
				continue 2
			fi
			i=$((i + 1))
		done
	done

	i=0 fname=
	while [ $i -lt "$max" ]
	do
		seg="$(pad "$i" "$((len - ${#dir} - 1))")"
		if ! [ -e "$dir/$seg" ]
		then
			fname="$dir/$seg"
			break
		fi
		i=$((i + 1))
	done

	# This should be guaranteed, but who knows?
	[ "${#fname}" -eq "$len" ] ||
		err 'failed to generate long filename.'

	printf '%s\n' "$fname"
)

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

# Pad $str with $ch up to length $n.
pad() (
	str="${1?}" n="${2:?}" ch="${3:-x}"
	pipe="${TMPDIR:-/tmp}/pad-$$.fifo" rc=0
	mkfifo -m 0700 "$pipe"
	printf '%*s\n' "$n" "$str" >"$pipe" & printf=$!
	tr ' ' "$ch" <"$pipe" || rc=$?
	wait $printf          || rc=$?
	rm -f "$pipe"
	return $rc
)

# Find a user whose UID is between $minuid and $maxuid and who
# only belongs to groups with GIDs between $mingid and $maxgid.
reguser() (
	minuid="${1:?}" maxuid="${2:?}" mingid="${3:?}" maxgid="${4:?}"

	pipe="${TMPDIR:-/tmp}/ids-$$.fifo" rc=0
	mkfifo -m 700 "$pipe"

	ids >"$pipe" & ids=$!
	while read -r uid user
	do
		[ "$uid" -lt "$minuid" ] || [ "$uid" -gt "$maxuid" ] &&
			continue

		case $user in (*[!A-Za-z0-9_]*)
			continue
		esac

		for gid in $(id -G "$user")
		do
			[ "$gid" -lt "$mingid" ] || [ "$gid" -gt "$maxgid" ] &&
				continue 2
		done

		break
	done <"$pipe"

	wait $ids || rc=$?
	rm -f "$pipe"
	[ "$rc" -eq 0 ] && [ "$user" ] || return 1

	printf '%s\n' "$user"
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
	mkdir -m 0700 "$__tmpdir_tmpdir" || exit
	cleanup="rm -rf \"\$__tmpdir_tmpdir\"; ${cleanup-}"
	catch=x
	[ "${caught-}" ] && exit $((caught + 128))
	# shellcheck disable=2031
	export TMPDIR="$__tmpdir_tmpdir"
}

# Find an unallocated ID in a given range.
# -g looks for a group ID.
unallocid() (
	OPTIND=1 OPTARG='' opt=
	opts=
	while getopts 'g' opt
	do
		case $opt in
		(g) opts=-g ;;
		(*) return 2
		esac
	done
	shift $((OPTIND - 1))
	start="${1:?}" end="${2:?}"

	pipe="${TMPDIR:-/tmp}/ids-$$.fifo" rc=0
	mkfifo -m 0700 "$pipe"
	ids $opts >"$pipe" 2>/dev/null & pid=$!
	# shellcheck disable=2086
	ids="$(cut -d' ' -f1 <"$pipe")" || rc=$?
	rm -f "$pipe"

	[ "$rc" -eq 0 ] || return $rc
	wait "$pid" || [ $? -eq 67 ]

	i="$start"
	while [ "$i" -le "$end" ]
	do
		# shellcheck disable=2086
		inlist -eq "$i" $ids || break
		i=$((i + 1))
	done

	printf '%d\n' "$i"
)

# Print $* to stderr.
# -l prefixes the messages with the number of the line warn was called from.
# -q suppresses output if $quiet is set.
# -v suppresses output unless $verbose is set.
warn() (
	line=''
	OPTIND=1 OPTARG='' opt=''
	while getopts 'lqv' opt
	do
		case $opt in
		(l) line="${_line-}" ;;
		(q) [ "${quiet-}" ] && return 0 ;;
		(v) [ "${verbose-}" ] || return 0 ;;
		(*) return 1
		esac
	done
	shift $((OPTIND - 1))

	exec >&2
	printf '%s: ' "${prog_name:-$0}"
	[ "$line" ] && printf 'line %d: ' "$line"
	# shellcheck disable=SC2059
	printf -- "$@"
	echo

	return 0
)
