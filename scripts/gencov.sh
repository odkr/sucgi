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
# Initialiation
#

set -Ceu
scripts_dir="$(cd -P "$(dirname -- "$0")" && pwd)"
src_dir="$(dirname -- "$scripts_dir")"
# shellcheck disable=2034
readonly scripts_dir src_dir
# shellcheck disable=1091
. "$src_dir/tests/funcs.sh" || exit
init || exit
tmpdir cov .


#
# Constants
#

# Current working directory.
cwd="$(pwd)"
readonly cwd


#
# Defaults
#

# Compiler to use.
comp=

# Where to store GCov files.
gcovdir=-

# Where to store the lcov.info file.
lcovinfo=-

# Where to store the LCov report.
htmldir=cov



#
# Functions
#

# Run $@ and log its output.
logged() (
	logfile="$(basename "${1:?}").log"

	if "$@" >"$TMPDIR/$logfile" 2>&1
	then
		rm "$TMPDIR/$logfile"
	else
		warn '%s: exited with status %d.' "$*" "$?"
		warn 'see %s for details.' "$logfile"
		mv "$TMPDIR/$logfile" "$cwd"
		return 8
	fi
)


#
# Options
#

OPTIND=1 OPTARG='' opt=''
# shellcheck disable=2034
while getopts C:G:H:L:hq opt; do
	# shellcheck disable=2154
	case $opt in
	(h) exec cat <<EOF
$prog_name - generate coverage reports

Usage:       $prog_name [-q] [-CCOMP] [-HDIR] [-LFILE]
             $prog_name -h

Options:
    -C COMP  Use compiler COMP (default is system-dependent).
    -G DIR   Store GCov files in DIR (default: $gcovdir).
    -H DIR   Store HTML report in DIR (default: $htmldir).
    -L FILE  Store LCov data in FILE (default: $lcovinfo).
    -q       Be quiet.
    -h       Show this help screen.

    Use a dash as DIR or FILE to skip generation.
EOF
	    ;;
	(C) comp="$OPTARG" ;;
	(G) gcovdir="$OPTARG" ;;
	(H) htmldir="$OPTARG" ;;
	(L) lcovinfo="$OPTARG" ;;
	(q) quiet=y ;;
	(*) exit 1
	esac
done
shift $((OPTIND - 1))
unset opt

if [ $# -ne 0 ]
then
exit 64
fi

case $gcovdir in
(-|/*)	: ;;
(*)	gcovdir="$cwd/$gcovdir"
esac
readonly gcovdir

case $lcovinfo in
(-|/*)	: ;;
(*)	lcovinfo="$cwd/$lcovinfo"
esac
readonly lcovinfo

case $htmldir in
(-|/*)	: ;;
(*)	htmldir="$cwd/$htmldir"
esac
readonly htmldir


#
# Setup
#

cd -P "$src_dir" || exit

uid="$(id -u)"
[ "$uid" -eq 0 ] || err 'not invoked by the superuser.'
distdir="$TMPDIR/dist"

owner="$(owner "$src_dir")"
group="$(id -g "$owner")"

warn -q 'copying source files ...'
logged make dist_name="$distdir" "$distdir"
cd -P "$distdir" || exit

warn -q 'generating build configuration ... '
CC="$comp" logged ./configure

warn -q 'compiling tools ...'
logged make tools

warn -q 'compiling tests ...'
logged make CFLAGS='-O2 --coverage' all checks


#
# Environment
#

warn -q 'adapting environment ...'

export PATH="$distdir/tools:$distdir/tests:$PATH"
eval "$("$distdir/tests/main" -C | grep -vE '^PATH=')"
reguser="$(reguser "$MIN_UID" "$MAX_UID" "$MIN_GID" "$MAX_GID")"
[ "$reguser" ] || err 'no regular user found.'
reggroup="$(id -gn "$reguser")"
[ "$reggroup" ] || err 'no regular group found.'

# Neither umask 0 nor extensive write privileges work,
# because main calls umask and permissions would have
# to be changed while the tests are running.
chown -R "$reguser:$reggroup" "$TMPDIR"
chmod -R ugo+r "$TMPDIR"
find "$TMPDIR" -type d -exec chmod ug+s,o+x '{}' +


#
# Main
#

warn -q 'collecting coverage data ...'
find tests -type f '(' -perm +=x -o -name '*.sh' ')' ! -name 'lib.*' |
while read -r check
do
	[ -e "$check.sh" ] && continue

	basename="$(basename "$check")"
	logfile="$basename.log" infofile="$basename.info"

	set +e
	"$check" >>"$logfile" 2>&1
	case $? in
	(0|75)	: ;;
	(*)	warn '%s: exited with status %d.' "$check" "$?"
		warn 'see %s for details.' "$logfile"
		mv "$logfile" "$cwd"
		exit 8
	esac
	set -e

	geninfo -q -o "$infofile"				\
		--exclude '*/tests/*'				\
		--exclude '/usr/*' --exclude '/Library/*'	\
		"$distdir" 2>/dev/null

	if	{ [ "$htmldir"  ] && [ "$htmldir"  != - ]; } ||
		{ [ "$lcovinfo" ] && [ "$lcovinfo" != - ]; }
	then
		lcov -q -o lcov.info -a "$infofile"
	fi
done

if [ "$gcovdir" ] && [ "$gcovdir" != - ]
then
	warn -q 'saving GCov files in %s ...' "$gcovdir"

	mkdir -p "$gcovdir"
	find . -maxdepth 1 -type f -name '*.c' -exec gcov '{}' + >/dev/null

	find . -type f -name '*.c.gcov' |
	while read -r fname
	do
		dir="$(dirname "$fname")"
		mkdir -p "$gcovdir/$dir"
		mv "$fname" "$gcovdir/$dir"
	done

	chown -R "$owner:$group" "$gcovdir"
fi

if [ "$htmldir" ] && [ "$htmldir" != - ]
then
	warn -q 'saving report to %s ...' "$htmldir"
	genhtml -q -o "$htmldir" lcov.info
	chown -R "$owner:$group" "$htmldir"
fi

if [ "$lcovinfo" ] && [ "$lcovinfo" != - ]
then
	warn -q 'saving data to %s ...' "$lcovinfo"
	mv lcov.info "$lcovinfo"
	chown -R "$owner:$group" "$lcovinfo"
fi
