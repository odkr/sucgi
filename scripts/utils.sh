#!/bin/sh
# Common functions for scripts.

# Print a message to STDERR and exit with a non-zero status.
abort() {
	warn "${red-}$*${reset-}"
	exit 8
}

# Exit the with $signo + 128 if $catch. Otherwise set $caught to $signo. 
catch() {
	signo="${1:?}"
	warn "caught signal $signo."
	[ "${catch-}" ] && exit "$((signo + 128))"
	# shellcheck disable=2034
	caught="$signo"
}

# Abort if a programme doesn't print $err to STDERR or exits with status zero.
checkerr() (
	err="${1:?}"
	shift
	: "${1:?no programme given}"
	: "${__tmpdir_tmpdir:?checkok needs a temporary directory}"
	fifo="$TMPDIR/${1##*/}.fifo"
	mkfifo "$fifo"
	"${@:?}" >/dev/null 2>"$fifo" & pid="$!"
	match "$err" <"$fifo"
	# shellcheck disable=2154
	wait "$pid" && abort "$bold$*$reset exited with status 0" \
	                     "for error $bold$err$reset."
	rm "$fifo"
)

# Abort if a programme doesn't print $msg or exits with a non-zero status.
checkok() (
	msg="${1:?}"
	shift
	: "${1:?no programme given}"
	: "${__tmpdir_tmpdir:?checkok needs a temporary directory}"
	fifo="$TMPDIR/${1##*/}.fifo"
	mkfifo "$fifo"
	"${@:?}" >"$fifo" 2>&1 & pid="$!"
	match "$msg" <"$fifo"
	# shellcheck disable=2154
	wait "$pid" || abort "$bold$*$reset exited with non-zero status $?."
	rm "$fifo"
)

# Send TERM to all children, eval $cleanup and exit with status $?.
cleanup() {
	status=$?
	set +e
	trap : EXIT HUP INT TERM
	kill -15 -$$ >/dev/null 2>&1
	[ "${cleanup-}" ] && eval "$cleanup"
	[ "${reset-}" ] && printf %s "$reset"
	exit "$status"
}

# Register signal handlers, set global variables, a umask, and enable colours.
init() {
	trap cleanup EXIT;
	trap 'catch 1' HUP;
	trap 'catch 2' INT;
	trap 'catch 15' TERM;
	catch=x

	umask 077

	reset='' bold='' green='' red=''
	if [ -t 2 ]
	then
		case ${TERM-} in (*color*)
			# shellcheck disable=2034
			if reset="$(tput sgr0 2>/dev/null)" && [ "$reset" ]
			then
				bold="$(tput bold 2>/dev/null)" || : 
				green="$(tput setaf 2 2>/dev/null)" || :
				red="$(tput setaf 1 2>/dev/null)" || :
			fi
		esac
	fi
	# shellcheck disable=2034
	readonly reset bold green red

	readonly lf="
"
}

# Check if a path is portable.
isportable() (
	path="${1:?}"
	# shellcheck disable=1001
	case $path in (*[!\/\.\-\_0-9A-Za-z]*)
		return 1
	esac
	return 0
)

# Abort if no line on STDIN contains $string.
match() (
	string="${1:?}"
	file=
	while read -r line
	do
		case $line in (*$string*)
			return 0 ;;
		esac
		file="$file$line$lf"
	done
	abort "'$bold$string$reset$red' not in:$lf$bold${file%"$lf"}$reset"
)

# Print the canonical path of a directory.
realdir() (
	dir="${1:?}"
	cd -P "$dir" || exit
	pwd
)

# Create a directory with the filename $prefix-$$ in $dir,
# register it for deletion via $cleanup, and set it as $TMPDIR.
tmpdir() {
	__tmpdir_prefix="${1:?}" __tmpdir_dir="${2:-"${TMPDIR:-.}"}"
	[ "${__tmpdir_tmpdir-}" ] && return
	readonly __tmpdir_tmpdir="$__tmpdir_dir/$__tmpdir_prefix-$$"
	[ -e "$__tmpdir_tmpdir" ] && abort "$__tmpdir_tmpdir: exists."
	catch=
	mkdir -m 0700 "$__tmpdir_tmpdir" || exit
	cleanup="rm -rf \"\$__tmpdir_tmpdir\"; ${cleanup-}"
	catch=x
	[ "${caught-}" ] && exit $((caught + 128))
	export TMPDIR="$__tmpdir_tmpdir"
}

# Print a message to STDERR.
warn() {
	: "${__warn_basename:="${0##*/}"}"
	# shellcheck disable=2059
	printf '%s: %s\n' "${__warn_basename:-"$0"}" "$*" >&2
}
