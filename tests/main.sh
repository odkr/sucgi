#!/bin/sh
# Test sucgi via main.
#shellcheck disable=1091,2015,2016,2034


#
# Functions
#

# Get the UID of the user who invoked the script,
# even if the script has been invoked via su or sudo.
get_non_root_uid() (
	pivot="$$"
	fifo="$TMPDIR/pstab"
	mkfifo "$fifo"

	while true
	do
		ps -Ao 'pid= ppid= user=' | sort -r >"$fifo" & sort=$!
		while read -r pid ppid user
		do
			uid="$(id -u "$user")" && [ "$uid" ] || continue
			if [ "$uid" -ne 0 ]
			then
				echo "$uid"
				return 0
			fi
			
			[ "$pid" -eq "$pivot" ] && pivot="$ppid"
			[ "$pivot" -gt 1 ] || return 1 
		done <"$fifo"
		wait "$sort"
	done
)


#
# Prelude
#

set -Cefu

dir="$(dirname "$0")" && [ "$dir" ]
cd -P "$dir" || exit
. ./utils.sh || exit

trap cleanup EXIT

CAUGHT=0
trap 'CAUGHT=1' HUP
trap 'CAUGHT=2' INT
trap 'CAUGHT=15' TERM

readonly TMP="${TMPDIR:-.}/check-$$"
mkdir -m 0700 "$TMP" || exit
# shellcheck disable=2034
CLEANUP="[ \"${TMP-}\" ] && rm -rf \"\$TMP\""
export TMPDIR="$TMP"

trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

[ "$CAUGHT" -gt 0 ] && exit $((CAUGHT + 128))

fifo="$TMP/fifo"
mkfifo -m 0700 "$fifo"

str_max="$(getconf PATH_MAX .)" && [ "$str_max" ] && 
	[ "$str_max" -ge 4096 ] || str_max=4096
long_str="$PWD"
while [ "${#long_str}" -lt "$str_max" ]
	do long_str="$long_str/$long_str"
done
long_str="$long_str/foo"

if	path_max="$(getconf PATH_MAX .)" &&
	[ "$path_max" ] &&
	[ "$path_max" -gt -1 ]
then
	long_path="$PWD"
	while [ "${#long_path}" -lt "$path_max" ]
		do long_path="$long_path/foo"
	done
	long_path="$long_path/foo"
fi


#
# Checks
#

./main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'DOCUMENT_ROOT: not set.' <"$fifo" ||
	abort 'wrong error message for missing document root.'
wait "$pid" &&
	abort 'no error for missing document root.'

DOCUMENT_ROOT='' ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'DOCUMENT_ROOT: is the empty string.' <"$fifo" ||
	abort 'wrong error message for empty document root.'
wait "$pid" &&
	abort 'no error for empty document root.'

DOCUMENT_ROOT="$long_str" ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'environment variable too long.' <"$fifo" ||
	abort 'wrong error message for overy long document root variable.'
wait "$pid" &&
	abort 'no error for overy long document root variable.'

if [ "$long_path" ]
then
	DOCUMENT_ROOT="$long_path" ./main >"$fifo" 2>&1 & pid="$!"
	grep -Fq 'too long.' <"$fifo" ||
		abort 'wrong error message for overy long document root path.'
	wait "$pid" &&
		abort 'no error for overy long document root path.'
fi

DOCUMENT_ROOT='/::no-such-file!!' ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq '$DOCUMENT_ROOT: No such file or directory.' <"$fifo" ||
	abort 'wrong error message for non-existing document root.'
wait "$pid" &&
	abort 'no error for non-existing document root.'

script="$(basename "$0")" && [ "$script" ] ||
	alert "failed to get basename of $0"
DOCUMENT_ROOT="./$script" ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq '$DOCUMENT_ROOT: not a directory.' <"$fifo" ||
	abort 'wrong error message for non-directory document root.'
wait "$pid" &&
	abort 'no error for non-directory document root.'

DOCUMENT_ROOT=. ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq '$DOCUMENT_ROOT: does not match /*.' <"$fifo" ||
	abort 'wrong error message for illegal document root.'
wait "$pid" &&
	abort 'no error for illegal document root.'

DOCUMENT_ROOT="$PWD" ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'PATH_TRANSLATED: not set.' <"$fifo" ||
	abort 'wrong error message for missing path.'
wait "$pid" &&
	abort 'no error for missing path.'

DOCUMENT_ROOT="$PWD" PATH_TRANSLATED='' ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'PATH_TRANSLATED: is the empty string.' <"$fifo" ||
	abort 'wrong error message for empty path.'
wait "$pid" &&
	abort 'no error for empty path.'

DOCUMENT_ROOT=/ PATH_TRANSLATED="$long_str" ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'environment variable too long.' <"$fifo" ||
	abort 'wrong error message for overy long path variable.'
wait "$pid" &&
	abort 'no error for overy long path variable.'

if [ "$long_path" ]
then
	DOCUMENT_ROOT=/ PATH_TRANSLATED="$long_path" ./main >"$fifo" 2>&1 & pid="$!"
	grep -Fq 'too long.' <"$fifo" ||
		abort 'wrong error message for overy long path.'
	wait "$pid" &&
		abort 'no error for overy long path.'
fi

DOCUMENT_ROOT=/ PATH_TRANSLATED='/::no-such-file!!' ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq '$PATH_TRANSLATED: No such file or directory.' <"$fifo" ||
	abort 'wrong error message for non-existing path.'
wait "$pid" &&
	abort 'no error for non-existing path.'

DOCUMENT_ROOT="$PWD" PATH_TRANSLATED=/ ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq "\$PATH_TRANSLATED: not in document root $PWD." <"$fifo" ||
	abort 'wrong error message for illegal path.'
wait "$pid" &&
	abort 'no error for illegal path.'

DOCUMENT_ROOT=/ PATH_TRANSLATED="$PWD" ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq '$PATH_TRANSLATED: not a regular file.' <"$fifo" ||
	abort 'wrong error message for non-regular script file.'
wait "$pid" &&
	abort 'no error for non-regular script file.'

true="$(command -v true >/dev/null 2>&1 || :)" || :
: "${true:=/usr/bin/true}"
"$true" || abort "true: exited with status $?."
DOCUMENT_ROOT=/ PATH_TRANSLATED="$true" ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'owned by the superuser.' <"$fifo" ||
	abort 'wrong error message for superuser-owned programme.'
wait "$pid" &&
	abort 'no error for superuser-owned programme.'

uid="$(id -u)" && [ "$uid" ] ||
	abort "failed to get process' effective UID."

# The checks below only work if main.sh is invoked as root.
[ "$uid" -ne 0 ] && exit

uid="$(get_non_root_uid)" && [ "$uid" ] ||
	abort "failed to get non-root user ID of caller."
user="$(id -un "$uid")" && [ "$user" ] ||
	abort "failed to name of user with ID $uid."
gid="$(id -g "$user")" && [ "$gid" ] ||
	abort "failed to get ID of $user's primary group."

RESTRICTED="${PWD:?}/tmp"
readonly RESTRICTED

CAUGHT=0
trap 'CAUGHT=1' HUP
trap 'CAUGHT=2' INT
trap 'CAUGHT=15' TERM

mkdir -m 0700 "$RESTRICTED" || exit
CLEANUP="rm -rf \"\${RESTRICTED:?}\"; $CLEANUP"

trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

[ "$CAUGHT" -gt 0 ] && exit $((CAUGHT + 128))

script="$RESTRICTED/script.sh"
echo : > "$script"
chown -R "$uid:$gid" "$RESTRICTED"
chmod ug=,o=w "$script"
DOCUMENT_ROOT="$PWD" PATH_TRANSLATED="$script" ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq "$script: can be altered by users other than $user." <"$fifo" ||
	abort 'wrong error for non-exclusively writable script.'
wait "$pid" &&
	abort 'no error for non-exclusively writable script.'

DOCUMENT_ROOT="$PWD" PATH_TRANSLATED="$PWD/script.sh" ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'This is a test script for main.sh and run_script.sh.' <"$fifo" ||
	abort 'wrong script.sh output.'
wait "$pid" ||
	abort "script.sh exited with status $?."

user="$RESTRICTED/user.sh"
cp ./user.sh "$user"
chown "$uid:$gid" "$user"
chmod u=rwx,go= "$user"
DOCUMENT_ROOT="$PWD" PATH_TRANSLATED="$user" ./main >"$fifo" 2>&1 & pid="$!"
grep -Fq "$uid:$gid" <"$fifo" ||
	abort 'failed to drop privileges.'
wait "$pid" ||
	abort "$user exited with status $?."

DOCUMENT_ROOT="$PWD" PATH_TRANSLATED="$PWD/env.sh" FOO=bar \
	./main >"$fifo" 2>&1 & pid="$!"
grep -Fq 'FOO=bar' <"$fifo" &&
	abort 'environment was not cleared.'
wait "$pid" ||
	abort "./env.sh exited with status $?."

exit 0
