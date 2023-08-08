# Testing suCGI

## Overview

Run the test suite by:

    make check

See below for details.


## System requirements

The test suite is less portable that suCGI itself.

It requires a system that supports the "Spawn" POSIX.1-2008 extension and
the **<err.h>** header file, which some systems do not provide.


## Security

`make check` will compile the test suite and the utilities used by scripted
tests. Among these utilities are *badenv* and *badexec*. *badenv* enables
users to call applications with maliciously crafted environment variables.
*badexec* allows user to call applications with an arbitrary *argv[0]*.

If your system is set up so that non-system users can run applications
that they compiled themselves, then compiling *badenv* and *badexec* makes
no difference. But if you set up your system so that users cannot compile
their own applications or at least cannot run the applications that they
compiled, then compiling *badenv* and *badexec* may open up an attack
vector that had previously been foreclosed.


## Exit statuses

| Status        | Meaning               |
| ------------- | --------------------- |
| 0             | Test passed.          |
| 70            | Test failed.          |
| 75            | Test was skipped.     |
| 69            | Something went wrong. |

Any other exit status indicates a bug in the test suite.


## Superuser privileges

SuCGI requires superuser privileges. Consequently, so do some tests.
Of those, some can use mock-up system calls and may therefore be run
without superuser privileges. Others, however, have to be skipped.

Conversely, some tests can only be run as a non-system user. If you
run the test suite with superuser privileges, those tests are run as
an *arbitrary* non-system user; and that user can interfere with
testing. If no such user is found, those tests are skipped.
