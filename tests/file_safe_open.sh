#!/bin/sh
#
# Test if file_safe_open only opens safe files.
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
readonly script_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
readonly src_dir="$(cd -P "$script_dir/.." && pwd)"
readonly tools_dir="$src_dir/tools"
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

checkok  $$                                  file_safe_open "$file" f
checkerr 'Not a directory'                   file_safe_open "$file" d
checkerr 'Too many levels of symbolic links' file_safe_open "$symlink" d
checkerr 'No such file or directory'         file_safe_open '/lib/<nofile!>' f

warn -g "all tests passed."
