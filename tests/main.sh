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
. "$tests_dir/funcs.sh" || exit
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

# Starting with, but excluding, $dirname run $dircmd for every path segment
# of $fname. If $fcmd is given, run $dircmd for every path segment up to,
# but excluding $fname, and run $fcmd for $fname.
dirwalk() (
	dirname="${1:?}" fname="${2:?}" dircmd="${3:?}" fcmd="${4:-"$3"}"

	IFS=/
	# shellcheck disable=2086
	set -- ${fname#"${dirname%/}/"}
	unset IFS
	cd "$dirname" || exit

	while [ "$#" -gt 0 ]
	do
		fname="$1"
		shift
		[ "$fname" ] || continue

		case $# in
		(0)
			eval "$fcmd"
			return
			;;
		(*)
			eval "$dircmd"
			dirname="${dirname%/}/$fname"
			cd "$fname" || return
			;;
		esac
	done
)

# Create a path that is $len characters long in $basepath.
mklongpath() (
	basepath="${1:?}" len="${2:?}" max=99999
	namemax="$(getconf NAME_MAX "$basepath" 2>/dev/null)" || namemax=14
	dirs=$((len / namemax + 1))
	dirlen=$((len / dirs))

	dir="$basepath" target=$((len - dirlen - 1))
	while [ ${#dir} -le $target ]
	do
		i=0
		while [ $i -lt "$max" ]
		do
			seg="$(pad "$dirlen" "$i")"
			if ! [ -e "$dir/$seg" ]
			then
				dir="$dir/$seg"
				continue 2
			fi
			i=$((i + 1))
		done
	done

	i=0 fname=
	while [ $i -lt "$max" ]
	do
		seg="$(pad "$((len - ${#dir} - 1))" "$i")"
		if ! [ -e "$dir/$seg" ]
		then
			fname="$dir/$seg"
			break
		fi
		i=$((i + 1))
	done

	# This should be guaranteed, but who knows?
	[ "${#fname}" -eq "$len" ] ||
		err 'failed to generate long filename.'

	printf '%s\n' "$fname"
)

# Take filenames and print non-canonical mutations of those filenames.
mutatefnames() {
	printf '%s\n' "$@"				|
	sed -n 'p; s/^\([^/]\)/.\/\1/p'			|
	sed -n 'p; s/\//\/.\//p'			|
	sed -n 'p; s/\/\([^/]*\)\//\/\1\/..\/\1\//p'	|
	sed -n 'p; s/\//\/\//p'
}

# Pad $str with $fill on $side up to length $n.
pad() (
	n="${1:?}" str="${2-}" fill="${3:-x}" side="${4:-l}"

	strlen="${#str}" fillen="${#fill}"
	limit="$((n - fillen))" pad=
	while [ "$strlen" -le "$limit" ]
	do pad="$pad$fill" strlen=$((strlen + fillen))
	done

	case $side in
	(l) printf '%s%s\n' "$pad" "$str" ;;
	(r) printf '%s%s\n' "$str" "$pad" ;;
	esac
)

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

# Find an unallocated user ID in a given range.
unallocuid() (
	start="${1:?}" end="${2:?}"

	pipe="${TMPDIR:-/tmp}/uids-$$.fifo" retval=0
	mkfifo -m 0700 "$pipe"
	uids >"$pipe" 2>/dev/null & pid=$!
	# shellcheck disable=2086
	uids="$(cut -d' ' -f1 <"$pipe")" || retval=$?
	rm -f "$pipe"

	[ "$retval" -eq 0 ] || return $retval
	wait "$pid" || [ $? -eq 67 ]

	i="$start"
	while [ "$i" -le "$end" ]
	do
		# shellcheck disable=2086
		inlist -eq "$i" $uids || break
		i=$((i + 1))
	done

	printf '%d\n' "$i"
)

# Wait for the given processes and return 0 iff all of them returned 0.
waitn() {
	__waitn_ret=0
	for __waitn_pid
	do wait "$__waitn_pid" || __waitn_ret="$?"
	done
	return $__waitn_ret
}

#
# Build configuration
#

eval "$(main -C | grep -vE '^PATH=')"


#
# Common setup for non-privileged tests
#

# Current user.
logname="$(id -un)"
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
envsan_varname="$(pad $((MAX_VARNAME_LEN - 1)) v)"
check -s1 -e"discarding \$$envsan_varname" \
      "$envsan_varname=" main	|| result=70

# Variable name is too long.
envsan_varname="$(pad "$MAX_VARNAME_LEN" v)"
envsan_var="$envsan_varname=foo"
check -s1 -e'discarding variable with overly long name.' \
      "$envsan_var" main	|| result=70

# Variable has maximum length.
envsan_var="$(pad $((MAX_VAR_LEN - 1)) var= x r)"
check -s1 -e"discarding \$${envsan_var%=*}" \
      "$envsan_var" main	|| result=70

# Variable is too long.
envsan_var="$(pad "$MAX_VAR_LEN" var= x r)"
check -s1 -e'discarding overly long variable.'	\
      "$envsan_var" main	|| result=70

# Variable name is illegal.
pids=
for envsan_varname in 'foo ' '0foo' '$(echo foo)' '`echo foo`'
do
	check -s1 -e"variable \$$envsan_varname: bad name." \
	      badenv "$envsan_varname=" "$tests_dir/main" & pids="$pids $!"
done
# shellcheck disable=2086
waitn $pids || result=70

# Variable is not of the form <key>=<value>.
pids=
for envsan_var in 'foo' '0foo' 'foo '
do
	check -s1 -e"variable \$$envsan_var: no value." \
	      badenv -n1 "$envsan_var" "$tests_dir/main" & pids="$pids $!"
done
# shellcheck disable=2086
waitn $pids || result=70

# Not a CGI variable.
check -s1 -e'discarding $foo' foo=foo main || result=70

# Cleanup.
unset envsan_var envsan_varname envsan_vars pids


#
# Missing argv[0]
#

case $(uname) in
(FreeBSD) : ;; # FreeBSD's execvp insists on a non-NULL argument.
(*) check -s1 -e'empty argument vector' badexec main	|| result=70
esac

check -s1 -e'empty argument vector' badexec main ''	|| result=70


#
# Option handling
#

# Help.
check -o'Print this help screen.' main -h || result=70

# Version.
check -o'suCGI v' main -V || result=70

# Build configuration.
(
	eval "$(main -C | grep -vE ^PATH=)" || exit 70
	sed -n 's/#define \([[:alnum:]_]*\).*/\1/p' "$src_dir/defaults.h" |
	sort -u |
	while read -r opthdl_var
	do
		[ "$opthdl_var" != DEFAULTS_H ] || continue
		eval val="\${$opthdl_var}"
		if [ "${val-x}" = x ] && [ "${val-}" != x ]
		then err -s70 'main -C: %s: undefined.' "$opthdl_var"
		fi
	done
) || result=70

# Usage message.
pids=
for opthdl_args in -X -XX -x --x - -- '-h -C' '-hC' '-h -V' '-hC'
do
	# shellcheck disable=2086
	check -s1 -e'usage: sucgi' main $opthdl_args & pids="$pids $!"
done
# shellcheck disable=2086
waitn $pids || result=70

check -s1 -e'usage: sucgi' main '' || result=70

# Cleanup.
unset opthdl_args opthdl_var pids


#
# Script filename sanitisation
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
[ "$caught" ] && kill -s"$caught" $$

# Create a link to that path.
catch=
scrsan_hugelink="$TMPDIR/$(dirwalk "$TMPDIR" "$scrsan_hugepath" \
	'ln -s "$fname" s.d && printf s.d/' \
	'ln -s "$fname" s.f && printf s.f\\n')"
readonly scrsan_hugelink
cleanup="rmtree \"$scrsan_hugelink\" \"$TMPDIR\"; ${cleanup-}"
catch=x
[ "$caught" ] && kill -s"$caught" $$

# Create a file of the wrong type.
scrsan_wrongftype="$TMPDIR/scrsan-wrongftype"
mkdir "$scrsan_wrongftype"

# Create a valid script.
scrsan_script="$TMPDIR/scrsan-valid.php"
touch "$scrsan_script"

# PATH_TRANSLATED is undefined.
check -s1 -e'$PATH_TRANSLATED: not set.' main			|| result=70

# $PATH_TRANSLATED is empty.
check -s1 -e'$PATH_TRANSLATED: empty.' PATH_TRANSLATED= main	|| result=70

# $PATH_TRANSLATED is too long.
check -s1 -e'long' PATH_TRANSLATED="$scrsan_hugepath" main	|| result=70

# Path to script is too long after having been resolved.
check -s1 -e'long' PATH_TRANSLATED="$scrsan_hugelink" main	|| result=70

# Script is of the wrong type.
check -s1 -e"script $scrsan_wrongftype: not a regular file." \
	PATH_TRANSLATED="$scrsan_wrongftype" main		|| result=70

# Script does not exist.
check -s1 -e"realpath $TMPDIR/<nosuchfile>: No such file or directory." \
	PATH_TRANSLATED="$TMPDIR/<nosuchfile>" main		|| result=70

# Error is system-dependent.
case $uid in
(0)
	scrsan_err="owned by privileged user."
	;;
(*)
	isreguser=x logname="$(id -un)"
	if	[ "$uid" -lt "$MIN_UID" ] ||
		[ "$uid" -gt "$MAX_UID" ]
	then
		isreguser=
	else
		for gid in $(id -G "$logname")
		do
			if	[ "$gid" -lt "$MIN_GID" ] ||
				[ "$gid" -gt "$MAX_GID" ]
			then
				isreguser=
				break
			fi
		done
	fi
	unset gid

	if [ "$isreguser" ]
	then scrsan_err='seteuid: Operation not permitted.'
	else scrsan_err="user $logname: member of privileged group"
	fi
	;;
esac

# PATH_TRANSLATED is valid.
mutatefnames "$scrsan_script" | {
	pids=
	while read -r scrsan_fname
	do
		check -s1 -e"$scrsan_err" \
		      PATH_TRANSLATED="$scrsan_fname" main & pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# $PATH_TRANSLATED is valid despite its long name.
if [ "${LIBC-}" ]
then
	check -s1 -e"$scrsan_err" PATH_TRANSLATED="$scrsan_longpath" main ||
	result=70
else
	# The libc may be musl. And musl truncates long syslog messages.
	nskipped=$((nskipped + 1))
fi

# Cleanup.
unset scrsan_err scrsan_longpath scrsan_script scrsan_fname scrsan_wrongftype


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
usrval_unallocuid="$(unallocuid "$MIN_UID" "$MAX_UID")"

# Store a list of all user IDs.
usrval_uids="$TMPDIR/usrval-uids.list"
uids >"$usrval_uids"

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
check -s1 -e"script $usrval_script: no owner." \
      PATH_TRANSLATED="$usrval_script" main || result=70

# Owned by a privileged user
mutatefnames "$usrval_script" |
while read -r usrval_fname
do
	for usrval_uid in 0 "$usrval_lowuid" "$usrval_highuid"
	do
		[ "$usrval_uid" ] || continue

		chown "$usrval_uid" "$usrval_script"

		check -s1 -e"script $usrval_fname: owned by privileged user" \
		      PATH_TRANSLATED="$usrval_fname" main
	done
done

# Cleanup.
unset usrval_script usrval_fname usrval_uid usrval_lowuid usrval_highuid \
      usrval_uids usrval_unallocuid


#
# Common setup for privileged tests
#

# Search for a regular user.
if ! reguser="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")"
then
	case $result in
	(0) err -s75 'no regular user found.' ;;
	(*) err -s"$result" 'no regular user found.' ;;
	esac
fi

runas "$reguser" main -C >/dev/null 2>&1 ||
	err -s75 '%s cannot execute main.' "$reguser"

reguid="$(id -u "$reguser")"
reggid="$(id -g "$reguser")"

# Determine user directory.
# shellcheck disable=2059
case $USER_DIR in
(/*%s*) userdir="$(printf -- "$USER_DIR" "$reguser")" ;;
(*%s*)	err 'user directories in home directories are unsupported.' ;;
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
[ "$caught" ] && kill -s"$caught" $$

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
# Non-superuser
#

check -s1 -e'seteuid: Operation not permitted.' \
	PATH_TRANSLATED="$penv_sfx" runas "$reguser" main


#
# Privilege dropping
#

priv_output="uid=$reguid egid=$reggid ruid=$reguid rgid=$reggid"
for priv_script in "$procids_sfx" "$procids_nosfx"
do
	# /tmp might be mounted noexec.
	if [ -x "$priv_script" ] && ! $priv_script >/dev/null 2>&1
	then
		nskipped=$((nskipped + 1))
		continue
	fi


	mutatefnames "$priv_script" | {
		pids=
		while read -r priv_fname
		do
			check -s0 -o"$priv_output" \
			      PATH_TRANSLATED="$priv_fname" main &
			pids="$pids $!"
		done
		# shellcheck disable=2086
		waitn $pids
	} || result=70
done
unset priv_script priv_fname priv_output


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

# Pretend the script is inside using dots.
dirval_dots="$userdir/../dirval-outside.sh"
touch "$dirval_dots"
chown "$reguser" "$dirval_dots"

# Create a link from inside to outside the user directory.
dirval_intoout="$userdir/in-to-out.sh"
ln -s "$dirval_outside" "$dirval_intoout"

# Forbidden location.
mutatefnames "$dirval_outside" "$dirval_dots" "$dirval_intoout" | {
	pids=
	while read -r dirval_fname
	do
		error="script $dirval_fname: not in $reguser's user directory."
		check -s1 -e"$error" \
		      PATH_TRANSLATED="$dirval_fname" main &
		pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# Permitted location.
dirval_output="uid=$reguid egid=$reggid ruid=$reguid rgid=$reggid"
mutatefnames "$procids_sfx" "$dirval_outtoin" | {
	pids=
	while read -r dirval_fname
	do
		check -s0 -o"$dirval_output" \
		      PATH_TRANSLATED="$dirval_fname" main &
		pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# Cleanup.
unset dirval_outtoin dirval_outside dirval_intoout dirval_output dirval_fname


#
# Set-process-IDs on execute bits
#

# Set-user-ID on execute bit set.
chmod u=rws,g=r,o=r "$procids_sfx"

mutatefnames "$procids_sfx" | {
	pids=
	while read -r procids_fname
	do
		error="script $procids_fname: set-user-ID on execute bit is set."
		check -s1 -e"$error" \
		      PATH_TRANSLATED="$procids_fname" main &
		pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# Set-group-ID on execute bit set.
chmod u=rx,g=rs,o=r "$procids_sfx"

mutatefnames "$procids_sfx" | {
	pids=
	while read -r procids_fname
	do
		error="script $procids_fname: set-group-ID on execute bit is set."
		check -s1 -e"$error" \
		      PATH_TRANSLATED="$procids_fname" main &
		pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# Cleanup.
chmod u=rwx,go= "$procids_sfx"
unset procids_fname


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
mutatefnames "$hidden_file" "$hidden_indir" | {
	pids=
	while read -r hidden_fname
	do
		check -s1 -e"path $hidden_fname: contains hidden files." \
			PATH_TRANSLATED="$hidden_fname" main &
		pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# Cleanup.
unset hidden_file hidden_dir hidden_indir hidden_fname


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
hdl_suffix=$(pad "$MAX_SUFFIX_LEN" . x r)
hdl_hugesuffix="$userdir/script$hdl_suffix"
touch "$hdl_hugesuffix"

# Adapt ownership.
chown -R "$reguser" "$userdir"

# Bad handler.
mutatefnames "$hdl_emptyhandler" | {
	pids=
	while read -r hdl_fname
	do
		check -s1 -e"script $hdl_fname: bad handler." \
		      PATH_TRANSLATED="$hdl_fname" main & pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# Suffix is too long.
mutatefnames "$hdl_hugesuffix" | {
	pids=
	while read -r hdl_fname
	do
		check -s1 -e"script $hdl_fname: filename suffix too long." \
		      PATH_TRANSLATED="$hdl_fname" main & pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# Unknown suffix.
mutatefnames "$hdl_unknownsuffix" | {
	pids=
	while read -r hdl_fname
	do
		check -s1 -e"execl $hdl_fname: Permission denied." \
		      PATH_TRANSLATED="$hdl_fname" main & pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# Not executable.
mutatefnames "$hdl_nosuffix" | {
	pids=
	while read -r hdl_fname
	do
		check -s1 -e"execl $hdl_fname: Permission denied." \
		      PATH_TRANSLATED="$hdl_fname" main & pids="$pids $!"
	done
	# shellcheck disable=2086
	waitn $pids
} || result=70

# Cleanup.
unset hdl_nosuffix hdl_unknownsuffix hdl_emptyhandler \
      hdl_suffix hdl_hugesuffix hdl_fname


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
	IFS=':'						\
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
