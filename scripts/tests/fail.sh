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
PATH="${TESTSDIR:-./build/tests}:$script_dir/../../build/tests:$PATH"

tmpdir chk


#
# Main
#

checkerr foo fail
