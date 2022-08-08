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

ruid="$(regularuid)" && [ "$ruid" ] ||
	abort "failed to get non-root user ID of caller."
user="$(username "$ruid")" && [ "$user" ] ||
	abort "failed to get username associated with ID $ruid."
rgid="$(id -g "$user")" && [ "$rgid" ] ||
	abort "failed to get ID of $user's primary group."

home="$(homedir "$user")" && [ "$home" ] ||
	abort "failed to get $user's home directory."

file="$TMPDIR/file"
touch "$file"
dir="$TMPDIR/dir"
mkdir "$dir"
root_symlink="$TMPDIR/root"
ln -s / "$root_symlink"
root_dotdot="$TMPDIR/../../../../../../../../../../../../../../../../../.."

chown -R "$ruid:$rgid" "$TMPDIR"

str_max="$(getconf PATH_MAX .)" && [ "$str_max" ] && 
	[ "$str_max" -ge 4096 ] || str_max=4096
long_str="x"
while [ "${#long_str}" -le "$str_max" ]
	do long_str="${long_str}x"
done
long_str="${long_str}x"

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
	long_name="$PWD/${long_name}/x"
fi

true="$(command -v true >/dev/null 2>&1 || :)" || :
: "${true:=/usr/bin/true}"
"$true" ||
	abort "true: exited with status $?."
true_dir="$(dirname "$true")" && [ "$true_dir" ] ||
	abort "$true: failed to get directory."


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

checkerr 'an environment variable is malformed.' \
	evilenv foo "$TESTSDIR/main"

checkerr 'an environment variable is malformed.' \
	evilenv '=bar' "$TESTSDIR/main"

checkerr 'an environment variable is malformed.' \
	evilenv '' "$TESTSDIR/main"

eval "export $long_str=\"\$long_str\""
checkerr 'an environment variable name is too long.' main
unset "$long_str"

checkerr 'DOCUMENT_ROOT: not set.' main

DOCUMENT_ROOT='' \
	checkerr 'DOCUMENT_ROOT: is the empty string.' main

[ "$long_path" ] &&
	DOCUMENT_ROOT="$long_path" \
		checkerr '$DOCUMENT_ROOT: path too long.' main

[ "$long_name" ] &&
	DOCUMENT_ROOT="$long_name" \
		checkerr '$DOCUMENT_ROOT: filename too long.' main

DOCUMENT_ROOT='/::no-such-file!!' \
	checkerr '$DOCUMENT_ROOT: No such file or directory.' main

DOCUMENT_ROOT="$file" \
	checkerr '$DOCUMENT_ROOT: not a directory.' main

DOCUMENT_ROOT="$TMPDIR" \
	checkerr 'PATH_TRANSLATED: not set.' main

DOCUMENT_ROOT="$TMPDIR" PATH_TRANSLATED='' \
	checkerr 'PATH_TRANSLATED: is the empty string.' main

[ "$long_path" ] &&
	DOCUMENT_ROOT="$PWD" PATH_TRANSLATED="$long_path" \
		checkerr '$PATH_TRANSLATED: path too long.' main

[ "$long_name" ] &&
	DOCUMENT_ROOT="$PWD" PATH_TRANSLATED="$long_name" \
		checkerr '$PATH_TRANSLATED: filename too long.' main

DOCUMENT_ROOT="$PWD" PATH_TRANSLATED="$PWD/::no-such-file!!" \
	checkerr '$PATH_TRANSLATED: No such file or directory.' main

DOCUMENT_ROOT="$TMPDIR" PATH_TRANSLATED="$dir" \
	checkerr '$PATH_TRANSLATED: not a regular file.' main

DOCUMENT_ROOT="$true_dir" PATH_TRANSLATED="$true" \
	checkerr 'owned by the superuser.' main

DOCUMENT_ROOT=/ PATH_TRANSLATED="$true" \
	checkerr '$DOCUMENT_ROOT: does not match /*/*.' main

DOCUMENT_ROOT="$root_symlink" PATH_TRANSLATED="$true" \
	checkerr '$DOCUMENT_ROOT: does not match /*/*.' main

DOCUMENT_ROOT="$root_dotdot" PATH_TRANSLATED="$true" \
	checkerr '$DOCUMENT_ROOT: does not match /*/*.' main

DOCUMENT_ROOT="$home" PATH_TRANSLATED="$true" \
	checkerr "\$PATH_TRANSLATED: not in document root $home." main


#
# Interlude
#

euid="$(id -u)" && [ "$euid" ] ||
	abort "failed to get process' effective UID."

# The checks below only work if main.sh is invoked as root.
[ "$euid" -ne 0 ] && exit

tmpdir="${home:?}/.tmp-$$"
readonly tmpdir

catch=
mkdir -m 0700 "$tmpdir" || exit
cleanup="rm -rf \"\${tmpdir:?}\"; ${cleanup-}"
catch=x
[ "${caught-}" ] && exit $((caught + 128))

unused_ids="$(unallocids)" && [ "$unused_ids" ] ||
	abort "failed to find an unused UID and GID."

unused_uid="${unused_ids##*:}"
unused_gid="${unused_ids%%:*}"

[ "$unused_uid" ] || abort "cannot parse unused UID."
[ "$unused_gid" ] || abort "cannot parse unused GID."

cp -a "$script_dir/tools/." "$tmpdir/."

outside="$tmpdir/outside"
ln -s "$TMPDIR" "$outside"

script="$tmpdir/script.sh"
cp "$script" "$tmpdir/script"
chmod ugo-x "$script"
chmod +x "$tmpdir/script"

su="$tmpdir/su.sh"
cp "$tmpdir/script.sh" "$su"

sg="$tmpdir/sg.sh"
cp "$tmpdir/script.sh" "$sg"

ltmin="$tmpdir/ltmin.sh"
cp "$script" "$ltmin"

ltmin="$tmpdir/ltmin.sh"
cp "$script" "$ltmin"

gtmax="$tmpdir/gtmax.sh"
cp "$script" "$gtmax"

nouser="$tmpdir/nouser.sh"
cp "$script" "$nouser"

grpw="$tmpdir/grpw.sh"
cp "$script" "$grpw"
chmod ug=w "$grpw"

reportuser="$tmpdir/user.sh"
chmod u=rwx,go= "$reportuser"

chown -R "$ruid:$rgid" "$tmpdir"
chown 0 "$su"
chgrp 0 "$sg"
chown 1:1 "$ltmin"
chown 30001:30001 "$gtmax"
chown "$unused_uid" "$nouser"

fifo="$tmpdir/fifo"
mkfifo "$fifo"


#
# Root checks
#

DOCUMENT_ROOT="$TMPDIR" PATH_TRANSLATED="$file" \
	checkerr "document root $TMPDIR is not in $user's home directory." main

DOCUMENT_ROOT="$outside" PATH_TRANSLATED="$file" \
	checkerr "document root $TMPDIR is not in $user's home directory." main

DOCUMENT_ROOT="$home/../../../../../../$TMPDIR" PATH_TRANSLATED="$file" \
	checkerr "not in $user's home directory." main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$su" \
	checkerr "$su: owned by the superuser." main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$sg" \
	checkerr "$sg: owned by the supergroup." main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$ltmin" \
	checkerr "$ltmin: owned by non-regular UID 1." main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$gtmax" \
	checkerr "$gtmax: owned by non-regular UID 30001." main main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$nouser" \
	checkerr "$nouser: getpwuid $unused_uid: no such user." main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$grpw" \
	checkerr "$grpw: can be altered by users other than $user." main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$script" \
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
