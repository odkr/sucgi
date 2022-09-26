#!/bin/sh
# Test if env_get_fname only returns safe filenames.
# shellcheck disable=1091,2015

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
# Prelude
#

str_max="$(getconf PATH_MAX .)" && [ "$str_max" ] && 
	[ "$str_max" -ge 4096 ] || str_max=4096
long_str="$(printf '%*s\n' "$((str_max + 1))" | tr ' ' x)" && 
	[ "$long_str" ] || abort "failed to generate a long string."

if path_max="$(getconf PATH_MAX .)" && [ "${path_max:-"-1"}" -gt -1 ]
then
	long_path="$pwd/$(printf '%*s\n' $((path_max + 1)) | tr ' ' x)" &&
		[ "$long_path" != "$pwd/" ] ||
			abort "failed to generate a long path."
fi

if name_max="$(getconf NAME_MAX .)" && [ "${name_max:-"-1"}" -gt -1 ]
then
	long_name="$pwd/$(printf '%*s\n' $((name_max + 1)) | tr ' ' x)" &&
		[ "$long_name" != "$pwd/" ] ||
			abort "failed to generate a long name."
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
checkerr 'env_get_fname: $var: unset or empty.' \
	 env_get_fname var f

var=
checkerr 'env_get_fname: $var: unset or empty.' env_get_fname var f

var="$long_str" \
checkerr 'env_get_fname: $var: path too long.' env_get_fname var f

var="$long_path" \
checkerr 'env_get_fname: $var: path too long.' env_get_fname var f

var="$long_name" \
checkerr 'env_get_fname: $var: File name too long.' env_get_fname var f

var="$file" \
checkerr 'env_get_fname: $var: not of type d.' env_get_fname var d

var="$TMPDIR" \
checkerr 'env_get_fname: $var: not of type f.' env_get_fname var f

var="$file" \
checkok "uid=$uid" env_get_fname var f

var="$TMPDIR" \
checkok "uid=$uid" env_get_fname var d

