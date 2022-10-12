#!/bin/sh
# Test sucgi via main.
#shellcheck disable=1091,2015,2016,2034

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
# Prelude
#

pwd="$(cd -P "$script_dir" && pwd)" && [ "$pwd" ] && [ -d "$pwd" ] ||
	err "failed to find working directory."

ruid="$(regularuid)" && [ "$ruid" ] ||
	err "failed to get non-root user ID of caller."
# shellcheck disable=2154
user="$(getlogname "$ruid")" && [ "$user" ] ||
	err "failed to get name of user with ID $bold$uid$reset$red."
rgid="$(id -g "$user")" && [ "$rgid" ] ||
	err "failed to get ID of $bold$user$reset$red's primary group."

case $user in (*[!A-Za-z0-9_]*)
	err "$bold$user$reset$red: not a portable name."
esac

eval home="~$user" && [ "${home-}" ] && [ "$home" != "~$user" ] ||
	err "failed to get $bold$user$reset$red's home directory."

[ -d "$home" ] ||
	err "$bold$home$reset$red: no such directory."

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
long_str="$(printf '%*s\n' "$((str_max + 1))" x | tr ' ' x)" && 
	[ "$long_str" ] || err "failed to generate a long string."

if path_max="$(getconf PATH_MAX .)" && [ "${path_max:-"-1"}" -gt -1 ]
then
	long_path="$pwd/$(printf '%*s\n' "$((path_max + 1))" x | tr ' ' x)" &&
		[ "$long_path" != "$pwd/" ] ||
			err "failed to generate a long path."
fi

if name_max="$(getconf NAME_MAX .)" && [ "${name_max:-"-1"}" -gt -1 ]
then
	long_name="$pwd/$(printf '%*s\n' "$((name_max + 1))" x | tr ' ' x)" &&
		[ "$long_name" != "$pwd/" ] ||
			err "failed to generate a long name."
fi

true="$(command -v true >/dev/null 2>&1 || :)" || :
: "${true:=/usr/bin/true}"
"$true" || err "${bold}true$reset$red: exited with status $?."
true_dir="$(dirname "$true")" && [ "$true_dir" ] ||
	err "$bold$true$reset$red: failed to get directory."


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
	badenv -n3 foo bar=bar baz=baz "$pwd/main"

checkerr 'an environment variable is malformed.' \
	badenv foo=foo 0bar=bar baz=baz "$pwd/main"

checkerr 'an environment variable is malformed.' \
	badenv foo=foo bar=bar '=baz' "$pwd/main"

checkerr 'an environment variable is malformed.' \
	badenv -n2 '' foo=foo "$pwd/main"

checkerr 'an environment variable is too long.' \
	var="$long_str" main

checkerr 'DOCUMENT_ROOT: unset or empty.' \
	main

checkerr 'DOCUMENT_ROOT: unset or empty.' \
	DOCUMENT_ROOT= main

[ "$long_path" ] &&
	checkerr 'an environment variable is too long.' \
		DOCUMENT_ROOT="$long_path" main

[ "$long_name" ] &&
	checkerr '$DOCUMENT_ROOT: File name too long.' \
		DOCUMENT_ROOT="$long_name" main

checkerr '$DOCUMENT_ROOT: No such file or directory.' \
	DOCUMENT_ROOT='/::no-such-file!!' main

checkerr '$DOCUMENT_ROOT: Not a directory.' \
	DOCUMENT_ROOT="$file" main

checkerr 'PATH_TRANSLATED: unset or empty.' \
	DOCUMENT_ROOT="$TMPDIR" main

checkerr 'PATH_TRANSLATED: unset or empty.' \
	DOCUMENT_ROOT="$TMPDIR" PATH_TRANSLATED= main

[ "$long_path" ] &&
	checkerr 'an environment variable is too long.' \
		DOCUMENT_ROOT="$pwd" PATH_TRANSLATED="$long_path" main

[ "$long_name" ] &&
	checkerr '$PATH_TRANSLATED: File name too long.' \
		DOCUMENT_ROOT="$pwd" PATH_TRANSLATED="$long_name" main

checkerr '$PATH_TRANSLATED: No such file or directory.' \
	DOCUMENT_ROOT="$pwd" PATH_TRANSLATED="$pwd/::no-such-file!!" main

checkerr '$PATH_TRANSLATED: not a regular file.' \
	DOCUMENT_ROOT="$TMPDIR" PATH_TRANSLATED="$dir" main

checkerr 'owned by privileged UID 0.' \
	DOCUMENT_ROOT="$true_dir" PATH_TRANSLATED="$true" main

checkerr '$DOCUMENT_ROOT: not within /.' \
	DOCUMENT_ROOT=/ PATH_TRANSLATED="$file" main

checkerr '$DOCUMENT_ROOT: not within /.' \
	DOCUMENT_ROOT="$root_symlink" PATH_TRANSLATED="$file" main

checkerr '$DOCUMENT_ROOT: not within /.' \
	DOCUMENT_ROOT="$root_dotdot" PATH_TRANSLATED="$file" main

checkerr "\$PATH_TRANSLATED: not within \$DOCUMENT_ROOT." \
	DOCUMENT_ROOT="$home" PATH_TRANSLATED="$file" main


#
# Interlude
#

euid="$(id -u)" && [ "$euid" ] ||
	err "failed to get process' effective UID."

# The checks below only work if main.sh is invoked as root.
[ "$euid" -ne 0 ] && exit

tmpdir="${home:?}/.tmp-$$"
readonly tmpdir

catch=
mkdir -m 0700 "$tmpdir" || exit
cleanup="rm -rf \"\${tmpdir:?}\"; ${cleanup-}"
catch=x
[ "${caught-}" ] && exit $((caught + 128))

unalloc_uid="$(unallocid -u 1000 30000)" && [ "$unalloc_uid" ] ||
	err "failed to find an unallocated user ID."
unalloc_gid="$(unallocid -g 1000 30000)" && [ "$unalloc_gid" ] ||
	err "failed to find an unallocated group ID."

cp -a "$script_dir/../tools/." "$tmpdir/."

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
chown "$unalloc_uid" "$nouser"

fifo="$tmpdir/fifo"
mkfifo "$fifo"


#
# Root checks
#

checkerr "document root $TMPDIR is not in $user's home directory." \
	DOCUMENT_ROOT="$TMPDIR" PATH_TRANSLATED="$file" main

checkerr "document root $TMPDIR is not in $user's home directory." \
	DOCUMENT_ROOT="$outside" PATH_TRANSLATED="$file" main

checkerr "not in $user's home directory." \
	DOCUMENT_ROOT="$home/../../../../$TMPDIR" PATH_TRANSLATED="$file" main

checkerr "$su: owned by the superuser." \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$su" main

checkerr "$sg: owned by the supergroup." \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$sg" main

checkerr "$ltmin: owned by non-regular UID 1." \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$ltmin" main

checkerr "$gtmax: owned by non-regular UID 30001." \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$gtmax" main

checkerr "$nouser: getpwuid $unalloc_uid: no such user." \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$nouser" main

checkerr "$grpw: can be altered by users other than $user." \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$grpw" main

checkok 'This is a test script for main.sh and run_script.sh.' \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$script" main

checkok 'This is a test script for main.sh and run_script.sh.' \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$tmpdir/script" main

checkok "$ruid:$rgid" \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$reportuser" main

DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$tmpdir/env.sh" FOO=bar \
	main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'FOO=bar' <"$fifo" && err 'environment was not cleared.'
wait "$pid" || err "./env.sh exited with non-status $?."

exit 0
