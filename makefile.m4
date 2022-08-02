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
COVDIR = cov


#
# Tests
#

CHECKBINS = $(BUILDDIR)/tests/tools/evil-env \
            $(BUILDDIR)/tests/tools/run-as \
            $(BUILDDIR)/tests/tools/unused-ids \
            $(BUILDDIR)/tests/change_identity \
            $(BUILDDIR)/tests/fail $(BUILDDIR)/tests/env_clear \
            $(BUILDDIR)/tests/env_get_fname \
	    $(BUILDDIR)/tests/env_sanitise \
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
            $(BUILDDIR)/tests/str_cpn \
            $(BUILDDIR)/tests/str_eq \
            $(BUILDDIR)/tests/str_fnmatchn \
            $(BUILDDIR)/tests/str_len \
            $(BUILDDIR)/tests/str_split

CHECKS = $(SCRIPTDIR)/tests/change_identity.sh \
         $(SCRIPTDIR)/tests/fail.sh \
         $(BUILDDIR)/tests/env_clear \
         $(SCRIPTDIR)/tests/env_get_fname.sh \
	 $(BUILDDIR)/tests/env_sanitise \
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
         $(BUILDDIR)/tests/str_cpn \
         $(BUILDDIR)/tests/str_eq \
         $(BUILDDIR)/tests/str_fnmatchn \
         $(BUILDDIR)/tests/str_len \
         $(BUILDDIR)/tests/str_split


#
# Analysers
#

CPPCHECKFLAGS =	-f -q --error-exitcode=8 --inconclusive \
	--language=c --std=c99 --library=posix --platform=unix64 \
	--suppress=missingIncludeSystem --inline-suppr \
	-D __GNUC__ -I .


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
	test -e $(BUILDDIR) || mkdir -p $(BUILDDIR)
	touch $(BUILDDIR)/.sentinel

$(BUILDDIR)/tests/.sentinel: $(BUILDDIR)/.sentinel
	test -e $(BUILDDIR)/tests || mkdir -p $(BUILDDIR)/tests
	touch $(BUILDDIR)/tests/.sentinel

$(BUILDDIR)/tests/tools/.sentinel: $(BUILDDIR)/tests/.sentinel
	test -e $(BUILDDIR)/tools/tests || mkdir -p $(BUILDDIR)/tests/tools
	touch $(BUILDDIR)/tests/tools/.sentinel

$(BUILDDIR)/env.o:	$(SRCDIR)/env.c $(SRCDIR)/env.h	\
			$(SRCDIR)/attr.h $(SRCDIR)/err.h $(SRCDIR)/str.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/err.o:	$(SRCDIR)/err.c $(SRCDIR)/err.h \
			$(SRCDIR)/attr.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/file.o:	$(SRCDIR)/file.c $(SRCDIR)/file.h \
			$(SRCDIR)/attr.h $(SRCDIR)/err.h \
			$(SRCDIR)/path.h $(SRCDIR)/str.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/path.o:	$(SRCDIR)/path.c $(SRCDIR)/path.h \
			$(SRCDIR)/attr.h $(SRCDIR)/err.h $(SRCDIR)/str.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/str.o:	$(SRCDIR)/str.c $(SRCDIR)/str.h \
			$(SRCDIR)/attr.h $(SRCDIR)/err.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/utils.o:	$(SRCDIR)/utils.c $(SRCDIR)/utils.h \
			$(SRCDIR)/attr.h $(SRCDIR)/err.h $(SRCDIR)/str.h \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/sucgi:	$(SRCDIR)/main.c config.h \
			$(BUILDDIR)/env.o $(BUILDDIR)/err.o \
			$(BUILDDIR)/file.o $(BUILDDIR)/path.o \
			$(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
			$(BUILDDIR)/.sentinel
	$(CC) -I . -DPACKAGE=$(PACKAGE) -DVERSION=$(VERSION) \
		$(LDFLAGS) $(CFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/env.o $(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
		$(LDLIBS)

$(BUILDDIR)/tests/env.o:	$(SRCDIR)/tests/env.c \
				$(SRCDIR)/tests/env.h \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/tests/utils.o:	$(SRCDIR)/tests/utils.c \
				$(SRCDIR)/tests/utils.h \
				$(SRCDIR)/str.h \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/tests/str.o:	$(SRCDIR)/tests/str.c \
				$(SRCDIR)/str.h \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/tests/tools/evil-env:	$(SRCDIR)/tests/tools/evil-env.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/tests/tools/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) -o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(LDLIBS)

$(BUILDDIR)/tests/tools/run-as:	$(SRCDIR)/tests/tools/run-as.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/tests/str.o \
				$(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/tools/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) -o $@ $< \
		$(BUILDDIR)/tests/str.o $(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/tools/unused-ids:	$(SRCDIR)/tests/tools/unused-ids.c \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/tests/tools/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) -o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(LDLIBS)

$(BUILDDIR)/tests/change_identity:	$(SRCDIR)/tests/change_identity.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/utils.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
		$(LDLIBS)

$(BUILDDIR)/tests/fail:		$(SRCDIR)/tests/fail.c \
				$(BUILDDIR)/err.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS) \
		-o $@ $< \
		$(BUILDDIR)/err.o \
		$(LDLIBS)

$(BUILDDIR)/tests/env_clear:	$(SRCDIR)/tests/env_clear.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/env.o \
				$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
				$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
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
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/env.o $(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/env_sanitise:	$(SRCDIR)/tests/env_sanitise.c \
				$(BUILDDIR)/tests/env.o \
				$(BUILDDIR)/tests/str.o \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/env.o $(BUILDDIR)/err.o \
				$(BUILDDIR)/file.o $(BUILDDIR)/path.o \
				$(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/tests/env.o $(BUILDDIR)/tests/str.o \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/env.o $(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/file_is_exec:	$(SRCDIR)/tests/file_is_exec.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/file.o \
				$(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/file_is_wexcl:	$(SRCDIR)/tests/file_is_wexcl.c \
					$(BUILDDIR)/tests/str.o \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/tests/str.o $(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/file_safe_open:	$(SRCDIR)/tests/file_safe_open.c \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/file.o \
					$(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/file_safe_stat:	$(SRCDIR)/tests/file_safe_stat.c \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/file.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/path_check_len:	$(SRCDIR)/tests/path_check_len.c \
					$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
					$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/path_check_wexcl:	$(SRCDIR)/tests/path_check_wexcl.c \
					$(BUILDDIR)/tests/str.o \
					$(BUILDDIR)/tests/utils.o \
					$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
					$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/tests/str.o $(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/path_contains:	$(SRCDIR)/tests/path_contains.c \
					$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
					$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
					$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/reraise:	$(SRCDIR)/tests/reraise.c \
				$(BUILDDIR)/err.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/err.o \
		$(LDLIBS)

$(BUILDDIR)/tests/run_script:	$(SRCDIR)/tests/run_script.c \
				$(BUILDDIR)/tests/utils.o \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/utils.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/tests/utils.o \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_cp:	$(SRCDIR)/tests/str_cp.c \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_cpn:	$(SRCDIR)/tests/str_cpn.c \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_eq:	$(SRCDIR)/tests/str_eq.c \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_fnmatchn:	$(SRCDIR)/tests/str_fnmatchn.c \
				$(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_len:	$(SRCDIR)/tests/str_len.c \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/str_split:	$(SRCDIR)/tests/str_split.c \
				$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
				$(BUILDDIR)/tests/.sentinel
	$(CC) -I . $(LDFLAGS) $(CFLAGS)  \
		-o $@ $< \
		$(BUILDDIR)/err.o $(BUILDDIR)/str.o \
		$(LDLIBS)

$(BUILDDIR)/tests/main:	$(SRCDIR)/main.c \
			$(BUILDDIR)/env.o $(BUILDDIR)/err.o \
			$(BUILDDIR)/file.o $(BUILDDIR)/path.o \
			$(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
			$(BUILDDIR)/tests/.sentinel
	$(CC) -I . -D TESTING=1 -D PACKAGE=fake-$(PACKAGE) \
		-D VERSION=$(VERSION) $(LDFLAGS) $(CFLAGS) \
		-Wno-unused-macros \
		-o $@ $< \
		$(BUILDDIR)/env.o $(BUILDDIR)/err.o $(BUILDDIR)/file.o \
		$(BUILDDIR)/path.o $(BUILDDIR)/str.o $(BUILDDIR)/utils.o \
		$(LDLIBS)

analysis:
	! grep -ri fixme $(SRCDIR)
	cppcheck $(CPPCHECKFLAGS) --enable=all \
		-D __NR_openat2=437 -U O_NOFOLLOW_ANY $(SRCDIR)
	cppcheck $(CPPCHECKFLAGS) --enable=all \
		-U __NR_openat2 -D O_NOFOLLOW_ANY=0x20000000 $(SRCDIR)/file.c
	cppcheck $(CPPCHECKFLAGS) --enable=unusedFunction $(SRCDIR)/*.c
	#cppcheck $(CPPCHECKFLAGS) --addon=misra.py $(SRCDIR)
	flawfinder --error-level=1 -m 0 -D -Q .
	find $(SCRIPTDIR) -type f | xargs shellcheck configure

check: $(CHECKBINS)
	$(SCRIPTDIR)/check -t $(BUILDDIR)/tests $(CHECKS)

clean:
	rm -rf $(BUILDDIR) cov $(DISTNAME)
	rm -f $(DISTAR) $(DISTAR).asc lcov.info
	find . -type d -name 'tmp-*' -exec rm '{}' +

cov:
	test -e cov || mkdir cov
	cd cov && CC=$(CC) CFLAGS=--coverage ../configure -q && make covgen

covgen: $(CHECKBINS)
	for check in $(CHECKS); do				\
		printf 'checking %s ...\n' "$$check"	&&	\
		chown -R $$($(SCRIPTDIR)/realids) .	&&	\
		chmod -R u+rw,go+r .			&&	\
		TESTSDIR=./build/tests "$$check"	;	\
	done

#covbuild: $(CHECKBINS)
#	chown -R $$($(SCRIPTDIR)/realids) .
#	for check in $$(CHECKS); do TESTSDIR=./build/tests $(CHECK)

lcov.info: cov
	lcov -c -d cov -o lcov.info --exclude '*/tests/*'

cov/html/index.html: lcov.info
	test -e cov/html || mkdir cov/html
	genhtml -o cov/html lcov.info

covhtml: cov/html/index.html

dist: $(DISTAR) $(DISTAR).asc

distcheck:
	$(SCRIPTDIR)/distcheck $(DISTNAME)

distclean: clean
	rm -f makefile config.h

$(DISTAR): $(DISTFILES)
	$(SCRIPTDIR)/dist -a $(DISTAR) $(DISTNAME) $(DISTFILES)

$(DISTAR).asc: $(DISTAR)
	gpg -qab --batch --yes $(DISTAR)

install: $(BUILDDIR)/sucgi
	$(SCRIPTDIR)/install -b $(BUILDDIR) -p "$(DESTDIR)$(PREFIX)" \
		-c $(CGIBIN) -w $(WWWGRP)

uninstall:
	rm -f $(CGIBIN)/sucgi $(DESTDIR)$(PREFIX)/libexec/sucgi

.SILENT: analysis check cov cov covgen dist distcheck install

.PHONY: all analysis check clean cov covhtml gcov lcov.info \
	dist distcheck distclean install uninstall

.IGNORE: analysis
