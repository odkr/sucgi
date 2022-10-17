#!/bin/sh
# Test if file_safe_stat only stats safe files.
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
# Main
#

euid="$(id -u)" && [ "$euid" ] ||
	err "failed to get process' effective UID."

file="$TMPDIR/file"
touch "$file"
symlink="$TMPDIR/symlink"
ln -s "$TMPDIR" "$symlink"


checkok "uid=$euid" \
	file_safe_stat "$file"

checkerr "file_safe_stat: open $symlink: Too many levels of symbolic links" \
	file_safe_stat "$symlink"

# shellcheck disable=2154
warn "${green}success.$reset"
