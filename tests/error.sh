#!/bin/sh
# Check whether error prints a message to STDERR that contains "foo" and
# then exits with a non-zero status.
# shellcheck disable=1091

#
# Initialisation
#

set -Cefu
script_dir="${0%/*}"
[ "$script_dir" = "$0" ] && script_dir=.
readonly script_dir
# shellcheck disable=1091
. "$script_dir/../tools/lib.sh" || exit
init || exit
tmpdir chk


#
# Main
#

for message in foo bar baz
do
	checkerr "$message" error "$message"
done
