#!/bin/sh
# Test priv_drop.
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
oldtmp="${TMPDIR-}"
tmpdir chk


#
# Prelude
#

export PATH

: "${LOGNAME:?}"

euid="$(id -u)" && [ "$euid" ] ||
	err "failed to get process' effective UID."


#
# Test
#

if [ "$euid" -eq 0 ]
then
	ruser="$(ls -ld "$script_dir" | cut -d ' ' -f4)"
	if ! [ "$ruser" ] || [ "$ruser" = root ]
	then
		ruser="$(regularuser)" && [ "$ruser" ] ||
			err "failed to get non-root user."
	fi
	
	ruid="$(id -u "$ruser")" && [ "$ruid" ] ||
		err "failed to get user ID of $ruser."
	rgid="$(id -g "$ruser")" && [ "$rgid" ] ||
		err "failed to get primary group ID of $ruser."

	checkerr 'Operation not permitted' \
		runas "$ruid" "$rgid" priv_drop "$ruser"

	checkerr 'could resume privileges' \
		priv_drop "$LOGNAME"

	checkok "euid=$ruid egid=$rgid ruid=$ruid rgid=$rgid" \
		priv_drop "$ruser"

	TMPDIR="$oldtmp" runas "$ruid" "$rgid" "$0"
else
	checkerr 'Operation not permitted' priv_drop "$LOGNAME"

	warn "${green}all tests passed.$reset"
fi
