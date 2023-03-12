#!/bin/sh
# Run checks using different virtual machines.
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

# shellcheck disable=2015


#
# Defaults
#

# Virtual machines to test.
readonly vms='diotima'

# Branch to run tests on.
branch=devel

# Check command to run.
check="scripts/checkall.sh"

# Repository directory on the virtual machine.
dir='repos/sucgi'

# Be more quiet?
quiet=

# Store a log in the current working directory?
storelogs=y



#
# Initialiation
#

set -Cefu
tools_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(dirname -- "$tools_dir")"
# shellcheck disable=2034
readonly tools_dir src_dir
# shellcheck disable=1091
. "$tools_dir/lib.sh" || exit
init || exit
tmpdir


#
# Options
#

OPTIND=1 OPTARG='' opt=''
# shellcheck disable=2034
while getopts b:c:d:hq opt; do
	# shellcheck disable=2154
	case $opt in
	(h) exec cat <<EOF
$prog_name - run tests with different virtual machines

Usage:         $prog_name [VM ...]
               $prog_name -h

Operands:
    VM         A virtual machine name.

Options:
    -b BRANCH  Branch to checkout (default: $branch).
    -c CHECK   Check to run (default: $check).
    -d DIR     Repository directory on VM (default: $dir).
    -h         Show this help screen.
EOF
	    ;;
	(b) branch="$OPTARG" ;;
	(c) check="$OPTARG" ;;
	(d) dir="$OPTARG" ;;
	(q) quiet=y ;;
	(*) exit 1
	esac
done
shift $((OPTIND - 1))
unset opt

[ $# -eq 0 ] && set -- $vms

readonly comm='
	cd "$dir"			&&
	git stash			&&
	git pull			&&
	git checkout "$branch"		;
	[ -e makefile ]			&&
	make distclean			||
	exit
	"$check"
'


#
# Main
#

utmpid="$(ps -xo pid=,comm= | awk '$2 ~ /\/UTM$/ {print $1; exit}')"

if ! [ "$utmpid" ]
then
	cleanup="osascript -e 'tell application \"UTM\" to quit'; ${cleanup-}"
	osascript -e 'tell application "UTM" to start'
fi

cleanup="
	warn 'shutting down ...'
	if
		[ \"\${vm-}\" ]					&&
		[ \"\${started-}\" = \"\$vm\" ]			&&
		[ \"\$(utmctl status \"\$vm\")\" = started ]
	then
		utmctl stop \"\$vm\"
	fi
	${cleanup-}
"


failures=
for vm
do
	warn -n 'testing with %s ... ' "$vm"

	started=
	if [ "$(utmctl status "$vm")" = stopped ]
	then
		utmctl start "$vm"
		started="$vm"
	fi

	i=0
	while [ "$i" -lt 5 ]
	do
		ssh "$vm" true >/dev/null 2>&1 && break
		i=$((i + 1))
	done
	[ "$i" -eq 5 ] && err 'connection failure.'

	logfile="$TMPDIR/checkvm-$vm.log"
	if (
		if [ "$storelogs" ]
		then exec >"$logfile" 2>&1
		else exec >/dev/null 2>&1
		fi

		eval ssh "$vm" "$comm"
	)
	then
		printf 'pass\n' >&2
	else
		printf '%s\n' fail >&2
		[ "$storelogs" ] && [ -e "$logfile" ] && mv "$logfile" .
		failures="$failures $vm"
	fi

	[ "$started" ] && utmctl stop "$vm"
done

if [ "$failures" ]
then err -s70 'failures: %s' "${failures# }"
else warn -q 'all tests passed.'
fi
