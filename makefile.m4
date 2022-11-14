dnl Template for the makefile.
dnl
dnl Edit this file, not the makefile itself, and generate the makefile
dnl by calling ./configure. See docs/BUILDING.rst for details.
dnl
changecom()dnl
define(`default', `ifdef(`$1', `ifelse($1, `', `$2', `$1')', `$2')')dnl
define(`ifnempty', `ifdef(`$1', `ifelse($1, `', `', `$2')', `')')dnl
# Do not edit this file! It may get overwritten! Edit makefile.m4 instead and
# generate the makefile using ./configure. See docs/BUILDING.rst for details.
.POSIX:

#
# Compiler variables
#

ifnempty(`__CC__', `CC = __CC__
')dnl
CFLAGS = default(`__CFLAGS__', `-O2 -s -ftrapv')
ifnempty(`__ARFLAGS__', `ARFLAGS = __ARFLAGS__
')dnl
ifnempty(`__LDFLAGS__', `LDFLAGS = __LDFLAGS__
')dnl
ifnempty(`__LDLIBS__', `LDFLAGS = __LDFLAGS__
')dnl

cov_cc = default(`__cov_cc__', `$(CC)')


#
# Object files
#

objs = 	lib.a(env.o)  lib.a(error.o)  lib.a(file.o) lib.a(path.o) \
	lib.a(priv.o) lib.a(script.o) lib.a(str.o)  lib.a(userdir.o)


#
# Headers
#

def_hdrs = max.h attr.h types.h
test_hdrs = $(def_hdrs) tests/testdefs.h


#
# Test suite
#

tool_bins =	tools/badenv tools/ents tools/owner tools/runas

check_bins =	tests/error tests/envfopen tests/envisname tests/envrestore \
		tests/main tests/fileisexe tests/fileiswex tests/filesopen \
		tests/filesopenp tests/privdrop tests/pathchkxcl \
		tests/pathissub tests/scptgetint tests/strcp tests/strsplit \
		tests/userdirres

checks =	tests/error.sh tests/envfopen.sh tests/envisname \
		tests/envrestore tests/main.sh tests/fileisexe.sh \
		tests/fileiswex.sh tests/filesopen.sh tests/filesopenp.sh \
		tests/privdrop.sh tests/pathchkxcl.sh tests/pathissub \
		tests/scptgetint tests/strcp tests/strsplit \
		tests/userdirres.sh

bins =		$(tool_bins) $(check_bins)


#
# Analyser settings
#

inspect = *.c *.h

cppcheck_flags =	--quiet --language=c --std=c99 --platform=unix64 \
			--library=posix --library=cppcheck/library.cfg \
			--project=cppcheck/sucgi.cppcheck \
			--suppressions-list=cppcheck/suppr.txt

cppcheck_addons =	--addon=cppcheck/cert.py --addon=misra.py


#
# Distribution settings
#

package = sucgi
version = 0
dist_name = $(package)-$(version)
dist_ar = $(dist_name).tgz
dist_files = *.c *.h *.m4 configure cppcheck devel.env docs \
	prod.env README.rst tests tools


#
# Installer settings
#

ifnempty(`__DESTDIR__', `DESTDIR = __DESTDIR__
')dnl
PREFIX = default(`__PREFIX__', `/usr/local')
install_dir = $(DESTDIR)$(PREFIX)
www_grp = default(`__www_grp__', `www-data')
cgi_dir = default(`__cgi_dir__', `/usr/lib/cgi-bin')


#
# Default target
#

all: sucgi


#
# Commands
#

dnl TODO: Add -DNDEBUG once the software is mature enough.
sucgi:
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $< lib.a $(LDLIBS)

$(check_bins):
	$(CC) -DTESTING=1 $(LDFLAGS) $(CFLAGS) -o $@ $< lib.a $(LDLIBS)


#
# Prerequisites
#

sucgi: sucgi.c config.h $(def_hdrs) lib.a

$(objs): $(def_hdrs)

lib.a: $(objs)

lib.a(env.o): env.c env.h lib.a(file.o) lib.a(path.o)

lib.a(error.o): error.c error.h

lib.a(file.o): file.c file.h lib.a(str.o)

lib.a(script.o): script.c script.h lib.a(str.o)

lib.a(path.o): path.c path.h lib.a(file.o) lib.a(str.o)

lib.a(priv.o): priv.o priv.h

lib.a(str.o): str.c str.h

lib.a(userdir.o): userdir.c userdir.h

tests/error: tests/error.c $(test_hdrs) lib.a(error.o) 

tests/envfopen: tests/envfopen.c $(test_hdrs) lib.a(env.o)

tests/envisname: tests/envisname.c $(test_hdrs) lib.a(env.o)

tests/envrestore: tests/envrestore.c $(test_hdrs) lib.a(env.o)

tests/fileisexe: tests/fileisexe.c $(test_hdrs) lib.a(file.o)

tests/fileiswex: tests/fileiswex.c $(test_hdrs) lib.a(file.o)

tests/filesopen: tests/filesopen.c $(test_hdrs) lib.a(file.o)

tests/filesopenp: tests/filesopenp.c $(test_hdrs) lib.a(file.o)

tests/scptgetint: tests/scptgetint.c $(test_hdrs) lib.a(script.o)

tests/pathissub: tests/pathissub.c $(test_hdrs) lib.a(path.o) 

tests/pathchkxcl: tests/pathchkxcl.c $(test_hdrs) lib.a(path.o)

tests/privdrop: tests/privdrop.c $(test_hdrs) lib.a(priv.o) lib.a(error.o)

tests/strcp: tests/strcp.c $(test_hdrs) lib.a(str.o) 

tests/strdup: tests/strdup.c $(test_hdrs) lib.a(str.o) 

tests/strsplit: tests/strsplit.c $(test_hdrs) lib.a(str.o)

tests/userdirres: tests/userdirres.c $(test_hdrs) lib.a(userdir.o)

tests/main: sucgi.c config.h $(test_hdrs) lib.a


#
# Cleanup
#

clean:
	find . '(' \
           -name '*.ctu-info'  -o -name '*.dump'                    \
        -o -name '*.gcda'      -o -name '*.gcno'  -o -name '*.dSYM' \
       ')' -exec rm -rf '{}' +
	rm -rf *.o lib.a sucgi tmp-* $(bins) $(dist_name) $(dist_name).* 


#
# Tests
#

check: $(tool_bins) $(check_bins)
	tools/check $(checks)


#
# Distribution
#

dist: $(dist_ar) $(dist_ar).asc

distclean: clean
	rm -rf config.status cov gcov lcov.info makefile

distcheck: $(dist_ar)
	tar -xzf $(dist_ar)
	$(dist_name)/configure
	cd $(dist_name) && make all check dist
	rm -rf $(dist_ar)

$(dist_ar): clean
	mkdir $(dist_name)
	cp -r $(dist_files) $(dist_name)
	chmod -R u+rw,go= $(dist_name)
	tar -czf $(dist_ar) $(dist_name)
	rm -rf $(dist_name)

$(dist_ar).asc: $(dist_ar)
	gpg -qab --batch --yes $(dist_ar)


#
# Installation
#

$(install_dir)/libexec/sucgi: sucgi
	mkdir -p $(install_dir)/libexec
	cp sucgi $(install_dir)/libexec
	chown 0:$(www_grp) $(install_dir)/libexec/sucgi
	chmod u=rws,g=x,o= $(install_dir)/libexec/sucgi

$(cgi_dir)/sucgi: $(install_dir)/libexec/sucgi
	ln -s $(install_dir)/libexec/sucgi $(cgi_dir)/sucgi

install: $(install_dir)/libexec/sucgi $(cgi_dir)/sucgi

uninstall:
	rm -f $(cgi_dir)/sucgi $(DESTDIR)$(PREFIX)/libexec/sucgi


#
# Coverage reports
#

cov: clean $(tool_bins)
	make CC=$(cov_cc) CFLAGS="--coverage -O2" $(check_bins)
	-tools/check -qs $(check_bins)
	chown -R "$$(tools/owner .)" .
	tools/check -s $(checks)

gcov: cov
	ar -t lib.a | sed -n '/^.*\.o$$/p' | xargs gcov -p sucgi
	mkdir -p gcov
	find . -type f -name '*.gcov' -exec mv '{}' gcov ';'
	chown -R "$((tools/owner .)" gcov

lcov.info: cov tools/owner
	lcov -c -d . -o $@ --exclude '*/tests/*' --exclude '*/tools/*' \
		--exclude '/usr/*' --exclude '/Library/*'
	chown "$$(tools/owner .)" $@

cov/index.html: lcov.info tools/owner
	genhtml -o cov lcov.info
	chown -R "$$(tools/owner .)" cov

covhtml: cov/index.html


#
# Code analysis
#

analysis:
	! grep -nri FIXME $(inspect)
	rats --resultsonly $(inspect)
	flawfinder -DQF $(inspect)
	cppcheck $(cppcheck_flags) --enable=all $(cppcheck_addons) $(inspect)

shellcheck:
	! grep -nri FIXME configure tools/check tools/lib.sh tests/*.sh
	shellcheck -x configure tools/check tools/lib.sh tests/*.sh


#
# Special targets
#

.PHONY:	all analysis check clean cov covhtml \
	dist distcheck distclean install uninstall \
	rats shellcheck

.IGNORE: analysis shellcheck
