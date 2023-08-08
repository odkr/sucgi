/*
 * Test priv_drop.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * SuCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * SuCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#define _BSD_SOURCE
/* _DARWIN_C_SOURCE must NOT be set, or else getgroups will be pointless. */
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <search.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../attr.h"
#include "../compat.h"
#include "../groups.h"
#include "../macros.h"
#include "../params.h"
#include "../priv.h"
#include "util/array.h"
#include "util/user.h"
#include "util/types.h"


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    uid_t ruid;                 /* Initial real process UID. */
    uid_t euid;                 /* Initial effective process UID. */
    uid_t targetuid;            /* UID to set. */
    gid_t targetgid;            /* Primary GID to set. */
    int ntargetgids;            /* Number of supplementary GIDs to set. */
    Error retval;               /* Return value. */
    int signal;                 /* Signal raised, if any. */
} PrivDropArgs;


/*
 * Prototypes
 */

/*
 * Return a string representation of the given arguments.
 * Terminates the calling process if an error occurs.
 */
_read_only(1) _nonnull(1)
static char *fnsig_to_str(const PrivDropArgs *args);

/*
 * Copy a string representation of the given arguments to the memory area
 * pointed to by "str", which must be of the given size and large enough
 * to hold the resulting string, including the terminating NUL.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno may be set.
 */
_read_only(1) _write_only(3, 2) _nonnull(1)
static int fnsig_copy_to_str(const PrivDropArgs *args, size_t size, char *str);


/*
 * Main
 */

int
main(void)
{
    ASSERT(sizeof(GRP_T) == sizeof(gid_t));
    ASSERT(MAX_NGRPS_VAL >= INT_MAX);
    ASSERT((NGRPS_T) MAX_NGRPS_VAL == MAX_NGRPS_VAL);

    const uid_t superuid = 0;
    const gid_t supergid = 0;
    uid_t regularuid;
    uid_t regulargid;

    if (getuid() == 0) {
        struct passwd pwd;

        if (user_get_regular(&pwd, err) != 0) {
            errx(SKIP, "no regular user found");
        }

        regularuid = pwd.pw_uid;
        regulargid = pwd.pw_gid;
    } else {
        regularuid = getuid();
        regulargid = getgid();
    }

    const PrivDropArgs cases[] = {
#if !defined(NDEBUG)
        /* Illegal arguments. */
        {
            superuid, superuid,
            superuid, supergid, MAX_NGROUPS,
            OK, SIGABRT
        },
        {
            superuid, superuid,
            regularuid, supergid, MAX_NGROUPS,
            OK, SIGABRT
        },
        {
            regularuid, regularuid,
            superuid, supergid, MAX_NGROUPS,
            ERR_SYS, SIGABRT
        },
        {
            regularuid, regularuid,
            regularuid, supergid, MAX_NGROUPS,
            ERR_SYS, SIGABRT
        },
#endif

        {
            superuid,   superuid,
            regularuid, regulargid, MAX_NGROUPS,
            OK, 0
        },
        {
            regularuid, superuid,
            regularuid, regulargid, MAX_NGROUPS,
            OK, 0
        },
        {
            regularuid, superuid,
            regularuid, regulargid, 0,
            OK, 0
        },
        {
            regularuid, superuid,
            regularuid, regulargid, 1,
            OK, 0
        },
        {
            superuid, regularuid,
            regularuid, regulargid, MAX_NGROUPS,
            ERR_SYS, 0
        },
        {
            regularuid, regularuid,
            regularuid, regulargid, MAX_NGROUPS,
            ERR_SYS, 0
        }
    };

    int result = PASS;

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const PrivDropArgs args = cases[i];
        struct passwd *targetuser;
        pid_t pid;

        errno = 0;
        targetuser = getpwuid(args.targetuid);
        if (targetuser == NULL) {
            /* cppcheck-suppress misra-c2012-22.10; getpwuid sets errno. */
            if (errno == 0) {
                errx(ERROR, "UID %s: no such user",
                     user_id_to_str(args.targetuid, err));
            } else {
                err(ERROR, "getpwuid");
            }
        }

        /* RATS: ignore; fork is used safely. */
        pid = fork();
        if (pid == 0) {
            gid_t targetgids[MAX_NGROUPS];
            gid_t gids[MAX_NGROUPS];
            uid_t ruid;
            uid_t euid;
            gid_t rgid;
            gid_t egid;
            int ntargetgids = NELEMS(targetgids);
            int ngids;
            Error retval;

            if (geteuid() == 0) {
                gid_t args_rgid;
                gid_t args_egid;

                if (user_get_gid(args.euid, &args_egid, err) != 0) {
                    errx(ERROR, "UID %s: no such user",
                         user_id_to_str(args.euid, err));
                }

                if (user_get_gid(args.ruid, &args_rgid, err) != 0) {
                    errx(ERROR, "UID %s: no such user",
                         user_id_to_str(args.ruid, err));
                }

                errno = 0;
                if (setregid(args_rgid, args_egid) != 0) {
                    err(ERROR, "setregid");
                }

                errno = 0;
                if (setreuid(args.ruid, args.euid) != 0) {
                    err(ERROR, "setreuid");
                }
            } else {
                if (getuid() != args.ruid || geteuid() != args.euid) {
                    errx(SKIP, "skipping %s ...", fnsig_to_str(&args));
                }
            }

            if (getgrouplist(targetuser->pw_name, (GRP_T) args.targetgid,
                             (GRP_T *) targetgids, &ntargetgids) < 0)
            {
                errx(SKIP, "user %s: belongs to too many groups.",
                     targetuser->pw_name);
            }

            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            retval = priv_drop(args.targetuid, args.targetgid,
                              (NGRPS_T) ntargetgids, targetgids);
            if (retval != args.retval) {
                errx(FAIL, "%s → %u [!]", fnsig_to_str(&args), retval);
            }

            if (retval != OK) {
                exit(0);
            }

            ruid = getuid();
            if (ruid != args.targetuid) {
                errx(FAIL, "%s ─→ <ruid> = %s [!]",
                     fnsig_to_str(&args), user_id_to_str(ruid, err));
            }

            euid = geteuid();
            if (euid != args.targetuid) {
                errx(FAIL, "%s ─→ <euid> = %s [!]",
                     fnsig_to_str(&args), user_id_to_str(euid, err));
            }

            rgid = getgid();
            if (rgid != args.targetgid) {
                errx(FAIL, "%s ─→ <rgid> = %s [!]",
                     fnsig_to_str(&args), user_id_to_str(rgid, err));
            }

            egid = getegid();
            if (egid != args.targetgid) {
                errx(FAIL, "%s ─→ <egid> = %s [!]",
                     fnsig_to_str(&args), user_id_to_str(egid, err));
            }

            errno = 0;
            ngids = getgroups(MAX_NGROUPS, gids);
            if (ngids == -1) {
                err(ERROR, "getgroups");
            }

            if (ntargetgids == ngids) {
                if (
                    !array_eq(
                        gids, (size_t) ngids, sizeof(*gids),
                        targetgids, (size_t) ntargetgids, sizeof(*targetgids),
                        (CompFn) groups_compare
                    )
                ) {
                    errx(FAIL, "%s ─→ supplementary GID is missing [!]",
                         fnsig_to_str(&args));
                }
            } else if (ntargetgids > ngids) {
                if (
                    !array_is_sub(
                        gids, (size_t) ngids, sizeof(*gids),
                        targetgids, (size_t) ntargetgids, sizeof(*targetgids),
                        (CompFn) groups_compare
                    )
                ) {
                    errx(FAIL, "%s ─→ supplementary GID is wrong [!]",
                         fnsig_to_str(&args));
                }
            } else {
                errx(FAIL, "%s ─→ too many supplementary GIDs set",
                     fnsig_to_str(&args));
            }

            /*
             * Tests are run by this process, so coverage
             * data should be written by this process, too.
             */
            exit(0);
        } else {
            int status;

            do {
                errno = 0;
                if (waitpid(pid, &status, 0) >= 0) {
                    break;
                }
                /* cppcheck-suppress misra-c2012-22.10; waitpid sets errno. */
                if (errno != EINTR) {
                    err(ERROR, "waitpid %d", pid);
                }
            } while (true);

            /* cppcheck-suppress misra-c2012-14.4; return value is boolean. */
            if (WIFEXITED(status)) {
                int exitstatus = WEXITSTATUS(status);

                switch (exitstatus) {
                case PASS:
                    break;
                case FAIL:
                    result = FAIL;
                    break;
                case SKIP:
                    if (result == (int) PASS) {
                        result = SKIP;
                    }
                    break;
                default:
                    return ERROR;
                }
            /* cppcheck-suppress misra-c2012-14.4; return value is boolean. */
            } else if (WIFSIGNALED(status)) {
                int signal = WTERMSIG(status);

                if (signal != args.signal) {
                    result = FAIL;
                    warnx("%zu: %s ↑ %s [!]",
                          i, fnsig_to_str(&args), strsignal(signal));
                }
            } else {
                errx(ERROR, "child %d exited abnormally", pid);
            }
        }
    }

    /* Running atexit functions in this process breaks coverage reports. */
    _exit(result);
}


/*
 * Functions
 */

static char *
fnsig_to_str(const PrivDropArgs *const args)
{
    char *buffer;
    int len;
    size_t size;

    len = fnsig_copy_to_str(args, 0, NULL);
    if (len < 0) {
        err(ERROR, "snprintf");
    }

    size = (size_t) len + 1U;

    /* cppcheck-suppress misra-c2012-11.5; bad advice for malloc. */
    buffer = malloc(size);
    if (buffer == NULL) {
        err(ERROR, "malloc");
    }

    len = fnsig_copy_to_str(args, size, buffer);
    if (len < 0) {
        err(ERROR, "snprintf");
    }

    return buffer;
}

static int
fnsig_copy_to_str(const PrivDropArgs *const args,
                  const size_t size, char *const str)
{
    int nchars = 0;

    errno = 0;
    if (ISSIGNED(id_t)) {
        /* RATS: ignore; format is a literal and expansion is bounded. */
        nchars = snprintf(
            str,
            size,
            "<ruid>=%lld <euid>=%lld ─→ (%lld, %lld, %d, <gids>)",
            (long long) args->ruid,
            (long long) args->euid,
            (long long) args->targetuid,
            (long long) args->targetgid,
            args->ntargetgids
        );
    } else {
        /* RATS: ignore; format is a literal and expansion is bounded. */
        nchars = snprintf(
            str,
            size,
            "<ruid>=%llu <euid>=%llu ─→ (%llu, %llu, %d, <gids>)",
            (unsigned long long) args->ruid,
            (unsigned long long) args->euid,
            (unsigned long long) args->targetuid,
            (unsigned long long) args->targetgid,
            args->ntargetgids);
    }

    return nchars;
}

