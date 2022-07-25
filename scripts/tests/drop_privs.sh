#!/bin/sh
# Test run_script.
# shellcheck disable=1091,2015

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
: "${LOGNAME:?}"


#
# Tests
#

checkerr 'Operation not permitted.' drop_privs "$LOGNAME"

exit 0
