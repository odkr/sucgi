#!/bin/sh
# Check if fail prints a message to STDERR that contains "foo"
# and exits with a non-zero status.
# shellcheck disable=1091

#
# Initialisation
#

set -Cefu
readonly script_dir="${0%/*}"
# shellcheck disable=1091
. "$script_dir/../utils.sh" || exit
init || exit
PATH="${TESTSDIR:-./build/tests}:$PATH"

tmpdir chk


#
# Main
#


fifo="$TMPDIR/fifo"
mkfifo -m 0700 "$fifo"

fail >"$fifo" 2>&1 & pid=$!

# Fail if the error message does not contain "foo".
grep -q foo <"$fifo" || exit 1

# Fail if fail did not exit with a non-zero status.
wait $pid && exit 2

# All good.
exit 0
