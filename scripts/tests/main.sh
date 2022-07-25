#!/bin/sh
# Test sucgi via main.
#shellcheck disable=1091,2015,2016,2034

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
TMPDIR="$(realdir "$TMPDIR")" && [ "$TMPDIR" ] ||
	abort "failed to get canonical path of temporary directory."


#
# Prelude
#

: "${HOME:?}"

str_max="$(getconf PATH_MAX .)" && [ "$str_max" ] && 
	[ "$str_max" -ge 4096 ] || str_max=4096
long_str="$HOME"
while [ "${#long_str}" -lt "$str_max" ]
	do long_str="$long_str/$long_str"
done
long_str="$long_str/foo"

if	path_max="$(getconf PATH_MAX .)" &&
	[ "$path_max" ] &&
	[ "$path_max" -gt -1 ]
then
	long_path="$HOME"
	while [ "${#long_path}" -lt "$path_max" ]
		do long_path="$long_path/foo"
	done
	long_path="$long_path/foo"
fi

true="$(command -v true >/dev/null 2>&1 || :)" || :
: "${true:=/usr/bin/true}"
"$true" || abort "true: exited with status $?."


#
# Non-root checks
#

checkerr 'DOCUMENT_ROOT: not set.' main

DOCUMENT_ROOT='' \
	checkerr 'DOCUMENT_ROOT: is the empty string.' main

DOCUMENT_ROOT="$long_str" \
	checkerr 'environment variable too long.' main

if [ "$long_path" ]
then
	DOCUMENT_ROOT="$long_path" \
	checkerr 'too long.' main
fi

DOCUMENT_ROOT='/::no-such-file!!' \
	checkerr '$DOCUMENT_ROOT: No such file or directory.' main

DOCUMENT_ROOT="$0" \
	checkerr '$DOCUMENT_ROOT: not a directory.' main

DOCUMENT_ROOT=. \
	checkerr '$DOCUMENT_ROOT: does not match /*.' main

DOCUMENT_ROOT="$HOME" \
	checkerr 'PATH_TRANSLATED: not set.' main

DOCUMENT_ROOT="$HOME" PATH_TRANSLATED='' \
	checkerr 'PATH_TRANSLATED: is the empty string.' main

DOCUMENT_ROOT=/ PATH_TRANSLATED="$long_str" \
	checkerr 'environment variable too long.' main

[ "$long_path" ] &&
	DOCUMENT_ROOT=/ PATH_TRANSLATED="$long_path" \
		checkerr 'too long.' main

DOCUMENT_ROOT=/ PATH_TRANSLATED='/::no-such-file!!' \
	checkerr '$PATH_TRANSLATED: No such file or directory.' main

DOCUMENT_ROOT="$HOME" PATH_TRANSLATED=/ \
	checkerr "\$PATH_TRANSLATED: not in document root $HOME." main

DOCUMENT_ROOT=/ PATH_TRANSLATED="$HOME" \
	checkerr '$PATH_TRANSLATED: not a regular file.' main

DOCUMENT_ROOT=/ PATH_TRANSLATED="$true" \
	checkerr 'owned by the superuser.' main


#
# Interlude
#

uid="$(id -u)" && [ "$uid" ] ||
	abort "failed to get process' effective UID."

# The checks below only work if main.sh is invoked as root.
[ "$uid" -ne 0 ] && exit

uid="$(regularuid)" && [ "$uid" ] ||
	abort "failed to get non-root user ID of caller."
user="$(id -un "$uid")" && [ "$user" ] ||
	abort "failed to name of user with ID $uid."
gid="$(id -g "$user")" && [ "$gid" ] ||
	abort "failed to get ID of $user's primary group."

tmpdir="${HOME:?}/.tmp-$$"
readonly tmpdir

catch=
mkdir -m 0700 "$tmpdir" || exit
cleanup="rm -rf \"\${tmpdir:?}\"; ${cleanup-}"
catch=x
[ "${caught-}" ] && exit $((caught + 128))

cp -a "$script_dir/scripts/." "$tmpdir/."

grpw="$tmpdir/grpw.sh"
echo : > "$grpw"
chown -R "$uid:$gid" "$tmpdir"
chmod ug=w "$grpw"

reportuser="$tmpdir/user.sh"
chown "$uid:$gid" "$reportuser"
chmod u=rwx,go= "$reportuser"

fifo="$tmpdir/fifo"
mkfifo "$fifo"


#
# Root checks
#

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$grpw" \
	checkerr "$grpw: can be altered by users other than $user." main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$tmpdir/script.sh" \
	checkok 'This is a test script for main.sh and run_script.sh.' main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$reportuser" \
	checkok "$uid:$gid" main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$tmpdir/env.sh" FOO=bar \
	main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'FOO=bar' <"$fifo" && abort 'environment was not cleared.'
wait "$pid" || abort "./env.sh exited with non-status $?."

exit 0
