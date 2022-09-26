#!/bin/sh
# Utility functions for the test suite and the tools.
# shellcheck disable=2015

# Print a message to STDERR and exit with a non-zero status.
abort() {
	warn "${red-}$*${reset-}"
	exit 8
}

# Exit the with $signo + 128 if $catch. Otherwise set $caught to $signo. 
catch() {
	signo="${1:?}"
	sig="$(kill -l "$signo")" && [ "$sig" ] || sig="signal $signo"
	warn "caught $sig."
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
	kill -15 $(jobs -p) -$$ >/dev/null 2>&1
	[ "${cleanup-}" ] && eval "$cleanup"
	[ "${reset-}" ] && printf %s "$reset" >&2
	exit "$status"
}

# Register signals, set PATH, global variables, a umask, and enable colours.
init() {
	trap cleanup EXIT;
	trap 'catch 1' HUP;
	trap 'catch 2' INT;
	trap 'catch 15' TERM;
	catch=x

	umask 077

	reset='' bold='' green='' red='' yellow=''
	if [ -t 2 ]
	then
		case ${TERM-} in (*color*)
			# shellcheck disable=2034
			if reset="$(tput sgr0 2>/dev/null)" && [ "$reset" ]
			then
				bold="$(tput bold 2>/dev/null)" || : 
				red="$(tput setaf 1 2>/dev/null)" || :
				green="$(tput setaf 2 2>/dev/null)" || :
				yellow="$(tput setaf 3 2>/dev/null)" || :
			fi
		esac
	fi
	# shellcheck disable=2034
	readonly reset bold green red

	readonly lf="
"

	# shellcheck disable=2154
	PATH="$script_dir:$script_dir/../tools:$PATH"
}

# Check if a line on STDIN contains $string.
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
	warn "$red'$bold$string$reset$red' not in:$lf$bold${file%"$lf"}$reset"
	return 1
)

# Get the UID of the user who invoked the script,
# even if the script has been invoked via su or sudo.
regularuid() (
        pivot="$$"
        fifo="${TMPDIR:?}/ps.fifo"
        mkfifo "$fifo"

        while true
        do
                ps -Ao 'pid= ppid= user=' | 
		sort -r >"$fifo" & sort=$!
                while read -r pid ppid user
                do
                        [ "$pid" -eq "$pivot" ] || continue

                        uid="$(id -u "$user")" && [ "$uid" ] || continue
                        if [ "$uid" -ne 0 ]
                        then
                                echo "$uid"
                                return 0
                        elif [ "$ppid" -gt 1 ]
                        then
                                pivot="$ppid"
                        else
                                return 1
                        fi
                done <"$fifo"
                wait "$sort"
        done
)

# Create a directory with the filename $prefix-$$ in $dir,
# register it for deletion via $cleanup, and set it as $TMPDIR.
tmpdir() {
	[ "${__tmpdir_tmpdir-}" ] && return
	__tmpdir_prefix="${1:?}" __tmpdir_dir="${2:-"${TMPDIR:-/tmp}"}"
	__tmpdir_real="$(cd -P "$__tmpdir_dir" && pwd)" &&
		[ "$__tmpdir_real" ] && [ -d "$__tmpdir_real" ] ||
			abort "failed to get real path of $__tmpdir_dir."
	readonly __tmpdir_tmpdir="$__tmpdir_real/$__tmpdir_prefix-$$"
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
