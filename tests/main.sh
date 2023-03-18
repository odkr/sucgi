#!/bin/sh
#
# Test suCGI.
#
# Copyright 2023 Odin Kroeger.
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
tests_dir="$src_dir/tests"
readonly script_dir src_dir tests_dir
# shellcheck disable=1091
. "$src_dir/scripts/funcs.sh" || exit
init || exit
# shellcheck disable=2119
tmpdir


#
# Globals
#

# Overall result.
result=0

# Number of skipped tests.
nskipped=0


#
# Functions
#

# Check if suCGI might have been compiled against musl.
maybemusl() {
	[ "${LIBC-}" ] && return 1;

	case ${CC-} in (musl-*)
		return 0
	esac

	[ "${OS-}" = Linux ]
}

# Delete every segment of $path up to $stop. Should ignore PATH_MAX.
# shellcheck disable=2317
rmtree() (
	path="${1:?}" stop="${2:?}"

	while true
	do
		case $path in
		($stop/*)	: ;;
		(*)		break ;;
		esac

		dirwalk "$stop" "$path" : 'rm -rf "$fname"'

		path="$(dirname "$path")"
	done
)


#
# Build configuration
#

eval "$(main -C | grep -vE '^PATH=')"


#
# Common setup for non-privileged tests
#

# User ID.
uid="$(id -u)"


#
# Environment sanitisation
#

# Too many variables.
i=0 envsan_vars=
while [ "$i" -le "$MAX_NVARS" ]
do
	i=$((i + 1))
	envsan_vars="$envsan_vars var$i="
done
unset i

# shellcheck disable=2086
check -s1 -e'too many environment variables.' \
      $envsan_vars main		|| result=70

# Variable name has maximum length.
envsan_varname="$(pad v $((MAX_VARNAME_LEN - 1)))"
check -s1 -e"discarding \$$envsan_varname" \
      "$envsan_varname=" main	|| result=70

# Variable name is too long.
envsan_varname="$(pad v "$MAX_VARNAME_LEN")"
envsan_var="$envsan_varname=foo"
check -s1 -e'discarding variable with overly long name.' \
      "$envsan_var" main	|| result=70

# Variable has maximum length.
envsan_var="$(pad var= $((MAX_VAR_LEN - 1)) x r)"
check -s1 -e"discarding \$${envsan_var%=*}" \
      "$envsan_var" main	|| result=70

# Variable is too long.
envsan_var="$(pad var= "$MAX_VAR_LEN" x r)"
check -s1 -e'discarding overly long variable.'	\
      "$envsan_var" main	|| result=70

# Variable name is illegal.
for envsan_varname in 'foo ' '0foo' '$(echo foo)' '`echo foo`'
do
	check -s1 -e"variable \$$envsan_varname: bad name." \
	      badenv "$envsan_varname=" "$tests_dir/main"	|| result=70
done

# Variable is not of the form <key>=<value>.
for envsan_var in 'foo' '0foo' 'foo '
do
	check -s1 -e"variable \$$envsan_var: no value." \
	      badenv -n1 "$envsan_var" "$tests_dir/main"	|| result=70
done

# Not a CGI variable.
check -s1 -e'discarding $foo' foo=foo main || result=70

# Cleanup.
unset envsan_var envsan_varname envsan_vars



#
# Missing argv[0]
#

check -s1 -e'empty argument vector' badexec main	|| result=70
check -s1 -e'empty argument vector' badexec main ''	|| result=70


#
# Option handling
#

# Help.
check -o'Print this help screen.'	main -h		|| result=70

# Version.
check -o'suCGI v'			main -V		|| result=70

# Build configuration.
sed -n 's/#define \([[:alnum:]_]*\).*/\1/p' "$src_dir/config.h"	|
sort -u								|
while read -r opthdl_var
do
	[ "$opthdl_var" != CONFIG_H ] 	&&
	check -o"$opthdl_var" main -C	||
	result=70
done

# Usage message.
for opthdl_args in -X -XX -x --x - -- '-h -C' '-hC' '-h -V' '-hC'
do
	# shellcheck disable=2086
	check -s1 -e'usage: sucgi' main $opthdl_args	|| result=70
done

check -s1 -e'usage: sucgi' main ''			|| result=70

# Cleanup.
unset opthdl_args opthdl_var


#
# Script filename validation
#

# Create a path that is as long as suCGI permits.
scrsan_varname='PATH_TRANSLATED='
scrsan_maxlen="$((MAX_VAR_LEN - ${#scrsan_varname}))"
[ "$scrsan_maxlen" -gt "$MAX_FNAME_LEN" ] && scrsan_maxlen="$MAX_FNAME_LEN"
scrsan_longpath="$(mklongpath "$TMPDIR" "$((scrsan_maxlen - 1))")"
mkdir -p "$(dirname -- "$scrsan_longpath")"
touch "$scrsan_longpath"
unset scrsan_varname scrsan_maxlen

# Create a path that is longer than suCGI permits.
scrsan_hugepath="$(mklongpath "$TMPDIR" "$MAX_FNAME_LEN")"
readonly scrsan_hugepath
catch=
dirwalk "$TMPDIR" "$scrsan_hugepath" 'mkdir "$fname"' 'touch "$fname"'
cleanup="rmtree \"$scrsan_hugepath\" \"$TMPDIR\"; ${cleanup-}"
catch=x
[ "$caught" ] && kill "-$caught" $$

# Create a link to that path.
catch=
scrsan_hugelink="$TMPDIR/$(dirwalk "$TMPDIR" "$scrsan_hugepath" \
	'ln -s "$fname" s.d && printf s.d/' \
	'ln -s "$fname" s.f && printf s.f\\n')"
readonly scrsan_hugelink
cleanup="rmtree \"$scrsan_hugelink\" \"$TMPDIR\"; ${cleanup-}"
catch=x
[ "$caught" ] && kill "-$caught" $$

# Create a file of the wrong type.
scrsan_wrongftype="$TMPDIR/scrsan-wrongftype"
mkdir "$scrsan_wrongftype"

# Create a valid script.
scrsan_script="$TMPDIR/scrsan-valid.php"
touch "$scrsan_script"

# PATH_TRANSLATED is undefined.
check -s1 -e'$PATH_TRANSLATED not set.'	 main			|| result=70

# $PATH_TRANSLATED is empty.
check -s1 -e'$PATH_TRANSLATED is empty.' PATH_TRANSLATED= main	|| result=70

# $PATH_TRANSLATED is too long.
check -s1 -e'long'	PATH_TRANSLATED="$scrsan_hugepath" main	|| result=70

# Path to script is too long after having been resolved.
check -s1 -e'long'	PATH_TRANSLATED="$scrsan_hugelink" main	|| result=70

# Script is of the wrong type.
check -s1 -e"$scrsan_wrongftype is not a regular file." \
	PATH_TRANSLATED="$scrsan_wrongftype" main	|| result=70

# Script does not exist.
check -s1 -e"realpath $TMPDIR/<nosuchfile>: No such file or directory." \
	PATH_TRANSLATED="$TMPDIR/<nosuchfile>" main	|| result=70

# PATH_TRANSLATED is valid.
case $uid in
(0) scrsan_err="$scrsan_script is owned by privileged user root." ;;
(*) scrsan_err='seteuid: Operation not permitted.' ;;
esac
check -s1 -e"$scrsan_err" PATH_TRANSLATED="$scrsan_script" main || result=70

# $PATH_TRANSLATED is valid despite its long name.
if maybemusl
	nskipped=$((nskipped + 1))
then
	case $uid in
	(0) scrsan_err="$scrsan_longpath is owned by privileged user root." ;;
	(*) scrsan_err='seteuid: Operation not permitted.' ;;
	esac

	check -s1 -e"$scrsan_err" PATH_TRANSLATED="$scrsan_longpath" main ||
	result=70
fi


# Cleanup.
unset scrsan_err scrsan_longpath scrsan_script scrsan_wrongftype


#
# Stop unless run by the superuser
#

if [ "$uid" -ne 0 ]
then
	case $result in
	(0) err -s75 'skipping superuser tests.' ;;
	(*) err -s"$result" 'skipping superuser tests.' ;;
	esac
fi

#
# User validation
#

# Create a script.
usrval_script="$TMPDIR/usrval-script.sh"
touch "$usrval_script"

# Find an unallocated user ID.
usrval_unallocuid="$(unallocid "$MIN_UID" "$MAX_UID")"

# Store a list of all user IDs.
usrval_uids="$TMPDIR/usrval-uids.list"
ids >"$usrval_uids"

# Find a user with an ID < $MIN_UID
usrval_lowuid="$(
	awk -vmax="$MIN_UID" \
	    '0 < $1 && $1 < max {print $2; exit}' "$usrval_uids"
)"

# Find a user with an ID > $MAX_UID
usrval_highuid="$(
	awk -vmin="$MAX_UID" \
	    '$1 > min {print $2; exit}' "$usrval_uids"
)"

# Owned by non-existing user.
chown "$usrval_unallocuid" "$usrval_script"
check -s1 -e"$usrval_script has no owner." \
      PATH_TRANSLATED="$usrval_script" main || result=70

# Owned by a privileged user
for usrval_uid in 0 "$usrval_lowuid" "$usrval_highuid"
do
	[ "$usrval_uid" ] || continue

	chown "$usrval_uid" "$usrval_script"
	check -s1 -e"$usrval_script is owned by privileged user" \
		PATH_TRANSLATED="$usrval_script" main || result=70
done

# Cleanup.
unset usrval_script usrval_uid usrval_lowuid usrval_highuid \
      usrval_uids usrval_unallocuid


#
# Common setup for privileged tests
#

# Search for a regular user.
reguser="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")"

if ! [ "$reguser" ]
then
	case $result in
	(0) err -s75 'no regular user found.' ;;
	(*) err -s"$result" 'no regular user found.' ;;
	esac
fi
reguid="$(id -u "$reguser")"
reggid="$(id -g "$reguser")"

# Determine user directory.
# shellcheck disable=2059
case $USER_DIR in
(/*%s*) userdir="$(printf -- "$USER_DIR" "$reguser")" ;;
(*%s*)	err 'user directories within home directories are unsupported.' ;;
(*)	userdir="$USER_DIR/$reguser" ;;
esac

# Create another temporary directory.
tmpdir="${userdir%/"$reguser"*}"
case $tmpdir in
(/tmp/*)	: ;;
(*)		err 'temporary directory %s is outside of /tmp.' "$tmpdir"
esac
readonly tmpdir

catch=
mkdir -m 0755 "$tmpdir"
cleanup="rm -rf \"\$tmpdir\"; ${cleanup}"
catch=x
[ "$caught" ] && kill -"$caught" $$

TMPDIR="$tmpdir"
export TMPDIR

# Create the user directory.
mkdir -p "$userdir"
userdir="$(cd -P "$userdir" && pwd)" && [ "$userdir" ] || exit
readonly userdir

# Create a script that prints process IDs.
readonly procids_sfx="$userdir/ids.sh"
cat <<'EOF' >"$procids_sfx"
#!/bin/sh
printf 'uid=%s egid=%s ruid=%s rgid=%s\n' \
       "$(id -u)" "$(id -g)" "$(id -ur)" "$(id -gr)"
EOF
chmod -x "$procids_sfx"

# The same script w/o a suffix.
readonly procids_nosfx="$userdir/ids"
cp "$procids_sfx" "$procids_nosfx"
chmod +x "$procids_nosfx"

# Create a script that prints the environment.
readonly penv_sfx="$userdir/penv.sh"
cat <<'EOF' >"$penv_sfx"
#!/bin/sh
env
EOF
chmod -x "$penv_sfx"

# The same script w/o a suffix.
readonly penv_nosfx="$userdir/penv"
cp "$penv_sfx" "$penv_nosfx"
chmod +x "$penv_nosfx"

# Adapt ownership.
chown -R "$reguser" "$userdir"


#
# Privilege dropping
#

for priv_script in "$procids_sfx" "$procids_nosfx"
do
	# /tmp might be mounted noexec.
	if [ -x "$priv_script" ] && ! $priv_script >/dev/null 2>&1
	then
		nskipped=$((nskipped + 1))
		continue
	fi

	check -s0 -o"uid=$reguid egid=$reggid ruid=$reguid rgid=$reggid" \
	      PATH_TRANSLATED="$priv_script" main  || result=70
done
unset priv_script


#
# User directory validation
#

# Create a link from outside to inside the user directory.
dirval_outtoin="$tmpdir/dirval-out-to-in.sh"
ln -s "$procids_sfx" "$dirval_outtoin"

# Create a script outside the user directory.
dirval_outside="$tmpdir/dirval-outside.sh"
touch "$dirval_outside"
chown "$reguser" "$dirval_outside"

# Create a link from inside to outside the user directory.
dirval_intoout="$userdir/in-to-out.sh"
ln -s "$dirval_outside" "$dirval_intoout"

# Forbidden location.
for dirval_forb in "$dirval_outside" "$dirval_intoout"
do
	check -s1 -e"script $dirval_forb: not within $reguser's user directory." \
	      PATH_TRANSLATED="$dirval_forb" main || result=70
done

# Permitted location.
for dirval_perm in "$procids_sfx" "$dirval_outtoin"
do
	check -s0 -o"uid=$reguid egid=$reggid ruid=$reguid rgid=$reggid" \
	      PATH_TRANSLATED="$dirval_perm" main || result=70
done

# Cleanup.
unset dirval_outtoin dirval_outside dirval_intoout dirval_forb dirval_perm


#
# Exclusive write access
#

# Create a file in the user directory.
exclw_shallow="$userdir/exclw-file"
touch "$exclw_shallow"

# Create a file in a sub-directory.
exclw_subdir="$userdir/exclw-dir"
mkdir "$exclw_subdir"
exclw_deeper="$exclw_subdir/file"
touch "$exclw_deeper"

# Change ownership.
chown -R "$reguser" "$userdir"

# Not exclusively writable.
exclw_no="u=r,g=w,o= u=r,g=,o=w u=rw,g=w,o= u=rw,g=,o=w u=rw,go=w u=rw,go=w"
for exclw_mode in $exclw_no
do
	chmod "$exclw_mode" "$exclw_shallow"
	check -s1 -e"script $exclw_shallow: writable by users other than $reguser" \
	      PATH_TRANSLATED="$exclw_shallow" main || result=70

	chmod "$exclw_mode" "$exclw_subdir"
	chmod u+x "$exclw_subdir"
	check -s1 -e"script $exclw_deeper: writable by users other than $reguser" \
	      PATH_TRANSLATED="$exclw_deeper" main || result=70
done

# Exclusively writable.
exclw_yes="u=rw,go= ugo=r"
for exclw_mode in $exclw_yes
do
	chmod go-w "$exclw_subdir" "$exclw_shallow" "$exclw_deeper"

	chmod "$exclw_mode" "$exclw_shallow"
	check -s1 -e"script $exclw_shallow: no handler found." \
	      PATH_TRANSLATED="$exclw_shallow" main || result=70

	chmod "$exclw_mode" "$exclw_subdir"
	chmod u+wx,go= "$exclw_subdir"
	check -s1 -e"script $exclw_deeper: no handler found." \
	      PATH_TRANSLATED="$exclw_deeper" main || result=70

	chmod g+w,o= "$exclw_deeper"
	check -s1 -e"script $exclw_deeper: writable by users other than $reguser" \
	      PATH_TRANSLATED="$exclw_deeper" main || result=70

	chmod g=,o+w "$exclw_deeper"
	check -s1 -e"script $exclw_deeper: writable by users other than $reguser" \
	      PATH_TRANSLATED="$exclw_deeper" main || result=70
done

# Cleanup.
unset exclw_shallow exclw_deeper exclw_subdir exclw_yes exclw_no excl_mode


#
# Set-process-IDs on execute bits
#

# Set-user-ID on execute bit set.
chmod u=rws,g=r,o=r "$procids_sfx"
check -s1 -e"script $procids_sfx: set-user-ID on execute bit is set." \
      PATH_TRANSLATED="$procids_sfx" main || result=70

# Set-group-ID on execute bit set.
chmod u=rx,g=rs,o=r "$procids_sfx"
check -s1 -e"script $procids_sfx: set-group-ID on execute bit is set." \
      PATH_TRANSLATED="$procids_sfx" main || result=70

# Cleanup.
chmod u=rwx,go= "$procids_sfx"


#
# Hidden scripts
#

# Create a hidden script.
hidden_file="$userdir/.hidden.sh"
touch "$hidden_file"

# Create a script in a hidden directory.
hidden_dir="$userdir/.hidden"
mkdir "$hidden_dir"
hidden_indir="$hidden_dir/script.sh"
touch "$hidden_indir"

# Adapt ownership.
chown -R "$reguser" "$userdir"

# Test.
for hidden_script in "$hidden_file" "$hidden_indir"
do
	check -s1 -e"path $hidden_script: contains hidden files." \
		PATH_TRANSLATED="$hidden_script" main || result=70
done

# Cleanup.
unset hidden_file hidden_dir hidden_indir hidden_script


#
# Script handler
#

# Create a non-executable script w/o a suffix.
hdl_nosuffix="$userdir/script"
touch "$hdl_nosuffix"

# Create a script with an unknown suffix.
hdl_unknownsuffix="$userdir/script.nil"
touch "$hdl_unknownsuffix"

# Create a script for which an empty handler is registered.
hdl_emptyhandler="$userdir/script.empty"
touch "$hdl_emptyhandler"

# Create a script with a too long filename suffix.
hdl_suffix=$(pad . "$MAX_SUFFIX_LEN" x r)
hdl_hugesuffix="$userdir/script$hdl_suffix"
touch "$hdl_hugesuffix"

# Adapt ownership.
chown -R "$reguser" "$userdir"

# Bad handler.
check -s1 -e"script $hdl_emptyhandler: bad handler." \
      PATH_TRANSLATED="$hdl_emptyhandler" main || result=70

# Suffix is too long.
check -s1 -e"script $hdl_hugesuffix: filename suffix too long." \
	PATH_TRANSLATED="$hdl_hugesuffix" main || result=70

# No known suffix.
for hdl_script in "$hdl_nosuffix" "$hdl_unknownsuffix"
do
	check -s1 -e"script $hdl_script: no handler found." \
		PATH_TRANSLATED="$hdl_script" main || result=70
done

# Cleanup.
unset hdl_nosuffix hdl_unknownsuffix hdl_emptyhandler \
      hdl_suffix hdl_hugesuffix hdl_script


#
# Environment cleanup.
#

# Get home directory.
eval "envcln_homedir=~$reguser"

for envcln_script in "$penv_sfx" "$penv_nosfx"
do
	# /tmp might be mounted noexec.
	if [ -x "$envcln_script" ] && ! $envcln_script >/dev/null 2>&1
	then
		nskipped=$((nskipped + 1))
		continue
	fi

	envcln_logfile="$envcln_script.log"

	PATH_TRANSLATED="$envcln_script"		\
	CLICOLOR='x'					\
	DOCUMENT_ROOT='/home/jdoe/public_html'		\
	EDITOR='vim'					\
	HTTP_HOST='www.foo.example'			\
	HTTP_REFERER='https://www.bar.example'		\
	HTTP_USER_AGENT='FakeZilla/1'			\
	HTTPS='on'					\
	IFS=':' 					\
	LANG='en_GB.UTF-8'				\
	LOGNAME='jdoe'					\
	PAGER='less'					\
	PWD='/home/jdoe'				\
	QUERY_STRING='foo=bar'				\
	REMOTE_ADDR='100::1:2:3'			\
	REMOTE_HOST='100::1:2:3'			\
	REMOTE_PORT=50000				\
	REQUEST_METHOD='GET'				\
	REQUEST_URI='/index.php'			\
	SCRIPT_NAME='index.php'				\
	SERVER_ADMIN='admin@foo.example'		\
	SERVER_NAME='www.foo.example'			\
	SERVER_PORT=443					\
	SERVER_SOFTWARE='Apache v2.4' 			\
	USER='jdoe'					\
	VISUAL='vim'					\
	main >"$envcln_logfile" 2>/dev/null		||
	err -s70 'main exited with status %d.' $?

	# shellcheck disable=2154
	for envcln_var in				\
		"PATH_TRANSLATED=$envcln_script"	\
		"DOCUMENT_ROOT=$userdir"		\
		"HOME=$envcln_homedir"			\
		'HTTPS=on'				\
		'HTTP_HOST=www.foo.example'		\
		'HTTP_REFERER=https://www.bar.example'	\
		'HTTP_USER_AGENT=FakeZilla/1'		\
		'PATH=/usr/bin:/bin'			\
		'QUERY_STRING=foo=bar'			\
		'REMOTE_ADDR=100::1:2:3'		\
		'REMOTE_HOST=100::1:2:3'		\
		'REMOTE_PORT=50000'			\
		'REQUEST_METHOD=GET'			\
		'REQUEST_URI=/index.php'		\
		"SCRIPT_FILENAME=$envcln_script"	\
		'SCRIPT_NAME=index.php'			\
		'SERVER_ADMIN=admin@foo.example'	\
		'SERVER_NAME=www.foo.example'		\
		'SERVER_PORT=443'			\
		'SERVER_SOFTWARE=Apache v2.4'		\
		"USER_NAME=$reguser"
	do
		if ! grep -Eq "^$envcln_var$" "$envcln_logfile"
		then
			warn 'PATH_TRANSLATED=%s [...] main: $%s: not set.' \
	     		     "$envcln_script" "$envcln_var"
			result=70
		fi
	done

	for envcln_var in				\
		CLICOLOR=				\
		EDITOR=					\
		IFS=					\
		LANG=					\
		LOGNAME=				\
		PAGER=					\
		USER=jdoe				\
		VISUAL=
	do
		if grep -Eq "^$envcln_var" "$envcln_logfile"
		then
			warn 'PATH_TRANSLATED=%s [...] main: $%s: set.' \
	     		     "$envcln_script" "$envcln_var"
			result=70
		fi
	done
done

# Cleanup.
unset envcln_homedir envcln_script envcln_logfile envcln_var



#
# Success
#

case $result in
(0) [ "$nskipped" -gt 0 ] && err -s75 'skipped %d tests.' "$nskipped" ;;
(*) [ "$nskipped" -gt 0 ] && err -s"$result" 'skipped %d tests.' "$nskipped" ;;
esac

exit "$result"
