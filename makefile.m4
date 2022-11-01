dnl Template for the makefile.
dnl
dnl Edit this file, not the makefile itself, and generate the makefile
dnl by calling ./configure. See docs/BUILDING.rst for details.
dnl
changequote([, ])dnl
define([default], [ifdef([$1], [ifelse($1, [], [$2], [$1])], [$2])])dnl
define([ifnempty], [ifdef([$1], [ifelse($1, [], [], [$2])], [])])dnl
# Do not edit this file! It may get overwritten! Edit makefile.m4 instead and
# generate the makefile using ./configure. See docs/BUILDING.rst for details.
.POSIX:

#
# Compiler variables
#

ifnempty([__CC__], [CC = __CC__
])dnl
CFLAGS = default([__CFLAGS__], [-O2 -s -ftrapv])
ifnempty([__ARFLAGS__], [ARFLAGS = __ARFLAGS__
])dnl
ifnempty([__LDFLAGS__], [LDFLAGS = __LDFLAGS__
])dnl
ifnempty([__LDLIBS__], [LDFLAGS = __LDFLAGS__
])dnl

cov_cc = default([__cov_cc__], [$(CC)])


#
# Default libraries
#

core_objs = lib.a(error.o) lib.a(str.o)


#
# Tests suite
#

tool_bins =	tools/badenv tools/ents tools/owner tools/runas

check_bins =	tests/error tests/env_clear tests/env_file_open \
		tests/env_is_name tests/env_restore tests/main \
		tests/file_is_exec tests/file_is_wexcl tests/file_safe_open \
		tests/gids_get tests/priv_drop tests/path_check_wexcl \
		tests/path_contains tests/script_get_inter tests/str_cp \
		tests/str_split tests/try

checks =	tests/error.sh tests/env_clear tests/env_file_open.sh \
		tests/env_is_name tests/env_restore tests/main.sh \
		tests/file_is_exec.sh tests/file_is_wexcl.sh \
		tests/file_safe_open.sh tests/gids_get.sh \
		tests/priv_drop.sh tests/path_check_wexcl.sh \
		tests/path_contains tests/script_get_inter \
		tests/str_cp tests/str_split tests/try

bins =		$(tool_bins) $(check_bins)

#
# Analyser settings
#

inspect = *.c *.h tools/*.c tests/*.c

cppchk_flags =	--quiet --error-exitcode=8 \
		--language=c --std=c99 --platform=unix64 --library=posix \
		--library=cppcheck/library.cfg \
		--project=cppcheck/sucgi.cppcheck \
		--suppressions-list=cppcheck/suppressions.txt \
		--force --inconclusive

cppchk_addons =	--addon=cppcheck/cert.py --addon=misra.py


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

ifnempty([__DESTDIR__], [DESTDIR = __DESTDIR__
])dnl
PREFIX = default([__PREFIX__], [/usr/local])
install_dir = $(DESTDIR)$(PREFIX)
www_grp = default([__www_grp__], [www-data])
cgi_dir = default([__cgi_dir__], [/usr/lib/cgi-bin])


#
# Build targets
#

all: sucgi

lib.a(env.o):	env.c env.h config.h macros.h \
		$(core_objs) lib.a(file.o) lib.a(path.o)

lib.a(error.o):	error.c error.h macros.h

lib.a(file.o):	file.c file.h macros.h $(core_objs) lib.a(path.o)

lib.a(gids.o):	gids.c gids.h config.h macros.h lib.a(error.o)

lib.a(path.o):	path.c path.h macros.h $(core_objs)

lib.a(priv.o):	priv.o priv.h macros.h lib.a(error.o)

lib.a(script.o):	script.c script.h macros.h $(core_objs)

lib.a(str.o):	str.c str.h config.h macros.h lib.a(error.o)

lib.a:	lib.a(env.o)  lib.a(error.o)  lib.a(file.o) lib.a(gids.o) \
	lib.a(path.o) lib.a(priv.o) lib.a(script.o) lib.a(str.o)

$(tool_bins):
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.c $(LDLIBS)

tools/badenv: tools/badenv.c

tools/ents: tools/ents.c

tools/runas: tools/runas.c

.c:
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $< lib.a $(LDLIBS)

tests/error: tests/error.c lib.a(error.o) 

tests/env_clear: tests/env_clear.c lib.a(env.o)

tests/env_file_open: tests/env_file_open.c lib.a(env.o) lib.a(error.o)

tests/env_is_name: tests/env_is_name.c lib.a(env.o)

tests/env_restore: tests/env_restore.c lib.a(env.o)

tests/file_is_exec: tests/file_is_exec.c lib.a(file.o)

tests/file_is_wexcl: tests/file_is_wexcl.c lib.a(file.o)

tests/file_safe_open: tests/file_safe_open.c lib.a(file.o) lib.a(error.o)

tests/gids_get: tests/gids_get.c lib.a(gids.o)

tests/path_contains: tests/path_contains.c lib.a(path.o) 

tests/path_check_wexcl: tests/path_check_wexcl.c lib.a(path.o) lib.a(error.o)

tests/priv_drop: tests/priv_drop.c lib.a(priv.o) lib.a(error.o)

tests/script_get_inter: tests/script_get_inter.c lib.a(script.o)

tests/str_cp: tests/str_cp.c lib.a(str.o) 

tests/str_split: tests/str_split.c lib.a(str.o)

tests/try: tests/try.c lib.a(error.o)

tests/main: sucgi.c config.h macros.h lib.a
	$(CC) -DTESTING=1 $(LDFLAGS) $(CFLAGS) -o $@ $< lib.a $(LDLIBS)

dnl TODO: Add -DNDEBUG once the software is mature enough.
sucgi: sucgi.c config.h macros.h lib.a


#
# Phony targets
#

clean:
	find . '(' \
           -name '*.ctu-info' -o -name '*.dump' \
        -o -name '*.gcda'     -o -name '*.gcno' \
       ')' -exec rm -rf '{}' +
	rm -rf *.o lib.a cov sucgi tmp-* $(bins) $(dist_name) $(dist_name).* 

check: $(tool_bins) $(check_bins)
	tools/check $(checks)

dist: $(dist_ar) $(dist_ar).asc

distclean: clean
	rm -f config.status lcov.info makefile

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

analysis:
	grep -nri fixme $(inspect)
	#flawfinder --error-level=1 -m 0 -D -Q $(inspect)
	#rats --resultsonly -w3 *.c *.h $(inspect)
	#cppcheck $(cppchk_flags) --enable=all $(cppchk_addons) $(inspect)

shellcheck:
	grep -nri fixme configure *.env tools/check tools/lib.sh tests/*.sh
	shellcheck -x configure *.env tools/check tools/lib.sh tests/*.sh

cov: clean $(tool_bins)
	chown -R "$$(tools/owner .)" tools/*
	make CC=$(cov_cc) CFLAGS="-O2 --coverage" $(check_bins)
	-tools/check -qs $(check_bins)
	find . '(' -name '*.gcda' -o -name '*.gcno' ')' \
	-exec chown "$$(tools/owner .)" '{}' +
	tools/check -s $(checks)

lcov.info: cov tools/owner
	lcov -c -d . -o $@ --exclude '*/tests/*' --exclude '*/tools/*' \
		--exclude '/Library/*'
	chown "$$(tools/owner .)" $@

cov/index.html: lcov.info tools/owner
	genhtml -o cov lcov.info
	chown -R "$$(tools/owner .)" cov

covhtml: cov/index.html

.PHONY:	all analysis check clean cov covhtml \
	dist distcheck distclean install uninstall

.IGNORE: analysis shellcheck
