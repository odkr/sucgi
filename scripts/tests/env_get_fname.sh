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


#
# Prelude
#

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

uid="$(id -u)" && [ "$uid" ] ||
	abort "failed to get process' effective UID."

file="$TMPDIR/file"
touch "$file"
symlink="$TMPDIR/symlink"
ln -s "$TMPDIR" "$symlink"


#
# Main
#

unset var
checkerr "env_get_fname: env_get_fname var f returned 10." \
	env_get_fname var f

var='' \
	checkerr "env_get_fname: env_get_fname var f returned 8." \
		env_get_fname var f

var="$long_str" \
	checkerr "env_get_fname: env_get_fname var f returned 6." \
		env_get_fname var f

var="$long_path" \
	checkerr "env_get_fname: env_get_fname var f returned 6." \
		env_get_fname var f

var="$long_name" \
	checkerr "env_get_fname: env_get_fname var f returned 3." \
		env_get_fname var f

var="$symlink" \
	checkerr "env_get_fname: env_get_fname var f returned 7." \
		env_get_fname var f

var="$file" \
	checkerr "env_get_fname: env_get_fname var d returned 4." \
		env_get_fname var d

var="$TMPDIR" \
	checkerr "env_get_fname: env_get_fname var f returned 4." \
		env_get_fname var f

var="$file" \
	checkok "UID $uid" \
		env_get_fname var f


exit 0
