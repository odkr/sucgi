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

# shellcheck disable=2015


#
# Aliases
#

alias checkerr='_line="$LINENO" _checkerr'
alias checkok='_line="$LINENO" _checkok'
alias err='_line="$LINENO" _err'
alias warn='_line="$LINENO" _warn'


#
# Functions
#

# Print a message to STDERR and exit with a non-zero status.
_err() {
	_warn -r "$@"
	exit 2
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

# Abort if a programme doesn't print $err to STDERR or exits with status zero.
_checkerr() (
	err="${1?}"
	shift
	: "${@:?no command given}"
	tmpfile="${TMPDIR-/tmp}/checkerr-$$.tmp" rc=0
	_warn "checking ${bld-}$*${rst-} ..."
	if env "$@" >/dev/null 2>"$tmpfile"
	then
		rc=2
		_warn -lr "${bld-}$*${rst_r-} exited with status 0."
	elif ! grep -Fq "$err" "$tmpfile"
	then
		prefix="${red-}${bld-}>${rst_r-} " rc=2
		_warn -lr "${bld-}expected${rst_r} error:"
		echo "$prefix$err${rst-}" >&2
		_warn -lr "${bld-}actual${rst_r} error:"
		sed "s/^/$prefix /; s/\$/${rst-}/" <"$tmpfile" >&2
	fi
	rm "$tmpfile" || rc=$?
	return $rc
)

# Abort if a programme doesn't print $msg or exits with a non-zero status.
_checkok() (
	msg="${1?}"
	shift
	: "${@:?no command given}"
	tmpfile="${TMPDIR-/tmp}/checkok-$$.tmp" rc=0
	_warn "checking ${bld-}$*${rst-} ..."
	if ! env "$@" >"$tmpfile" 2>&1
	then
		rc=2
		_warn -lr "${bld-}$*${rst_r-} exited with status $?."
	elif ! grep -Fq "$msg" "$tmpfile"
	then
		prefix="${red-}${bld-}>${rst_r-} " rc=2
		_warn -lr "${bld-}expected${rst_r} output:"
		echo "$prefix$msg${rst-}" >&2
		_warn -lr "${bld-}actual${rst_r} output:"
		sed "s/^/$prefix /; s/\$/${rst-}/" <"$tmpfile" >&2
	fi
	rm "$tmpfile" || rc=$?
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

# Register signals, set variables, a umask, and enable colours.
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
	umask 077

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

	# shellcheck disable=2034
	readonly rst bld grn red ylw rst_g rst_r rst_y

	# Programme name.
	prog_name="$(basename "$0")" || prog_name="$0"
	readonly prog_name

	# Linefeed.
	readonly lf="
"
}

# Print the owner of $file
owner() (
	file="${1:?}"
	pipe="${TMPDIR:-/tmp}/ls-$$.fifo" rc=0
	mkfifo "$pipe"
	ls -ld "$file" >"$pipe" & ls=$!
	awk '{print $3}' <"$pipe" & awk=$!
	wait "$ls"    || rc=$?
	wait "$awk"   || rc=$?
	rm -f "$pipe" || rc=$?
	return $rc
)

# Check if a line on STDIN contains $str.
match() (
	: "${lf:?}"
	str="${1?}" file=

	while read -r line
	do
		case $line in (*"$str"*)
			return 0
		esac
		if [ "$file" ]
			then file="$file$lf$line"
			else file="$line"
		fi
	done

	return 1
)
	
# Create a path of $len length in $basepath.
mklongpath() (
	basepath="${1:?}" len="${2:?}" max=99999
	name_max="$(getconf NAME_MAX "$basepath")" || name_max=14

	path="$basepath"
	while [ "$((${#path} + name_max + 2))" -le "$len" ]
	do
		i=0
		while [ $i -lt "$max" ]
		do
			seg="$(pad "-$$-$i" "$name_max")"
			if ! [ -e "$path/$seg" ]
			then
				path="$path/$seg"
				continue 2
			fi
			i=$((i + 1))
		done
	done

	i=0
	while [ $i -lt "$max" ]
	do
		seg="$(pad "-$$-$i" "$((len - ${#path} - 1))")"
		if ! [ -e "$path/$seg" ]
		then
			printf '%s\n' "$path/$seg"
			return 0
		fi
		i=$((i + 1))
	done
	
	err "failed to generate filename."
)

# Pad $str with $ch up to length $n.
pad() (
	str="${1:?}" n="${2:?}" ch="${3:-x}"
	pipe="${TMPDIR:-/tmp}/pad-$$.fifo"
	mkfifo -m 0700 "$pipe"
	printf '%*s\n' "$n" "$str" >"$pipe" & printf=$!
	tr ' ' "$ch" <"$pipe" & tr=$!
	rm -f "$pipe"
	wait $printf $tr
)


# Get the login name of the user who invoked the script,
# even if the script has been invoked via su or sudo.
regularuser() (
	: "${TMPDIR:=/tmp}"
	proc="$TMPDIR/ps-$$.fifo" sorted="$TMPDIR/sort-$$.fifo" rc=1
	mkfifo -m 0700 "$proc" "$sorted"

	pivot="$$"
	while true
	do
		ps -Ao 'pid= ppid= user=' >"$proc" & ps=$! 
		sort -r <"$proc" >"$sorted" & sort=$!

		cur_user=
		while read -r pid ppid user
		do
			if [ "$pid" -le 1 ]
			then
				break 2
			elif [ "$pid" -eq "$pivot" ]
			then
				continue
			fi

			if	[ "$user" != "$cur_user" ] &&
				! uid="$(id -u "$user")"
			then
				break
			fi

			if [ "$uid" -ne 0 ]
			then
				printf '%s\n' "$user"
				rc=0
				break
			fi

			pivot="$ppid" cur_user="$user"
		done <"$sorted"
		
		wait "$ps" "$sort" || break
 	done

	rm -f "$proc" "$sorted"
	return $rc
)

# Create a directory with the filename $prefix-$$ in $dir,
# register it for deletion via $cleanup, and set it as $TMPDIR.
tmpdir() {
	[ "${__tmpdir_tmpdir-}" ] && return
	__tmpdir_prefix="${1:-tmp}" __tmpdir_dir="${2:-"${TMPDIR:-/tmp}"}"
	__tmpdir_real="$(cd -P "$__tmpdir_dir" && pwd)" ||
		err "cd -P $__tmpdir_dir && pwd: exited with status $?."
	readonly __tmpdir_tmpdir="$__tmpdir_real/$__tmpdir_prefix-$$"
	catch=
	mkdir -m 0700 "$__tmpdir_tmpdir" || exit
	cleanup="rm -rf \"\$__tmpdir_tmpdir\"; ${cleanup-}"
	catch=x
	[ "${caught-}" ] && exit $((caught + 128))
	export TMPDIR="$__tmpdir_tmpdir"
}

# Starting with, but excluding, $dirname run $dircmd for every path segment
# of $fname. If $fcmd is given, run $dircmd for every path segment up to,
# but excluding $fname, and run $fcmd for $fname.
traverse() (
	dirname="${1:?}" fname="${2:?}" dircmd="${3:?}" fcmd="${4:-"$3"}"

	IFS=/
	set -- ${fname#"${dirname%/}/"}
	unset IFS
	cd "$dirname" || exit

	while [ "$#" -gt 0 ]
	do
		fname="$1"
		shift

		case $# in
		 	(0)	eval "$fcmd"
				return ;;
			(*)	eval "$dircmd"
		 		dirname="${dirname%/}/$fname"
		 		cd "$fname" ;;
		esac
	done
)

# Print a message to STDERR.
# -r, -y, -g colour the message red, yellow, and green respectively.
# -q tells warn to respect $quiet.
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
	                               printf '\a\n'

	return 0
)
