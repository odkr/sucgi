#!/bin/sh
#
# Test sucgi via main.
#
# Copyright 2022 Odin Kroeger
#
# This file is part of suCGI.
#
# suCGI is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# suCGI is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with suCGI. If not, see <https://www.gnu.org/licenses>.
#
# shellcheck disable=1091,2015,2016,2034

#
# Initialisation
#

set -Cefu
readonly script_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
readonly src_dir="$(cd -P "$script_dir/.." && pwd)"
readonly tools_dir="$src_dir/tools"
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit


#
# Prelude
#

# Effective UID.
euid="$(id -u)"

# A regular user.
if [ "$euid" -eq 0 ]
then
	reguser="$(owner "$src_dir")"
	[ "$reguser" = root ] && reguser="$(regularuser)"
fi

# Paths
tests_dir=$(cd -P "$script_dir" && pwd)

# Create a temporary directory in $HOME.
logname="$(logname)"
case $logname in (*[!A-Za-z0-9_]*)
	err "$logname is not a portable name."
esac
eval home="~$logname"
TMPDIR="$home"
tmpdir

# Create a list of more environment variables than suCGI permits.
env_max=256
for i in $(seq $((env_max + 1)))
do
	vars="${vars-} v$i="
done
unset i

# Create a document root.
readonly doc_root="$TMPDIR/docroot"
mkdir "$doc_root"

# Get the system's maximum path length in $TMPDIR.
path_max="$(getconf PATH_MAX "$TMPDIR")"
[ "$path_max" -lt 0 ] && path_max=255

# Create a path that is as long as the system and suCGI permit.
if [ "$path_max" -gt 1024 ]
	then max=1024
	else max="$path_max"
fi
long_path="$(mklongpath "$doc_root" "$((max - 1))")"
mkdir -p "$(dirname "$long_path")"
echo $$ >"$long_path"

# Create a path that is longer than the system permits.
huge_path="$(mklongpath "$doc_root" "$path_max")"
traverse "$doc_root" "$huge_path" 'mkdir "$fname"' 'echo $$ >"$fname"'

# Create a path that is longer than suCGI permits.
huge_str="$(mklongpath "$doc_root" 1024)"
traverse "$doc_root" "$huge_str" 'mkdir "$fname"' 'echo $$ >"$fname"'

# Create a shortcut to the path that is longer than the system permits.
readonly huge_path_link="$doc_root/$(traverse "$doc_root" "$huge_path" \
	'ln -s "$fname" d && printf d/' \
	'ln -s "$fname" f && printf f\\n')"

# Create a shortcut to the path that is longer than suCGI permits.
readonly huge_str_link="$doc_root/$(traverse "$doc_root" "$huge_str" \
	'ln -s "$fname" d && printf d/' \
	'ln -s "$fname" f && printf f\\n')"

# Create a link to /.
readonly root_link="$doc_root/root"
ln -s / "$root_link"

# Create a directory inside the document root.
readonly dir="$doc_root/dir"
mkdir "$dir"

# Create a file inside the document root.
readonly inside="$doc_root/inside"
echo $$ >"$inside"

# Create a file outside the document root.
readonly outside="$TMPDIR/outside"
touch "$outside"

# Create a link to the outside.
readonly out_link="$doc_root/outside"
ln -s "$outside" "$out_link"

# Create a link to the inside
readonly in_link="$TMPDIR/to-inside"
ln -s "$inside" "$in_link"

# Locate env
readonly env_bin="$(command -v env)"
readonly env_dir="$(dirname "$env_bin")"

# Make sure the document root is owned by a regular user.
[ "$euid" -eq 0 ] && chown -R "$reguser:$(id -g "$reguser")" "$doc_root"


#
# Too many environment variables.
#

checkerr 'too many environment variables.' \
	$vars main


#
# Malformed environment variables.
#

tests_dir=$(cd -P "$script_dir" && pwd) ||
	err 'failed to get working directory.'

checkerr 'an environment variable is malformed.' \
	badenv -n3 foo bar=bar baz=baz "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv -n3 bar=bar foo baz=baz "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv -n3 bar=bar baz=baz foo "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv 0bar=bar foo=foo baz=baz "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv foo=foo 0bar=bar baz=baz "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv foo=foo baz=baz 0bar=bar "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv '=baz' foo=foo bar=bar "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv foo=foo '=baz' bar=bar "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv foo=foo bar=bar '=baz' "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv -n3 '' foo=foo bar=bar "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv -n3 foo=foo '' bar=bar "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv -n3 foo=foo bar=bar '' "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv 'foo foo=foo' bar=bar baz=baz "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv bar=bar 'foo foo=foo' baz=baz "$tests_dir/main"

checkerr 'an environment variable is malformed.' \
	badenv bar=bar baz=baz 'foo foo=foo' "$tests_dir/main"


#
# Verification of $DOCUMENT_ROOT.
#

# TODO:
# Wrong JAILs require separate binaries.
# (non-existent jail)

# No DOCUMENT_ROOT given.
checkerr '$DOCUMENT_ROOT is unset or empty.' \
	main

# $DOCUMENT_ROOT is empty.
checkerr '$DOCUMENT_ROOT is unset or empty.' \
	DOCUMENT_ROOT= main

# $DOCUMENT_ROOT is too long (suCGI).
checkerr 'an environment variable is too long.' \
	DOCUMENT_ROOT="$huge_str" main

# $DOCUMENT_ROOT is too long (system).
checkerr 'too long.' \
	DOCUMENT_ROOT="$huge_path" main

# Path to document root is too long after having been resolved (system).
checkerr 'too long' \
	DOCUMENT_ROOT="$huge_path_link" main

# Path to document root is too long after having been resolved (suCGI).
checkerr 'too long' \
	DOCUMENT_ROOT="$huge_str_link" main

# Path to document root points to outside of jail.
checkerr "document root / not within jail" \
	DOCUMENT_ROOT=/ main

# Resolved document root points to outside of jail (dots).
checkerr "document root / not within jail" \
	DOCUMENT_ROOT="$doc_root/../../../../../../../../../../../../.." main

# Resolved path to document roots to outside of document root (symlink).
checkerr "document root / not within jail" \
	DOCUMENT_ROOT="$root_link" main

# Document root is of the wrong type.
checkerr "open $outside: Not a directory." \
	DOCUMENT_ROOT="$outside" main

# Document root does not exist.
checkerr "realpath /lib/<no file!>: No such file or directory." \
	DOCUMENT_ROOT="/lib/<no file!>" main

# Simple test. Should fail but regard DOCUMENT_ROOT as valid.
checkerr '$PATH_TRANSLATED is unset or empty.' \
	DOCUMENT_ROOT="$doc_root" main

# Long filename. Should fail but regard DOCUMENT_ROOT as valid.
long_doc_root="${long_path%??????????????}"
checkerr "realpath $long_doc_root: No such file or directory" \
	DOCUMENT_ROOT="$long_doc_root" main


#
# Verification of $PATH_TRANSLATED.
#

# PATH_TRANSLATED is undefined.
checkerr '$PATH_TRANSLATED is unset or empty.' \
	DOCUMENT_ROOT="$doc_root" main

# $PATH_TRANSLATED is empty.
checkerr '$PATH_TRANSLATED is unset or empty.' \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED= main

# $PATH_TRANSLATED is too long (system).
checkerr 'too long' \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$huge_path" main

# $PATH_TRANSLATED is too long (suCGI).
checkerr 'too long' \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$huge_str" main

# Path to script is too long after having been resolved (system).
checkerr 'too long' \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$huge_path_link" main

# Path to script is too long after having been resolved (suCGI).
checkerr 'too long' \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$huge_str_link" main

# Document root does not exist.
checkerr "realpath /lib/<no file!>: No such file or directory." \
	DOCUMENT_ROOT="/lib/<no file!>" PATH_TRANSLATED=x main

# Path to script points to outside of document root.
checkerr "script "$outside" not within document root" \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$outside" main

# Resolved path to script points to outside of document root (dots).
checkerr "script "$outside" not within document root" \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$doc_root/../outside" main

# Resolved path to script points to outside of document root (symlink).
checkerr "script "$outside" not within document root" \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$out_link" main

# Script is of the wrong type.
checkerr "script "$dir" is not a regular file" \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$dir" main

# Script does not exist.
checkerr "realpath $doc_root/<no file!>: No such file or directory." \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$doc_root/<no file!>" main

# Simple test. Should fail but regard PATH_TRANSLANTED as valid.
err='seteuid 0: Operation not permitted.'
[ "$euid" -eq 0 ] && err="chdir $doc_root: Permission denied."
checkerr "$err" \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$inside" main

# Symlink into jail. Should fail but regard PATH_TRANSLATED as valid.
checkerr "$err" \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$in_link" main

# Long filename. Should fail but regard PATH_TRANSLANTED as valid.
long_path_trans="${long_path%????????????????}"
checkerr "realpath $long_path_trans: No such file or directory." \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$long_path_trans" main


#
# Simple tests for script ownership.
#

checkerr "script $env_bin is owned by privileged user root" \
	DOCUMENT_ROOT="$env_dir" PATH_TRANSLATED="$env_bin" main

exit



#
# Interlude
#

euid="$(id -u)" && [ "$euid" ] ||
	err "failed to get process' effective UID."

# The checks below only work if main.sh is invoked as root.
[ "$euid" -ne 0 ] && exit

tmpdir="${home:?}/tmp-$$"
readonly tmpdir



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



checkerr "$su: owned by privileged UID 0." \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$su" main

#checkerr "$sg: owned by privileged GID 0." \
	DOCUMENT_ROOT="$tmpdir" PATH_TRANSLATED="$sg" main
exit 999
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
