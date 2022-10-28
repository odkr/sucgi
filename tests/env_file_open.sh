#!/bin/sh
#
# Test if env_file_open only returns safe filenames.
#
# Copyright 2022 Odin Kroeger
#
# This file is part of suCGI.
#
# suCGI is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# suCGI is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with suCGI. If not, see <https://www.gnu.org/licenses>.
#
# shellcheck disable=1091,2015

#
# Initialisation
#

set -Cefu
script_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(cd -P "$script_dir/.." && pwd)"
tools_dir="$src_dir/tools"
readonly script_dir src_dir tools_dir
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit
tmpdir chk


#
# Prelude
#

# Create the jail.
readonly jail="$TMPDIR/jail"
mkdir "$jail"

# Get the system's maximum path length in $jail.
path_max="$(getconf PATH_MAX "$jail")"
[ "$path_max" -lt 0 ] && path_max=255

# Create a path that is as long as the system and suCGI permit.
if [ "$path_max" -gt 1024 ]
	then max=1024
	else max="$path_max"
fi
long_path="$(mklongpath "$jail" "$((max - 1))")"
mkdir -p "$(dirname "$long_path")"
echo $$ >"$long_path"

# Create a path that is longer than the system permits.
huge_path="$(mklongpath "$jail" "$path_max")"
# shellcheck disable=2016
traverse "$jail" "$huge_path" 'mkdir "$fname"' 'echo $$ >"$fname"'

# Create a path that is longer than suCGI permits.
huge_str="$(mklongpath "$jail" 1024)"
# shellcheck disable=2016
traverse "$jail" "$huge_str" 'mkdir "$fname"' 'echo $$ >"$fname"'

# Create a shortcut to the path that is longer than the system permits.
# shellcheck disable=2016
huge_path_link="$jail/$(traverse "$jail" "$huge_path" \
	'ln -s "$fname" d && printf d/' \
	'ln -s "$fname" f && printf f\\n')"

# Create a shortcut to the path that is longer than suCGI permits.
# shellcheck disable=2016
huge_str_link="$jail/$(traverse "$jail" "$huge_str" \
	'ln -s "$fname" d && printf d/' \
	'ln -s "$fname" f && printf f\\n')"

# Create a file inside the jail.
inside="$jail/inside"
echo $$ >"$inside"

# Create a file outside the jail.
outside="$TMPDIR/outside"
touch "$outside"

# Create a link to the outside.
out_link="$jail/outside"
ln -s "$outside" "$out_link"

# Create a link to the inside
in_link="$TMPDIR/to-inside"
ln -fs "$inside" "$in_link"


#
# Main
#

# Jail directory is the empty string.
checkerr "*jail != '\\0'" \
	var="$jail/file" env_file_open "" var f

# Environment variable is the empty string.
checkerr "*varname != '\\0'" \
	var="$jail/file" env_file_open "$jail" "" f

# Path to jail directory is longer than MAX_STR.
checkerr 'strnlen(jail, MAX_STR) < MAX_STR' \
	var="$jail" env_file_open "$huge_str" var f

# Jail directory does not exist.
checkerr 'access(jail, F_OK) == 0' \
	var="/lib/<no file!>/foo" env_file_open '/lib/<no file!>' var f

# Path to jail directory is not canonical.
checkerr 'strncmp(jail, realpath(jail, NULL), MAX_STR) == 0' \
	var="$jail/foo" env_file_open "$in_link" var f

# shellcheck disable=2016
checkerr '$var unset or empty' \
	env_file_open "$jail" var f

# shellcheck disable=2016
checkerr '$var unset or empty' \
	var= env_file_open "$jail" var f

# Path to file is too long (system).
checkerr 'too long' \
	var="$huge_str" env_file_open "$jail" var f

# Value of environment variable is too long (suCGI).
checkerr 'path too long' \
	var="$huge_str" env_file_open "$jail" var f

# Path to file is too long after having been resolved (system).
checkerr 'too long' \
 	var="$huge_path_link" env_file_open "$jail" var f

# Path to file is too long after having been resolved (suCGI).
checkerr 'too long' \
 	var="$huge_str_link" env_file_open "$jail" var f

# Path points to outside of jail directory.
checkerr "file $outside not within jail" \
	var="$outside" env_file_open "$jail" var f

# Resolved path points to outside of jail directory (dots).
checkerr "file $outside not within jail" \
	var="$jail/../outside" env_file_open "$jail" var f

# Resolved path points to outside of jail directory (symlink).
checkerr "file $outside not within jail" \
	var="$out_link" env_file_open "$jail" var f

# File is of the wrong type.
checkerr "open $inside: Not a directory" \
	var="$inside" env_file_open "$jail" var d

# File does not exist.
checkerr "realpath /lib/<no such file!>: No such file or directory" \
	var="/lib/<no such file!>" env_file_open "$jail" var d

# Simple test.
checkok $$ \
	var="$inside" env_file_open "$jail" var f

# Long filename.
checkok $$ \
	var="$long_path" env_file_open "$jail" var f

# Resolved path is inside of jail.
checkok $$ \
	var="$in_link" env_file_open "$jail" var f

# All good.
warn -g "all tests passed."
