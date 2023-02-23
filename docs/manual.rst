NAME
====

**sucgi** - Run CGI programmes under the permissions of their owner


SYNOPSIS
========

**sucgi**


DESCRIPTION
===========

**suCGI** checks if the programme pointed to by the environment variable
*PATH_TRANSLATED* is secure, sets the process' effective UID and GID to the
UID and the GID of that programme's owner, cleans up the environment,
and then runs the programme.


CONFIGURATION
=============

**sucgi** is configured by adapting *config.h* before compilation.

DOC_ROOT
    CGI programmes are only executed if they are inside a directory matching
    this shell wildcard pattern. See fnmatch(3) for the sytax. '*' matches
    neither ('/') nor leading dots ('.'). Should correspond to the UserDir
    directive of your Apache configuration (or its equivalent).

MIN_UID
    Smallest UID that may have been assigned to a regular user.
    On most systems, this will be 500 (e.g., macOS) or 1,000 (e.g, Debian).

MAX_UID
    Largest UID that may have been assigned to a regular user.
    On most systems, this will be 60,000 (though some use 32,767 for nobody).

SCRIPT_HANDLERS
    Handlers to run scripts with if their executable bit has NOT been set.
    Array of filename suffix-handler pairs, where the filename suffix must
    be given including the leading dot (e.g., ".php") and the handler is
    looked up in $PATH if its filename is relative (e.g., "php"). The array
    must be terminated with a pair of NULLs.

SECURE_PATH
    A secure $PATH.

Just in case your C is rusty: ``#define`` statements are *not* terminated
with a semicolon; strings must be enclosed in double quotes ("..."), *not*
single quotes ('...'), numbers must *not* be enclosed in quotes at all,
and for a ``#define`` statement to span multiple lines, the linefeed
must be escaped with a backslash ('\').


ENVIRONMENT
===========

DOCUMENT_ROOT
	The root directory of a website.
	Only programmes within this directory are run.
	Canonicalised before **PATH_TRANSLATED** is run.

PATH_TRANSLATED
	Path of the CGI programme to run.
	Canonicalised before the programme is run.

PATH
	Overwritten with **SECURE_PATH** before **PATH_TRANSLATED** is run.

Discuss CGI vars

SECURITY
========

Immature
--------

**sucgi** has *neither* been tested in the real-world, *nor* revied.


Introduction
------------

**sucgi** *aims* to achieve security by being correct, following best
practices, and being easy to use. It *tries* to comply with POSIX.1-2018
and implements similar, but more modern and more paranoid, mechanims as
`Apache's suEXEC <https://httpd.apache.org/docs/2.4/suexec.html>`_.

Environment
-----------

Only safe environment variables are kept and **PATH** is set to the
**SECURE_PATH** given in *config.h* before calling **PATH_TRANSLATED**.

Checks
------

The following checks are performed before calling the programme:

1. Is **DOCUMENT_ROOT** a canonical path? Is it a directory?
   Does it match **DOC_ROOT**? Is it within the home directory of
   **PATH_TRANSLATED**'s owner?
2. Is **PATH_TRANSLATED** a canonical path? Is it a regular file?
   Is it within **DOCUMENT_ROOT**?
3. Is **PATH_TRANSLATED**, its parent directory, the parent directory's
   parent directory, etc., up to the home directory of **PATH_TRANSLATED**'s
   owner only w



User and group checks:

1. Is the script file's UID greater than 0?
2. Is it a UID from **SCRIPT_MIN_UID** to **SCRIPT_MAX_UID**?
3. Does a user with that UID exist?



CGI
---

You should also consider the `security issues that come with running PHP
as a CGI handler <https://www.php.net/manual/en/security.cgi-bin.php>`_.


DIAGNOSTICS
===========

**sucgi** logs errors to STDERR, which your webserver
the system log (see syslog(1))


EXIT STATUSES
=============

**sucgi** passes control to the given CGI programme, so its exit status
may be that of that programme. If


AUTHOR
======

Odin Kroeger
