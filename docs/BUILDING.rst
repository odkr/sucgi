==============
Building suCGI
==============

Configuration
=============

Use *configure* to generate the *makefile*.

*configure* reads the requested build configuration from the environment
and the shell script *prod.env* (with the environment taking precedence),
gathers information from the system, and then generates the *makefile*
from the template *makefile.m4*.

*configure* accepts the same variables as the *makefile* (see below);
passing a variable to *configure* changes its default in the *makefile*. 
For example::

	CC=/opt/obscure/bin/occ ./configure

*configure* saves the information it gathered in the shell script
*config.status* Running this script regenerates the *makefile* using
that information. 

See ``./configure -h`` for more information.

If *configure* fails, you can also create the *makefile* by::

	m4 makefile.m4 >makefile

The difference between ``./configure`` and ``m4 makefile.m4 >makefile``
is that ``./configure`` enables control flow protection, stack protection,
and undefined behaviour sanitisation depending on which of those features
your C compiler supports.


Development
===========

suCGI's default build configuration for development resides in *devel.env*;
load it by passing ``-d`` to *configure*.

Have a look at *configure*, *prod.env*, and *devel.env* for details.


Compilation
===========

Compile suCGI by calling ``make``.

Makefile macros
---------------

CC
    The C compiler

CFLAGS
    Flags to give to the C compiler

ARFLAGS
    Flags to give to the archiver

LDFLAGS
    Flags to give to the compiler when it invokes the linker

LDLIBS
    Flags or names to give to the compiler when it invokes the linker


Compiler macros
---------------

MAX_ENV
    How many environment variables suCGI may accept. Unsigned integer.
    suCGI aborts if the environment contains more variables. Defaults to 256.
    Note, values lower than 192 may break CGI scripts.

MAX_GROUPS
    How many groups a user may be a member of. Unsigned interger.
    suCGI refuses users who belong to more groups. Defaults to 32.

MAX_STR
    Maximum string size, including the terminating NUL. Unsigned integer.
    suCGI aborts if encounters an environment variable that is longer.
    Note, the length of a variables includes its name and the "=".
    Defaults to 1024 or PATH_MAX, whichever is lower.

TESTING
    Whether to build for testing. Boolean value.
    Overrides *config.h*. *Test builds are insecure!*

MAX_ENV, MAX_GROUPS, and MAX_STR cannot be set in *config.h*;
TESTING should not be set in *config.h*.


Testing
=======

Run the test suite by ``make check``. 

For the whole test suite to run:

(1) The repository must be owned by a regular user.
(2) The test suite must be invoked as the superuser.

Create coverage reports by ``make covhtml``. The report can then be found
in *cov*. Coverage reports require Gcov_ (or Clang_'s Gcov clone) and LCOV_.
Coverage reports are only accurate if they are generated as the superuser.
They are currently broken.


Installation
============

Install suCGI by ``sudo make install``.
Uninstall it by ``sudo make uninstall``.

You can pass the following variables to ``make`` to adapt the installation:

DESTDIR
    Prefix for staged installations
    (default depends on *make*).

PREFIX
    Prefix for installation targets
    (defaults to */usr/local*).

cgi_dir
    Path to your webserver's */cgi-bin* directory
    (defaults to */usr/lib/cgi-bin*).

www_grp
    Group the webserver runs as
    (defaults to "www-data").

``make install`` *and* ``make uninstall`` must be given the same variables.
That being so, you will want to set these variables using *configure*.


Other targets
=============

The *makefile* supports the following 'phony' targets:

all
    Alias for "sucgi" and the default target.

analysis
    Analyse the code with Cppcheck_, Flawfinder_, RATS_, and
    ShellCheck_, if they are installed.

check
    Perform tests. Must be run as superuser to perform all tests.

cov
    Generate coverage data.
    Must be run as superuser to generate a complete report.
    Only tested with Clang_. Currently broken.

covhtml
    Generate a coverage report. Alias for "cov/index.html". Requires LCOV_.
    Currently broken.

clean
    Delete binaries, coverage data, temporary files, and distribution files.

dist
    Make a distribution package. Requires *tar* and GnuPG_.

distcheck
    Check if the distribution compiles,
    passes the test suite, and is self-contained.

distclean
    Delete *config.status*, *lcov.info*, *makefile* and
    everything ``make clean`` deletes.

install
    Install suCGI.

uninstall
    Uninstall suCGI.


.. _Clang: https://clang.llvm.org/

.. _Cppcheck: https://cppcheck.sourceforge.io/

.. _Flawfinder: https://dwheeler.com/flawfinder/

.. _`GNU Make`: https://www.gnu.org/software/make/

.. _RATS: https://github.com/andrew-d/rough-auditing-tool-for-security

.. _ShellCheck: https://www.shellcheck.net/

.. _Gcov: https://gcc.gnu.org/onlinedocs/gcc/Gcov.html

.. _LCOV: https://github.com/linux-test-project/lcov

.. _GnuPG: https://www.gnupg.org/
