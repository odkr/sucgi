/*
 * Test priv_suspend.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
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
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../attr.h"
#include "../compat.h"
#include "../macros.h"
#include "../params.h"
#include "../priv.h"
#include "util/abort.h"
#include "util/user.h"
#include "util/types.h"

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif


/*
 * Data types
 */

/* Combinations of effective and real process UIDs. */
typedef struct {
    uid_t ruid;
    uid_t euid;
} PrivSuspendArgs;


/*
 * Prototypes
 */

/*
 * Return a string representation of the given arguments.
 * Terminates the calling process if an error occurs.
 */
_read_only(1) _nonnull(1)
static char *fnsig_to_str(const PrivSuspendArgs *args);

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
static int fnsig_copy_to_str(const PrivSuspendArgs *args,
                             size_t size, char *str);


/*
 * Main
 */

int
main(void)
{
    /* User ID of the superuser. */
    const uid_t superuid = 0;

    /* User ID that only a regular user would have. */
    uid_t reguid = START_UID;

    if (getuid() == 0) {
        struct passwd reguser;

        if (user_get_regular(&reguser, err) == 0) {
            reguid = reguser.pw_uid;
        }
    } else {
        reguid = getuid();
    }

    /* Test cases. */
    const PrivSuspendArgs cases[] = {
        {superuid, superuid},
        {superuid, reguid},
        {reguid, superuid},
        {reguid, reguid}
    };

    /* Test result. */
    volatile int result = PASS;

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const PrivSuspendArgs args = cases[i];
        gid_t rgid;

        if (user_get_gid(args.ruid, &rgid, err) != 0) {
            errx(ERROR, "UID %s: no such user",
                 user_id_to_str(args.ruid, err));
        }

        if (geteuid() == 0) {
            gid_t egid;

            if (user_get_gid(args.euid, &egid, err) != 0) {
                errx(ERROR, "UID %s: no such user",
                     user_id_to_str(args.euid, err));
            }

            errno = 0;
            if (setregid(rgid, egid) != 0) {
                err(ERROR, "setregid");
            }
            if (setreuid(args.ruid, args.euid) != 0) {
                err(ERROR, "setreuid");
            }
        } else {
            if (getuid() != args.ruid || geteuid() != args.euid) {
                result = SKIP;
                warnx("skipping %s ...", fnsig_to_str(&args));
                continue;
            }
        }

        if (sigsetjmp(abort_env, 1) == 0) {
            const bool issuper = (geteuid() == 0);
            Error retval;

            (void) abort_catch(err);
            retval = priv_suspend();
            (void) abort_reset(err);

            if (retval != OK) {
                result = FAIL;
                warnx("%zu: %s → %u [!]", i, fnsig_to_str(&args), retval);
            }

            if (getuid() != args.ruid) {
                result = FAIL;
                warnx("%zu: %s ─→ <ruid> = %s [!]",
                      i, fnsig_to_str(&args), user_id_to_str(getuid(), err));
            }

            if (geteuid() != args.ruid) {
                result = FAIL;
                warnx("%zu: %s ─→ <euid> = %s [!]",
                      i, fnsig_to_str(&args), user_id_to_str(geteuid(), err));
            }

            if (getgid() != rgid) {
                result = FAIL;
                warnx("%zu: %s ─→ <rgid> = %s [!]",
                      i, fnsig_to_str(&args), user_id_to_str(getgid(), err));
            }

            if (getegid() != rgid) {
                result = FAIL;
                warnx("%zu: %s ─→ <egid> = %s [!]",
                      i, fnsig_to_str(&args), user_id_to_str(getegid(), err));
            }

            if (issuper) {
                gid_t groups[MAX_NGROUPS];
                int ngroups;

                errno = 0;
                ngroups = getgroups(MAX_NGROUPS, groups);
                if (ngroups == -1) {
                    err(ERROR, "getgroups");
                }

                if (ngroups != 1) {
                    result = FAIL;
                    warnx("%zu: %s ─→ <ngroups> = %d [!]",
                          i, fnsig_to_str(&args), ngroups);
                }

                if (groups[0] != rgid) {
                    result = FAIL;
                    warnx("%zu: %s ─→ <groups[0]> = %s [!]",
                          i, fnsig_to_str(&args),
                          user_id_to_str(groups[0], err));
                }

                errno = 0;
                if (setuid(0) != 0) {
                    err(ERROR, "setuid");
                }
            }
        }

        if (abort_signal != 0) {
            result = FAIL;
            warnx("%zu: %s ↑ %d [!]", i, fnsig_to_str(&args), abort_signal);
        }
    }

    return result;
}


/*
 * Functions
 */

static char *
fnsig_to_str(const PrivSuspendArgs *const args)
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
fnsig_copy_to_str(const PrivSuspendArgs *const args,
                  const size_t size, char *const str)
{
    int nchars = 0;

    errno = 0;
    if (ISSIGNED(id_t)) {
        /* RATS: ignore; format is a literal and expansion is bounded. */
        nchars = snprintf(str, size, "<ruid>=%lld <euid>=%lld ─→ ()",
                          (long long) args->ruid,
                          (long long) args->euid);
    } else {
        /* RATS: ignore; format is a literal and expansion is bounded. */
        nchars = snprintf(str, size, "<ruid>=%llu <euid>=%llu ─→ ()",
                          (unsigned long long) args->ruid,
                          (unsigned long long) args->euid);
    }

    return nchars;
}
