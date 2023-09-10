# Testing suCGI

## Overview

Run the test suite by:

    make check

It must be permitted to execute files in the repository
(i.e., it must not reside on a filesystem that is mounted noexec).

See below for details.


## Requirements

The test suite is less portable that suCGI itself; it requires a system that
supports the POSIX.1-2008 extensions "Spawn" and "Thread-Safe Functions" as
well as the **<err.h>** header file, which some systems do not provide.


## Security

`make check` will compile the test suite and the utilities used by scripted
tests. Among these utilities are *badenv* and *badexec*. *badenv* enables
users to call applications with maliciously crafted environment variables.
*badexec* allows user to call applications with an arbitrary *argv[0]*.
If the system is set up so that users either cannot compile applications or
cannot run applications they compiled, then compiling *badenv* or *badexec*
may open up attacks vectors that had previously been foreclosed.


## Exit statuses

| Status        | Meaning               |
| ------------- | --------------------- |
| 0             | Test passed.          |
| 70            | Test failed.          |
| 75            | Test was skipped.     |
| 69            | Something went wrong. |

Any other exit status indicates a bug in the test suite.


## Superuser privileges

suCGI requires superuser privileges. Consequently, so do some tests.
These will be skipped if the test suite is run without superuser privileges.

Conversely, some tests can only be run as a non-system user. Those will
be run as *arbitrary* non-system user if the test suite is run with superuser
privileges; note that the user with the permissions of which those tests are
run can interfere with testing.


## Running individual tests

Use

    make check checks="$tests"

to compile and run individual tests only, where `$tests` is
a whitespace-separated lists of filenames.


## Testing different compilers, makes, and shells

* **scripts/checkcomps** tests different compilers.
* **scripts/checkmakes** tests different makes.
* **scripts/checkshells** tests different shells.
* **scripts/checkall** does all of the above.
