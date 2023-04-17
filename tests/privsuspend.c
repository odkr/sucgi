/*
 * Test privsuspend.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with suCGI. If not, see <https://www.gnu.org/licenses>.
 */

#define _BSD_SOURCE
/* _DARWIN_C_SOURCE must NOT be set, or else getgroups will be pointless. */
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../macros.h"
#include "../max.h"
#include "../priv.h"
#include "lib/priv.h"
#include "lib/util.h"


/*
 * Data types
 */

/* Combinations of effective and real process UIDs. */
typedef struct {
    uid_t *euid;
    uid_t *ruid;
} Args;


/*
 * Module variables
 */

/* The user ID of the superuser. */
uid_t superuid = 0;

/* A user ID of a regular user. */
uid_t reguid = INT_MAX;


/* Test cases. */
const Args cases[] = {
    {&superuid, &superuid},
    {&superuid, &reguid  },
    {&reguid,   &superuid},
    {&reguid,   &reguid  }
};


/*
 * Main
 */

int
main (void) {
    volatile int result = TEST_PASSED;

    if (getuid() == 0) {
        if (!checkgetreguid(&reguid) && errno != 0) {
            err(TEST_ERROR, "getpwent");
        }
    } else {
        reguid = getuid();
    }

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        struct passwd ruser, euser;
        pid_t pid;

        checkgetuser(*args.ruid, &ruser);
        checkgetuser(*args.euid, &euser);

        pid = fork();
        if (pid == 0) {
            gid_t groups[MAX_NGROUPS];
            uid_t ruid, euid;
            gid_t rgid, egid;
            int ngroups;
            bool super;
            Error retval;

            if (geteuid() == 0) {
                errno = 0;
                if (setregid(ruser.pw_gid, euser.pw_gid) != 0) {
                    check_err(TEST_ERROR, "setregid");
                }
                if (setreuid(ruser.pw_uid, euser.pw_uid) != 0) {
                    check_err(TEST_ERROR, "setreuid");
                }
            } else {
                if (getuid() != *args.ruid || geteuid() != *args.euid) {
                    check_errx(TEST_SKIPPED, "skipping (%s, %s) ...",
                               ruser.pw_name, euser.pw_name);
                }
            }

            super = geteuid() == 0;

            retval = privsuspend();
            if (retval != OK) {
                /* Should be unreachable. */
                check_errx(TEST_FAILED, "(%s, %s) → %u [!]",
                           ruser.pw_name, euser.pw_name, retval);
            }

            ruid = getuid();
            if (ruid != ruser.pw_uid) {
                check_errx(TEST_FAILED, "(%s, %s) ─→ <ruid> = %llu [!]",
                           ruser.pw_name, euser.pw_name,
                           (unsigned long long) ruid);
            }

            euid = geteuid();
            if (euid != ruser.pw_uid) {
                check_errx(TEST_FAILED, "(%s, %s) ─→ <euid> = %llu [!]",
                           ruser.pw_name, euser.pw_name,
                           (unsigned long long) euid);
            }

            rgid = getgid();
            if (rgid != ruser.pw_gid) {
                check_errx(TEST_FAILED, "(%s, %s) ─→ <rgid> = %llu [!]",
                           ruser.pw_name, euser.pw_name,
                           (unsigned long long) rgid);
            }

            egid = getegid();
            if (egid != ruser.pw_gid) {
                check_errx(TEST_FAILED, "(%s, %s) ─→ <egid> = %llu [!]",
                           ruser.pw_name, euser.pw_name,
                           (unsigned long long) egid);
            }

            if (super) {
                errno = 0;
                ngroups = getgroups(MAX_NGROUPS, groups);
                if (ngroups == -1) {
                    check_err(TEST_ERROR, "getgroups");
                }

                if (ngroups != 1) {
                    check_errx(TEST_FAILED, "(%s, %s) ─→ <ngroups> = %d [!]",
                               ruser.pw_name, euser.pw_name, ngroups);
                }

                if (groups[0] != rgid) {
                    check_errx(TEST_FAILED,
                               "(%s, %s) ─→ <groups[0]> = %llu [!]",
                               ruser.pw_name, euser.pw_name,
                               (unsigned long long) groups[0]);
                }
            }

            _exit(0);
        } else {
            int retval;
            int status;

            do {
                errno = 0;
                retval = waitpid(pid, &status, 0);
            } while (retval < 0 && errno == EINTR);

            if (retval < 0) {
                err(EXIT_FAILURE, "waitpid %d", pid);
            }

            if (WIFEXITED(status)) {
                int exitstatus;

                exitstatus = WEXITSTATUS(status);
                switch (exitstatus) {
                case TEST_PASSED:
                    break;
                case TEST_FAILED:
                    result = TEST_FAILED;
                    break;
                case TEST_SKIPPED:
                    if (result == TEST_PASSED) result = TEST_SKIPPED;
                    break;
                default:
                    return TEST_ERROR;
                    break;
                }
            } else if (WIFSIGNALED(status)) {
                int signo;
                const char *signame;

                result = TEST_FAILED;
                signo = WTERMSIG(status);
                signame = strsignal(signo);
                warnx("(%s, %s) ↑ %s [!]",
                      ruser.pw_name, euser.pw_name, signame);
            } else {
                errx(TEST_ERROR, "child %d exited abnormally", pid);
            }
        }
    }

    return result;
}
