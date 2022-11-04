#!/bin/sh
# Utility functions for the test suite and the tools.
#
# Copyright 2022 Odin Kroeger
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
# Aliases
#

alias check='_line="${LINENO-}" _check'
alias err='_line="${LINENO-}" _err'
alias warn='_line="${LINENO-}" _warn'


#
# Functions
#

# Print a message to STDERR and exit with a non-zero status.
# -s sets a different status.
_err() {
	rc=2
	OPTIND=1 OPTARG='' opt=''
	while getopts s: opt
	do
		case $opt in
			(s) OPTARG="$rc" ;;
			(*) exit 70
		esac
	done
	shift $((OPTIND - 1))

	_warn -r "$@"
	
	exit "$rc"
}

# Exit with status $signo + 128 if $catch is set.
# Otherwise set $caught to $signo. 
catch() {
	signo="${1:?}"
	sig="$(kill -l "$signo")" || sig="signal no. $signo"
	_warn "caught ${bld-}$sig${rst-}."
	[ "${catch-}" ] && exit "$((signo + 128))"
	# shellcheck disable=2034
	caught="$signo"
}

# Check if calling a programme produces a result.
# -s checks for an exit status (defaults to 0),
# -o STR for STR on stdout, -e STR for STR on stderr.
_check() (
	exp_stat=0 exp_out='' exp_err=''
	OPTIND=1 OPTARG='' opt=''
	while getopts 's:o:e:v' opt
	do
		# shellcheck disable=2034
		case $opt in
			(s) exp_stat="$OPTARG" ;;
			(o) exp_out="$OPTARG" ;;
			(e) exp_err="$OPTARG" ;;
			(*) return 2
		esac
	done
	shift $((OPTIND - 1))
	: "${@?no command given}"

	# shellcheck disable=2030
	: "${TMPDIR:=/tmp}"
	for ch in out err
	do
		eval exp="\"\${exp_${ch}-}\""
		if [ "$exp" ]
		then
			pipe="$TMPDIR/std$ch-$$.fifo"
			mkfifo -m 0700 "$pipe"
		else
			pipe=/dev/null
		fi

		eval "pipe_${ch}=\"\$pipe\""
	done

	_warn -q "checking ${bld-}$*${rst-} ..."
	# shellcheck disable=2154
	env "$@" >"$pipe_out" 2>"$pipe_err" & env=$!

	lf="
"
	for ch in out err
	do
		eval exp="\"\${exp_${ch}-}\""
		eval pipe="\"\${pipe_${ch}-}\""

		str=
		while read -r line
		do
			[ "$exp" ] || break
			case $line in (*"$exp"*)
				eval "pat_${ch}=x"
				break
			esac
			str="$str${red-}${bld-}>${rst_r-} $line${rst-}$lf"
		done <"$pipe"

		eval "str_${ch}=\"\$str\""
	done

	stat=0
	wait "$env" || stat=$?

	for pipe in "$pipe_out" "$pipe_err"
	do
		[ "$pipe" = /dev/null ] || rm -f "$pipe"
	done

	[ "$stat" -eq "$exp_stat" ] || [ "$stat" -eq 141 ] ||
		_err -s70 -l"${red-}exited with status ${bld-}$stat${rst_r-}."

	rc=0
	for ch in out err
	do
		eval exp="\"\${exp_${ch}-}\""
		[ "$exp" ] || continue

		eval pat="\"\${pat_${ch}-}\""
		[ "$pat" ] && continue
		
		eval str="\"\${str_${ch}-}\""
		_warn -lr "${bld-}expected text on std$ch${rst_r}:"
		printf '%s\n' "${red-}${bld-}>${rst_r-} $exp${rst-}" >&2
		_warn -lr "${bld-}actual output${rst_r-}:"
		printf '%s' "$str" >&2
		rc=70
	done

	return $rc
)

# Send TERM to all children, eval $cleanup and exit with status $?.
cleanup() {
	rc=$?
	set +e
	trap : EXIT HUP INT QUIT TERM
	# shellcheck disable=2046
	kill -15 $(jobs -p) -$$ >/dev/null 2>&1
	[ "${cleanup-}" ] && [ "$rc" -ne 131 ] && eval "$cleanup"
	[ "${rst-}" ] && printf %s "$rst" >&2
	exit "$rc"
}

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
	trap 'catch 3' QUIT
	trap 'catch 15' TERM
	trap cleanup EXIT
	[ "$caught" ] && exit "$((caught + 128))"

	# Permission mask.
	umask 022

	# Colours and formatting.
	ncols="$(tput colors 2>/dev/null)" || ncols=0
	rst='' bld='' grn='' red='' ylw=''
	if [ "$ncols" -gt 0 ] && [ -t 2 ]
	then
		# shellcheck disable=2034
		if rst="$(tput sgr0 2>/dev/null)"
		then
			bld="$(tput bold    2>/dev/null)" || : 
			red="$(tput setaf 1 2>/dev/null)" || :
			grn="$(tput setaf 2 2>/dev/null)" || :
			ylw="$(tput setaf 3 2>/dev/null)" || :
		fi
	fi
	rst_g="$rst$grn" rst_r="$rst$red" rst_y="$rst$ylw"
	quiet=

	# shellcheck disable=2034
	readonly rst bld grn red ylw rst_g rst_r rst_y

	# Programme name.
	prog_name="$(basename "$0")" || prog_name="$0"
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

# Create a path of $len length in $basepath.
mklongpath() (
	basepath="${1:?}" len="${2:?}" max=99999
	name_max="$(getconf NAME_MAX "$basepath" 2>/dev/null)" || name_max=14

	path="$basepath"
	while [ "$((${#path} + name_max + 2))" -le "$len" ]
	do
		i=0
		while [ $i -lt "$max" ]
		do
			seg="$(pad "$i" "$name_max")"
			if ! [ -e "$path/$seg" ]
			then
				path="$path/$seg"
				continue 2
			fi
			i=$((i + 1))
		done
	done

	i=0 fname=
	while [ $i -lt "$max" ]
	do
		seg="$(pad "$i" "$((len - ${#path} - 1))")"
		if ! [ -e "$path/$seg" ]
		then
			fname="$path/$seg"
			break
		fi
		i=$((i + 1))
	done

	# $path may get so long that the first "$path/$i" that does not exist
	# is so long that $fname as a whole gets longer than $len.
	[ "${fname-}" ] && [ "${#fname}" -eq "$len" ] ||
		err "failed to generate long filename."

	printf '%s\n' "$fname"
)

# Pad $str with $ch up to length $n.
pad() (
	str="${1:?}" n="${2:?}" ch="${3:-x}"
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

	pipe="${TMPDIR:-/tmp}/ent-$$.fifo" rc=0
	mkfifo -m 700 "$pipe"	
	ents -u -f"$minuid" -c"$maxuid" >"$pipe" & ents=$!
	while IFS=: read -r _ user
	do
		for gid in $(id -G "$user")
		do
			[ "$gid" -lt "$mingid" ] || [ "$gid" -gt "$maxgid" ] &&
				continue 2
		done

		case $user in (*[!A-Za-z0-9_]*)
			continue 2
		esac

		break
	done <"$pipe"
	wait $ents || rc=$?
	rm -f "$pipe"
	[ "$rc" -eq 0 ] && [ "$user" ] || return 1

	printf '%s\n' "$user"
)

# Create a directory with the filename $prefix-$$ in $dir,
# register it for deletion via $cleanup, and set it as $TMPDIR.
tmpdir() {
	[ "${__tmpdir_tmpdir-}" ] && return
	# shellcheck disable=2031
	__tmpdir_prefix="${1:-tmp}" __tmpdir_dir="${2:-"${TMPDIR:-/tmp}"}"
	__tmpdir_real="$(cd -P "$__tmpdir_dir" && pwd)" ||
		err "cd -P $__tmpdir_dir && pwd: exited with status $?."
	readonly __tmpdir_tmpdir="$__tmpdir_real/$__tmpdir_prefix-$$"
	catch=
	mkdir -m 0700 "$__tmpdir_tmpdir" || exit
	cleanup="rm -rf \"\$__tmpdir_tmpdir\"; ${cleanup-}"
	catch=x
	[ "${caught-}" ] && exit $((caught + 128))
	# shellcheck disable=2031
	export TMPDIR="$__tmpdir_tmpdir"
}

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
		 	(0)	eval "$fcmd"
				return ;;
			(*)	eval "$dircmd"
		 		dirname="${dirname%/}/$fname"
		 		cd "$fname" || return ;;
		esac
	done
)

# Find an unallocated ID in a given range.
# -u finds a user ID, -g a group ID.
unallocid() (
	OPTIND=1 OPTARG='' opt=
	opts=''
	while getopts 'ug' opt
	do
		case $opt in
			(u) opts="$opts -u" ;;
			(g) opts="$opts -g" ;;
			(*) return 2
		esac
	done
	shift $((OPTIND - 1))
	start="${1:?}" end="${2:?}"
	pipe="${TMPDIR:-/tmp}/ents-$$.fifo" rc=0

	mkfifo -m 0700 "$pipe"
	# shellcheck disable=2086
	ents -f"$start" -c"$end" $opts >"$pipe" 2>/dev/null & ents=$!
	ids="$(cut -d: -f1 <"$pipe")" || rc=$?
	rm -f "$pipe"
	[ "$rc" -eq 0 ] || return $rc
	wait "$ents" || [ $? -eq 67 ]

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
# -r, -y, -g colour the message red, yellow, and green respectively.
# -l prefixes it with the number of the line warn was called from.
# -q suppresses output if $quiet is set.
_warn() (
	col='' line=
	OPTIND=1 OPTARG='' opt=''
	while getopts 'L:lgryq' opt
	do
		case $opt in
			(l) line=x ;;
			(g) col="${grn-}" ;;
			(r) col="${red-}" ;;
			(y) col="${ylw-}" ;;
			(q) [ "${quiet-}" ] && return 0 ;;
			(*) return 1
		esac
	done
	shift $((OPTIND - 1))

	exec >&2
	                               printf '%s: ' "${prog_name:-$0}"
	[ "$line" ] && [ "$_line" ] && printf 'line %d: ' "$_line"
	[ "$col" ] && [ "${rst-}" ] && printf '%s' "$col"
	                               printf '%s' "$*"
	[ "$col" ] && [ "${rst-}" ] && printf '%s' "$rst"
	echo

	return 0
)

