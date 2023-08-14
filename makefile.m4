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

.PHONY: all analysis check clean distcheck distclean mrproper shellcheck tidy

.IGNORE: analysis shellcheck

.SUFFIXES: .8 .gv .md .pdf .svg


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

hdrs = attr.h compat.h config.h macros.h params.h types.h

objs = env.o error.o groups.o handler.o pair.o path.o priv.o str.o userdir.o

libs = libsucgi.a

env.o: env.c env.h

error.o: error.c error.h

groups.o: groups.c groups.h

pair.o: pair.c pair.h

path.o: path.c path.h

priv.o: priv.c priv.h

handler.o: handler.c handler.h

str.o: str.c str.h

userdir.o: userdir.c userdir.h

$(objs): $(hdrs)

libsucgi.a: $(objs)

sucgi: main.c $(hdrs) libsucgi.a

libsucgi.a: $(objs)
	$(AR) $(ARFLAGS) libsucgi.a $?

sucgi:
	$(CC) $(LDFLAGS) $(CFLAGS) -o$@ main.c $(libs) $(LDLIBS)


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

# tests/libutil.a
util_objs = tests/util/abort.o tests/util/array.o tests/util/dir.o \
	tests/util/path.o tests/util/sigs.o tests/util/str.o \
	tests/util/tmpdir.o tests/util/user.o

tests/util/abort.o: tests/util/abort.c tests/util/abort.h

tests/util/array.o: tests/util/array.c tests/util/array.h

tests/util/dir.o: tests/util/dir.c tests/util/dir.h

tests/util/path.o: tests/util/path.c tests/util/path.h

tests/util/sigs.o: tests/util/sigs.c tests/util/sigs.h

tests/util/str.o: tests/util/str.c tests/util/str.h

tests/util/tmpdir.o: tests/util/tmpdir.c tests/util/tmpdir.h

tests/util/user.o: tests/util/user.c tests/util/user.h

$(util_objs): tests/util/types.h

tests/libutil.a: $(util_objs)

$(util_objs):
	$(CC) $(LDFLAGS) -c -o$@ $< $(CFLAGS) $(LDLIBS)

tests/libutil.a:
	$(AR) $(ARFLAGS) tests/libutil.a $?

ifnempty(`__SUCGI_SHARED', `dnl
ifelse(__SUCGI_SHARED, `-dynamiclib', `dnl
# tests/libmock.dylib
mock_lib=tests/libmock.dylib
', __SUCGI_SHARED, `-shared', `dnl
# tests/libmock.so
mock_lib=tests/libmock.so
')dnl

mock_objs = tests/mock/mockstd.o

tests/mock/mockstd.o: tests/mock/mockstd.c tests/mock/mockstd.h

$(mock_lib): $(mock_objs)

$(mock_objs):
ifelse(`index(__CFLAGS, `-fsanitize')', `-1', `dnl
	$(CC) $(LDFLAGS) -c -o$@ $(CFLAGS) -fpic $< $(LDLIBS)
', `dnl
	$(CC) $(LDFLAGS) -c -o$@ $(CFLAGS) -fpic -fno-sanitize=all $< $(LDLIBS)
')dnl

$(mock_lib):
	$(CC) $(LDFLAGS) __SUCGI_SHARED -o $@ -fpic $(mock_objs) $(LDLIBS)

')dnl ifnempty __SUCGI_SHARED

# Unit tests
macro_test_bins = tests/ISSIGNED tests/NELEMS tests/MAXSVAL

env_test_bins = tests/env_copy_var tests/env_is_name tests/env_restore

handler_test_bins = tests/handler_find

groups_test_bins = tests/groups_comp

pair_test_bins = tests/pair_find

path_test_bins = tests/path_real tests/path_suffix tests/path_within

priv_test_bins = tests/priv_drop tests/priv_suspend

str_test_bins = tests/str_copy tests/str_fmtspecs tests/str_split

userdir_test_bins = tests/userdir_exp

unit_libs = libsucgi.a tests/libutil.a

unit_bins = $(macro_test_bins) $(env_test_bins) $(handler_test_bins) \
	$(groups_test_bins) $(pair_test_bins) $(path_test_bins) \
	$(priv_test_bins) $(str_test_bins) $(userdir_test_bins)

tests/ISSIGNED: tests/ISSIGNED.c

tests/NELEMS: tests/NELEMS.c

tests/MAXSVAL: tests/MAXSVAL.c

tests/env_copy_var: tests/env_copy_var.c

tests/env_is_name: tests/env_is_name.c

tests/env_restore: tests/env_restore.c

tests/handler_find: tests/handler_find.c

tests/groups_comp: tests/groups_comp.c

tests/pair_find: tests/pair_find.c params.h

tests/path_real: tests/path_real.c

tests/path_suffix: tests/path_suffix.c

tests/path_within: tests/path_within.c

tests/priv_drop: tests/priv_drop.c

tests/priv_suspend: tests/priv_suspend.c

tests/str_copy: tests/str_copy.c

tests/str_fmtspecs: tests/str_fmtspecs.c

tests/str_split: tests/str_split.c

tests/userdir_exp: tests/userdir_exp.c

$(unit_bins): $(unit_libs)

$(unit_bins):
	$(CC) $(LDFLAGS) -DTESTING $(CFLAGS) -o $@ $@.c $(unit_libs) $(LDLIBS)


# Utilities
util_bins = utils/badenv utils/badexec utils/uids utils/runpara utils/runas

utils: $(util_bins)


# Scripted tests
check_scripts = tests/scripts/BUG tests/scripts/error tests/scripts/main

script_bins = tests/BUG tests/error tests/main

tests/scripts/BUG: tests/BUG

tests/scripts/error: tests/error

tests/scripts/main: utils/badenv utils/badexec utils/uids utils/runas

tests/scripts/funcs.sh: scripts/funcs.sh

scripts/funcs.sh: utils/uids

tests/BUG: tests/BUG.c

tests/error: tests/error.c

tests/main: main.c

$(check_scripts): tests/main tests/scripts/funcs.sh

$(script_bins): libsucgi.a $(hdrs) tests/util/types.h

tests/BUG tests/error:
	$(CC) $(LDFLAGS) -DTESTING $(CFLAGS) -o $@ $@.c $(libs) $(LDLIBS)

tests/main:
	$(CC) $(LDFLAGS) -DTESTING $(CFLAGS) -o $@ main.c $(libs) $(LDLIBS)


# Execution
check_bins = $(unit_bins) $(script_bins)

checks = $(check_scripts) $(unit_bins)

ifelse(__SUCGI_UNAME, `Darwin', `dnl
preload = DYLD_INSERT_LIBRARIES

environ = MallocNanoZone=0
', `dnl
preload = LD_PRELOAD

environ =
')dnl

runpara_flags = -ci75 -j8

checks: $(checks)

ifnempty(`__SUCGI_SHARED', `dnl
check: utils/runpara $(checks) $(mock_lib)
', `dnl
check: utils/runpara $(checks)
')dnl

check:
ifnempty(`__SUCGI_SHARED', `dnl
	[ "$$(id -u)" -eq 0 ] \
&& $(environ) utils/runpara $(runpara_flags) $(checks) \
|| $(environ) utils/runpara $(runpara_flags) $(preload)=$(mock_lib) $(checks)
', `dnl
	$(environ) utils/runpara $(runpara_flags) $(checks)
')dnl


#
# Cleanup
#

bins = sucgi $(check_bins) $(util_bins)

tidy:
	rm -f $(dist_name).tgz
	find . '(' \
	-name '*.bak' -o \
	-name '*.ctu-info' -o \
	-name '*.expand' -o \
	-name '*.dump' -o \
	-name '*.log' \
	')' -exec rm -f '{}' +
	find . -type d -name 'tmp-*' -exec rm -rf '{}' +

clean: tidy
	rm -f $(bins)
	find . '(' \
	-name '*.a' -o \
	-name '*.o' -o \
	-name '*.so' \
	')' -exec rm -f '{}' +

mrproper: clean
	rm -f cppcheck/build/* docs/callgraph.*
	find . '(' \
	-name '*.gcda' -o \
	-name '*.gcno' -o \
	-name '*.gcov' -o \
	-name '*.info' \
	')' -exec rm -f '{}' +


#
# Documentation
#

.gv.svg:
	dot -Tsvg -o$@ $<

.gv.pdf:
	dot -Tpdf -o$@ $<

.md.8:
	pandoc -Mdate="$$(date +"%B %d, %Y")" -stman -o$@ $(pandoc_flags) $<

callgraph_cflags = -DNDEBUG -O0 -fno-inline-functions

egypt_flags = -omit config,help,usage,version

docs/callgraph.gv: main.c $(objs:.o=.c)
	$(MAKE) CFLAGS="-fdump-rtl-expand $(callgraph_cflags)" clean all
	egypt --callees main $(egypt_flags) *.expand >$@

docs/callgraph.pdf: docs/callgraph.gv

docs/callgraph.svg: docs/callgraph.gv


#
# Distribution
#

package = sucgi
version = 0
dist_name = $(package)-$(version)
dist_ar = $(dist_name).tgz
dist_files = *.c *.h *.m4 README.md LICENSE.txt \
	configure prepare conf cppcheck docs man tests utils scripts

distclean: mrproper

$(dist_name): distclean man/sucgi.8

$(dist_ar): $(dist_name)

$(dist_ar).asc: $(dist_ar)

dist: $(dist_ar)

sigdist: dist $(dist_ar).asc

distcheck: dist

distclean:
	rm -f compat.h config.status makefile $(dist_ar)
	rm -rf $(dist_name)

$(dist_name):
	mkdir $(dist_name)
	cp -a $(dist_files) $(dist_name)

$(dist_ar):
	tar -X conf/dist.excl -czf $(dist_ar) $(dist_name)

$(dist_ar).asc:
	gpg -qab --batch --yes $(dist_ar)

dist: $(dist_ar)
	rm -rf $(dist_name)

distcheck:
	tar -xzf $(dist_ar)
	$(dist_name)/configure
	cd $(dist_name) && $(MAKE) -e all check dist
	rm -rf $(dist_name)


#
# Static code analysis
#

srcs = *.h *.c \
	tests/*.c \
	tests/util/*.h tests/util/*.c \
	tests/mock/*.h tests/mock/*.c

scripts = configure prepare scripts/*

clang_tidy_flags = --quiet

cppcheck_flags = --quiet --force --language=c --std=c99 \
	--project=cppcheck/sucgi.cppcheck --library=posix \
	--library=cppcheck/c99.cfg --library=cppcheck/gnuc.cfg \
	--library=cppcheck/posix.cfg --library=cppcheck/bsd.cfg \
	--library=cppcheck/linux.cfg --library=cppcheck/sucgi.cfg \
	--suppress-xml=cppcheck/suppress.xml --inline-suppr \
	--enable=all --addon=cppcheck/cert.py --addon=misra.py

flawfinder_flags = --falsepositive --dataonly --quiet

rats_flags = --resultsonly --quiet --warning 3

shellcheck_flags = -x

analysis:
	! grep -i FIXME $(srcs)
	clang-tidy $(clang_tidy_flags) $(srcs) -- -std=c99
	cppcheck $(cppcheck_flags) $(srcs)
	flawfinder $(flawfinder_flags) $(srcs)
	rats $(rats_flags) $(srcs)

shellcheck:
	! grep -i FIXME $(scripts)
	shellcheck $(shellcheck_flags) $(scripts) $(check_scripts)

