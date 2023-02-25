#!/bin/sh
#
# Test sucgi via main.
#
# Copyright 2022 Odin Kroeger.
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
# Functions
#

# This is hack to make coverage reports more accurate.
covown() {
	find "$src_dir" '(' -name '*.gcda' -o -name '*.gcno' ')' \
	                -exec chown "${1:?}" '{}' +
}


#
# Prelude
#

# Load build configuration.
eval "$(main -C | grep -vE '^PATH=')"

# Get the effective UID.
uid="$(id -u)"

# Find a regular user.
if [ "$uid" -eq 0 ]
then
	if user="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")"
	then
		# This is a hack to make coverage reports more accurate.
		owner="$(owner "$src_dir")"
		readonly owner
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

# Get the maximum path length that suCGI permits.
long_path="$(mklongpath "$doc_root" "$((MAX_FNAME_LEN - 1))")"
mkdir -p "$(dirname -- "$long_path")"
echo $$ >"$long_path"

# Create a path that is longer than suCGI permits.
huge_path="$(mklongpath "$doc_root" "$MAX_FNAME_LEN")"
dirwalk "$doc_root" "$huge_path" 'mkdir "$fname"' 'echo $$ >"$fname"'

# Create a shortcut to that path.
huge_path_link="$doc_root/$(dirwalk "$doc_root" "$huge_path" \
	'ln -s "$fname" s.d && printf s.d/' \
	'ln -s "$fname" s.f && printf s.f\\n')"

# Create a directory inside the document root.
dir="$doc_root/dir"
mkdir "$dir"

# Create a file inside the document root.
inside="$doc_root/inside"
echo $$ >"$inside"

# Create a link to the inside
in_link="$TMPDIR/to-inside"
ln -s "$inside" "$in_link"

# Locate env
env_bin="$(command -v env)"
env_dir="$(dirname -- "$env_bin")"

# Make sure the files are owned by the right user.
[ "$uid" -eq 0 ] && chown -R "$user:$group" "$TMPDIR"

# Were tests skipped?
skipped=



#
# Check whether suCGI stops if no arguments are given.
#

check -s1 -e'empty argument vector' badexec main
check -s1 -e'empty argument vector' badexec main ''


#
# Check the help dialogue.
#

check -o'Print this help screen.' main -h


#
# Check the configuration dump.
#

check -o'USER_DIR' main -C


#
# Check the version dump.
#

check -o'suCGI' main -V


#
# Check the usage message.
#

for arg in '' -X -XX -x --x - --
do
	check -s1 -e'usage: sucgi' main "$arg"
done

check -s1 -e'usage: sucgi' main -h -C
check -s1 -e'usage: sucgi' main -hV


#
# Verification of $PATH_TRANSLATED (pre-privilege drop).
#

# PATH_TRANSLATED is undefined.
check -s1 -e'$PATH_TRANSLATED not set.' \
	main

# $PATH_TRANSLATED is empty.
check -s1 -e'$PATH_TRANSLATED is empty.' \
	PATH_TRANSLATED= main

# $PATH_TRANSLATED is too long.
check -s1 -e'long' \
	PATH_TRANSLATED="$huge_path" main

# Path to script is too long after having been resolved.
check -s1 -e'long' \
	PATH_TRANSLATED="$huge_path_link" main

# Script is of the wrong type.
check -s1 -e"$dir is not a regular file." \
	PATH_TRANSLATED="$dir" main

# Script does not exist.
check -s1 -e"realpath $doc_root/<no file!>: No such file or directory." \
	PATH_TRANSLATED="$doc_root/<no file!>" main


#
# $PATH_TRANSLATED is valid.
#

err='seteuid: Operation not permitted.'
[ "$uid" -eq 0 ] && err="has no filename suffix."

# Simple test. Should fail but regard PATH_TRANSLANTED as valid.
check -s1 -e"$err" PATH_TRANSLATED="$inside" main

# Symlink into jail. Should fail but regard PATH_TRANSLATED as valid.
check -s1 -e"$err" PATH_TRANSLATED="$in_link" main

# Long filename. Should fail but not because the name is too long.
long_path_trans="${long_path%????????????????}"
check -s1 -e"realpath $long_path_trans: No such file or directory." \
	PATH_TRANSLATED="$long_path_trans" main


#
# Simple tests for script ownership.
#

check -s1 -e"$env_bin is owned by privileged user root" \
	PATH_TRANSLATED="$env_bin" main


#
# Abort unless run by the superuser.
#

if [ "$uid" -ne 0 ]
then
	warn 'all non-root tests passed.'
	exit
fi


#
# Interlude
#

# Get the user's ID and group.
uid="$(id -u "$user")"
gid="$(id -g "$user")"

# Create a file outside the document root.
outside="$TMPDIR/outside"
touch "$outside"
chown "$user" "$outside"

# Create a link to the outside.
out_link="$doc_root/outside"
ln -s "$outside" "$out_link"

# Create a directory without an owner.
unalloc_uid="$(unallocid 1000 30000)"
noowner="$doc_root/noowner"
touch "$noowner"
chown "$unalloc_uid" "$noowner"

# Create a directory owned by root.
root_owned="$doc_root/priv-root"
touch "$root_owned"

# Store IDs.
uids="$TMPDIR/uids.list"
tools/ids >"$uids"

# Create a file owned by a non-root privileged user with a low UID.
if low_uid="$(awk '0 < $1 && $1 < 500 {print $2; exit}' "$uids")"
then
	low_uid_owned="$doc_root/priv-low-uid"
	touch "$low_uid_owned"
	chown "${low_uid#*:}" "$low_uid_owned"
else
	warn "no user with an ID < 500."
	skipped=x
fi

# Create a file owned by a non-root privileged user with a high UID.
if high_uid="$(awk '$1 > 30000 {print $2; exit}' "$uids")"
then
	high_uid_owned="$doc_root/priv-high-uid"
	touch "$high_uid_owned"
	chown "${high_uid#*:}" "$high_uid_owned"
else
	warn "no user with an ID > 60,000."
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
if ! [ -x "$script" ] || ! "$script" >/dev/null 2>&1
then
	warn "cannot execute test script."
	skipped=x script=
fi

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
if ! [ -x "$env" ] || ! "$env" >/dev/null 2>&1
then
	warn "cannot execute environment wrapper."
	skipped=x env=
fi


#
# Verification of owner
#

# Owned by non-existing user.
check -s1 -e"$noowner has no owner." \
	PATH_TRANSLATED="$noowner" main

# Owned by root.
check -s1 -e"$root_owned is owned by privileged user" \
	PATH_TRANSLATED="$root_owned" main

# Owned by a non-root privileged user with a low UID.
if [ "$low_uid" ]
then
	check -s1 -e"$low_uid_owned is owned by privileged user" \
		PATH_TRANSLATED="$low_uid_owned" main
fi

# Owned by a non-root privileged user with a high UID.
if [ "$high_uid" ]
then
	check -s1 -e"$high_uid_owned is owned by privileged user" \
		PATH_TRANSLATED="$high_uid_owned" main
fi


#
# Bail out if there is no unprivileged user.
#

[ "$user" = "$(id -un 0)" ] && err -s75 "skipping post-privilege drop tests."


#
# Verification of PATH_TRANSLATED (post-privilege drop)
#

check -s1 -e"$outside is not within $user's user directory." \
	PATH_TRANSLATED="$outside" main

check -s1 -e"$out_link is not within $user's user directory." \
	PATH_TRANSLATED="$out_link" main


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
	dir="$(dirname -- "$file")"
	check -s1 -e"$file is writable by users other than $user." \
		PATH_TRANSLATED="$file" main
done


#
# Set-UID or set-GID bit set.
#

check -s1 -e"$setuid's set-user-ID bit is set." \
	PATH_TRANSLATED="$setuid" main

check -s1 -e"$setgid's set-group-ID bit is set." \
	PATH_TRANSLATED="$setgid" main


#
# Hidden file.
#

for hidden in "$hidden_file" "$hidden_dir"
do
	check -s1 -e"path $hidden contains hidden files." \
		PATH_TRANSLATED="$hidden" main
done


#
# Check suffix errors.
#

check -s1 -e"$suffix_none has no filename suffix." \
	PATH_TRANSLATED="$suffix_none" main

check -s1 -e"no handler for $suffix_unknown's filename suffix." \
	PATH_TRANSLATED="$suffix_unknown" main


#
# Check whether privileges have been dropped.
#


for path in "$script" "$script_sh"
do
	[ "$path" ] || continue
	check -s0 -o"uid=$uid egid=$gid ruid=$uid rgid=$gid" \
		PATH_TRANSLATED="$path" main
done


#
# Check whether the environment has been cleared.
#

for path in "$env" "$env_sh"
do
	[ "$path" ] || continue
	warn "checking PATH_TRANSLATED=$env_sh foo=foo main ..."

	PATH_TRANSLATED="$path" foo=foo main |
	grep -Fq foo= && err -l "environment was not cleared."
done


#
# All done
#

if [ "$skipped" ]
then
	err -s75 "some tests were skipped."
else
	warn 'all tests passed.'
fi
