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

# Print a message to STDERR and exit with a non-zero status.
err() {
	warn -r "$@"
	exit 8
}

# Exit with status $signo + 128 if $catch is set.
# Otherwise set $caught to $signo. 
catch() {
	signo="${1:?}"
	sig="$(kill -l "$signo")" || sig="signal no. $signo"
	warn "caught ${bld-}$sig${rst-}."
	[ "${catch-}" ] && exit "$((signo + 128))"
	# shellcheck disable=2034
	caught="$signo"
}

# Abort if a programme doesn't print $err to STDERR or exits with status zero.
checkerr() (
	err="${1?}"
	shift
	: "${@:?no command given}"
	pipe="${TMPDIR-/tmp}/checkerr-$$.fifo" rc=0
	warn "checking ${bld-}$*${rst-} ..."
	mkfifo -m 0700 "$pipe"
	env "$@" >/dev/null 2>"$pipe" & env_pid=$!
	match "$err" <"$pipe" & match_pid=$!
	wait $env_pid && {
		# shellcheck disable=2154
		warn -r "${bld-}$*${rst_r-} exited with status 0."
		rc=1
	}
	wait $match_pid	|| rc=$?
	rm -f "$pipe"	|| rc=$?
	return $rc
)

# Abort if a programme doesn't print $msg or exits with a non-zero status.
checkok() (
	msg="${1?}"
	shift
	: "${@:?no command given}"
	pipe="${TMPDIR-/tmp}/checkok-$$.fifo" rc=0
	warn "checking ${bld-}$*${rst-} ..."
	mkfifo -m 0700 "$pipe"
	env "$@" >"$pipe" 2>&1 & env_pid=$!
	match "$msg" <"$pipe" & match_pid=$!
	wait $env_pid || {
		# shellcheck disable=2154
		warn -r "${bld-}$*${rst_r-} exited with status $?."
		rc=1
	}
	wait $match_pid	|| rc=$?
	rm -f "$pipe"	|| rc=$?
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
	catch=x caught=
	trap 'catch 1' HUP
	trap 'catch 2' INT
	trap 'catch 3' QUIT
	trap 'catch 15' TERM
	trap cleanup EXIT
	[ "$caught" ] && exit "$((caught + 128))"

	umask 077

	rst='' bld='' grn='' red='' ylw=''
	if [ -t 2 ]
	then
		case ${TERM-} in (*color*)
			# shellcheck disable=2034
			if rst="$(tput sgr0 2>/dev/null)"
			then
				bld="$(tput bold    2>/dev/null)" || : 
				red="$(tput setaf 1 2>/dev/null)" || :
				grn="$(tput setaf 2 2>/dev/null)" || :
				ylw="$(tput setaf 3 2>/dev/null)" || :
			fi
		esac
	fi
	
	rst_g="$rst$grn" rst_r="$rst$red" rst_y="$rst$ylw"
	
	# shellcheck disable=2034
	readonly rst bld grn red ylw

	prog_name="$(basename "$0")" || prog_name="$0"
	readonly prog_name

	readonly lf="
"

	# shellcheck disable=2154
	PATH="$script_dir:$script_dir/../tools:$PATH"
}

# Print the owner of $file
owner() (
	file="${1:?}"
	pipe="${TMPDIR:-/tmp}/ls-$$.fifo" rc=0

	mkfifo "$pipe"
	ls -l "$file" >"$pipe" & ls=$!
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

	warn -r "'${bld-}$str${rst_r-}' not in:$lf${bld-}$file${rst-}"
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
			[ "$pid" -eq "$pivot" ]	|| continue

			if	[ "$user" != "$cur_user" ] &&
				! uid="$(id -u "$user")"
			then
				warn -r "id -u $user: exited with status $?."
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
warn() (
	col=
	OPTIND=1 OPTARG='' opt=''
	while getopts 'gryq' opt
	do
		case $opt in
			(g) col="${grn-}" ;;
			(r) col="${red-}" ;;
			(y) col="${ylw-}" ;;
			(q) [ "${quiet-}" ] && return 0 ;;
			(*) return 1
		esac
	done
	shift $((OPTIND - 1))
	
	exec >&2

	[ "${prog_name-}" ]         && printf '%s: ' "$prog_name"
	[ "$col" ] && [ "${rst-}" ] && printf '%s' "$col"
	                               printf '%s' "$*"
	[ "$col" ] && [ "${rst-}" ] && printf '%s' "$rst"
	                               printf '\a\n'

	return 0
)
