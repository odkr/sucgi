#!/bin/sh
# Test if env_get_fname only returns safe filenames.
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

: "${TMPDIR:=.}"
OWD="$(pwd)" && [ "$OWD" ] ||
	abort "failed to save working directory."
cd -P "$TMPDIR" || exit
TMPDIR="$(pwd)" && [ "$TMPDIR" ] ||
	abort "failed to get canonical path of temporary directory."
cd "$OWD" || exit

readonly TMP="${TMPDIR:-.}/check-$$"
mkdir -m 0700 "$TMP" || exit
# shellcheck disable=2034
CLEANUP="[ \"${TMP-}\" ] && rm -rf \"\$TMP\""
export TMPDIR="$TMP"

trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

[ "$CAUGHT" -gt 0 ] && exit $((CAUGHT + 128))

umask 077
touch "$TMP/file"
ln -s "$TMP" "$TMP/symlink"

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

unset var
./env_get_fname var &&
	abort "env_get_fname accepted an undefined variable."

var='' ./env_get_fname var &&
	abort "env_get_fname accepted an empty variable."

var="$long_str" ./env_get_fname var &&
	abort "env_get_fname accepted an overly long string."

var="$long_path" ./env_get_fname var &&
	abort "env_get_fname accepted an overly long path."

var="$TMP/symlink/file" ./env_get_fname var &&
	abort "env_get_fname does not refuse $TMP/symlink/file."

var="$TMP/file" ./env_get_fname var ||
	abort "env_get_fname refuses $TMP/file."


exit 0
