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

all: sucgi

.PHONY: all analysis check clean distcheck distclean shellcheck

.IGNORE: analysis shellcheck


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
# Build configuration
#

makefile: makefile.m4

compat.h: compat.h.m4

makefile compat.h:
	[ -e config.status ] && ./config.status $@ || m4 $@.m4 >$@


#
# Build
#

hdrs = cattr.h compat.h macros.h max.h types.h

objs = env.o error.o handler.o pair.o path.o priv.o str.o userdir.o

libs = libsucgi.a

compat.h: config.h

params.h: config.h

testing.h: params.h

max.h: config.h

libsucgi.a(env.o): env.c env.h libsucgi.a(str.o)

libsucgi.a(error.o): error.c error.h

libsucgi.a(pair.o): pair.c pair.h

libsucgi.a(path.o): path.c path.h libsucgi.a(str.o)

libsucgi.a(priv.o): priv.c priv.h

libsucgi.a(handler.o): handler.c handler.h libsucgi.a(pair.o path.o)

libsucgi.a(str.o): str.c str.h

libsucgi.a(userdir.o): userdir.c userdir.h libsucgi.a(str.o)

libsucgi.a($(objs)): $(hdrs)

libsucgi.a: libsucgi.a($(objs))

sucgi: main.c params.h testing.h $(hdrs) $(libs)

libsucgi.a($(objs)):
	$(CC) $(LDFLAGS) -c -o $*.o $(CFLAGS) $*.c $(LDLIBS)
	$(AR) $(ARFLAGS) libsucgi.a $*.o
	rm -f $*.o

sucgi:
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ main.c $(libs) $(LDLIBS)


#
# Installation
#

ifnempty(`__DESTDIR', `DESTDIR = __DESTDIR
')dnl
PREFIX = default(`__PREFIX', `/usr/local')
www_grp = default(`__SUCGI_WWW_GRP', `www-data')
cgi_dir = default(`__SUCGI_CGI_DIR', `/usr/lib/cgi-bin')
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

check_objs = tests/check.o tests/priv.o tests/str.o

check_libs = tests/libcheck.a $(libs)

macro_check_bins = tests/ISSIGNED tests/NELEMS tests/SIGNEDMAX

other_check_bins = tests/envcopyvar tests/envisname tests/envrestore \
	tests/error tests/handlerfind tests/pairfind tests/pathchkloc \
	tests/pathreal tests/pathsuffix tests/privdrop tests/privsuspend \
	tests/copystr tests/getspecstrs tests/splitstr tests/userdirexp

check_bins = $(macro_check_bins) $(other_check_bins)

check_scripts = tests/main.sh tests/error.sh

checks = $(check_scripts) $(macro_check_bins) tests/envcopyvar \
	tests/envisname tests/envrestore tests/handlerfind tests/pairfind \
	tests/pathchkloc tests/pathsuffix tests/privdrop tests/privsuspend \
	tests/copystr tests/getspecstrs tests/splitstr tests/userdirexp

tool_bins = tools/badenv tools/badexec tools/uids tools/runpara tools/runas

preloadvar = default(`__SUCGI_PRELOAD_VAR', `LD_PRELOAD')

runpara_flags = -ci75 -j8

tests/libcheck.a(tests/check.o): tests/check.c tests/check.h

tests/libcheck.a(tests/priv.o): tests/priv.c tests/priv.h

tests/libcheck.a(tests/str.o): tests/str.c tests/str.h

tests/libcheck.a: tests/libcheck.a($(check_objs))

ifnempty(`__SUCGI_SHARED_LIBS', `dnl
tests/mockstd.o: tests/mockstd.c tests/mockstd.h

tests/libmock.so: tests/mockstd.o

')dnl
tests/ISSIGNED: tests/ISSIGNED.c

tests/NELEMS: tests/NELEMS.c

tests/SIGNEDMAX: tests/SIGNEDMAX.c

tests/envisname: tests/envisname.c libsucgi.a(env.o)

tests/envrestore: tests/envrestore.c params.h libsucgi.a(env.o)

tests/envrestore: tests/libcheck.a(tests/str.o)

tests/envcopyvar: tests/envcopyvar.c libsucgi.a(env.o)

tests/error: tests/error.c libsucgi.a(error.o)

tests/handlerfind: tests/handlerfind.c libsucgi.a(handler.o)

tests/main: main.c params.h testing.h $(hdrs) libsucgi.a

tests/pairfind: tests/pairfind.c params.h libsucgi.a(pair.o)

tests/pathchkloc: tests/pathchkloc.c libsucgi.a(path.o)

tests/pathreal: tests/pathreal.c libsucgi.a(path.o)

tests/pathsuffix: tests/pathsuffix.c libsucgi.a(path.o)

tests/privdrop: tests/privdrop.c libsucgi.a(priv.o)

tests/privsuspend: tests/privsuspend.c libsucgi.a(priv.o)

tests/privdrop tests/privsuspend: tests/libcheck.a(tests/priv.o)

tests/copystr: tests/copystr.c libsucgi.a(str.o)

tests/getspecstrs: tests/getspecstrs.c libsucgi.a(str.o)

tests/splitstr: tests/splitstr.c libsucgi.a(str.o)

tests/userdirexp: tests/userdirexp.c libsucgi.a(userdir.o)

$(check_bins): $(hdrs) tests/check.h

$(other_check_bins): tests/libcheck.a(tests/check.o)

tests/main.sh: tests/main tools/badenv tools/badexec tools/uids tools/runas

tests/error.sh: tests/error tests/main

tests/funcs.sh: tools/uids

scripts/funcs.sh: tests/funcs.sh

$(check_scripts): tests/main tests/funcs.sh

tools: $(tool_bins)

checks: $(checks)

ifnempty(`__SUCGI_SHARED_LIBS', `dnl
check: tools/runpara $(checks) tests/libmock.so
', `dnl
check: tools/runpara $(checks)
')dnl

tests/libcheck.a($(check_objs)):
	$(CC) $(LDFLAGS) -c -o $*.o $(CFLAGS) $*.c $(LDLIBS)
	$(AR) $(ARFLAGS) tests/libcheck.a $*.o
	rm -f $*.o

ifnempty(`__SUCGI_SHARED_LIBS', `dnl
tests/mock.o:
	$(CC) $(LDFLAGS) -c -o $@ -fpic $(CFLAGS) $< $(LDLIBS)

tests/libmock.so:
	$(CC) $(LDFLAGS) -shared -o $@ -fpic tests/mockstd.o $(LDLIBS)

')dnl
$(macro_check_bins):
	$(CC) $(LDFLAGS) -DCHECK $(CFLAGS) -o $@ $@.c $(LDLIBS)

$(other_check_bins):
	$(CC) $(LDFLAGS) -DCHECK $(CFLAGS) -o $@ $@.c $(check_libs) $(LDLIBS)

tests/main:
	$(CC) $(LDFLAGS) -DCHECK $(CFLAGS) -o $@ main.c $(libs) $(LDLIBS)

check:
ifnempty(`__SUCGI_SHARED_LIBS', `dnl
	[ "$$(id -u)" -eq 0 ] \
&& tools/runpara $(runpara_flags) $(checks) \
|| tools/runpara $(runpara_flags) $(preloadvar)=tests/libmock.so $(checks)
', `dnl
	tools/runpara $(runpara_flags) $(checks)
')dnl


#
# Cleanup
#

bins = sucgi tests/main $(check_bins) $(tool_bins)

covclean: clean

clean:
	find . \( -name '*.a' -o -name '*.o' -o -name '*.so' \) -exec rm '{}' +
	rm -f $(bins)


#
# Distribution
#

package = sucgi
version = 0
dist_name = $(package)-$(version)
dist_ar = $(dist_name).tgz
dist_files = *.c *.h *.env *.excl *.m4 README.rst LICENSE.txt \
	clang-tidy.yml configure prepare cppcheck docs tests tools scripts

distclean: clean

$(dist_name): distclean

$(dist_ar): $(dist_name)

$(dist_ar).asc: $(dist_ar)

dist: $(dist_ar)

sigdist: dist $(dist_ar).asc

distcheck: dist

distclean:
	rm -f compat.h config.status makefile *.tgz
	rm -rf $(dist_name)

$(dist_name):
	mkdir $(dist_name)
	cp -a $(dist_files) $(dist_name)

$(dist_ar):
	tar -X dist.excl -czf $(dist_ar) $(dist_name)

$(dist_ar).asc:
	gpg -qab --batch --yes $(dist_ar)

distcheck:
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

ifnempty(`__SUCGI_CLANG_TIDY', `dnl
clang_tidy_flags = --config-file=clang-tidy.yml

')dnl
ifnempty(`__SUCGI_CPPCHECK', `dnl
cppcheck_flags = --quiet --force --language=c --std=c99 \
	--project=cppcheck/sucgi.cppcheck --library=posix \
	--library=cppcheck/bsd.cfg --library=cppcheck/funcs.cfg \
	--suppressions-list=cppcheck/suppr.txt --inline-suppr \
	--enable=all --addon=cppcheck/cert.py --addon=misra.py

')dnl
ifnempty(`__SUCGI_FLAWFINDER', `dnl
flawfinder_flags = --falsepositive --dataonly --quiet

')dnl
ifnempty(`__SUCGI_RATS', `dnl
rats_flags = --resultsonly --quiet --warning 3

')dnl
ifnempty(`__SUCGI_SHELLCHECK', `dnl
shellcheck_flags = -x

')dnl
analysis:
	! grep -i FIXME $(inspect)
ifnempty(`__SUCGI_CLANG_TIDY', `dnl
	__SUCGI_CLANG_TIDY $(clang_tidy_flags) $(inspect) -- -std=c99
')dnl
ifnempty(`__SUCGI_CPPCHECK', `dnl
	__SUCGI_CPPCHECK $(cppcheck_flags) $(inspect)
')dnl
ifnempty(`__SUCGI_FLAWFINDER', `dnl
	__SUCGI_FLAWFINDER $(flawfinder_flags) $(inspect)
')dnl
ifnempty(`__SUCGI_RATS', `dnl
	__SUCGI_RATS $(rats_flags) $(inspect)
')dnl

shellcheck:
	! grep -i FIXME $(scripts)
ifnempty(`__SUCGI_SHELLCHECK', `dnl
	__SUCGI_SHELLCHECK $(shellcheck_flags) $(scripts)
')dnl
