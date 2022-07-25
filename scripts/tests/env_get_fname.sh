#!/bin/sh
# Test if env_get_fname only returns safe filenames.
# shellcheck disable=1091,2015

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
TMPDIR="$(realdir "$TMPDIR")" && [ "$TMPDIR" ] ||
	abort "failed to get canonical path of temporary directory."


#
# Main
#

touch "$TMPDIR/file"
ln -s "$TMPDIR" "$TMPDIR/symlink"

str_max="$(getconf PATH_MAX .)" && [ "$str_max" ] && 
	[ "$str_max" -ge 4096 ] || str_max=4096
long_str="$PWD"
while [ "${#long_str}" -lt "$str_max" ]
	do long_str="$long_str/$long_str"
done
long_str="$long_str/foo"

if	path_max="$(getconf PATH_MAX .)" &&
	[ "$path_max" ] &&
	[ "$path_max" -gt -1 ]
then
	long_path="$PWD"
	while [ "${#long_path}" -lt "$path_max" ]
		do long_path="$long_path/foo"
	done
	long_path="$long_path/foo"
fi

unset var
env_get_fname var &&
	abort "env_get_fname accepted an undefined variable."

var='' env_get_fname var &&
	abort "env_get_fname accepted an empty variable."

var="$long_str" env_get_fname var &&
	abort "env_get_fname accepted an overly long string."

var="$long_path" env_get_fname var &&
	abort "env_get_fname accepted an overly long path."

var="$TMPDIR/symlink/file" env_get_fname var &&
	abort "env_get_fname does not refuse $TMPDIR/symlink/file."

var="$TMPDIR/file" env_get_fname var ||
	abort "env_get_fname refuses $TMPDIR/file."


exit 0
