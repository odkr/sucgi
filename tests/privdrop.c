/*
 * Test privdrop.
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
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <search.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../attr.h"
#include "../compat.h"
#include "../macros.h"
#include "../params.h"
#include "../priv.h"
#include "lib/check.h"
#include "lib/user.h"
#include "lib/str.h"

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    uid_t *euid;    /* Effective process UID. */
    uid_t *ruid;    /* Real process UID. */
    uid_t *dropuid; /* UID to drop to. */
    gid_t *dropgid; /* GID to drop to. Use NULL to use UID's primary GID. */
    int ndropgrps;  /* Number of groups to ste. Use -1 */
    Error retval;   /* Return value. */
    int signo;      /* Signal raised, if any. */
} Args;

/* Shorthand for comparison function type. */
typedef int (*Compar)(const void *, const void *);


/*
 * Module variables
 */

/* The user ID of the superuser. */
uid_t superuid = 0;

/* The group ID of the superuser. */
gid_t supergid = 0;

/* A user ID of a regular user. */
uid_t reguid = INT_MAX;

/* A group ID of a regular user. */
uid_t reggid = INT_MAX;

/* Test cases. */ /* FIXME: explain. */
const Args cases[] = {
    /* Illegal arguments. */
#if !defined(NDEBUG)
    {&superuid, &superuid, &superuid, NULL,      -1, OK,      SIGABRT},
    {&superuid, &superuid, &reguid,   &supergid, -1, OK,      SIGABRT},
    {&superuid, &superuid, &reguid,   NULL,       0, OK,      SIGABRT},
    {&reguid,   &reguid,   &superuid, NULL,      -1, ERR_SYS, SIGABRT},
    {&reguid,   &reguid,   &reguid,   &supergid, -1, ERR_SYS, SIGABRT},
    {&reguid,   &reguid,   &reguid,   NULL,       0, ERR_SYS, SIGABRT},
#endif

    {&superuid, &superuid, &reguid,   NULL,      -1, OK,      0},
    {&superuid, &reguid,   &reguid,   NULL,      -1, OK,      0},
    {&reguid,   &superuid, &reguid,   NULL,      -1, ERR_SYS, 0},
    {&reguid,   &reguid,   &reguid,   NULL,      -1, ERR_SYS, 0},
    {&superuid, &superuid, &reguid,   &reggid,   -1, OK,      0},
    {&superuid, &reguid,   &reguid,   &reggid,   -1, OK,      0},
    {&reguid,   &superuid, &reguid,   &reggid,   -1, ERR_SYS, 0},
    {&reguid,   &reguid,   &reguid,   &reggid,   -1, ERR_SYS, 0}
};


/*
 * Prototypes
 */

/*
 * Check whether GID A is smaller than, equal to, or greater than B.
 *
 * Return value:
 *   -1  A is smaller than B.
 *    0  A is equal to B.
 *    1  A is greater than B.
 */
__attribute__((nonnull(1, 2)))
static int cmpgids(gid_t *a, gid_t *b);

/* Wrapper around userget. */
__attribute__((nonnull(2)))
static void myuserget(const uid_t uid, struct passwd *pwd);


/*
 * Functions
 */

static int
cmpgids(gid_t *a, gid_t *b)
{
    if (*a < *b) {
        return -1;
    }

    if (*a > *b) {
        return 1;
    }

    return 0;
}

static void
myuserget(const uid_t uid, struct passwd *pwd)
{
    UserError retval;

    retval = userget(uid, pwd);
    switch (retval) {
    case USER_SUCCESS:
        break;
    case USER_NOTFOUND:
        errx(TEST_ERROR, "user %s: not found", idtostr(uid));
    case USER_ERROR:
        err(TEST_ERROR, "userget");
    default:
        errx(TEST_ERROR, "%s:%d: userget(%s, → %p) → %u [!]",
             __FILE__, __LINE__, idtostr(uid), (void *) pwd, retval);
    }
}


/*
 * Main
 */

int
main (void) {
    volatile int result = TEST_PASSED;

    ERRORIF(sizeof(GRP_T) != sizeof(gid_t));
    ERRORIF(SIGNEDMAX(NGRPS_T) < SIGNEDMAX(int));
    ERRORIF((uint64_t) INT_MAX > (uint64_t) SIGNEDMAX(uid_t));
    ERRORIF((uint64_t) INT_MAX > (uint64_t) SIGNEDMAX(gid_t));
    ERRORIF((uint64_t) INT_MAX > (uint64_t) SIGNEDMAX(GRP_T));

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

        myuserget(reguid, &pwd);
        reggid = pwd.pw_gid;
    } else {
        reguid = getuid();
        reggid = getgid();
    }

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        struct passwd ruser, euser, drop;
        uid_t dropuid;
        gid_t dropgid;
        pid_t pid;

        myuserget(*args.ruid, &ruser);
        myuserget(*args.euid, &euser);
        myuserget(*args.dropuid, &drop);

        dropuid = drop.pw_uid;
        dropgid = (args.dropgid == NULL) ? drop.pw_gid : *args.dropgid;

        pid = fork();
        if (pid == 0) {
            gid_t groups[MAX_NGROUPS], dropgroups[MAX_NGROUPS];
            uid_t ruid, euid;
            gid_t rgid, egid;
            int ngroups, ndropgrps, ndroppedgrps;
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
                    errx(TEST_SKIPPED, "skipping (%s, %s, %s, %s, %d) ...",
                         ruser.pw_name, euser.pw_name,
                         idtostr(dropuid), idtostr(dropgid),
                         args.ndropgrps);
                }
            }

            ngroups = MAX_NGROUPS;
            ndropgrps = (args.ndropgrps == -1) ? ngroups : args.ndropgrps;
            if (getgrouplist(drop.pw_name, (GRP_T) dropgid,
                             (GRP_T *) groups, &ngroups) < 0)
            {
                errx(TEST_SKIPPED, "user %s: belongs to too many groups.",
                     drop.pw_name);
            }

            retval = privdrop(dropuid, dropgid,
                              (NGRPS_T) ngroups, groups);
            if (retval != args.retval) {
                /* Should be unreachable. */
                errx(TEST_FAILED, "(%s, %s, %s, %s, %d) → %u [!]",
                     ruser.pw_name, euser.pw_name,
                     idtostr(dropuid), idtostr(dropgid),
                     args.ndropgrps, retval);
            }

            if (retval != OK) {
                exit(0);
            }

            ruid = getuid();
            if (ruid != dropuid) {
                errx(TEST_FAILED,
                     "(%s, %s, %s, %s, %d) ─→ <ruid> = %s [!]",
                     ruser.pw_name, euser.pw_name,
                     idtostr(dropuid), idtostr(dropgid),
                     args.ndropgrps, idtostr(ruid));
            }

            euid = geteuid();
            if (euid != dropuid) {
                errx(TEST_FAILED,
                     "(%s, %s, %s, %s, %d) ─→ <euid> = %s [!]",
                     ruser.pw_name, euser.pw_name,
                     idtostr(dropuid), idtostr(dropgid),
                     args.ndropgrps, idtostr(euid));
            }

            rgid = getgid();
            if (rgid != dropgid) {
                errx(TEST_FAILED,
                     "(%s, %s, %s, %s, %d) ─→ <rgid> = %s [!]",
                     ruser.pw_name, euser.pw_name,
                     idtostr(dropuid), idtostr(dropgid),
                     args.ndropgrps, idtostr(rgid));
            }

            egid = getegid();
            if (egid != dropgid) {
                errx(TEST_FAILED,
                     "(%s, %s, %s, %s, %d) ─→ <egid> = %s [!]",
                     ruser.pw_name, euser.pw_name,
                     idtostr(dropuid), idtostr(dropgid),
                     args.ndropgrps, idtostr(egid));
            }

            errno = 0;
            ndroppedgrps = getgroups(MAX_NGROUPS, dropgroups);
            if (ndroppedgrps == -1) {
                err(TEST_ERROR, "getgroups");
            }

            if (ndropgrps == ngroups) {
                for (int j = 0; j < ngroups; ++j) {
                    size_t nelems = (size_t) ndroppedgrps;
                    gid_t grp = groups[j];
                    void *grpp;

                    grpp = lfind(&grp, dropgroups, &nelems,
                                 sizeof(*dropgroups), (Compar) cmpgids);
                    if (grpp == NULL) {
                        errx(TEST_FAILED,
                             "(%s, %s, %s, %s, %d) ─→ missing GID %s [!]",
                             ruser.pw_name, euser.pw_name,
                             idtostr(dropuid), idtostr(dropgid),
                             args.ndropgrps, idtostr(grp));
                    }
                }
            }

            for (int j = 0; j < ndroppedgrps; ++j) {
                size_t nelems = (size_t) ngroups;
                gid_t grp = dropgroups[j];
                void *grpp;

                grpp = lfind(&grp, groups, &nelems,
                             sizeof(*groups), (Compar) cmpgids);
                if (grpp == NULL && grp != dropgid) {
                    errx(TEST_FAILED,
                         "(%s, %s, %s, %s, %d) ─→ wrong GID %s [!]",
                         ruser.pw_name, euser.pw_name,
                         idtostr(dropuid), idtostr(dropgid),
                         args.ndropgrps, idtostr(grp));
                }
            }

            exit(0);
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

                signo = WTERMSIG(status);
                if (signo != args.signo) {
                    result = TEST_FAILED;
                    warnx("(%s, %s, %s, %s, %d) ↑ %s [!]",
                         ruser.pw_name, euser.pw_name,
                         idtostr(dropuid), idtostr(dropgid),
                         args.ndropgrps, strsignal(signo));
                }
            } else {
                errx(TEST_ERROR, "child %d exited abnormally", pid);
            }
        }
    }

    /* Running atexit functions tends to break coverage reports. */
    _exit(result);
}
