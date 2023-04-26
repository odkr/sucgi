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
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../compat.h"
#include "../macros.h"
#include "../params.h"
#include "../priv.h"
#include "lib/check.h"
#include "lib/user.h"


#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif


/*
 * Data types
 */

/* Combinations of effective and real process UIDs. */
typedef struct {
    uid_t *ruid;
    uid_t *euid;
} Args;


/*
 * Module variables
 */

/* The user ID of the superuser. */
uid_t superuid = 0;

/* A user ID of a regular user. */
uid_t reguid = 1000;


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

    if (checkinit() != 0) {
        err(TEST_ERROR, "sigaction");
    }

    if (getuid() == 0) {
        struct passwd pwd;
        UserError usererr;

        usererr = usergetregular(&reguid);
        switch (usererr) {
            case USER_SUCCESS:
                break;
            case USER_NOTFOUND:
                errx(TEST_SKIPPED, "no regular user found");
            case USER_ERROR:
                err(TEST_ERROR, "getpwent");
            default:
                errx(TEST_ERROR, "%s:%d: usergetregular(→ %p) → %u [!]",
                     __FILE__, __LINE__, (void *) &reguid, usererr);
        }

        userget(reguid, &pwd);
    } else {
        reguid = getuid();
    }

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        struct passwd ruser, euser;
        int jumpval;
        UserError usererr;

        usererr = userget(*args.ruid, &ruser);
        switch (usererr) {
            case USER_SUCCESS:
                break;
            case USER_NOTFOUND:
                if (ISSIGNED(uid_t)) {
                    errx(TEST_ERROR, "user %lld: not found",
                         (long long) *args.ruid);
                } else {
                    errx(TEST_ERROR, "user %llu: not found",
                         (unsigned long long) *args.ruid);
                }
            case USER_ERROR:
                err(TEST_ERROR, "userget");
            default:
                if (ISSIGNED(uid_t)) {
                    errx(TEST_ERROR, "%s:%d: userget(%lld, → %p) → %u [!]",
                         __FILE__, __LINE__, (long long) *args.ruid,
                         (void *) &ruser, usererr);
                } else {
                    errx(TEST_ERROR, "%s:%d: userget(%llu, → %p) → %u [!]",
                         __FILE__, __LINE__, (unsigned long long) *args.ruid,
                         (void *) &ruser, usererr);
                }
        }

        usererr = userget(*args.euid, &euser);
        switch (usererr) {
            case USER_SUCCESS:
                break;
            case USER_NOTFOUND:
                if (ISSIGNED(uid_t)) {
                    errx(TEST_ERROR, "user %lld: not found",
                         (long long) *args.euid);
                } else {
                    errx(TEST_ERROR, "user %llu: not found",
                         (unsigned long long) *args.euid);
                }
            case USER_ERROR:
                err(TEST_ERROR, "userget");
            default:
                if (ISSIGNED(uid_t)) {
                    errx(TEST_ERROR, "%s:%d: userget(%lld, → %p) → %u [!]",
                         __FILE__, __LINE__, (long long) *args.euid,
                         (void *) &euser, usererr);
                } else {
                    errx(TEST_ERROR, "%s:%d: userget(%llu, → %p) → %u [!]",
                         __FILE__, __LINE__, (unsigned long long) *args.euid,
                         (void *) &euser, usererr);
                }
        }

        jumpval = sigsetjmp(checkenv, true);
        if (jumpval == 0) {
            gid_t groups[MAX_NGROUPS];
            uid_t ruid, euid;
            gid_t rgid, egid;
            int ngroups;
            bool super;
            Error retval;

            if (geteuid() == 0) {
                errno = 0;
                if (setregid(ruser.pw_gid, euser.pw_gid) != 0) {
                    err(TEST_ERROR, "setregid");
                }
                if (setreuid(ruser.pw_uid, euser.pw_uid) != 0) {
                    err(TEST_ERROR, "setreuid");
                }
            } else {
                if (getuid() != *args.ruid || geteuid() != *args.euid) {
                    result = TEST_SKIPPED;
                    warnx("skipping (%s, %s) ...",
                          ruser.pw_name, euser.pw_name);
                    continue;
                }
            }

            super = geteuid() == 0;

            retval = privsuspend();
            if (retval != OK) {
                result = TEST_FAILED;
                warnx("(%s, %s) → %u [!]",
                      ruser.pw_name, euser.pw_name, retval);
            }

            ruid = getuid();
            if (ruid != ruser.pw_uid) {
                result = TEST_FAILED;
                warnx("(%s, %s) ─→ <ruid> = %llu [!]",
                      ruser.pw_name, euser.pw_name,
                      (unsigned long long) ruid);
            }

            euid = geteuid();
            if (euid != ruser.pw_uid) {
                result = TEST_FAILED;
                warnx("(%s, %s) ─→ <euid> = %llu [!]",
                      ruser.pw_name, euser.pw_name,
                      (unsigned long long) euid);
            }

            rgid = getgid();
            if (rgid != ruser.pw_gid) {
                result = TEST_FAILED;
                warnx("(%s, %s) ─→ <rgid> = %llu [!]",
                      ruser.pw_name, euser.pw_name,
                      (unsigned long long) rgid);
            }

            egid = getegid();
            if (egid != ruser.pw_gid) {
                result = TEST_FAILED;
                warnx("(%s, %s) ─→ <egid> = %llu [!]",
                      ruser.pw_name, euser.pw_name,
                      (unsigned long long) egid);
            }

            if (super) {
                errno = 0;
                ngroups = getgroups(MAX_NGROUPS, groups);
                if (ngroups == -1) {
                    err(TEST_ERROR, "getgroups");
                }

                if (ngroups != 1) {
                    result = TEST_FAILED;
                    warnx("(%s, %s) ─→ <ngroups> = %d [!]",
                          ruser.pw_name, euser.pw_name, ngroups);
                }

                if (groups[0] != ruser.pw_gid) {
                    result = TEST_FAILED;
                    warnx("(%s, %s) ─→ <groups[0]> = %llu [!]",
                          ruser.pw_name, euser.pw_name,
                          (unsigned long long) groups[0]);
                }

                errno = 0;
                if (setuid(0) != 0) {
                    err(TEST_ERROR, "setuid");
                }
            }
        } else {
            result = TEST_FAILED;
            warnx("(%s, %s) ↑ %d [!]", ruser.pw_name, euser.pw_name, jumpval);
        }
    }

    return result;
}
