changequote([, ])dnl
define([default], [ifdef([$1], [ifelse($1, [], [$2], [$1])], [$2])])dnl
.POSIX:

#
# Compiler
#

ifdef([__CC], [ifelse(__CC, [], [], [CC = __CC
])], [])dnl
dnl TODO: Add -DNDEBUG once the software is mature enough.
CFLAGS = default([__CFLAGS], [-O2 -s -ftrapv])


#
# Objects
#

core_objs = lib.a(err.o) lib.a(str.o)

check_err_objs = lib.a(err.o) lib.a(tools/lib.o)
check_env_objs = lib.a(env.o) lib.a(tools/lib.o)
check_file_objs = lib.a(file.o) lib.a(tools/lib.o)
check_gids_objs = lib.a(gids.o) lib.a(tools/lib.o)
check_path_objs = lib.a(path.o) lib.a(tools/lib.o)
check_priv_objs = lib.a(priv.o) lib.a(tools/lib.o)
check_scpt_objs = lib.a(scpt.o) lib.a(tools/lib.o)
check_str_objs = lib.a(str.o) lib.a(tools/lib.o)


#
# Tests
#

check_bins =	tools/evilenv tools/runas tools/unallocids tools/getlogname \
		tests/error tests/env_clear tests/env_file_openat tests/env_get_fname \
		tests/env_name_valid tests/env_restore tests/main \
		tests/file_is_exec tests/file_is_wexcl tests/file_safe_open \
		tests/file_safe_stat tests/gids_get_list tests/priv_drop \
		tests/path_check_wexcl tests/path_contains \
		tests/scpt_get_handler tests/str_cp tests/str_matchv \
		tests/str_split tests/try

checks =	tests/error.sh tests/env_clear tests/env_file_openat.sh tests/env_get_fname.sh \
		tests/env_name_valid tests/env_restore tests/main.sh \
		tests/file_is_exec.sh tests/file_is_wexcl.sh \
		tests/file_safe_open.sh tests/file_safe_stat.sh \
		tests/gids_get_list.sh tests/priv_drop.sh \
		tests/path_check_wexcl.sh tests/path_contains \
		tests/scpt_get_handler tests/str_cp \
		tests/str_matchv tests/str_split tests/try


#
# Analysers
#

cppchk_flags =	--quiet --error-exitcode=8 \
		--language=c --std=c99 --library=posix --platform=unix64 \
		--project=cppcheck/sucgi.cppcheck \
		--library=cppcheck/library.cfg \
		--suppressions-list=cppcheck/suppressions.txt  \
		--force --inconclusive \
		-UTESTING -D__NR_openat2=437 -DPATH_MAX=1024U \
		-DUID_MAX=2147483647U -DLOG_PERROR=32

cppchk_addons =	--addon=cppcheck/cert.py --addon=misra.py


#
# Distribution
#

package = sucgi
version = 0
dist_name = $(package)-$(version)
dist_ar = $(dist_name).tgz
dist_files = *.c *.h *.m4 configure configure.env README.rst tests tools


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

lib.a(env.o): env.c env.h defs.h $(core_objs) lib.a(file.o) lib.a(path.o)

lib.a(err.o): err.c err.h defs.h

lib.a(file.o): file.c file.h defs.h $(core_objs) lib.a(path.o)

lib.a(gids.o): gids.c gids.h defs.h lib.a(err.o)

lib.a(path.o): path.c path.h defs.h $(core_objs)

lib.a(priv.o): priv.o priv.h defs.h lib.a(err.o)

lib.a(scpt.o): scpt.c scpt.h defs.h $(core_objs)

lib.a(str.o): str.c str.h defs.h lib.a(err.o)

lib.a(tools/lib.o): tools/lib.c tools/lib.h defs.h lib.a(err.o) lib.a(str.o)

lib.a:	lib.a(env.o)  lib.a(err.o)  lib.a(file.o) lib.a(gids.o) \
	lib.a(path.o) lib.a(priv.o) lib.a(scpt.o) lib.a(str.o)

.c:
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $< lib.a $(LDLIBS)

sucgi: sucgi.c config.h defs.h lib.a

tests/error: tests/error.c lib.a(err.o) 

tests/env_clear: tests/env_clear.c $(check_env_objs)

tests/env_file_openat: tests/env_file_openat.c $(check_env_objs)

tests/env_get_fname: tests/env_get_fname.c $(check_env_objs)

tests/env_name_valid: tests/env_name_valid.c $(check_env_objs)

tests/env_restore: tests/env_restore.c $(check_env_objs)

tests/file_is_exec: tests/file_is_exec.c $(check_file_objs)

tests/file_is_wexcl: tests/file_is_wexcl.c $(check_file_objs)

tests/file_safe_open: tests/file_safe_open.c $(check_file_objs)

tests/file_safe_stat: tests/file_safe_stat.c $(check_file_objs)

tests/gids_get_list: tests/gids_get_list.c $(check_gids_objs)

tests/path_contains: tests/path_contains.c $(check_path_objs) 

tests/path_check_wexcl:	tests/path_check_wexcl.c $(check_path_objs)

tests/priv_drop: tests/priv_drop.c $(check_priv_objs) 

tests/scpt_get_handler: tests/scpt_get_handler.c $(check_scpt_objs)

tests/str_cp: tests/str_cp.c $(check_str_objs) 

tests/str_eq: tests/str_eq.c $(check_str_objs) 

tests/str_matchv: tests/str_matchv.c $(check_str_objs) 

tests/str_split: tests/str_split.c $(check_str_objs)

tests/try: tests/try.c $(check_err_objs)

tests/main: sucgi.c config.h defs.h lib.a
	$(CC) -D TESTING=1 $(LDFLAGS) $(CFLAGS) -o $@ $< lib.a $(LDLIBS)

tools/evilenv: tools/evilenv.c lib.a(tools/lib.o)

tools/runas: tools/runas.c lib.a(tools/lib.o)

tools/unallocids: tools/unallocids.c lib.a(tools/lib.o)

tools/getlogname: tools/getlogname.c lib.a(tools/lib.o)

clean:
	find . '(' -name '*.o' \
        -o -name '*.ctu-info' -o -name '*.dump' \
        -o -name '*.gcda' -o -name '*.gcno' \
        -o -name 'tmp-*' \
       ')' -exec rm -rf '{}' +
	rm -rf lib.a cov sucgi $(check_bins) $(dist_name) $(dist_name).*

analysis:
	find * ! -name 'makefile*' -exec grep -nri fixme '{}' +
	flawfinder --error-level=1 -m 0 -D -Q .
	rats --resultsonly -w3 .
	cppcheck $(cppchk_flags) --enable=all $(cppchk_addons) .
	cppcheck $(cppchk_flags) --enable=unusedFunction *.c .
	find * -type f -name '*.sh' -exec shellcheck configure '{}' +

check: $(check_bins)
	tools/check.sh $(checks)

cov: clean
	make CC=$(CC) CFLAGS=--coverage $(check_bins)
	-tools/check.sh -s $(checks)

lcov.info: cov
	lcov -c -d . -o lcov.info --exclude '*/tests/*' --exclude '*/tools/*'

cov/index.html: lcov.info
	genhtml -o cov lcov.info

covhtml: cov/index.html

dist: $(dist_ar) $(dist_ar).asc

distclean: clean
	rm -f config.h lcov.info makefile

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
