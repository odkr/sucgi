#!/bin/sh
# Test run_script.
# shellcheck disable=1091

#
# Prelude
#

set -Cefu

dir="$(dirname "$0")" && [ "$dir" ]
cd -P "$dir" || exit
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


#
# Tests
#

./run_script foo >"$fifo" 2>&1 & pid="$!"
grep -Fq 'foo: no filename suffix.' <"$fifo" ||
	abort 'wrong error message for missing filename suffix.'
wait "$pid" &&
	abort 'no error for missing filename suffix.'

./run_script foo.sh "=sh .pl=perl" >"$fifo" 2>&1 & pid="$!"
grep -Fq 'script type 1: no filename suffix.' <"$fifo" ||
	abort 'wrong error message for missing script type.'
wait "$pid" &&
	abort 'no error for missing script type.'

./run_script foo.sh ".pl=perl sh=sh" >"$fifo" 2>&1 & pid="$!"
grep -Fq 'script type 2: weird filename suffix.' <"$fifo" ||
	abort 'wrong error message for missing leading dot.'
wait "$pid" &&
	abort 'no error for missing leading dot.'

./run_script foo.sh ".pl=perl .py=python .=sh" >"$fifo" 2>&1 & pid="$!"
grep -Fq 'script type 3: weird filename suffix.' <"$fifo" ||
	abort 'wrong error message for dot-only filename suffix.'
wait "$pid" &&
	abort 'no error for dot-only filename suffix.'

./run_script foo.sh ".py=python .sh=" >"$fifo" 2>&1 & pid="$!"
grep -Fq 'script type 2: no interpreter given.' <"$fifo" ||
	abort 'wrong error message for missing interpreter.'
wait "$pid" &&
	abort 'no error for missing interpreter.'

./run_script foo.nil '.sh=sh' >"$fifo" 2>&1 & pid="$!"
grep -Fq 'filename suffix .nil: no interpreter registered.' <"$fifo" ||
	abort 'wrong error message for undefined interpreter.'
wait "$pid" &&
	abort 'no error for undefined interpreter.'

./run_script foo.sh '.sh=/::no such file!!' >/dev/null 2>&1 &&
	abort 'no error for non-existing interpreter.'

./run_script script.sh '.sh=sh' >"$fifo" 2>&1 & pid="$!"
grep -Fq 'This is a test script for main.sh and run_script.sh.' <"$fifo" ||
	abort "wrong script.sh output."
wait "$pid" ||
	abort "script.sh exited with status $?."

exit 0
