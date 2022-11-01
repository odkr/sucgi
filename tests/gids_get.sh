#!/bin/sh
#
# Test gids_get_list.
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


#
# Main
#

ents |
while IFS=: read -r uid logname
do
	cmd="gids_get $logname"

	warn "checking ${bld-}$cmd${rst-} ..."

	set -- $($cmd)

	[ $# -gt 0 ] ||
		err "${bld-}$cmd${rst_r-} found no groups."

	groups="$(id -G "$logname")"
	for gid
	do
		inlist -eq "$gid" $groups && continue
		err "${bld-}$cmd${rst_r-} found wrong GID${bld-}$gid${rst_r-}."
	done
done
