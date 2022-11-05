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

# Load build configuration.
eval "$(main -c)"

# Get the effective UID.
uid="$(id -u)"

# Find a regular user.
if [ "$uid" -eq 0 ]
then
	if user="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")"
	then
		# This is a hack to make coverage reports accurate.
		covown() {
			find "$src_dir" \
				'(' -name '*.gcda' -o -name '*.gcno' ')' \
				-exec chown "${1:?}" '{}' +
		}

		owner="$(tools/owner "$src_dir")"
		readonly onwner
		cleanup="covown \"\$owner\"; ${cleanup-}"
		covown "$user"
	else
		user="$(id -un)"
	fi
else
	user="$(id -un)"
fi
group="$(id -g "$user")"
readonly user group

# Create a temporary directory and a document root.
# shellcheck disable=2059
doc_root="$(printf -- "$USER_DIR" "$user")"

IFS=/ i=0 tmp=
for seg in $doc_root
do
	[ "$seg" ] || continue
	tmp="${tmp%/}/$seg" i=$((i + 1))
	[ "$i" -eq 2 ] && break
done
unset IFS seg i

case $tmp in
	(/tmp/*) : ;;
	(*) err "document root $doc_root not within /tmp."
esac
readonly tmp

catch=
mkdir "$tmp"
cleanup="rm -rf \"\$tmp\"; ${cleanup-}"
catch=x
[ "$caught" ] && exit $((caught + 128))

TMPDIR="$(cd -P "$tmp" && pwd)"
export TMPDIR

mkdir -p "$doc_root"
doc_root="$(cd -P "$doc_root" && pwd)"
readonly doc_root

# Get the canonical location of main.
tests_dir=$(cd -P "$script_dir" && pwd)

# Create a list of more environment variables than suCGI permits.
env_max=256
for i in $(seq $((env_max + 1)))
do
	vars="${vars-} v$i="
done
unset i

# Get the system's maximum path length in $TMPDIR.
path_max="$(getconf PATH_MAX "$TMPDIR")"
[ "$path_max" -lt 0 ] && path_max=255

# Create a path that is as long as the system and suCGI permit.
if [ "$path_max" -lt 0 ]
	then max=4096
	else max="$path_max"
fi
long_path="$(mklongpath "$doc_root" "$((max - 1))")"
mkdir -p "$(dirname "$long_path")"
echo $$ >"$long_path"

# Create a path that is longer than the system permits.
huge_path="$(mklongpath "$doc_root" "$path_max")"
dirwalk "$doc_root" "$huge_path" 'mkdir "$fname"' 'echo $$ >"$fname"'

# Create a path that is longer than suCGI permits.
huge_str="$(mklongpath "$doc_root" "$PATH_SIZE")"
dirwalk "$doc_root" "$huge_str" 'mkdir "$fname"' 'echo $$ >"$fname"'

# Create a shortcut to the path that is longer than the system permits.
huge_path_link="$doc_root/$(dirwalk "$doc_root" "$huge_path" \
	'ln -s "$fname" p.d && printf p.d/' \
	'ln -s "$fname" p.f && printf p.f\\n')"

# Create a shortcut to the path that is longer than suCGI permits.
huge_str_link="$doc_root/$(dirwalk "$doc_root" "$huge_str" \
	'ln -s "$fname" s.d && printf s.d/' \
	'ln -s "$fname" s.f && printf s.f\\n')"

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

# Make sure the files are owned by the right user.
[ "$uid" -eq 0 ] && chown -R "$user:$group" "$TMPDIR"

# Were tests skipped?
skipped=


#
# Check the help dialogue.
#

check -o 'Print this help screen.' main -h


#
# Check the configuration dump.
#

check -o 'JAIL_DIR' main -c


#
# Check the version dump.
#

check -o 'suCGI' main -V


#
# Check the usage message.
#

check -s 1 -e 'usage: sucgi' main --no-long-opts 


#
# Too many environment variables.
#

# shellcheck disable=2086
check -s 1 -e'too many environment variables.' \
	$vars main


#
# Malformed environment variables.
#

tests_dir=$(cd -P "$script_dir" && pwd) ||
	err 'failed to get working directory.'

check -s1 -e'encountered malformed environment variable.' \
	badenv -n3 foo=foo '' bar=bar "$tests_dir/main"

check -s1 -e'encountered malformed environment variable.' \
	badenv -n3 bar=bar baz=baz foo "$tests_dir/main"

check -s1 -e"bad characters in variable name SSL_CLIENT_S_DN_ ." \
	badenv 'SSL_CLIENT_S_DN_ =bar' foo=foo baz=baz "$tests_dir/main"

#
# Verification of $DOCUMENT_ROOT (pre-privilege drop).
#

# No DOCUMENT_ROOT given.
check -s1 -e'$DOCUMENT_ROOT unset or empty.' \
	main

# $DOCUMENT_ROOT is empty.
check -s1 -e'$DOCUMENT_ROOT unset or empty.' \
	DOCUMENT_ROOT= main

# $DOCUMENT_ROOT is too long (suCGI).
check -s1 -e'$DOCUMENT_ROOT is too long.' \
	DOCUMENT_ROOT="$huge_str" main

# $DOCUMENT_ROOT is too long (system).
check -s1 -e'long' \
	DOCUMENT_ROOT="$huge_path" main

# Path to document root is too long after having been resolved (system).
check -s1 -e'long' \
	DOCUMENT_ROOT="$huge_path_link" main

# Path to document root is too long after having been resolved (suCGI).
check -s1 -e'long' \
	DOCUMENT_ROOT="$huge_str_link" main

# Path to document root points to outside of jail.
check -s1 -e"document root / not within jail." \
	DOCUMENT_ROOT=/ main

# Resolved document root points to outside of jail (dots).
check -s1 -e"document root / not within jail." \
	DOCUMENT_ROOT="$doc_root/../../../../../../../../../../../../.." main

# Resolved path to document roots to outside of document root (symlink).
check -s1 -e"document root / not within jail." \
	DOCUMENT_ROOT="$root_link" main

# Document root is of the wrong type.
check -s1 -e"open $outside: Not a directory." \
	DOCUMENT_ROOT="$outside" main

# Document root does not exist.
check -s1 -e"realpath <no file!>: No such file or directory." \
	DOCUMENT_ROOT="<no file!>" main

# Simple test. Should fail but regard DOCUMENT_ROOT as valid.
check -s1 -e'$PATH_TRANSLATED unset or empty.' \
	DOCUMENT_ROOT="$doc_root" main

# Long filename. Should fail but not because the name is too long.
long_doc_root="${long_path%??????????????}"
check -s1 -e"realpath $long_doc_root: No such file or directory" \
	DOCUMENT_ROOT="$long_doc_root" main

# DOCUMENT_ROOT remains mostly the same for the remainder of this script.
export DOCUMENT_ROOT="$doc_root"


#
# Verification of $PATH_TRANSLATED.
#

# PATH_TRANSLATED is undefined.
check -s1 -e'$PATH_TRANSLATED unset or empty.' \
	main

# $PATH_TRANSLATED is empty.
check -s1 -e'$PATH_TRANSLATED unset or empty.' \
	PATH_TRANSLATED= main

# $PATH_TRANSLATED is too long (system).
check -s1 -e'long' \
	PATH_TRANSLATED="$huge_path" main

# $PATH_TRANSLATED is too long (suCGI).
check -s1 -e'long' \
	PATH_TRANSLATED="$huge_str" main

# Path to script is too long after having been resolved (system).
check -s1 -e'long' \
	PATH_TRANSLATED="$huge_path_link" main

# Path to script is too long after having been resolved (suCGI).
check -s1 -e'long' \
	PATH_TRANSLATED="$huge_str_link" main

# Path to script points to outside of document root.
check -s1 -e"script $outside not within document root." \
	PATH_TRANSLATED="$outside" main

# Resolved path to script points to outside of document root (dots).
check -s1 -e"script $(dirname "$doc_root") not within document root." \
	PATH_TRANSLATED="$doc_root/../." main

# Resolved path to script points to outside of document root (symlink).
check -s1 -e"script $outside not within document root." \
	PATH_TRANSLATED="$out_link" main

# Script is of the wrong type.
check -s1 -e"script $dir is not a regular file." \
	PATH_TRANSLATED="$dir" main

# Script does not exist.
check -s1 -e"realpath $doc_root/<no file!>: No such file or directory." \
	PATH_TRANSLATED="$doc_root/<no file!>" main

# Simple test. Should fail but regard PATH_TRANSLANTED as valid.
# shellcheck disable=2046
if [ "$user" = "$(id -un 0)" ]
then
	err="script $inside is owned by privileged user $user."
elif [ "$uid" -eq 0 ]
then
	err="$inside has no filename suffix."
elif inlist -eq 0 $(id -G "$user")
then
	err="user $user belongs to privileged group 0."
else
	err='seteuid: Operation not permitted.'
fi
check -s1 -e"$err" \
	PATH_TRANSLATED="$inside" main

# Symlink into jail. Should fail but regard PATH_TRANSLATED as valid.
check -s1 -e"$err" \
	PATH_TRANSLATED="$in_link" main

# Long filename. Should fail but not because the name is too long.
long_path_trans="${long_path%????????????????}"
check -s1 -e"realpath $long_path_trans: No such file or directory." \
	PATH_TRANSLATED="$long_path_trans" main


#
# Simple tests for script ownership.
#

check -s1 -e"script $env_bin is owned by privileged user root" \
	DOCUMENT_ROOT="$env_dir" PATH_TRANSLATED="$env_bin" main


#
# Abort unless run by the superuser.
#

if [ "$uid" -ne 0 ]
then
	warn -y 'all non-root tests passed.'
	exit
fi


#
# Interlude
#

# Get the user's ID and group.
uid="$(id -u "$user")"
gid="$(id -g "$user")"

# Create a file without an owner.
unalloc_uid="$(unallocid -u 1000 30000)"
noowner="$doc_root/noowner"
touch "$noowner"
chown "$unalloc_uid" "$noowner"

# Create a file owned by root.
root_owned="$doc_root/priv-root"
touch "$root_owned"

# Create a file owned by a non-root privileged user with a low UID.
if low_uid="$(ents -f1 -c500 -n1)"
then
	low_uid_owned="$doc_root/priv-low-uid"
	touch "$low_uid_owned"
	chown "${low_uid#*:}" "$low_uid_owned"
else
	warn -y "no user with an ID < 500."
	skipped=x
fi

# Create a file owned by a non-root privileged user with a high UID.
if high_uid="$(ents -f60000 -n1 2>/dev/null)"
then
	high_uid_owned="$doc_root/priv-high-uid"
	touch "$high_uid_owned"
	chown "${high_uid#*:}" "$high_uid_owned"
else
	warn -y "no user with an ID > 60,000."
	skipped=x
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
printf 'uid=%s egid=%s ruid=%s rgid=%s\n' \
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
[ -x "$script" ] && "$script" >/dev/null 2>&1 || script=

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
[ -x "$env" ] && "$env" >/dev/null 2>&1 || env=


#
# Verification of owner 
#

# Owned by non-existing user.
check -s1 -e"script $noowner is owned by unallocated UID $unalloc_uid." \
	PATH_TRANSLATED="$noowner" main

# Owned by root.
check -s1 -e"script $root_owned is owned by privileged user" \
	PATH_TRANSLATED="$root_owned" main

# Owned by a non-root privileged user with a low UID.
if [ "$low_uid" ]
then
	check -s1 -e"script $low_uid_owned is owned by privileged user" \
		PATH_TRANSLATED="$low_uid_owned" main
fi

# Owned by a non-root privileged user with a high UID.
if [ "$high_uid" ]
then
	check -s1 -e"script $high_uid_owned is owned by privileged user" \
		PATH_TRANSLATED="$high_uid_owned" main
fi


#
# Bail out if there is no unprivileged user.
#

if [ "$user" = "$(id -un 0)" ]
then
	warn -y "skipping post-privilege drop tests."
	exit 75
fi


#
# Verification of DOCUMENT_ROOT (post-privilege drop)
#

# DOCUMENT_ROOT is not USER_DIR.
check -s1 -e"document root $TMPDIR is not $user's user directory." \
	DOCUMENT_ROOT="$TMPDIR" PATH_TRANSLATED="$script_sh" main


#
# Not a hidden file.
#

for hidden in "$hidden_file" "$hidden_dir"
do
	check -s1 -e"path $hidden contains hidden files." \
		PATH_TRANSLATED="$hidden" main
done

#
# Neither set-UID nor set-GID bit set.
#

check -s1 -e"script $setuid's set-user-ID bit is set." \
	PATH_TRANSLATED="$setuid" main

check -s1 -e"script $setgid's set-group-ID bit is set." \
	PATH_TRANSLATED="$setgid" main


#
# Verification of exclusive write-access.
#

for file in "$groupw_file" "$otherw_file"
do
	check -s1 -e"$file is writable by users other than $user." \
		PATH_TRANSLATED="$file" main
done

for file in "$groupw_dir" "$otherw_dir"
do
	dir="$(dirname "$file")"
	check -s1 -e"$dir is writable by users other than $user." \
		PATH_TRANSLATED="$file" main
done


#
# Check suffix errors.
#

check -s1 -e"$suffix_none has no filename suffix." \
	PATH_TRANSLATED="$suffix_none" main

check -s1 -e"no interpreter registered for $suffix_unknown." \
	PATH_TRANSLATED="$suffix_unknown" main


#
# Check whether privileges have been dropped.
#

for path in "$script" "$script_sh"
do
	if ! [ "$path" ]
	then
		skipped=y
		continue
	fi

	check -s0 -o "uid=$uid egid=$gid ruid=$uid rgid=$gid" \
		PATH_TRANSLATED="$path" main
done


#
# Check whether the environment has been cleared.
#

for path in "$env" "$env_sh"
do
	if ! [ "$path" ]
	then
		skipped=y
		continue
	fi

	warn "checking ${bld-}PATH_TRANSLATED=$env_sh foo=foo main${rst-} ..."

	PATH_TRANSLATED="$path" foo=foo main |
	grep -Fq foo= && err -l "environment was not cleared."
done


#
# All done
#

if [ "$skipped" ]
then
	warn -y "some tests were skipped."
	exit 75
else
	warn -g 'all tests passed.'
fi
