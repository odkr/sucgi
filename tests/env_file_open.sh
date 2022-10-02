#!/bin/sh
# Test if env_file_openat only returns safe filenames.
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
long_str="$(printf '%*s\n' "$((str_max + 1))" | tr ' ' x)" && 
	[ "$long_str" ] || abort "failed to generate a long string."

if path_max="$(getconf PATH_MAX .)" && [ "${path_max:-"-1"}" -gt -1 ]
then
	long_path="$(printf '%*s\n' $((path_max + 1)) | tr ' ' x)" &&
		[ "$long_path" ] || abort "failed to generate a long path."
fi

if name_max="$(getconf NAME_MAX .)" && [ "${name_max:-"-1"}" -gt -1 ]
then
	long_name="$(printf '%*s\n' $((name_max + 1)) | tr ' ' x)" &&
		[ "$long_name" ] || abort "failed to generate a long name."
fi

uid="$(id -u)" && [ "$uid" ] ||
	abort "failed to get process' effective UID."

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
checkerr 'env_file_openat: $var: unset or empty.' \
	 env_file_openat / var

var=
checkerr 'env_file_openat: $var: unset or empty.' env_file_openat / var

var="/$long_str" \
checkerr 'env_file_openat: $var: path too long.' env_file_openat / var

var="/$long_path" \
checkerr 'env_file_openat: $var: path too long.' env_file_openat / var

var="/$long_name" \
checkerr 'env_file_openat: $var: File name too long.' env_file_openat / var

var="$outside" \
checkerr "env_file_openat: \$var: not in $jail." env_file_openat "$jail" var

var="$inside" \
checkok "uid=$uid" env_file_openat / var

var="$in_to_out" \
checkerr "env_file_openat: \$var: not in $jail." env_file_openat "$jail" var

var="$out_to_in" \
checkok "uid=$uid" env_file_openat "$jail" var

