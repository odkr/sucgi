#!/bin/sh
# Test gids_get_list.
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

: "${LOGNAME:?}"

egid="$(id -g)" && [ "$egid" ] ||
	err "failed to get process' effective GID."


#
# Main
#

checkok "$egid" gids_get_list "$LOGNAME" "$egid"

checkok 0 gids_get_list "$LOGNAME" 0

checkok 500 gids_get_list "$LOGNAME" 500

