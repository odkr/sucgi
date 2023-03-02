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
	# Root directory of the repository.
	: "${src_dir:?}"

	# Load tool library.
	# shellcheck source=../tools/lib.sh
	. "$src_dir/tools/lib.sh" || exit

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

# Pad $str with $ch up to length $n.
pad() (
	str="${1?}" n="${2:?}" ch="${3:-x}" side="${4:-l}"
	pipe="${TMPDIR:-/tmp}/pad-$$.fifo" rc=0
	mkfifo -m 0700 "$pipe"
	case $side in
	(l) printf '%*s\n' "$n" "$str" >"$pipe" & printf=$! ;;
	(r) printf '%-*s\n' "$n" "$str" >"$pipe" & printf=$! ;;
	(*) err "side must be 'l' or 'r'."
	esac
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
