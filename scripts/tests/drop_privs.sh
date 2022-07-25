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


#
# Non-root test
#

checkerr 'Operation not permitted.' drop_privs "$LOGNAME"


#
# Interlude
#

uid="$(id -u)" && [ "$uid" ] ||
	abort "failed to get process' effective UID."

# The checks below only work if drop_privs.sh is invoked as root.
[ "$uid" -ne 0 ] && exit

uid="$(regularuid)" && [ "$uid" ] ||
	abort "failed to get non-root user ID of caller."
user="$(id -un "$uid")" && [ "$user" ] ||
	abort "failed to name of user with ID $uid."
gid="$(id -g "$user")" && [ "$gid" ] ||
	abort "failed to get ID of $user's primary group."

#
# Root test
#

checkok "effective: $uid:$gid; real: $uid:$gid." drop_privs "$user"

exit 0
