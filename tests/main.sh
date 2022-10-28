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
script_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(cd -P "$script_dir/.." && pwd)"
tools_dir="$src_dir/tools"
readonly script_dir src_dir tools_dir
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit


#
# Prelude
#

# Effective UID.
euid="$(id -u)"

# Get a regular user and a their primary group.
if [ "$euid" -eq 0 ]
then
	user="$(owner "$src_dir")"
	[ "$user" = root ] && user="$(regularuser)"
else
	user="$(logname)"
fi
group="$(id -g "$user")"

# Paths
tests_dir=$(cd -P "$script_dir" && pwd)

# Create a temporary directory in $HOME.
case $user in (*[!A-Za-z0-9_]*)
	err "$user is not a portable name."
esac

eval home="~$user"
readonly home

tmp="$(cd -P "${TMPDIR:-/tmp}" && pwd)"
readonly tmp

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
huge_path_link="$doc_root/$(traverse "$doc_root" "$huge_path" \
	'ln -s "$fname" d && printf d/' \
	'ln -s "$fname" f && printf f\\n')"

# Create a shortcut to the path that is longer than suCGI permits.
huge_str_link="$doc_root/$(traverse "$doc_root" "$huge_str" \
	'ln -s "$fname" d && printf d/' \
	'ln -s "$fname" f && printf f\\n')"

# Create a link to /.
root_link="$doc_root/root"
ln -s / "$root_link"

# Create a directory inside the document root.
dir="$doc_root/dir"
mkdir "$dir"

# Create a file inside the document root.
inside="$doc_root/inside"
echo $$ >"$inside"

# Create a file outside the document root.
outside="$TMPDIR/outside"
touch "$outside"

# Create a link to the outside.
out_link="$doc_root/outside"
ln -s "$outside" "$out_link"

# Create a link to the inside
in_link="$TMPDIR/to-inside"
ln -s "$inside" "$in_link"

# Locate env
env_bin="$(command -v env)"
env_dir="$(dirname "$env_bin")"

# Make sure the files are owned by a regular user.
[ "$euid" -eq 0 ] && chown -R "$user:$group" "$TMPDIR"


#
# Too many environment variables.
#

# shellcheck disable=2086
checkerr 'too many environment variables.' \
	$vars main


#
# Malformed environment variables.
#

tests_dir=$(cd -P "$script_dir" && pwd) ||
	err 'failed to get working directory.'


checkerr 'found malformed environment variable.' \
	badenv -n3 foo bar=bar baz=baz "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv -n3 bar=bar foo baz=baz "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv -n3 bar=bar baz=baz foo "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv 0bar=bar foo=foo baz=baz "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv foo=foo 0bar=bar baz=baz "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv foo=foo baz=baz 0bar=bar "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv '=baz' foo=foo bar=bar "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv foo=foo '=baz' bar=bar "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv foo=foo bar=bar '=baz' "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv -n3 '' foo=foo bar=bar "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv -n3 foo=foo '' bar=bar "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv -n3 foo=foo bar=bar '' "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv 'foo foo=foo' bar=bar baz=baz "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv bar=bar 'foo foo=foo' baz=baz "$tests_dir/main"

checkerr 'found malformed environment variable.' \
	badenv bar=bar baz=baz 'foo foo=foo' "$tests_dir/main"


#
# Verification of $DOCUMENT_ROOT (pre-privilege drop).
#

# TODO:
# Wrong JAILs require separate binaries.
# (non-existent jail)

# No DOCUMENT_ROOT given.
checkerr '$DOCUMENT_ROOT unset or empty.' \
	main

# $DOCUMENT_ROOT is empty.
checkerr '$DOCUMENT_ROOT unset or empty.' \
	DOCUMENT_ROOT= main

# $DOCUMENT_ROOT is too long (suCGI).
checkerr 'found too long environment variable.' \
	DOCUMENT_ROOT="$huge_str" main

# $DOCUMENT_ROOT is too long (system).
checkerr 'too long' \
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
checkerr '$PATH_TRANSLATED unset or empty.' \
	DOCUMENT_ROOT="$doc_root" main

# Long filename. Should fail but regard DOCUMENT_ROOT as valid.
long_doc_root="${long_path%??????????????}"
checkerr "realpath $long_doc_root: No such file or directory" \
	DOCUMENT_ROOT="$long_doc_root" main

# DOCUMENT_ROOT remains mostly the same for the remainder of this script.
export DOCUMENT_ROOT="$doc_root"


#
# Verification of $PATH_TRANSLATED.
#

# PATH_TRANSLATED is undefined.
checkerr '$PATH_TRANSLATED unset or empty.' \
	main

# $PATH_TRANSLATED is empty.
checkerr '$PATH_TRANSLATED unset or empty.' \
	PATH_TRANSLATED= main

# $PATH_TRANSLATED is too long (system).
checkerr 'too long' \
	PATH_TRANSLATED="$huge_path" main

# $PATH_TRANSLATED is too long (suCGI).
checkerr 'too long' \
	PATH_TRANSLATED="$huge_str" main

# Path to script is too long after having been resolved (system).
checkerr 'too long' \
	PATH_TRANSLATED="$huge_path_link" main

# Path to script is too long after having been resolved (suCGI).
checkerr 'too long' \
	PATH_TRANSLATED="$huge_str_link" main

# Path to script points to outside of document root.
checkerr "script $outside not within document root" \
	PATH_TRANSLATED="$outside" main

# Resolved path to script points to outside of document root (dots).
checkerr "script $outside not within document root" \
	PATH_TRANSLATED="$doc_root/../outside" main

# Resolved path to script points to outside of document root (symlink).
checkerr "script $outside not within document root" \
	PATH_TRANSLATED="$out_link" main

# Script is of the wrong type.
checkerr "script $dir is not a regular file" \
	PATH_TRANSLATED="$dir" main

# Script does not exist.
checkerr "realpath $doc_root/<no file!>: No such file or directory." \
	PATH_TRANSLATED="$doc_root/<no file!>" main

# Simple test. Should fail but regard PATH_TRANSLANTED as valid.
if [ "$euid" -eq 0 ]
	then err="document root $doc_root is not $user's user directory."
	else err='seteuid 0: Operation not permitted.'
fi
checkerr "$err" \
	PATH_TRANSLATED="$inside" main

# Symlink into jail. Should fail but regard PATH_TRANSLATED as valid.
checkerr "$err" \
	PATH_TRANSLATED="$in_link" main

# Long filename. Should fail but regard PATH_TRANSLANTED as valid.
long_path_trans="${long_path%????????????????}"
checkerr "realpath $long_path_trans: No such file or directory." \
	PATH_TRANSLATED="$long_path_trans" main


#
# Simple tests for script ownership.
#

checkerr "script $env_bin is owned by privileged user root" \
	DOCUMENT_ROOT="$env_dir" PATH_TRANSLATED="$env_bin" main


#
# Abort unless run by the superuser.
#

if [ "$euid" -ne 0 ]
then
	warn -g 'all tests passed.'
	exit
fi


#
# Interlude
#

# Get the user's ID and group.
uid="$(id -u "$user")"
gid="$(id -g "$user")"

# Create a file without an owner.
unalloc_uid="$(findid -nu 1000 30000)"
noowner="$doc_root/noowner"
touch "$noowner"
chown "$unalloc_uid" "$noowner"

# Create a file owned by root.
root_owned="$doc_root/priv-root"
touch "$root_owned"

# Create a file owned by a non-root privileged user with a low UID.
low_uid="$(findid -u 1 500)"
low_uid_owned="$doc_root/priv-low-uid"
touch "$low_uid_owned"
chown "$low_uid" "$low_uid_owned"

# Create a file owned by a non-root privileged user with a high UID.
if high_uid="$(findid -u 60000 65536 2>/dev/null)"
then
	high_uid_owned="$doc_root/priv-high-uid"
	touch "$high_uid_owned"
	chown "$high_uid" "$high_uid_owned"
fi

# Create a hidden file.
hidden_file="$doc_root/.hidden.f"
touch "$hidden_file"
chown "$user:$group" "$hidden_file"

# Create a file in a hidden directory.
hidden_dir="$doc_root/.hidden.d/file"
mkdir "$doc_root/.hidden.d"
touch "$hidden_dir"
chown -R "$user:$group" "$doc_root/.hidden.d"

# Create a set-UID script.
setuid="$doc_root/setuid"
touch "$setuid"
chown "$user:$group" "$setuid"
chmod u+s "$setuid"

# Create a set-GID script.
setgid="$doc_root/setgid"
touch "$setgid"
chown "$user:$group" "$setgid"
chmod g+s "$setgid"

# Create a file that is group-writable.
groupw_file="$doc_root/groupw.f"
touch "$groupw_file"
chown "$user:$group" "$groupw_file"
chmod g+w "$groupw_file"

# Create a file that is world-writable.
otherw_file="$doc_root/otherw.f"
touch "$otherw_file"
chown "$user:$group" "$otherw_file"
chmod o+w "$otherw_file"

# Create a file in a directory that is group-writable.
groupw_dir="$doc_root/groupw.d/file"
mkdir "$doc_root/groupw.d"
touch "$groupw_dir"
chown -R "$user:$group" "$doc_root/groupw.d"
chmod g+w "$doc_root/groupw.d"

# Create a file in a directory that is other-writable.
otherw_dir="$doc_root/otherw.d/file"
mkdir "$doc_root/otherw.d"
touch "$otherw_dir"
chown -R "$user:$group" "$doc_root/otherw.d"
chmod o+w "$doc_root/otherw.d"

# Create a script that prints the current user and group.
script_sh="$doc_root/script.sh"
cat <<'EOF' >"$script_sh"
#!/bin/sh
printf 'euid=%s egid=%s ruid=%s rgid=%s\n' \
       "$(id -u)" "$(id -g)" "$(id -ur)" "$(id -gr)"
EOF
chown "$user:$group" "$script_sh"

# Create a non-executable copy without a filename suffix.
suffix_none="$doc_root/suffix_none"
cp -p "$script_sh" "$suffix_none"

# Create a non-executable copy with an unknown suffix.
suffix_unknown="$doc_root/suffix.nil"
cp -p "$script_sh" "$suffix_unknown"

# Create an executable copy without a filename suffix.
script="${script_sh%.sh}"
cp -p "$script_sh" "$script"
chmod +x "$script"

# Create a script that prints the environment.
env_sh="$doc_root/env.sh"
cat <<'EOF' >"$env_sh"
#!/bin/sh
env
EOF
chown "$user:$group" "$env_sh"

# Create an executable copy without a filename suffix.
env="${env_sh%.sh}"
cp -p "$env_sh" "$env"
chmod +x "$env"


#
# Verification of owner 
#

# Owned by non-existing user.
checkerr "script $noowner is owned by unallocated UID $unalloc_uid." \
	PATH_TRANSLATED="$noowner" main

# Owned by root.
checkerr "script $root_owned is owned by privileged user" \
	PATH_TRANSLATED="$root_owned" main

# Owned by a non-root privileged user with a low UID.
checkerr "script $low_uid_owned is owned by privileged user" \
	PATH_TRANSLATED="$low_uid_owned" main

# Owned by a non-root privileged user with a high UID.
if [ "$high_uid" ]
then
	checkerr "script $high_uid_owned is owned by privileged user" \
		PATH_TRANSLATED="$high_uid_owned" main
fi


#
# Verification of DOCUMENT_ROOT (post-privilege drop)
#

# DOCUMENT_ROOT is not USER_DIR.
checkerr "document root $doc_root is not $user's user directory." \
	DOCUMENT_ROOT="$doc_root" PATH_TRANSLATED="$script" main

# Set a legal DOCUMENT_ROOT.
export DOCUMENT_ROOT="$home"


#
# Not a hidden file.
#

for hidden in "$hidden_file" "$hidden_dir"
do
	checkerr "path $hidden contains hidden files." \
		PATH_TRANSLATED="$hidden" main
done

#
# Neither set-UID nor set-GID bit set.
#

checkerr "script $setuid's set-user-ID bit is set." \
	PATH_TRANSLATED="$setuid" main

checkerr "script $setgid's set-group-ID bit is set." \
	PATH_TRANSLATED="$setgid" main


#
# Verification of exclusive write-access.
#

for file in "$groupw_file" "$otherw_file"
do
	checkerr "$file is writable by users other than $user." \
		PATH_TRANSLATED="$file" main
done

for file in "$groupw_dir" "$otherw_dir"
do
	checkerr "$(dirname "$file") is writable by users other than $user." \
		PATH_TRANSLATED="$file" main
done


#
# Check suffix errors.
#

checkerr "$suffix_none has no filename suffix." \
	PATH_TRANSLATED="$suffix_none" main

checkerr "no handler registered for $suffix_unknown." \
	PATH_TRANSLATED="$suffix_unknown" main


#
# Check whether privileges have been dropped.
#

for path in "$script" "$script_sh"
do
	checkok "euid=$uid egid=$gid ruid=$uid rgid=$gid" \
		PATH_TRANSLATED="$path" main
done


#
# Check whether the environment has been cleared.
#

for path in "$env" "$env_sh"
do
	warn "checking ${bld-}PATH_TRANSLATED=$env_sh foo=foo main${rst-} ..."

	PATH_TRANSLATED="$path" foo=foo main |
	grep -Fq foo= && err -l "environment was not cleared."
done


#
# Run tests as ordinary user.
#

warn "running tests as unprivileged user ..."

unset DOCUMENT_ROOT

# shellcheck disable=2154
runas "$uid" "$gid" "$tests_dir/$prog_name"
