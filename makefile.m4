changequote([, ])dnl
define([default], [ifdef([$1], [ifelse($1, [], [$2], [$1])], [$2])])dnl
.POSIX:

#
# Compiler
#

ifdef([__CC__], [ifelse(__CC__, [], [], [CC = __CC__
])], [])dnl
CFLAGS = default([__CFLAGS__], [-O2 -s -ftrapv])
COV_CC = default([__COV_CC__], [$(CC)])


#
# Libraries
#

core_objs = lib.a(error.o) lib.a(str.o)


#
# Tests
#

tools =		tools/badenv tools/runas tools/findid

check_bins =	tests/error tests/env_clear tests/env_file_open \
		tests/env_is_name tests/env_restore tests/main \
		tests/file_is_exec tests/file_is_wexcl tests/file_safe_open \
		tests/gids_get_list tests/priv_drop tests/path_check_wexcl \
		tests/path_contains tests/scpt_get_handler tests/str_cp \
		tests/str_split tests/try $(tools)

checks =	tests/error.sh tests/env_clear tests/env_file_open.sh \
		tests/env_is_name tests/env_restore tests/main.sh \
		tests/file_is_exec.sh tests/file_is_wexcl.sh \
		tests/file_safe_open.sh tests/gids_get_list \
		tests/priv_drop.sh tests/path_check_wexcl.sh \
		tests/path_contains tests/scpt_get_handler \
		tests/str_cp tests/str_split tests/try


#
# Analysers
#

cppcheck_flags =	--quiet --error-exitcode=8 \
			--language=c --std=c99 \
			--library=posix --platform=unix64 \
			--project=cppcheck/sucgi.cppcheck \
			--library=cppcheck/library.cfg \
			--suppressions-list=cppcheck/suppressions.txt \
			--force --inconclusive

cppcheck_addons =	--addon=cppcheck/cert.py --addon=misra.py


#
# Distribution
#

package = sucgi
version = 0
dist_name = $(package)-$(version)
dist_ar = $(dist_name).tgz
dist_files = *.c *.h *.m4 configure devel.env prod.env README.rst tests tools


#
# Installer
#

PREFIX = /usr/local
WWW_GRP = www-data
CGI_BIN = /usr/lib/cgi-bin


#
# Targets
#

all: sucgi

lib.a(env.o):	env.c env.h macros.h $(core_objs) lib.a(file.o) lib.a(path.o)

lib.a(error.o):	error.c error.h macros.h

lib.a(file.o):	file.c file.h macros.h $(core_objs) lib.a(path.o)

lib.a(gids.o):	gids.c gids.h macros.h lib.a(error.o)

lib.a(path.o):	path.c path.h macros.h $(core_objs)

lib.a(priv.o):	priv.o priv.h macros.h lib.a(error.o)

lib.a(scpt.o):	scpt.c scpt.h macros.h $(core_objs)

lib.a(str.o):	str.c str.h macros.h lib.a(error.o)

lib.a:	lib.a(env.o)  lib.a(error.o)  lib.a(file.o) lib.a(gids.o) \
	lib.a(path.o) lib.a(priv.o) lib.a(scpt.o) lib.a(str.o)
	
$(tools):
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.c $(LDLIBS)

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

tests/gids_get_list: tests/gids_get_list.c lib.a(gids.o)

tests/path_contains: tests/path_contains.c lib.a(path.o) 

tests/path_check_wexcl:	tests/path_check_wexcl.c lib.a(path.o) lib.a(error.o)

tests/priv_drop: tests/priv_drop.c lib.a(priv.o) lib.a(error.o)

tests/scpt_get_handler: tests/scpt_get_handler.c lib.a(scpt.o)

tests/str_cp: tests/str_cp.c lib.a(str.o) 

tests/str_split: tests/str_split.c lib.a(str.o)

tests/try: tests/try.c lib.a(error.o)

tests/main: sucgi.c config.h macros.h lib.a
	$(CC) -DTESTING=1 $(LDFLAGS) $(CFLAGS) -o $@ $< lib.a $(LDLIBS)

dnl TODO: Add -DNDEBUG once the software is mature enough.
sucgi: sucgi.c config.h macros.h lib.a

clean:
	find . '(' -name '*.o' \
        -o -name '*.ctu-info' -o -name '*.dump' \
        -o -name '*.gcda' -o -name '*.gcno' \
        -o -name 'tmp-*' \
       ')' -exec rm -rf '{}' +
	rm -rf lib.a cov sucgi $(check_bins) $(dist_name) $(dist_name).*

analysis:
	grep -nri fixme configure *.h *.c *.env tools/* tests/* doc/*
	flawfinder --error-level=1 -m 0 -D -Q .
	rats --resultsonly -w3 *.c *.h tools/*.c tests/*.c
	cppcheck $(cppcheck_flags) --enable=all $(cppcheck_addons) .
	cppcheck $(cppcheck_flags) --enable=unusedFunction *.h *.c
	shellcheck configure tools/*.sh tests/*.sh

check: $(check_bins)
	tools/check.sh $(checks)

cov: clean
	make CC=$(COV_CC) CFLAGS="$(CFLAGS) -O2 --coverage" $(check_bins)
	tools/check.sh -qs $(check_bins) || :
	find . '(' -name '*.gcda' -o -name '*.gcno' ')' \
	-exec chown "$$(tools/owner.sh .)" '{}' +
	-tools/check.sh -s $(checks)

lcov.info: cov
	lcov -c -d . -o $@ --exclude '*/tests/*' --exclude '*/tools/*' \
		--exclude '/Library/*'
	chown "$$(tools/owner.sh .)" $@

cov/index.html: lcov.info
	genhtml -o cov lcov.info
	chown -R "$$(tools/owner.sh .)" cov

covhtml: cov/index.html

dist: $(dist_ar) $(dist_ar).asc

distclean: clean
	rm -f lcov.info makefile

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

$(DESTDIR)/$(PREFIX)/libexec/sucgi: sucgi
	cp sucgi $(DESTDIR)/$(PREFIX)/libexec
	chown $(DESTDIR)/$(PREFIX)/libexec/sucgi root:$(WWW_GRP)
	chmod u=rws,g=x,o= $(DESTDIR)/$(PREFIX)/libexec/sucgi

$(CGI_BIN)/sucgi: $(DESTDIR)/$(PREFIX)/libexec/sucgi
	ln -s $(DESTDIR)/$(PREFIX)/libexec/sucgi $(CGI_BIN)/sucgi

install: $(DESTDIR)/$(PREFIX)/libexec/sucgi $(CGI_BIN)/sucgi

uninstall:
	rm -f $(CGI_BIN)/sucgi $(DESTDIR)$(PREFIX)/libexec/sucgi

.PHONY:	all analysis check clean cov covhtml \
	dist distcheck distclean install uninstall

.IGNORE: analysis
