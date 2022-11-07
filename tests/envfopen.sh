#!/bin/sh
#
# Test if envfopen only returns safe filenames.
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

# Load the build configuration
eval "$(main -C | grep -E '^PATH_MAX_LEN=')"

# Create the jail.
readonly jail="$TMPDIR/jail"
mkdir "$jail"

# Get the system's maximum path length in $jail.
path_max="$(getconf PATH_MAX "$jail")"
[ "$path_max" -lt 0 ] && path_max=255

# Create a path that is as long as the system and suCGI permit.
if [ "$path_max" -lt 0 ]
	then max="$PATH_MAX_LEN"
	else max="$path_max"
fi
long_path="$(mklongpath "$jail" "$((max - 1))")"
mkdir -p "$(dirname "$long_path")"
echo $$ >"$long_path"

# Create a path that is longer than the system permits.
huge_path="$(mklongpath "$jail" "$path_max")"
# shellcheck disable=2016
dirwalk "$jail" "$huge_path" 'mkdir "$fname"' 'echo $$ >"$fname"'

# Create a path that is longer than suCGI permits.
huge_str="$(mklongpath "$jail" 4096)"
# shellcheck disable=2016
dirwalk "$jail" "$huge_str" 'mkdir "$fname"' 'echo $$ >"$fname"'

# Create a shortcut to the path that is longer than the system permits.
# shellcheck disable=2016
huge_path_link="$jail/$(dirwalk "$jail" "$huge_path" \
	'ln -s "$fname" p.d && printf p.d/' \
	'ln -s "$fname" p.f && printf p.f\\n')"

# Create a shortcut to the path that is longer than suCGI permits.
# shellcheck disable=2016
huge_str_link="$jail/$(dirwalk "$jail" "$huge_str" \
	'ln -s "$fname" s.d && printf s.d/' \
	'ln -s "$fname" s.f && printf s.f\\n')"

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
check -s134 -e'*jail' \
	var="$jail/file" envfopen "" var f

# Environment variable is the empty string.
check -s134 -e'*var' \
	var="$jail/file" envfopen "$jail" "" f

# Path to jail directory is longer than PATH_MAX_LEN.
check -s134 -e'strnlen(jail, PATH_MAX_LEN) < PATH_MAX_LEN' \
	var="$jail" envfopen "$huge_str" var f

# Jail directory does not exist.
check -s134 -e'access(jail, F_OK) == 0' \
	var="<no file!>/foo" envfopen '<no file!>' var f

# Path to jail directory is not canonical.
check -s134 -e'strncmp(jail, realpath(jail, NULL), PATH_MAX_LEN) == 0' \
	var="$jail/foo" envfopen "$in_link" var f

# shellcheck disable=2016
check -s1 -e'$var unset or empty' \
	envfopen "$jail" var f

# shellcheck disable=2016
check -s1 -e'$var unset or empty' \
	var= envfopen "$jail" var f

# Path to file is too long (system).
check -s1 -e'too long' \
	var="$huge_str" envfopen "$jail" var f

# Value of environment variable is too long (suCGI).
check -s1 -e'path too long' \
	var="$huge_str" envfopen "$jail" var f

# Path to file is too long after having been resolved (system).
check -s1 -e'too long' \
 	var="$huge_path_link" envfopen "$jail" var f

# Path to file is too long after having been resolved (suCGI).
check -s1 -e'too long' \
 	var="$huge_str_link" envfopen "$jail" var f

# Path points to outside of jail directory.
check -s1 -e"file $outside not within jail" \
	var="$outside" envfopen "$jail" var f

# Resolved path points to outside of jail directory (dots).
check -s1 -e"file $outside not within jail" \
	var="$jail/../outside" envfopen "$jail" var f

# Resolved path points to outside of jail directory (symlink).
check -s1 -e"file $outside not within jail" \
	var="$out_link" envfopen "$jail" var f

# File is of the wrong type.
check -s1 -e"open $inside: Not a directory" \
	var="$inside" envfopen "$jail" var d

# File does not exist.
check -s1 -e"realpath <no file!>: No such file or directory" \
	var="<no file!>" envfopen "$jail" var d

# Simple test.
check -s0 -o$$ var="$inside" envfopen "$jail" var f

# Long filename.
check -s0 -o$$ var="$long_path" envfopen "$jail" var f

# Resolved path is inside of jail.
check -s0 -o$$ var="$in_link" envfopen "$jail" var f

# All good.
warn -g "all tests passed."
