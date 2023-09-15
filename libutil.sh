#
# Utility functions for scripts.
#
# Copyright 2022 Odin Kroeger.
#
# This file is part of suCGI.
#
# suCGI is free software: you can redistribute it and/or modify it
# under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# suCGI is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
# Public License for more details.
#
# You should have received a copy of the GNU Affero General Public
# License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
#

# shellcheck disable=2015,2031

# If $catch is non-empty, call cleanup and re-raise signal $1.
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

# Check if a programme behaves as expected.
# -s STATUS   Check for exit status STATUS (default: 0).
# -o STR      Check for STR on stdout.
# -e STR      Check for STR on stderr.
# -o and -e are mutually exclusive.
check() (
	_check_status=0 _check_stream=out
	OPTIND=1 OPTARG='' _check_opt=''

	_check_stream='' _check_pattern=''
	while getopts 's:o:e:v' _check_opt
	do
		# shellcheck disable=2034
		case $_check_opt in
		(s) _check_status="${OPTARG:?}" ;;
		(o) _check_stream=out _check_pattern="${OPTARG:?}" ;;
		(e) _check_stream=err _check_pattern="${OPTARG:?}" ;;
		(*) return 2
		esac
	done
	shift $((OPTIND - 1))

	: "${1:?no command given}"

	_check_err=0 _check_retval=0

	case $_check_stream in
	(out) _check_output="$(env "$@" 2>/dev/null)"     || _check_err=$? ;;
	(err) _check_output="$(env "$@" 2>&1 >/dev/null)" || _check_err=$? ;;
	('')  env "$@" >/dev/null 2>&1                    || _check_err=$? ;;
	(*)   err -s70 'line %d: %s: invalid case.' \
	               "${LINENO-???}" "$_check_stream"
	esac

	if	! inlist -eq "$_check_err" "$_check_status" 141
	then
		warn '%s: exited with status %d.' "$*" "$_check_err"
		_check_retval=70
	fi

	if	[ "$_check_stream" ] &&
		! printf '%s\n' "$_check_output" | grep -Fq "$_check_pattern"
	then
		_check_retval=70
		warn '%s printed to std%s:' "$*" "$_check_stream"
		printf '%s\n' "$_check_output" >&2
	fi

	return "$_check_retval"
)

# Disable signal handlers, terminate alls jobs and the process group,
# run and reset $cleanup, and exit with status $?.
cleanup() {
	# shellcheck disable=2319
	_cleanup_retval=$?
	set +e
	trap '' EXIT ALRM HUP INT TERM USR1 USR2
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
# Exit with STATUS if -s STATUS is given.
err() {
	_err_status=69
	OPTIND=1 OPTARG='' _err_opt=''
	while getopts s: _err_opt
	do
		case $_err_opt in
		(s) _err_status="${OPTARG:?}" ;;
		(*) exit 70
		esac
	done
	shift $((OPTIND - 1))

	warn -- "$@"

	exit "$_err_status"
}

# Enforce POSIX-compliance, register signal handlers, and set globals.
init() {
	# shellcheck disable=2039,3040
	[ "${BASH_VERSION-}" ] && set -o posix
	[ "${ZSH_VERSION-}" ] && emulate sh 2>/dev/null
	export BIN_SH=xpg4 NULLCMD=: POSIXLY_CORRECT=x CLICOLOR_FORCE=

	# Make sure IFS is safe.
	unset IFS

	# Trap signals that would terminate the script.
	catch='' caught=
	for _init_sig in ALRM HUP INT TERM USR1 USR2
	do
		# shellcheck disable=2064
		trap "catch $_init_sig" "$_init_sig"
	done
	unset _init_sig

	trap cleanup EXIT
	catch=y
	[ "$caught" ] && kill -s "$caught" "$$"

	# Safe permission mask.
	umask 022

	# Output control.
	quiet='' verbose=''

	# Programme name.
	progname="$(basename -- "$0")" || progname="$0"
	readonly progname
}

# Check if $2 matches any member of $@ using operator $1.
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
# -d DIR     Store log file in DIR (default: .).
# -i STATUS  Do not store a log if $@ exits with STATUS (default: 0).
# -l FNAME   Store the log in FNAME (default: basename of $1).
# -u USER    Make USER the owner of FNAME (default: current user).
# -g GROUP   Make GROUP the group of FNAME (default: current group).
logged() (
	_logged_dir=.
	_logged_fname=''
	_logged_group=''
	_logged_user=''
	_logged_mask=0
	_logged_status=0

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

	if [ "$verbose" ]
	then
		"$@" || _logged_status=$?
		return $_logged_status
	fi

	: "${TMPDIR:-/tmp}"
	: "${_logged_fname:="$(basename "${1:?}").log"}"
	_logged_log="$TMPDIR/$_logged_fname"

	"$@" >>"$_logged_log" 2>&1 || _logged_status=$?

	# shellcheck disable=2086
	if inlist -eq "$_logged_status" $_logged_mask
	then
		rm -f "$TMPDIR/$_logged_fname" >/dev/null 2>&1
	else
		warn '%s: exited with status %d.' "$*" "$_logged_status"

		if [ -e "$_logged_log" ]
		then
			if [ "$_logged_user" ]
			then
				: "${_logged_group:=$(id -gn "$_logged_user")}"
				chown "$_logged_user:$_logged_group" \
				      "$_logged_log"
			fi

			mv "$_logged_log" "$_logged_dir"
			warn 'see %s for details.' "$_logged_fname"
		fi
	fi

	return $_logged_status
)

# Print the login name of a file's owner.
owner() (
	# shellcheck disable=2012
	ls -ld "${1:?}" | awk '{print $3}'
)

# Find a user with an UID in the range [$1 .. $2] who
# only belongs to groups with GIDs in the range [$3 .. $4].
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

			# shellcheck disable=2086
			inlist -lt "$3" $_reguser_gids && continue
			# shellcheck disable=2086
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
	_tmpdir_prefix="${1:-tmp}" _tmpdir_dir="${2:-"${TMPDIR:-/tmp}"}"
	[ "${_tmpdir_tmpdir-}" ] && return

	# shellcheck disable=SC2016
	_tmpdir_real="$(cd -P "$_tmpdir_dir" && pwd)" ||
	err 'cd -P $_tmpdir_dir && pwd: exited with status %d.' $?
	readonly _tmpdir_tmpdir="$_tmpdir_real/$_tmpdir_prefix-$$"

	catch=
	mkdir -m 0755 "$_tmpdir_tmpdir" || exit
	cleanup="rm -rf \"\$_tmpdir_tmpdir\"; ${cleanup-:}"
	catch=x
	[ "${caught-}" ] && kill -s "$caught" "$$"

	export TMPDIR="$_tmpdir_tmpdir"
}

# Print the format $1 to stderr, using the remaing operands as arguments.
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
	printf '%s: ' "${progname:-$0}"
	# shellcheck disable=SC2059
	printf -- "$@"
	if [ "$_warn_lf" ]
	then echo
	fi
)
