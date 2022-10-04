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
	abort "failed to find working directory."

ruid="$(regularuid)" && [ "$ruid" ] ||
	abort "failed to get non-root user ID of caller."
# shellcheck disable=2154
user="$(getlogname "$ruid")" && [ "$user" ] ||
	abort "failed to get name of user with ID $bold$uid$reset$red."
rgid="$(id -g "$user")" && [ "$rgid" ] ||
	abort "failed to get ID of $bold$user$reset$red's primary group."

case $user in (*[!A-Za-z0-9_]*)
	abort "$bold$user$reset$red: not a portable name."
esac

eval home="~$user" && [ "${home-}" ] && [ "$home" != "~$user" ] ||
	abort "failed to get $bold$user$reset$red's home directory."

[ -d "$home" ] ||
	abort "$bold$home$reset$red: no such directory."

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
long_str="$(printf '%*s\n' "$((str_max + 1))" | tr ' ' x)" && 
	[ "$long_str" ] || abort "failed to generate a long string."

if path_max="$(getconf PATH_MAX .)" && [ "${path_max:-"-1"}" -gt -1 ]
then
	long_path="$pwd/$(printf '%*s\n' $((path_max + 1)) | tr ' ' x)" &&
		[ "$long_path" != "$pwd/" ] ||
			abort "failed to generate a long path."
fi

if name_max="$(getconf NAME_MAX .)" && [ "${name_max:-"-1"}" -gt -1 ]
then
	long_name="$pwd/$(printf '%*s\n' $((name_max + 1)) | tr ' ' x)" &&
		[ "$long_name" != "$pwd/" ] ||
			abort "failed to generate a long name."
fi

true="$(command -v true >/dev/null 2>&1 || :)" || :
: "${true:=/usr/bin/true}"
"$true" ||
	abort "${bold}true$reset$red: exited with status $?."
true_dir="$(dirname "$true")" && [ "$true_dir" ] ||
	abort "$bold$true$reset$red: failed to get directory."


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
	evilenv "$pwd/main" foo=foo bar baz=baz

checkerr 'an environment variable is malformed.' \
	evilenv "$pwd/main" 0foo=foo bar=bar baz=baz

checkerr 'an environment variable is malformed.' \
	evilenv "$pwd/main" foo=foo bar=bar '=baz'

checkerr 'an environment variable is malformed.' \
	evilenv "$pwd/main" '' foo=foo =bar

eval "export $long_str=\"\$long_str\""
checkerr 'an environment variable is too long.' main
unset "$long_str"

checkerr 'DOCUMENT_ROOT: unset or empty.' main

DOCUMENT_ROOT='' \
	checkerr 'DOCUMENT_ROOT: unset or empty.' main

[ "$long_path" ] &&
	DOCUMENT_ROOT="$long_path" \
		checkerr 'an environment variable is too long.' main

[ "$long_name" ] &&
	DOCUMENT_ROOT="$long_name" \
		checkerr '$DOCUMENT_ROOT: File name too long.' main

DOCUMENT_ROOT='/::no-such-file!!' \
	checkerr '$DOCUMENT_ROOT: No such file or directory.' main

DOCUMENT_ROOT="$file" \
	checkerr '$DOCUMENT_ROOT: Not a directory.' main

DOCUMENT_ROOT="$TMPDIR" \
	checkerr 'PATH_TRANSLATED: unset or empty.' main

DOCUMENT_ROOT="$TMPDIR" PATH_TRANSLATED='' \
	checkerr 'PATH_TRANSLATED: unset or empty.' main

[ "$long_path" ] &&
	DOCUMENT_ROOT="$pwd" PATH_TRANSLATED="$long_path" \
		checkerr 'an environment variable is too long.' main

[ "$long_name" ] &&
	DOCUMENT_ROOT="$pwd" PATH_TRANSLATED="$long_name" \
		checkerr '$PATH_TRANSLATED: File name too long.' main

DOCUMENT_ROOT="$pwd" PATH_TRANSLATED="$pwd/::no-such-file!!" \
	checkerr '$PATH_TRANSLATED: No such file or directory.' main

DOCUMENT_ROOT="$TMPDIR" PATH_TRANSLATED="$dir" \
	checkerr '$PATH_TRANSLATED: not a regular file.' main

DOCUMENT_ROOT="$true_dir" PATH_TRANSLATED="$true" \
	checkerr 'owned by privileged UID 0.' main

DOCUMENT_ROOT=/ PATH_TRANSLATED="$file" \
	checkerr '$DOCUMENT_ROOT: not within /.' main

DOCUMENT_ROOT="$root_symlink" PATH_TRANSLATED="$file" \
	checkerr '$DOCUMENT_ROOT: not within /.' main

DOCUMENT_ROOT="$root_dotdot" PATH_TRANSLATED="$file" \
	checkerr '$DOCUMENT_ROOT: not within /.' main

DOCUMENT_ROOT="$home" PATH_TRANSLATED="$file" \
	checkerr "\$PATH_TRANSLATED: not within \$DOCUMENT_ROOT." main


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

eval "$(unallocids)" && [ "$unalloc_uid" ] && [ "$unalloc_gid" ] ||
        abort "failed to find an unallocated UID and GID."

# shellcheck disable=2086
set -- $unused_ids
[ $# -eq 2 ] ||
	abort "unallocids: unparsable output."

eval "$(unallocids)" && [ "$uid" ] && [ "$gid" ] ||
        abort "failed to find an unallocated UID and GID."

unalloc_uid="$1"
unalloc_gid="$2"

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
	checkerr "$nouser: getpwuid $unalloc_uid: no such user." main

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
