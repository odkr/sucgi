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
tmpdir chk

: "${LOGNAME:?}"

euid="$(id -u)" && [ "$euid" ] ||
	abort "failed to get process' effective UID."


#
# Test
#

if [ "$euid" -eq 0 ]
then
	uid="$(regularuid)" && [ "$uid" ] ||
		abort "failed to get non-root user ID of caller."
	# shellcheck disable=2154
	user="$(getlogname "$uid")" && [ "$user" ] ||
		abort "failed to get name of user with ID $bold$uid$reset$red."
	gid="$(id -g "$user")" && [ "$gid" ] ||
		abort "failed to get ID of $user's primary group."

	export PATH
	checkerr 'Operation not permitted.' \
		runas "$uid" "$gid" priv_drop "$user"

	checkok "euid=$uid egid=$gid ruid=$uid rgid=$gid" priv_drop "$user"
else
	checkerr 'Operation not permitted.' priv_drop "$LOGNAME"
fi
