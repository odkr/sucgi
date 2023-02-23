dnl Template for the makefile.
dnl
dnl Edit this file, NOT the makefile itself.
dnl Then run ./configure to generate or ./config.status to update it.
dnl See docs/build.rst for details.
dnl
include(`m4/macros.m4')dnl
.POSIX:

#
# DO NOT EDIT THIS FILE! IT MAY GET OVERWRITTEN!
#
# Edit makefile.m4 instead and then either run ./configure to generate the
# makefile or ./config.status to update it. See docs/build.rst for details.
#

#
# Compiler variables
#

ifnempty(`__CC__', `CC = __CC__
')dnl
CFLAGS = default(`__CFLAGS__', `-O2 -s')
ifnempty(`__ARFLAGS__', `ARFLAGS = __ARFLAGS__
')dnl
ifnempty(`__LDFLAGS__', `LDFLAGS = __LDFLAGS__
')dnl
ifnempty(`__LDLIBS__', `LDFLAGS = __LDFLAGS__
')dnl

SC_COV_CC = default(`__SC_COV_CC__', `$(CC)')


#
# Headers
#

stdhdrs = cattr.h config.h macros.h max.h types.h


#
# Test suite
#

repo_owner = default(`__sc_repo_owner__', `$$(ls -ld . | awk "{print \$$3}")')

tool_bins = tools/badenv tools/badexec tools/ids tools/runpara tools/runas

macro_checks = tests/ISSIGNED tests/MIN tests/NELEMS tests/SIGNEDMAX

check_bins = tests/env_is_name tests/env_restore \
             tests/error tests/file_is_exe tests/file_is_wexcl \
             tests/handler_lookup tests/main \
             tests/path_check_wexcl tests/path_check_in tests/path_suffix \
             tests/priv_drop tests/priv_suspend \
             tests/str_cp tests/str_split tests/userdir_resolve

checks = $(macro_checks) \
         tests/env_is_name tests/env_restore \
         tests/error.sh tests/file_is_exe tests/file_is_wexcl \
         tests/handler_lookup tests/main.sh \
         tests/path_check_wexcl.sh tests/path_check_in tests/path_suffix \
         tests/priv_drop.sh tests/priv_suspend.sh \
         tests/str_cp tests/str_split tests/userdir_resolve

scripts = configure tools/check tools/chkccs tools/*.sh tests/*.sh

bins = $(tool_bins) $(macro_checks) $(check_bins)


#
# Static code analysis
#

inspect = *.h *.c

cppcheck_flags = --quiet --language=c --std=c99 \
                 --project=stat/sucgi.cppcheck \
                 --library=posix --library=stat/library.cfg \
                 --suppressions-list=stat/suppr.txt --inline-suppr \
                 --enable=all --force --addon=stat/cert.py --addon=misra.py

flawfinder_hitlist = stat/flawfinder.hits

flawfinder_flags = --falsepositive --dataonly --quiet

rats_hitlist = stat/rats.hits

rats_flags = --resultsonly --quiet --warning 3


#
# Distribution
#

package = sucgi
version = 0
dist_name = $(package)-$(version)
dist_ar = $(dist_name).tgz
dist_files = *.c *.h *.env *.excl *.m4 *.sample README.rst LICENSE.txt \
             configure stat docs m4 tests tools


#
# Installer
#

ifnempty(`__DESTDIR__', `DESTDIR = __DESTDIR__
')dnl
PREFIX = default(`__PREFIX__', `/usr/local')
install_dir = $(DESTDIR)$(PREFIX)
SC_WWW_GRP = default(`__SC_WWW_GRP__', `www-data')
SC_CGI_DIR = default(`__SC_CGI_DIR__', `/usr/lib/cgi-bin')


#
# Default target
#

all: sucgi


#
# Commands
#

.m4:
	$(SHELL) ./config.status $@

sucgi:
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ main.c lib.a $(LDLIBS)

$(macro_checks):
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $< tests/lib.o $(LDLIBS)

$(check_bins):
	$(CC) -DTESTING $(LDFLAGS) $(CFLAGS) -o $@ $< lib.a tests/lib.o $(LDLIBS)


#
# Prerequisites
#

.SUFFIXES: .m4

makefile: makefile.m4

build.h: build.h.m4

compat.h: compat.h.m4

lib.a: lib.a(env.o) lib.a(error.o) lib.a(file.o) lib.a(handler.o) \
       lib.a(path.o) lib.a(priv.o) lib.a(str.o) lib.a(userdir.o)

lib.a(env.o): env.c env.h $(stdhdrs) lib.a(str.o)

lib.a(error.o):	error.c error.h $(stdhdrs)

lib.a(file.o): file.c file.h $(stdhdrs) lib.a(str.o)

lib.a(path.o): path.c path.h $(stdhdrs) lib.a(file.o) lib.a(str.o)

lib.a(priv.o): priv.c priv.h $(stdhdrs)

lib.a(handler.o): handler.c handler.h $(stdhdrs) lib.a(path.o)

lib.a(str.o): str.c str.h $(stdhdrs)

lib.a(userdir.o): userdir.c userdir.h $(stdhdrs)

sucgi: main.c build.h compat.h $(stdhdrs) lib.a

$(macro_checks) $(check_bins): $(stdhdrs) tests/lib.o

tests/lib.o: tests/lib.c tests/lib.h $(stdhdrs)

tests/ISSIGNED: tests/ISSIGNED.c

tests/MIN: tests/MIN.c

tests/NELEMS: tests/NELEMS.c

tests/SIGNEDMAX: tests/SIGNEDMAX.c

tests/env_is_name: tests/env_is_name.c lib.a(env.o)

tests/env_restore: tests/env_restore.c lib.a(env.o)

tests/env_restore: tests/env_restore.c lib.a(env.o)

tests/error: tests/error.c lib.a(error.o)

tests/error.sh: tests/error

tests/file_is_exe: tests/file_is_exe.c lib.a(file.o)

tests/file_is_wexcl: tests/file_is_wexcl.c lib.a(file.o)

tests/handler_lookup: tests/handler_lookup.c lib.a(handler.o)

tests/path_check_sub: tests/path_check_sub.c lib.a(path.o)

tests/path_check_wexcl: tests/path_check_wexcl.c lib.a(path.o)

tests/path_check_wexcl.sh: tests/path_check_wexcl

tests/path_check_in: tests/path_check_in.c lib.a(path.o)

tests/path_suffix: tests/path_suffix.c lib.a(path.o)

tests/priv_drop: tests/priv_drop.c lib.a(priv.o)

tests/priv_drop.sh: tests/priv_drop

tests/priv_suspend: tests/priv_suspend.c lib.a(priv.o)

tests/priv_suspend.sh: tests/priv_suspend

tests/str_cp: tests/str_cp.c lib.a(str.o)

tests/str_split: tests/str_split.c lib.a(str.o)

tests/userdir_resolve: tests/userdir_resolve.c lib.a(userdir.o)

tests/main: main.c build.h compat.h lib.a

tests/main.sh: tests/main


#
# Cleanup
#

clean:
	rm -f *.c.* *.o a--.* lib.a sucgi tests/lib.o $(bins) $(dist_name).*
	rm -rf tmp-* $(dist_name)
	find . '(' \
           -name '*.ctu-info'  -o -name '*.dump'			\
        -o -name '*.gcda'      -o -name '*.gcno'  -o -name '*.dSYM'	\
       ')' -exec rm -rf '{}' +


#
# Tests
#

check: $(tool_bins) $(checks)
	tools/runpara -i75 $(check_args) $(checks)


#
# Distribution
#

dist: $(dist_ar) $(dist_ar).asc

distclean: clean
	rm -rf build.h config.status compat.h cov gcov lcov.info makefile

distcheck: $(dist_ar)
	tar -xzf $(dist_ar)
	$(dist_name)/configure
	cd $(dist_name) && cp config.h.sample config.h && make all check dist
	rm -rf $(dist_ar)

$(dist_ar): distclean
	mkdir $(dist_name)
	cp -r $(dist_files) $(dist_name)
	chmod -R u+rw,go= $(dist_name)
	tar -X dist.excl -czf $(dist_ar) $(dist_name)
	rm -rf $(dist_name)

$(dist_ar).asc: $(dist_ar)
	gpg -qab --batch --yes $(dist_ar)


#
# Installation
#

$(install_dir)/libexec/sucgi: sucgi
	mkdir -p $(install_dir)/libexec
	cp sucgi $(install_dir)/libexec
	chown 0:$(SC_WWW_GRP) $(install_dir)/libexec/sucgi
	chmod u=rws,g=x,o= $(install_dir)/libexec/sucgi

$(SC_CGI_DIR)/sucgi: $(install_dir)/libexec/sucgi
	ln -s $(install_dir)/libexec/sucgi $(SC_CGI_DIR)/sucgi

install: $(install_dir)/libexec/sucgi $(SC_CGI_DIR)/sucgi

uninstall:
	rm -f $(SC_CGI_DIR)/sucgi $(install_dir)/libexec/sucgi


#
# Coverage reports
#

cov: clean $(tool_bins) 
	make CC=$(SC_COV_CC) CFLAGS="--coverage -O2" \
		$(macro_checks) $(check_bins)
	tools/runpara -cj1 $(macro_checks) $(check_bins) || :
	chown -R "$(repo_owner)" .
	tools/runpara -i75 -j1 $(checks)

gcov: cov
	ar -t lib.a | sed -n '/^.*\.o$$/p' | xargs gcov -p sucgi
	mkdir -p gcov
	find . -type f -name '*.gcov' -exec mv '{}' gcov ';'
	chown -R "$(repo_owner)" gcov

lcov.info: cov
	lcov -c -d . -o $@ --exclude '*/tests/*' --exclude '*/tools/*' \
		--exclude '/usr/*' --exclude '/Library/*'
	chown "$(repo_owner)" $@

cov/index.html: lcov.info
	genhtml -o cov lcov.info
	chown -R "$(repo_owner)" cov

covhtml: cov/index.html


#
# Analysis
#

cppcheck:
	cppcheck $(cppcheck_flags) $(inspect)

$(flawfinder_hitlist): $(inspect)
	flawfinder --savehitlist=$(flawfinder_hitlist) \
           $(flawfinder_flags) $(inspect)

flawfinder:
	[ -e $(flawfinder_hitlist) ] \
	&& flawfinder --diffhitlist=$(flawfinder_hitlist) \
	              $(flawfinder_flags) $(inspect) \
	|| flawfinder $(flawfinder_flags) $(inspect)

$(rats_hitlist): $(inspect)
	rats $(rats_flags) $(inspect) \
	| sed -n '/^.*\.c:[1-9][0-9]*:/p' >$(rats_hitlist)

rats:
	[ -e $(rats_hitlist) ] \
	&& rats $(rats_flags) $(inspect) \
	   | sed -n '/^.*\.c:[1-9][0-9]*:/p' \
	   | diff $(rats_hitlist) - \
	|| rats $(rats_flags) $(inspect)

shellcheck:
	shellcheck -x $(scripts)

analysis: cppcheck flawfinder rats


#
# Special targets
#

.PHONY:	all analysis check clean cov covhtml cppcheck \
        dist distcheck distclean flawfinder install uninstall shellcheck

.IGNORE: analysis
