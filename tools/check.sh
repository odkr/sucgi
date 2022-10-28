#!/bin/sh
# Run the test suite.
# shellcheck disable=2015

#
# Initialiation
#

set -Cefu
tools_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(dirname "$tools_dir")"
# shellcheck disable=2034
readonly tools_dir src_dir
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit


#
# Globlas
#

# Number of child processes to spawn.
nchildren=4


#
# Options
#

quiet=
OPTIND=1 OPTARG='' opt=''
# shellcheck disable=2034
while getopts qsh opt; do
	case $opt in
	        (s) nchildren=1 ;;
		(q) quiet=x ;;
		(h) exec cat <<EOF ;;
check - run the test suite

Usage:    check [-s] [-q] TEST [...]
          check -h

Operands:
    TEST  A test.

Options:
    -s    Do not run tests in parallel.
    -q    Suppress all output.
    -h    Show this help screen.

See README.md for details.
EOF
		(*) exit 1
	esac
done
shift $((OPTIND - 1))
unset opt

if [ $# -eq 0 ]
then
	echo 'usage: check [-s] [-q] TEST [...]' >&2
	exit 1
fi


#
# Main
#

# Some tests do not handle signals, so they would ignore a TERM.
cleanup="[ \"\${pids-}\" ] && kill -9 \$pids >/dev/null 2>&1; ${cleanup-}"

errc=0 pids=
while [ $# -gt 0 ]; do
	name="$(basename "$1")"

	"$1" >/dev/null 2>&1 &
	pids="$pids $!"
	eval "test_$!"='$name'
	shift

	if [ $(($# % nchildren)) -eq 0 ]; then
		for pid in $pids; do
			eval name="\"\${test_$pid}\""
			if ! wait "$pid"; then
				# shellcheck disable=2154
				warn -rq "${bld-}$name${rst_r-} failed!"
				errc=$((errc + 1))
			fi
		done
		pids=
	fi
done

# shellcheck disable=2154
[ $errc -eq 0 ] && warn -gq "all tests passed."

exit $errc
