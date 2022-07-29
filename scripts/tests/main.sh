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
: "${TESTSDIR:=./build/tests}"
PATH="$TESTSDIR:$TESTSDIR/tools:$script_dir/../../build/tests:$PATH"

tmpdir chk


#
# Prelude
#

str_max="$(getconf PATH_MAX .)" && [ "$str_max" ] && 
	[ "$str_max" -ge 4096 ] || str_max=4096
long_str="$PWD/"
while [ "${#long_str}" -le "$str_max" ]
	do long_str="$long_str/x"
done
long_str="$long_str/x"

if	path_max="$(getconf PATH_MAX .)" &&
	[ "$path_max" ] &&
	[ "$path_max" -gt -1 ]
then
	long_path="$PWD/"
	while [ "${#long_path}" -le "$path_max" ]
		do long_path="$long_path/x"
	done
	long_path="$long_path/x"
fi

if	name_max="$(getconf NAME_MAX .)" &&
	[ "$name_max" ] &&
	[ "$name_max" -gt -1 ]
then
	long_name="x"
	while [ "${#long_name}" -le "$name_max" ]
		do long_name="${long_name}x"
	done
	long_name="$PWD/${long_name}x/x"
fi

true="$(command -v true >/dev/null 2>&1 || :)" || :
: "${true:=/usr/bin/true}"
"$true" || abort "true: exited with status $?."

ruid="$(regularuid)" && [ "$ruid" ] ||
	abort "failed to get non-root user ID of caller."
user="$(id -un "$ruid")" && [ "$user" ] ||
	abort "failed to name of user with ID $uid."
rgid="$(id -g "$user")" && [ "$rgid" ] ||
	abort "failed to get ID of $user's primary group."

home="$(homedir "$user")" && [ "$home" ] ||
	abort "failed to get $user's home directory."


#
# Non-root checks
#

i=0 vars=''
while [ $i -lt 1024 ]
do
	eval "export var$i=foo"
	vars="$vars var$i"
	i=$((i + 1))
done

checkerr 'too many environment variables.' main	

# shellcheck disable=2086
unset $vars

checkerr 'DOCUMENT_ROOT: not set.' main

DOCUMENT_ROOT='' \
	checkerr 'DOCUMENT_ROOT: is the empty string.' main

DOCUMENT_ROOT="$long_str" \
	checkerr 'an environment variable is too long.' main

[ "$long_path" ] &&
	DOCUMENT_ROOT="$long_path" \
		checkerr 'too long.' main

[ "$long_name" ] &&
	DOCUMENT_ROOT="$long_name" \
		checkerr '$DOCUMENT_ROOT: filename too long.' main

DOCUMENT_ROOT='/::no-such-file!!' \
	checkerr '$DOCUMENT_ROOT: No such file or directory.' main

DOCUMENT_ROOT="$0" \
	checkerr '$DOCUMENT_ROOT: not a directory.' main

DOCUMENT_ROOT=. \
	checkerr '$DOCUMENT_ROOT: does not match /*.' main

DOCUMENT_ROOT=/ \
	checkerr 'PATH_TRANSLATED: not set.' main

DOCUMENT_ROOT=/ PATH_TRANSLATED='' \
	checkerr 'PATH_TRANSLATED: is the empty string.' main

DOCUMENT_ROOT=/ PATH_TRANSLATED="$long_str" \
	checkerr 'an environment variable is too long.' main

[ "$long_path" ] &&
	DOCUMENT_ROOT=/ PATH_TRANSLATED="$long_path" \
		checkerr 'too long.' main

[ "$long_name" ] &&
	DOCUMENT_ROOT=/ PATH_TRANSLATED="$long_name" \
		checkerr '$PATH_TRANSLATED: filename too long.' main

DOCUMENT_ROOT=/ PATH_TRANSLATED='/::no-such-file!!' \
	checkerr '$PATH_TRANSLATED: No such file or directory.' main

DOCUMENT_ROOT="$home" PATH_TRANSLATED="$true" \
	checkerr "\$PATH_TRANSLATED: not in document root $home." main

DOCUMENT_ROOT="$home" PATH_TRANSLATED="$home" \
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

tmpdir="${home:?}/.tmp-$$"
readonly tmpdir

catch=
mkdir -m 0700 "$tmpdir" || exit
cleanup="rm -rf \"\${tmpdir:?}\"; ${cleanup-}"
catch=x
[ "${caught-}" ] && exit $((caught + 128))

cp -a "$script_dir/tools/." "$tmpdir/."

cp "$tmpdir/script.sh" "$tmpdir/script"
chmod +x "$tmpdir/script"

grpw="$tmpdir/grpw.sh"
echo : > "$grpw"
chown -R "$ruid:$rgid" "$tmpdir"
chmod ug=w "$grpw"

reportuser="$tmpdir/user.sh"
chown "$ruid:$rgid" "$reportuser"
chmod u=rwx,go= "$reportuser"

fifo="$tmpdir/fifo"
mkfifo "$fifo"


#
# Root checks
#

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$tmpdir/script.sh" \
	checkerr "yo" \
		run-as 1 1 main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$tmpdir/script.sh" \
	checkerr "yo" \
		run-as 30001 30001 main


DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$grpw" \
	checkerr "$grpw: can be altered by users other than $user." main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$tmpdir/script.sh" \
	checkok 'This is a test script for main.sh and run_script.sh.' main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$tmpdir/script" \
	checkok 'This is a test script for main.sh and run_script.sh.' main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$reportuser" \
	checkok "$ruid:$rgid" main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$tmpdir/env.sh" FOO=bar \
	main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'FOO=bar' <"$fifo" && abort 'environment was not cleared.'
wait "$pid" || abort "./env.sh exited with non-status $?."

exit 0
