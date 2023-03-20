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
readonly vms='aristides diotima euthyphro'

# Branch to run tests on.
branch=devel

# Check command to run.
check="
	if command -v sudo >/dev/null
	then sudo scripts/checkall.sh
	elif command -v doas >/dev/null
	then doas scripts/checkall.sh
	else scripts/checkall.sh
	fi
"

# Repository directory on the virtual machine.
dir='repos/sucgi'

# Be more quiet?
quiet=

# Be verbose?
verbose=

# How many seconds to wait for a VM to become reachable.
timeout=60


#
# Initialiation
#

set -Cefu
tools_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(dirname -- "$tools_dir")"
# shellcheck disable=2034
readonly tools_dir src_dir
# shellcheck disable=1091
. "$tools_dir/funcs.sh" || exit
init || exit
tmpdir


#
# Options
#

OPTIND=1 OPTARG='' opt=''
# shellcheck disable=2034
while getopts Db:c:d:hqt:v opt; do
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
    -t N       Wait at most N seconds for VM to boot (default: $timeout).
    -q         Be more quiet.
    -v         Be verbose, but do not log runs.
    -h         Show this help screen.
EOF
	    ;;
	(D) set -x ;;
	(b) branch="$OPTARG" ;;
	(c) check="$OPTARG" ;;
	(d) dir="$OPTARG" ;;
	(t) timeout="$OPTARG" ;;
	(q) quiet=y ;;
	(v) verbose=y ;;
	(*) exit 1
	esac
done
shift $((OPTIND - 1))
unset opt

# shellcheck disable=2086
[ $# -eq 0 ] && set -- $vms

readonly comm="
	set -ex
	cd \"$dir\"
	git stash
	git pull
	git checkout \"$branch\"
	[ -e makefile ] && make distclean
	$check
	[ -e makefile ] && make distclean
"


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
	[ "$verbose" ] && echo >&2

	warn -q 'waiting for %s to boot ...' "$vm"

	started=
	if [ "$(utmctl status "$vm")" = stopped ]
	then
		utmctl start "$vm"
		started="$vm"
	fi

	trap 'catch ALRM' ALRM
	pid="$$"
	( sleep "$timeout" & wait $! && kill -s ALRM "$pid"; ) & alarm=$!

	while ! [ "$caught" ]
	do ssh "$vm" true >/dev/null 2>&1 && break
	done

	[ "$caught" = ALRM ] && err 'connection failure.'
	kill "$alarm"

	logfile="$TMPDIR/checkvm-$vm.log"
	if (
		[ "$verbose" ] || exec >"$logfile" 2>&1

		# shellcheck disable=2029
		ssh "$vm" "$comm"
	)
	then
		[ "$verbose" ] || printf '%s\n' pass >&2
	else
		if ! [ "$verbose" ]
		then
			printf '%s\n' fail >&2
			[ -e "$logfile" ] && mv "$logfile" .
		fi
		failures="$failures $vm"
	fi

	[ "$started" ] && utmctl stop "$vm"
done

if [ "$failures" ]
then err -s70 'failures: %s' "${failures# }"
else warn -q 'all tests passed.'
fi
