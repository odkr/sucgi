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

# Check if calling a programme produces a result.
# -s checks for an exit status (defaults to 0),
# -o STR for STR on stdout, -e STR for STR on stderr.
# FIXME: Doesn't work with posh.
check() (
	exstatus=0 stream=''
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

	if [ "$stream" ]
	then
		# shellcheck disable=2030
		: "${TMPDIR:=/tmp}"

		tmpfile="$TMPDIR/std$stream-$$.log"
		eval "std$stream=\"\$tmpfile\""
	fi

	: "${1:?no command given}"
	: "${stdout:=/dev/null}"
	: "${stderr:=/dev/null}"

	err=0 retval=0

	# shellcheck disable=2154
	env "$@" >"$stdout" 2>"$stderr" || err=$?

	if [ "$stream" ]
	then
		if ! grep -Fq "$pattern" <"$tmpfile"
		then
			warn '%s printed to std%s:' "$*" "$stream"
			while read -r line
			do
				warn '> %s' "$line"
			done <"$tmpfile"
			rc=70
		fi

		rm -f "$tmpfile"
	fi


	if [ "$err" -ne "$exstatus" ] && [ "$err" -ne 141 ]
	then
		warn '%s: exited with status %d.' "$*" "$err"
		retval=70
	fi

	return "$retval"
)

# Re-raise $signal if $catch is set.
# Otherwise set $caught to $signal.
catch() {
	signal="${1:?}"
	if [ -t 2 ]
	then
		printf '\r' >&2
		tput el >&2
	fi
	warn 'caught %s.' "$signal"
	if [ "${catch-}" ]
	then
		cleanup
		trap - "$signal"
		kill -s "$signal" "$$"
	fi
	# shellcheck disable=2034
	caught="$signal"
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
	return $((retval % 128))
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

# Register signals, set variables, and enable colours.
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
			seg="$(pad "$dirlen" "$i")"
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
		seg="$(pad "$((len - ${#dir} - 1))" "$i")"
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

# Take filenames and print non-canonical mutations of those filenames.
mutatefnames() {
	printf '%s\n' "$@"				|
	sed -n 'p; s/^\([^/]\)/.\/\1/p'			|
	sed -n 'p; s/\//\/.\//p'			|
	sed -n 'p; s/\/\([^/]*\)\//\/\1\/..\/\1\//p'	|
	sed -n 'p; s/\//\/\//p'
}

# Pad $str with $fill on $side up to length $n.
pad() (
	n="${1:?}" str="${2-}" fill="${3:-x}" side="${4:-l}"

	strlen="${#str}" fillen="${#fill}"
	limit="$((n - fillen))" pad=
	while [ "$strlen" -le "$limit" ]
	do pad="$pad$fill" strlen=$((strlen + fillen))
	done

	case $side in
	(l) printf '%s%s\n' "$pad" "$str" ;;
	(r) printf '%s%s\n' "$str" "$pad" ;;
	esac
)

# Find a user whose UID is between $minuid and $maxuid and who
# only belongs to groups with GIDs between $mingid and $maxgid.
reguser() (
	minuid="${1:?}" maxuid="${2:?}" mingid="${3:?}" maxgid="${4:?}"
	pipe="${TMPDIR:-/tmp}/ids-$$.fifo" retval=0 reguser=
	mkfifo -m 700 "$pipe"

	ids >"$pipe" & ids=$!
	while read -r uid user
	do
		case $user in (*[!A-Za-z0-9_]*)
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

	wait $ids || retval=$?
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

	pipe="${TMPDIR:-/tmp}/ids-$$.fifo" retval=0
	mkfifo -m 0700 "$pipe"
	ids $opts >"$pipe" 2>/dev/null & pid=$!
	# shellcheck disable=2086
	ids="$(cut -d' ' -f1 <"$pipe")" || retval=$?
	rm -f "$pipe"

	[ "$retval" -eq 0 ] || return $retval
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
