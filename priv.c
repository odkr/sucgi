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

#include "compat.h"
#include "groups.h"
#include "macros.h"
#include "params.h"
#include "priv.h"
#include "types.h"

Error
priv_drop(const uid_t uid, const gid_t gid,
          const NGRPS_T ngroups, const gid_t *const groups)
{
    assert(uid > 0);
    assert(gid > 0);
    assert(ngroups > 0);

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

/* getgroups is unreliable on macOS. */
#if !defined(__MACH__)
    assert(groups_match((size_t) ngroups, groups));
#endif

    return OK;
}

Error
priv_suspend(void)
{
    const uid_t uid = getuid();
    const gid_t gid = getgid();
    const gid_t gids[] = {gid};

    if (geteuid() == 0) {
        errno = 0;
        if (setgroups(NELEMS(gids), gids) != 0) {
            /* NOTREACHED */
            return ERR_SYS;
        }
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

/* getgroups is unreliable on macOS. */
#if !defined(__MACH__)
    assert(groups_match(NELEMS(gids), (gids)));
#endif /* !defined(__MACH__) */

    return OK;
}
