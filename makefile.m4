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

.PHONY:	all analysis check clean distcheck distclean shellcheck

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
       funcs.a(path.o) funcs.a(priv.o)  funcs.a(str.o)  funcs.a(userdir.o)


#
# Build configuration
#

.SUFFIXES: .m4

makefile: makefile.m4

build.h: build.h.m4

compat.h: compat.h.m4

.m4:
ifhasfile(`config.status', `dnl
	./config.status $@
', `dnl
	m4 $@.m4 >$@
')dnl


#
# Build
#

funcs.a(env.o): env.c env.h funcs.a(str.o)

funcs.a(error.o): error.c error.h

funcs.a(file.o): file.c file.h funcs.a(str.o)

funcs.a(path.o): path.c path.h funcs.a(file.o) funcs.a(str.o)

funcs.a(priv.o): priv.c priv.h

funcs.a(handler.o): handler.c handler.h funcs.a(path.o)

funcs.a(str.o): str.c str.h

funcs.a(userdir.o): userdir.c userdir.h

$(objs): $(hdrs)
	$(CC) $(LDFLAGS) $(CFLAGS) -c $*.c $(LDLIBS)
	$(AR) -crsu funcs.a $%
	rm -f $%

sucgi: main.c build.h config.h testing.h $(hdrs) $(objs)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ main.c funcs.a $(LDLIBS)


#
# Installation
#

ifnempty(`__DESTDIR', `DESTDIR = __DESTDIR
')dnl
PREFIX = default(`__PREFIX', `/usr/local')
wwwgrp = default(`__SC_WWW_GRP', `www-data')
cgidir = default(`__SC_CGI_DIR', `/usr/lib/cgi-bin')
libexec = $(DESTDIR)$(PREFIX)/libexec

$(libexec)/sucgi: sucgi
	mkdir -p $(libexec)
	cp sucgi $(libexec)
	chown 0:$(wwwgrp) $(libexec)/sucgi
	chmod u=rws,g=x,o= $(libexec)/sucgi

$(cgidir)/sucgi: $(libexec)/sucgi
	ln -s $(libexec)/sucgi $(cgidir)/sucgi

install: $(libexec)/sucgi $(cgidir)/sucgi

uninstall:
	rm -f $(cgidir)/sucgi $(libexec)/sucgi


#
# Tests
#

checks = $(macro_check_bins) $(check_scripts) tests/env_is_name \
	tests/env_restore tests/file_is_exe tests/file_is_wexcl \
	tests/handler_lookup tests/path_check_in tests/path_suffix \
	tests/str_cp tests/str_split tests/userdir_resolve

macro_check_bins = tests/ISSIGNED tests/MIN tests/NELEMS tests/SIGNEDMAX

other_check_bins = tests/env_is_name tests/env_restore tests/error \
	tests/file_is_exe tests/file_is_wexcl \
	tests/handler_lookup tests/path_check_wexcl \
	tests/path_check_in tests/path_suffix tests/priv_drop \
	tests/priv_suspend tests/str_cp tests/str_split \
	tests/userdir_resolve

check_scripts =	tests/error.sh tests/main.sh tests/path_check_wexcl.sh \
	tests/priv_drop.sh tests/priv_suspend.sh

tool_bins = tools/badenv tools/badexec tools/ids tools/runpara tools/runas

runpara_flags =	-ci75 -j8

tests/ISSIGNED: tests/ISSIGNED.c

tests/MIN: tests/MIN.c

tests/NELEMS: tests/NELEMS.c

tests/SIGNEDMAX: tests/SIGNEDMAX.c

tests/env_is_name: tests/env_is_name.c funcs.a(env.o)

tests/env_restore: tests/env_restore.c funcs.a(env.o) funcs.a(str.o)

tests/error: tests/error.c funcs.a(error.o)

tests/file_is_exe: tests/file_is_exe.c funcs.a(file.o)

tests/file_is_wexcl: tests/file_is_wexcl.c funcs.a(file.o)

tests/handler_lookup: tests/handler_lookup.c funcs.a(handler.o)

tests/main: main.c build.h config.h testing.h $(hdrs) $(objs)

tests/path_check_wexcl: tests/path_check_wexcl.c funcs.a(path.o)

tests/path_check_in: tests/path_check_in.c funcs.a(path.o)

tests/path_suffix: tests/path_suffix.c funcs.a(path.o)

tests/priv_drop: tests/priv_drop.c funcs.a(priv.o)

tests/priv_suspend: tests/priv_suspend.c funcs.a(priv.o)

tests/str_cp: tests/str_cp.c funcs.a(str.o)

tests/str_split: tests/str_split.c funcs.a(str.o)

tests/userdir_resolve: tests/userdir_resolve.c funcs.a(userdir.o)

$(macro_check_bins) $(other_check_bins): $(hdrs)

tests/error.sh: tests/error

tests/main.sh: tests/main tools/badenv tools/badexec tools/ids

tests/path_check_wexcl.sh: tests/path_check_wexcl tools/ids

tests/priv_drop.sh: tests/priv_drop tools/ids tools/runas

tests/priv_suspend.sh: tests/priv_suspend tools/ids tools/runas

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
dist_files = *.c *.h *.env *.excl *.m4 *.sample README.rst LICENSE.txt \
	clang-tidy.yaml configure prepare cppcheck docs tests tools scripts

distclean: clean
	rm -f build.h compat.h makefile *.log *.lock *.status *.tgz
	rm -rf tmp-* $(dist_name)

$(dist_name):
	mkdir $(dist_name)
	cp -r $(dist_files) $(dist_name)
	chmod -R u+rw,go= $(dist_name)

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
	rm -rf $(dist_ar)


#
# Static code analysis
#

ifhascmd(`clang-tidy', `define(`__clang_tidy', `findcmd(`clang-tidy')')')dnl
ifhascmd(`cppcheck', `define(`__cppcheck', `findcmd(`cppcheck')')')dnl
ifhascmd(`flawfinder', `define(`__flawfinder', `findcmd(`flawfinder')')')dnl
ifhascmd(`rats', `define(`__rats', `findcmd(`rats')')')dnl
ifhascmd(`shellcheck', `define(`__shellcheck', `findcmd(`shellcheck')')')dnl

inspect	= *.h *.c
scripts = configure prepare scripts/* tests/*.sh

ifnempty(`__clang_tidy', `dnl
clang_tidy_flags = --config-file=clang-tidy.yaml

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



