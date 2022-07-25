changequote([, ])dnl
define([default], [ifdef([$1], [ifelse($1, [], [$2], [$1])], [$2])])dnl

.POSIX:

#
# Compiler settings
#

ifdef([__CC], [ifelse(__CC, [], [], [CC = __CC
])], [])dnl
CFLAGS = default([__CFLAGS], [-D_DEFAULT_SOURCE=1 -D_BSD_SOURCE=1 -O2 -s])


#
# Repository settings
#

PACKAGE = suCGI
VERSION = 0
PROJECTDIR = default([__PROJECTDIR], .)


#
# Directories
#

SRCDIR = $(PROJECTDIR)/src
SCRIPTDIR = $(PROJECTDIR)/scripts
BUILDDIR = build
COVDIR = coverage


#
# Tests
#

CHECKBINS = $(BUILDDIR)/tests/drop_privs \
            $(BUILDDIR)/tests/fail $(BUILDDIR)/tests/env_clear \
            $(BUILDDIR)/tests/env_get_fname \
            $(BUILDDIR)/tests/env_restore \
            $(BUILDDIR)/tests/main \
            $(BUILDDIR)/tests/file_is_exec \
            $(BUILDDIR)/tests/file_is_wexcl \
            $(BUILDDIR)/tests/file_safe_open \
            $(BUILDDIR)/tests/file_safe_stat \
            $(BUILDDIR)/tests/path_check_len \
            $(BUILDDIR)/tests/path_check_wexcl \
            $(BUILDDIR)/tests/path_contains \
            $(BUILDDIR)/tests/reraise \
            $(BUILDDIR)/tests/run_script \
            $(BUILDDIR)/tests/str_cp \
            $(BUILDDIR)/tests/str_eq \
            $(BUILDDIR)/tests/str_split \
            $(BUILDDIR)/tests/str_vsplit

CHECKS = $(SCRIPTDIR)/tests/drop_privs.sh \
         $(SCRIPTDIR)/tests/fail.sh \
         $(BUILDDIR)/tests/env_clear \
         $(SCRIPTDIR)/tests/env_get_fname.sh \
         $(BUILDDIR)/tests/env_restore \
         $(SCRIPTDIR)/tests/main.sh \
         $(SCRIPTDIR)/tests/file_is_exec.sh \
         $(SCRIPTDIR)/tests/file_is_wexcl.sh \
         $(SCRIPTDIR)/tests/file_safe_open.sh \
         $(SCRIPTDIR)/tests/file_safe_stat.sh \
         $(BUILDDIR)/tests/path_check_len \
         $(SCRIPTDIR)/tests/path_check_wexcl.sh \
         $(BUILDDIR)/tests/path_contains \
         $(BUILDDIR)/tests/reraise \
         $(SCRIPTDIR)/tests/run_script.sh \
         $(BUILDDIR)/tests/str_cp \
         $(BUILDDIR)/tests/str_eq \
         $(BUILDDIR)/tests/str_split \
         $(BUILDDIR)/tests/str_vsplit


#
# Analysers
#

CPPCHECKFLAGS =	-f -q --error-exitcode=8 --inconclusive \
		--language=c --std=c99 --library=posix --platform=unix64 \
		--suppress=missingIncludeSystem --inline-suppr


#
# Distribution
#

DISTNAME = $(PACKAGE)-$(VERSION)
DISTAR = $(DISTNAME).tgz

DISTFILES = $(PROJECTDIR)/config.h.m4 \
            $(PROJECTDIR)/configure \
            $(PROJECTDIR)/configure.env \
            $(PROJECTDIR)/makefile.m4 \
            $(PROJECTDIR)/README.rst \
            $(SCRIPTDIR) \
            $(SRCDIR)


#
# Installer
#

PREFIX = /usr/local
WWWGRP = www-data
CGIBIN = /usr/lib/cgi-bin


#
# Targets
#

all: $(BUILDDIR)/sucgi

$(BUILDDIR)/.sentinel:
	mkdir -p $(BUILDDIR)
	touch $(BUILDDIR)/.sentinel

$(BUILDDIR)/tests/.sentinel: $(BUILDDIR)/.sentinel
	mkdir $(BUILDDIR)/tests
	touch $(BUILDDIR)/tests/.sentinel

$(BUILDDIR)/env.o:	$(SRCDIR)/env.c $(SRCDIR)/env.h	\
			$(SRCDIR)/attr.h $(SRCDIR)/err.h $(SRCDIR)/str.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) $(GCOVFLAGS) -o $@ $<

$(BUILDDIR)/err.o:	$(SRCDIR)/err.c $(SRCDIR)/err.h \
			$(SRCDIR)/attr.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) $(GCOVFLAGS) -o $@ $<

$(BUILDDIR)/file.o:	$(SRCDIR)/file.c $(SRCDIR)/file.h \
			$(SRCDIR)/attr.h $(SRCDIR)/err.h \
			$(SRCDIR)/path.h $(SRCDIR)/str.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) $(GCOVFLAGS) -o $@ $<

$(BUILDDIR)/path.o:	$(SRCDIR)/path.c $(SRCDIR)/path.h \
			$(SRCDIR)/attr.h $(SRCDIR)/err.h $(SRCDIR)/str.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) $(GCOVFLAGS) -o $@ $<

$(BUILDDIR)/str.o:	$(SRCDIR)/str.c $(SRCDIR)/str.h \
			$(SRCDIR)/attr.h $(SRCDIR)/err.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) $(GCOVFLAGS) -o $@ $<

$(BUILDDIR)/utils.o:	$(SRCDIR)/utils.c $(SRCDIR)/utils.h \
			$(SRCDIR)/attr.h $(SRCDIR)/err.h $(SRCDIR)/str.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) $(GCOVFLAGS) -o $@ $<

$(BUILDDIR)/sucgi:	$(SRCDIR)/main.c config.h \
			$(BUILDDIR)/env.o $(BUILDDIR)/err.o \
			$(BUILDDIR)/file.o $(BUILDDIR)/path.o \
			$(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -DPACKAGE=$(PACKAGE) -DVERSION=$(VERSION) \
		$(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/env.o $(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
		$(LDLIBS)

$(BUILDDIR)/tests/utils.o:	$(SRCDIR)/tests/utils.c \
				$(SRCDIR)/tests/utils.h \
				$(SRCDIR)/str.h \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/tests/drop_privs:	$(SRCDIR)/tests/drop_privs.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/utils.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
		$(LDLIBS)

$(BUILDDIR)/tests/fail:		$(SRCDIR)/tests/fail.c \
				$(BUILDDIR)/err.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/err.o \
		$(LDLIBS)

$(BUILDDIR)/tests/env_clear:	$(SRCDIR)/tests/env_clear.c \
				$(BUILDDIR)/env.o \
				$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
				$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/env.o $(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/env_get_fname:	$(SRCDIR)/tests/env_get_fname.c \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/env.o \
					$(BUILDDIR)/err.o \
					$(BUILDDIR)/file.o \
					$(BUILDDIR)/path.o \
					$(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/env.o $(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/env_restore:	$(SRCDIR)/tests/env_restore.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/env.o $(BUILDDIR)/err.o \
				$(BUILDDIR)/file.o $(BUILDDIR)/path.o \
				$(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/env.o $(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/file_is_exec:	$(SRCDIR)/tests/file_is_exec.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/file.o \
				$(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/file_is_wexcl:	$(SRCDIR)/tests/file_is_wexcl.c \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/file_safe_open:	$(SRCDIR)/tests/file_safe_open.c \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/file.o \
					$(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/file_safe_stat:	$(SRCDIR)/tests/file_safe_stat.c \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/path_check_len:	$(SRCDIR)/tests/path_check_len.c \
					$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
					$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/path_check_wexcl:	$(SRCDIR)/tests/path_check_wexcl.c \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
					$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/path_contains:	$(SRCDIR)/tests/path_contains.c \
					$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
					$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/reraise:	$(SRCDIR)/tests/reraise.c \
				$(BUILDDIR)/err.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/err.o \
		$(LDLIBS)

$(BUILDDIR)/tests/run_script:	$(SRCDIR)/tests/run_script.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/utils.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_cp:	$(SRCDIR)/tests/str_cp.c \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_eq:	$(SRCDIR)/tests/str_eq.c \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_split:	$(SRCDIR)/tests/str_split.c \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_vsplit:	$(SRCDIR)/tests/str_vsplit.c \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  $(GCOVFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/main:	$(SRCDIR)/main.c \
			$(BUILDDIR)/env.o $(BUILDDIR)/err.o \
			$(BUILDDIR)/file.o $(BUILDDIR)/path.o \
			$(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
			$(BUILDDIR)/tests/.sentinel
	$(CC) -I . -D TESTING=1 -D PACKAGE=fake-$(PACKAGE) \
		-D VERSION=$(VERSION) $(LDFLAGS) $(CFLAGS) $(GCOVFLAGS) \
		-Wno-unused-macros \
		-o $@ $< \
		$(BUILDDIR)/env.o $(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
		$(LDLIBS)

analysis:
	cppcheck $(CPPCHECKFLAGS) --enable=all -I . \
		-U __NR_openat2 -D O_NOFOLLOW_ANY=1 $(SRCDIR)
	cppcheck $(CPPCHECKFLAGS) --enable=unusedFunction -I . \
		$(SRCDIR)/*.c
	flawfinder --error-level=1 -m 0 -D -Q .
	find $(SCRIPTDIR) -type f | xargs shellcheck configure

check: $(CHECKBINS)
	$(SCRIPTDIR)/check -t $(BUILDDIR)/tests $(CHECKS)

clean:
	rm -rf $(BUILDDIR) $(COVDIR) $(DISTNAME)
	rm -f $(DISTAR) $(DISTAR).asc lcov.info
	find . -type d -name 'tmp-*' -exec rm '{}' +

$(COVDIR):
	mkdir $(COVDIR)

lcov.info: clean coverage
	make CC=gcc GCOVFLAGS='-fprofile-arcs -ftest-coverage' check
	chmod -R u=rw *.gcno *.gcda
	mv *.gcno *.gcda coverage
	lcov --directory coverage --capture --output-file lcov.info

dist: $(DISTAR) $(DISTAR).asc

distcheck: dist
	$(SCRIPTDIR)/distcheck $(DISTNAME)

distclean: clean
	rm -f makefile config.h

$(DISTAR): $(DISTFILES)
	$(SCRIPTDIR)/dist -a $(DISTAR) $(DISTNAME) $(DISTFILES)

$(DISTAR).asc: $(DISTAR)
	gpg -qab --batch --yes $(DISTAR)

install: $(BUILDDIR)/sucgi
	$(SCRIPTDIR)/install -b $(BUILDDIR) -d $(DESTDIR) -p $(PREFIX) \
		-c $(CGIBIN) -w $(WWWGRP)

uninstall:
	rm -f $(CGIBIN)/sucgi $(DESTDIR)$(PREFIX)/libexec/sucgi

.PHONY: all analysis cov check clean dist distcheck distclean \
	install uninstall

.IGNORE: analysis
