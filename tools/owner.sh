#!/bin/sh
# Print the current user's real user and group ID, even when running as root.
# shellcheck disable=1091,2015

#
# Initialisation
#

set -Cefu
tools_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(dirname "$tools_dir")"
# shellcheck disable=2034
readonly tools_dir src_dir
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit


#
# Options
#

OPTIND=1 OPTARG='' opt=''
while getopts h opt; do
	# shellcheck disable=2154
	case $opt in
		(h) exec cat <<EOF ;;
$prog_name - print the owner of file

Usage:    owner FILE
          owner -h

Operands:
    FILE  The file.

Options:
    -h    Show this help screen.

Copyright 2022 Odin Kroeger.
Released under the GNU General Public License.
This programme comes with ABSOLUTELY NO WARRANTY.
EOF
		(*) exit 8
	esac
done
shift $((OPTIND - 1))
unset opt

[ $# -lt 1 ] && err 'no file given.'
[ $# -gt 1 ] && err 'too many operands.'


#
# Main
#

user="$(owner "$1")"
group="$(id -gn "$user")"

printf '%s:%s\n' "$user" "$group"

