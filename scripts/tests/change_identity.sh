#!/bin/sh
# Test run_script.
# shellcheck disable=1091,2015

#
# Initialisation
#

set -Cefu
readonly script_dir="${0%/*}"
# shellcheck disable=1091
. "$script_dir/../utils.sh" || exit
init || exit
PATH="${TESTSDIR:-./build/tests}:$script_dir/../../build/tests:$PATH"
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
	user="$(id -un "$uid")" && [ "$user" ] ||
		abort "failed to name of user with ID $uid."
	gid="$(id -g "$user")" && [ "$gid" ] ||
		abort "failed to get ID of $user's primary group."

	checkok "effective: $uid:$gid; real: $uid:$gid." \
		change_identity "$user"
else
	checkerr 'Operation not permitted.' \
		change_identity "$LOGNAME"
fi

exit 0