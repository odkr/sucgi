#!/bin/sh
#
# Test if filesopen only opens secure files.
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

file="$TMPDIR/file"
echo $$ >"$file"
symlink="$TMPDIR/symlink"
ln -s "$TMPDIR" "$symlink"


#
# Main
#

check -s0 -o$$					filesopen "$file" f
check -s1 -e'Not a directory'			filesopen "$file" d
check -s1 -e'Too many levels of symbolic links'	filesopen "$symlink" d
check -s1 -e'No such file or directory'		filesopen '<no file!>' f

warn -g "all tests passed."
