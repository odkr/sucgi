#!/bin/sh
# Test if file_safe_open only opens safe files.
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
# Prelude
#

file="$TMPDIR/file"
touch "$file"
symlink="$TMPDIR/symlink"
ln -s "$TMPDIR" "$symlink"
ln -s "$TMPDIR/loop-a" "$TMPDIR/loop-b"
ln -s "$TMPDIR/loop-b" "$TMPDIR/loop-a"
loop="$TMPDIR/loop-a/loop-b/foo"


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
	long_name="$PWD/${long_name}/x"
fi


#
# Main
#

path_check_len "$long_name" &&
	abort "path_check_len accepts overly long name."

path_check_len "$long_path" &&
	abort "path_check_len accepts overly long path."

path_check_len "$loop" &&
	abort "path_check_len accepts symlink loop."

# shellcheck disable=2154
path_check_len / ||
	abort "path_check_len refuses $bold/$reset."

# shellcheck disable=2154
path_check_len "$file" ||
	abort "path_check_len refuses $bold$file$reset."

# shellcheck disable=2154
path_check_len "$symlink" ||
	abort "path_check_len refuses $bold$symlink$reset."

exit 0
