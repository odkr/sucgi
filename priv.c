/*
 * Drop privileges.
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
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#if defined(__OPTIMIZE__) && !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <grp.h>
#include <search.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "groups.h"
#include "macros.h"
#include "params.h"
#include "priv.h"
#include "types.h"


/*
 * Macros
 */

/* getgroups is unreliable on macOS. */
#if !defined(__MACH__)
#define SGROUPS_ARE_SUBSET(...) (sgroups_are_subset(__VA_ARGS__))
#else
#define SGROUPS_ARE_SUBSET(...) (true)
#endif


/*
 * Prototypes
 */

/*
 * Check whether the secondary groups are a subset of the given groups.
 *
 * Caveats:
 *      Unreliable on macOS.
 */
static bool sgroups_are_subset(size_t ngroups, const gid_t *groups) _unused;


/*
 * Functions
 */

static bool
sgroups_are_subset(size_t ngroups, const gid_t *const groups)
{
    ASSERT(MAX_NGROUPS < SIZE_MAX);

    gid_t sgroups[MAX_NGROUPS];
    int numsgroups = getgroups(MAX_NGROUPS, sgroups);
    if (numsgroups < 0) {
        return false;
    }

    return groups_are_subset((size_t) numsgroups, sgroups, ngroups, groups);
}

Error
priv_drop(const uid_t uid, const gid_t gid,
          const NGRPS_T ngroups, const gid_t *const groups)
{
    assert(uid > 0);
    assert(gid > 0);
    assert(ngroups > 0);
    assert((uintmax_t) ngroups <= (uintmax_t) SIZE_MAX);

    errno = 0;
    if (setgroups(ngroups, groups) != 0) {
        return ERR_SYS;
    }

    errno = 0;
    if (setgid(gid) != 0) {
        /* NOTREACHED */
        return ERR_SYS;
    }

    errno = 0;
    if (setuid(uid) != 0) {
        /* NOTREACHED */
        return ERR_SYS;
    }

    if (setgroups(1, (gid_t [1]) {0}) != -1) {
        /* NOTREACHED */
        return ERR_PRIV;
    }

    if (setgid(0) != -1) {
        /* NOTREACHED */
        return ERR_PRIV;
    }

    if (setuid(0) != -1) {
        /* NOTREACHED */
        return ERR_PRIV;
    }

    assert(geteuid() == uid);
    assert(getegid() == gid);
    assert(getuid() == uid);
    assert(getgid() == gid);
    assert(SGROUPS_ARE_SUBSET((size_t) ngroups, groups));

    return OK;
}

Error
priv_suspend(void)
{
    const uid_t uid = getuid();
    const gid_t gid = getgid();

    if (geteuid() == 0) {
        const gid_t gids[] = {gid};
        const size_t ngids = NELEMS(gids);

        assert((uintmax_t) ngids < (uintmax_t) MAX_NGRPS_VAL);

        errno = 0;
        /* cppcheck-suppress misra-c2012-10.8; information cannot be lost. */
        if (setgroups((NGRPS_T) NELEMS(gids), gids) != 0) {
            /* NOTREACHED */
            return ERR_SYS;
        }

        assert(SGROUPS_ARE_SUBSET(ngids, gids));
    }

    errno = 0;
    if (setegid(gid) != 0) {
        /* NOTREACHED */
        return ERR_SYS;
    }

    errno = 0;
    if (seteuid(uid) != 0) {
        /* NOTREACHED */
        return ERR_SYS;
    }

    if (geteuid() != uid) {
        /* NOTREACHED */
        return ERR_PRIV;
    }

    if (getegid() != gid) {
        /* NOTREACHED */
        return ERR_PRIV;
    }

    assert(getuid() == geteuid());
    assert(getgid() == getegid());

    return OK;
}
