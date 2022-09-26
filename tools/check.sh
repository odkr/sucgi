#!/bin/sh
# Run the test suite.
# shellcheck disable=2015

#
# Initialiation
#

set -u
script_dir="${0%/*}"
[ "$script_dir" = "$0" ] && script_dir=.
readonly script_dir
# shellcheck disable=1091
. "$script_dir/lib.sh" || exit
init || exit


#
# Globlas
#

# Number of child processes to spawn.
nchildren=4


#
# Options
#

OPTIND=1 OPTARG='' opt=''
while getopts sh opt; do
	case $opt in
	        (s) nchildren=1 ;;
		(h) exec cat <<-EOF ;;
			check - Run the test suite.

			Synopsis:
			    check
			    check -h

			Operands:
			    TEST  A test.

			Options:
			    -s    Run one test after the other,
			          rather than running them in parallel.
			    -h    Show this help screen.
			
			See README.md for details.
			EOF
		(*) exit 8
	esac
done
shift $((OPTIND - 1))
unset opt

if [ $# -eq 0 ]
then
	echo 'usage: check TEST [TEST [...]]' >&2
	exit 8
fi


#
# Main
#

# Some tests do not handle signals, so they will ignore a TERM.
cleanup="[ \"\${pids-}\" ] && kill -9 \$pids; ${cleanup-}"

errors=0 pids=
while [ $# -gt 0 ]; do
	name="$(basename "$1")" && [ "$name" ] ||
		abort "failed to get basename of $1."

	"$1" >/dev/null 2>&1 &
	pids="$pids $!"
	eval "test_$!"='$name'
	shift

	if [ $(($# % nchildren)) -eq 0 ]; then
		for pid in $pids; do
			eval name="\"\$test_$pid\""
			if ! wait "$pid"; then
				# shellcheck disable=2154
				warn "$red$bold$name$reset$red failed!$reset"
				errors=$((errors + 1))
			fi
		done
		pids=
	fi
done

# shellcheck disable=2154
[ "$errors" -eq 0 ] && warn "${green}all tests passed.$reset"

exit "$errors"
