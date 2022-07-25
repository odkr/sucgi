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

checkerr 'script type 1: no filename suffix.' \
	run_script foo.sh "=sh .pl=perl"

checkerr 'script type 2: weird filename suffix.' \
	run_script foo.sh ".pl=perl sh=sh"

checkerr 'script type 3: weird filename suffix.' \
	run_script foo.sh ".pl=perl .py=python .=sh"

checkerr 'script type 2: no interpreter given.' \
	run_script foo.sh ".py=python .sh="

checkerr 'filename suffix .nil: no interpreter registered.' \
	run_script foo.nil '.sh=sh'

checkerr 'script type 2: weird filename suffix.' \
	run_script foo.sh '.sh=/::no such file!!'

checkok 'This is a test script for main.sh and run_script.sh.' \
	run_script "$script_dir/scripts/script.sh" '.sh=sh'

exit 0
