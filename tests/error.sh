#!/bin/sh
# Check if error prints a message to STDERR that contains "foo"
# and exits with a non-zero status.

set -Cefu

dir="$(dirname "$0")" && [ "$dir" ]
cd -P "$dir" || exit
# shellcheck disable=1091
. ./utils.sh || exit

trap cleanup EXIT

CAUGHT=0
trap 'CAUGHT=1' HUP
trap 'CAUGHT=2' INT
trap 'CAUGHT=15' TERM

readonly TMP="${TMPDIR:-.}/.check-$$"
mkdir -m 0700 "$TMP" || exit
# shellcheck disable=2034
CLEANUP="[ \"${TMP-}\" ] && rm -rf \"\$TMP\""
export TMPDIR="$TMP"

trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

[ "$CAUGHT" -gt 0 ] && exit $((CAUGHT + 128))

fifo="$TMP/fifo"
mkfifo -m 0700 "$fifo"

./error >"$fifo" 2>&1 & pid=$!

# Fail if the error message does not contain "foo".
grep -q foo <"$fifo" || exit 1

# Fail if ./error did not exit with a non-zero status.
wait $pid && exit 2

# All good.
exit 0