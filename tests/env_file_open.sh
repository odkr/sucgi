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

jail="$TMPDIR/jail"
mkdir "$jail"


#
# Errors 
#


#
# Jail is the empty string.
#

checkerr "*jail != '\\\0'" \
	var="$jail/file" env_file_open "" var f


#
# Variable is the empty string.
#

checkerr "*varname != '\\\0'" \
	var="$jail/file" env_file_open "$jail" "" f


#
# Path to jail is longer than STR_MAX.
#

path_max="$(getconf PATH_MAX "$jail")" && [ "$path_max" ] || path_max=-1
name_max="$(getconf NAME_MAX "$jail")" && [ "$name_max" ] || name_max=-1

if [ "$path_max" -gt -1 ] && [ "$path_max" -lt 2048 ]
	then str_max="$path_max"
	else str_max=2048
fi

path_seg="$(printf '%*s\n' "$name_max" .d | tr ' ' x)"
while true
do
	if [ "${long_path-}" ]
		then long_path="$long_path/$path_seg"
		else long_path="$jail"
	fi
	len=${#long_path}
	[ $((len + name_max + 2)) -gt "$str_max" ] && break
done

huge_path="$long_path/$(printf '%*s\n' $((str_max - len - 2)) .d | tr ' ' x)"
long_path="$long_path/$(printf '%*s\n' $((str_max - len - 2)) .f | tr ' ' x)"

huge_path="$huge_path/file"
unset path_seq len

[ "${#long_path}" -eq $((str_max - 1)) ] ||
	err "\$long_path is ${#long_path} bytes long."

checkerr 'strnlen(jail, STR_MAX) < STR_MAX' \
	var="$huge_path/foo" env_file_open "$huge_path" var f


#
# Path to jail is not canonical.
#

outside_to_jail="$TMPDIR/symlink"
ln -s "$jail" "$outside_to_jail"

checkerr 'strncmp(jail, realpath(jail, NULL), STR_MAX) == 0' \
	var="$jail/foo" env_file_open "$outside_to_jail" var f

inside_to_root="$jail/symlink"
ln -s "$jail" "$inside_to_root"

checkerr 'strncmp(jail, realpath(jail, NULL), STR_MAX) == 0' \
	var="$jail/foo" env_file_open "$inside_to_root" var f

# Environment variable is unset or empty.
# shellcheck disable=2016
checkerr '$var: unset or empty' \
	env_file_open "$jail" var f

# shellcheck disable=2016
checkerr '$var: unset or empty' \
	var= env_file_open "$jail" var f


#
# Value of environment variable is too long.
#

(
	dir="${huge_path%/file}"
	mkdir -p "$dir"
	cd -P "$dir"
	touch file
)

# shellcheck disable=2016
checkerr '$var: path too long' \
	var="$huge_path" env_file_open "$jail" var f


#
# Path to file is too long after having been resolved.
#

shortened="$(
	cd -P "$jail"
	printf %s/ "$jail"
	
	IFS=/
	for seg in ${huge_path##"$jail"/}
	do
		if [ -d "$seg" ]
		then
			cd "$seg"
			ln -s "$seg" ../d
			printf d/
		else
			ln -s "$seg" f
			printf 'f\n'	
		fi
	done
)"

# This test raises different errors on different systems. 
checkerr 'too long' \
	var="$shortened" env_file_open "$jail" var f


#
# Path points to outside of jail.
#

outside="$TMPDIR/outside"
touch "$outside"

checkerr "\$var: not in $jail" \
	var="$outside" env_file_open "$jail" var f


#
# Resolved path points to outside of jail (dots).
#

checkerr "\$var: not in $jail" \
	var="$jail/../outside" env_file_open "$jail" var f


#
# Resolved path points to outside of jail (symlink).
#

ln -s "$outside" "$jail/outside"

checkerr "\$var: not in $jail" \
	var="$jail/outside" env_file_open "$jail" var f


#
# File is of the wrong type.
#

file="$jail/file"
touch "$file"

# shellcheck disable=2016
checkerr '$var: Not a directory' \
	var="$file" env_file_open "$jail" var d


#
# Non-errors
#


#
# Simple test.
#

echo $$ >>"$file"
checkok $$ \
	var="$file" env_file_open "$jail" var f


#
# Long filename.
#

echo $$ >>"$long_path"
checkok $$ \
	var="$long_path" env_file_open "$jail" var f


#
# Resolved path is inside jail.
#

ln -fs "$file" "$outside"
checkok $$ \
	var="$outside" env_file_open "$jail" var f



#
# All good.
#

# shellcheck disable=2154
warn "${green}success.$reset"
