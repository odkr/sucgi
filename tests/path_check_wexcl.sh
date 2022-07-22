#!/bin/sh
# Test if path_check_wexcl correctly identifies exclusive write access.
# shellcheck disable=1091,2015

set -Cefu

dir="$(dirname "$0")" && [ "$dir" ]
cd -P "$dir" || exit
. ./utils.sh || exit

trap cleanup EXIT

CAUGHT=0
trap 'CAUGHT=1' HUP
trap 'CAUGHT=2' INT
trap 'CAUGHT=15' TERM

: "${PWD:=$(pwd)}"
: "${PWD:?}"
: "${TMPDIR:="$PWD"}"
readonly TMP="$TMPDIR/chk-$$.tmp"
mkdir -m 0700 "$TMP" || exit
# shellcheck disable=2034
CLEANUP="[ \"${TMP-}\" ] && rm -rf \"\$TMP\""
export TMPDIR="$TMP"

trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

[ "$CAUGHT" -gt 0 ] && exit $((CAUGHT + 128))

umask 0777
fname="$TMP/file"
touch "$fname"

uid="$(id -u)" && [ "$uid" ] ||
	abort "failed to get process' effective UID."
gid="$(id -g)" && [ "$gid" ] ||
	abort "failed to get process' effective GID."

no="o=w uo=w go=w ugo=w"
for mode in $no
do
	chown "$uid:$gid" "$fname"
	chmod "$mode" "$fname"
	./path_check_wexcl "$uid" "$gid" "$fname" "$TMP" &&
		abort "path_check_wexcl reports $mode as exclusively writable."
	chmod ugo= "$fname"
done

yes="ug=w u=w g=w"
for mode in $yes
do
	chown "$uid:$gid" "$fname"
	chmod "$mode" "$fname"
	./path_check_wexcl "$uid" "$gid" "$fname" "$TMP" ||
		abort "path_check_wexcl reports $mode as not exclusively writable."
	./path_check_wexcl "$uid" "$gid" "$fname" / &&
		abort "path_check_wexcl reports $fname as exclusively writable w/o stop condition."
	chmod ugo= "$fname"
done

./path_check_wexcl 0 0 /bin/sh / ||
	abort "path_check_wexcl reports /bin/sh as not exclusively writable."
