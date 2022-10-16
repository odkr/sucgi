#!/bin/sh
# Print the current user's real user and group ID, even when running as root.
# shellcheck disable=1091,2015

#
# Initialisation
#

set -Cefu
readonly script_dir="${0%/*}"
# shellcheck disable=1091
. "$script_dir/lib.sh" || exit
init || exit
tmpdir ru


#
# Options
#

OPTIND=1 OPTARG='' opt=''
while getopts b:c:p:w:h opt; do
	case $opt in
		(h) exec cat <<-EOF ;;
			regularids - Print the user's regular UID and GID.

			Synopsis:
			    regularids

			Options:
			    -h         Show this help screen.
			EOF
		(*) exit 8
	esac
done
shift $((OPTIND - 1))
unset opt

[ $# -gt 0 ] && err 'too many operands.'


#
# Main
#

ruid="$(regularuser)" && [ "$ruid" ] ||
	err 'failed to get non-root user ID.'
user="$(getlogname "$ruid")" && [ "$user" ] ||
	err "failed to get logname associated with ID $ruid."
rgid="$(id -g "$user")" && [ "$rgid" ] ||
	err 'failed to get non-root group ID.'

printf '%d:%d\n' "$ruid" "$rgid"
