dnl Template for the makefile.
dnl
dnl Edit this file, NOT the makefile itself.
dnl Then run ./configure to generate or ./config.status to update it.
dnl See docs/build.rst for details.
dnl
include(`macros.m4')dnl
.POSIX:

#
# DO NOT EDIT THIS FILE! IT MAY GET OVERWRITTEN!
#
# Edit makefile.m4 instead and then either run ./configure to generate the
# makefile or ./config.status to update it. See docs/build.rst for details.
#

#
# Special targets
#

.PHONY: all analysis check clean distcheck distclean shellcheck

.IGNORE: analysis

all: sucgi


#
# Flags
#

ifnempty(`__CC', `dnl
CC = __CC
')dnl
CFLAGS = default(`__CFLAGS', `-O1')
ifnempty(`__ARFLAGS', `dnl
ARFLAGS = __ARFLAGS
')dnl
ifnempty(`__AR', `dnl
AR = __AR
')dnl
ifnempty(`__LDFLAGS', `dnl
LDFLAGS = __LDFLAGS
')dnl
ifnempty(`__LDLIBS', `dnl
LDLIBS = __LDLIBS
')dnl


#
# Common macros
#

hdrs = cattr.h compat.h macros.h max.h types.h
objs = funcs.a(env.o)  funcs.a(error.o) funcs.a(file.o) funcs.a(handler.o) \
       funcs.a(pair.o) funcs.a(path.o)  funcs.a(priv.o) funcs.a(str.o)     \
       funcs.a(userdir.o)


#
# Build configuration
#

makefile: makefile.m4

compat.h: compat.h.m4

makefile compat.h:
	[ -e config.status ] && ./config.status $@ || m4 $@.m4 >$@


#
# Build
#

funcs.a(env.o): env.c env.h funcs.a(str.o)

funcs.a(error.o): error.c error.h

funcs.a(file.o): file.c file.h funcs.a(str.o)

funcs.a(pair.o): pair.c pair.h

funcs.a(path.o): path.c path.h funcs.a(file.o) funcs.a(str.o)

funcs.a(priv.o): priv.c priv.h

funcs.a(handler.o): handler.c handler.h funcs.a(pair.o) funcs.a(path.o)

funcs.a(str.o): str.c str.h

funcs.a(userdir.o): userdir.c userdir.h

$(objs): $(hdrs)
	$(CC) $(LDFLAGS) $(CFLAGS) -c $*.c $(LDLIBS)
	$(AR) $(ARFLAGS) funcs.a $%
	rm -f $%

sucgi: main.c config.h testing.h $(hdrs) $(objs)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ main.c funcs.a $(LDLIBS)


#
# Installation
#

ifnempty(`__DESTDIR', `DESTDIR = __DESTDIR
')dnl
PREFIX = default(`__PREFIX', `/usr/local')
www_grp = default(`__www_grp', `www-data')
cgi_dir = default(`__cgi_dir', `/usr/lib/cgi-bin')
libexec = $(DESTDIR)$(PREFIX)/libexec

$(libexec)/sucgi: sucgi
	mkdir -p $(libexec)
	cp sucgi $(libexec)
	chown 0:$(www_grp) $(libexec)/sucgi
	chmod u=rws,g=x,o= $(libexec)/sucgi

$(cgi_dir)/sucgi: $(libexec)/sucgi
	ln -s $(libexec)/sucgi $(cgi_dir)/sucgi

install: $(libexec)/sucgi $(cgi_dir)/sucgi

uninstall:
	rm -f $(cgi_dir)/sucgi $(libexec)/sucgi


#
# Tests
#

checks = $(check_scripts) $(macro_check_bins) tests/envisname \
	tests/envrestore tests/fileisexe tests/fileisxusrw \
	tests/handlerfind tests/pairfind tests/pathchkloc \
	tests/pathsuffix tests/cpstr tests/splitstr tests/userdirexp

macro_check_bins = tests/ISSIGNED tests/NELEMS tests/SIGNEDMAX

other_check_bins = tests/envisname tests/envrestore tests/error \
	tests/fileisexe tests/fileisxusrw tests/handlerfind \
	tests/pairfind tests/pathchkperm tests/pathchkloc \
	tests/pathsuffix tests/privdrop tests/privsuspend tests/cpstr \
	tests/splitstr tests/userdirexp

check_scripts = tests/error.sh tests/main.sh tests/pathchkperm.sh \
	tests/privdrop.sh tests/privsuspend.sh

tool_bins = tools/badenv tools/badexec tools/ids tools/runpara tools/runas

runpara_flags = -ci75 -j8

tests/ISSIGNED: tests/ISSIGNED.c

tests/NELEMS: tests/NELEMS.c

tests/SIGNEDMAX: tests/SIGNEDMAX.c

tests/envisname: tests/envisname.c funcs.a(env.o)

tests/envrestore: tests/envrestore.c funcs.a(env.o) funcs.a(str.o)

tests/error: tests/error.c funcs.a(error.o)

tests/fileisexe: tests/fileisexe.c funcs.a(file.o)

tests/fileisxusrw: tests/fileisxusrw.c funcs.a(file.o)

tests/handlerfind: tests/handlerfind.c funcs.a(handler.o)

tests/main: main.c config.h testing.h $(hdrs) $(objs)

tests/pairfind: tests/pairfind.c funcs.a(pair.o)

tests/pathchkperm: tests/pathchkperm.c funcs.a(path.o)

tests/pathchkloc: tests/pathchkloc.c funcs.a(path.o)

tests/pathsuffix: tests/pathsuffix.c funcs.a(path.o)

tests/privdrop: tests/privdrop.c funcs.a(priv.o)

tests/privsuspend: tests/privsuspend.c funcs.a(priv.o)

tests/cpstr: tests/cpstr.c funcs.a(str.o)

tests/splitstr: tests/splitstr.c funcs.a(str.o)

tests/userdirexp: tests/userdirexp.c funcs.a(userdir.o)

$(macro_check_bins) $(other_check_bins): $(hdrs)

tests/error.sh: tests/error

tests/main.sh: tests/main tools/badenv tools/badexec tools/ids

tests/pathchkperm.sh: tests/pathchkperm tools/ids

tests/privdrop.sh: tests/privdrop tools/ids tools/runas

tests/privsuspend.sh: tests/privsuspend tools/ids tools/runas

$(check_scripts): tests/main scripts/funcs.sh

scripts/funcs.sh: tools/ids

tools: $(tool_bins)

checks: $(checks)

check: tools/runpara $(checks)

tests/main:
	$(CC) $(LDFLAGS) -DCHECK $(CFLAGS) -o $@ main.c funcs.a $(LDLIBS)

$(macro_check_bins):
	$(CC) $(LDFLAGS) -DCHECK $(CFLAGS) -o $@ $@.c $(LDLIBS)

$(other_check_bins):
	$(CC) $(LDFLAGS) -DCHECK $(CFLAGS) -o $@ $@.c funcs.a $(LDLIBS)

check:
	tools/runpara $(runpara_flags) $(checks)


#
# Cleanup
#

bins = sucgi tests/main $(macro_check_bins) $(other_check_bins) $(tool_bins)

clean:
	rm -f *.a *.o $(bins)


#
# Distribution
#

package = sucgi
version = 0
dist_name = $(package)-$(version)
dist_ar = $(dist_name).tgz
dist_files = *.c *.h *.env *.excl *.m4 *.ex README.rst LICENSE.txt \
	clang-tidy.yml configure prepare cppcheck docs tests tools scripts

distclean: clean
	rm -f compat.h makefile *.log *.lock *.status *.tgz
	rm -rf tmp-* $(dist_name)

$(dist_name):
	mkdir $(dist_name)
	cp -a $(dist_files) $(dist_name)

$(dist_ar): distclean $(dist_name)
	tar -X dist.excl -czf $(dist_ar) $(dist_name)

$(dist_ar).asc: $(dist_ar)
	gpg -qab --batch --yes $(dist_ar)

dist: $(dist_ar)

sigdist: dist $(dist_ar).asc

distcheck: dist
	tar -xzf $(dist_ar)
	$(dist_name)/configure
	cd $(dist_name) && $(MAKE) -e all check dist
	rm -rf $(dist_name)
	rm -f $(dist_ar)


#
# Static code analysis
#

inspect = *.h *.c
scripts = configure prepare scripts/* tests/*.sh

ifnempty(`__clang_tidy', `dnl
clang_tidy_flags = --config-file=clang-tidy.yml

')dnl
ifnempty(`__cppcheck', `dnl
cppcheck_flags = --quiet --force --language=c --std=c99 \
	--project=cppcheck/sucgi.cppcheck --library=posix \
	--library=cppcheck/bsd.cfg --library=cppcheck/funcs.cfg \
	--suppressions-list=cppcheck/suppr.txt --inline-suppr \
	--enable=all --addon=cppcheck/cert.py --addon=misra.py

')dnl
ifnempty(`__flawfinder', `dnl
flawfinder_flags = --falsepositive --dataonly --quiet

')dnl
ifnempty(`__rats', `dnl
rats_flags = --resultsonly --quiet --warning 3

')dnl
analysis:
	! grep -i FIXME $(inspect)
ifnempty(`__clang_tidy', `dnl
	__clang_tidy $(clang_tidy_flags) $(inspect) -- -std=c99
')dnl
ifnempty(`__cppcheck', `dnl
	__cppcheck $(cppcheck_flags) $(inspect)
')dnl
ifnempty(`__flawfinder', `dnl
	__flawfinder $(flawfinder_flags) $(inspect)
')dnl
ifnempty(`__rats', `dnl
	__rats $(rats_flags) $(inspect)
')dnl

shellcheck:
	! grep -i FIXME $(scripts)
ifnempty(`__shellcheck', `dnl
	__shellcheck -x $(scripts)
')dnl



