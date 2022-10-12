#!/bin/sh
#
# Test if env_file_open only returns safe filenames.
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

str_max="$(getconf PATH_MAX .)" && [ "${str_max:-"-1"}" -gt -1 ] || 
	str_max=2048
long_str="$(printf '%*s\n' "$((str_max + 1))" x | tr ' ' x)" && 
	[ "$long_str" ] || err "failed to generate a long string."

if path_max="$(getconf PATH_MAX .)" && [ "${path_max:-"-1"}" -gt -1 ]
then
	long_path="$(printf '%*s\n' "$((path_max + 1))" x | tr ' ' x)" &&
		[ "$long_path" ] || err "failed to generate a long path."
fi

if name_max="$(getconf NAME_MAX .)" && [ "${name_max:-"-1"}" -gt -1 ]
then
	long_name="$(printf '%*s\n' "$((name_max + 1))" x | tr ' ' x)" &&
		[ "$long_name" ] || err "failed to generate a long name."
fi

uid="$(id -u)" && [ "$uid" ] ||
	err "failed to get process' effective UID."

jail="$TMPDIR/jail"
mkdir "$jail"

inside="$jail/file"
touch "$inside"

outside="$TMPDIR/file"
touch "$outside"

out_to_in="$TMPDIR/symlink"
ln -s "$inside" "$out_to_in"

in_to_out="$jail/symlink"
ln -s "$outside" "$in_to_out"


#
# Main
#

unset var
# shellcheck disable=2016
checkerr 'env_file_open: $var: unset or empty' \
	 env_file_open / var

# shellcheck disable=2016
checkerr 'env_file_open: $var: unset or empty' \
	var= env_file_open / var

# shellcheck disable=2016
checkerr 'env_file_open: $var: path too long' \
	var="/$long_str" env_file_open / var

# shellcheck disable=2016
checkerr 'env_file_open: $var: path too long' \
	var="/$long_path" env_file_open / var

# shellcheck disable=2016
checkerr 'env_file_open: $var: File name too long' \
	var="/$long_name" env_file_open / var

checkerr "env_file_open: \$var: not in $jail" \
	var="$outside" env_file_open "$jail" var

checkerr "env_file_open: \$var: not in $jail" \
	var="$in_to_out" env_file_open "$jail" var

checkok "uid=$uid" \
	var="$inside" env_file_open / var

checkok "uid=$uid" \
	var="$out_to_in" env_file_open "$jail" var

# shellcheck disable=2154
warn "${green}success.$reset"
