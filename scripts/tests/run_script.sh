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


#
# Tests
#

checkerr 'foo: no filename suffix.' \
	run_script foo

checkerr 'script handler 2: path to interpreter is the empty string.' \
	run_script foo.sh .py=python .sh=

checkerr 'filename suffix .nil: no interpreter registered.' \
	run_script foo.nil .sh=sh

checkerr 'foo.sh: exec /::no such file!!: No such file or directory.' \
	run_script foo.sh '.sh=/::no such file!!'

checkok 'This is a test script for main.sh and run_script.sh.' \
	run_script "$script_dir/scripts/script.sh" .sh=sh

exit 0
