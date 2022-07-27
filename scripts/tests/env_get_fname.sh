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
PATH="${TESTSDIR:-./build/tests}:$script_dir/../../build/tests:$PATH"

tmpdir chk
TMPDIR="$(realdir "$TMPDIR")" && [ "$TMPDIR" ] ||
	abort "failed to get canonical path of temporary directory."


#
# Prelude
#

touch "$TMPDIR/file"
ln -s "$TMPDIR" "$TMPDIR/symlink"

str_max="$(getconf PATH_MAX .)" && [ "$str_max" ] && 
	[ "$str_max" -ge 4096 ] || str_max=4096
long_str="$PWD/"
while [ "${#long_str}" -le "$str_max" ]
	do long_str="$long_str/x"
done
long_str="$long_str/x"

if	path_max="$(getconf PATH_MAX .)" &&
	[ "$path_max" ] &&
	[ "$path_max" -gt -1 ]
then
	long_path="$PWD/"
	while [ "${#long_path}" -le "$path_max" ]
		do long_path="$long_path/x"
	done
	long_path="$long_path/x"
fi

if	name_max="$(getconf NAME_MAX .)" &&
	[ "$name_max" ] &&
	[ "$name_max" -gt -1 ]
then
	long_name="x"
	while [ "${#long_name}" -le "$name_max" ]
		do long_name="${long_name}x"
	done
	long_name="$PWD/${long_name}x/x"
fi


#
# Main
#

unset var
env_get_fname var &&
	abort "env_get_fname accepted an undefined variable."

var='' env_get_fname var &&
	abort "env_get_fname accepted an empty variable."

var="$long_str" env_get_fname var &&
	abort "env_get_fname accepted an overly long string."

var="$long_path" env_get_fname var &&
	abort "env_get_fname accepted an overly long path."

var="$long_name" env_get_fname var &&
	abort "env_get_fname accepted an overly long filename."

var="$TMPDIR/symlink/file" env_get_fname var &&
	abort "env_get_fname does not refuse $TMPDIR/symlink/file."

var="$TMPDIR/file" env_get_fname var ||
	abort "env_get_fname refuses $TMPDIR/file."


exit 0
