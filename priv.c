/*
 * Drop privileges.
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
#include <unistd.h>

#include "compat.h"
#include "priv.h"
#include "types.h"


Error
privdrop(const uid_t uid, const gid_t gid,
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
        /* Should be unreachable. */
        return ERR_SYS;
    }

    errno = 0;
    if (setuid(uid) != 0) {
        /* Should be unreachable. */
        return ERR_SYS;
    }

    if (setgroups(1, (gid_t [1]) {0}) != -1) {
        /* Should be unreachable. */
        return ERR_PRIV;
    }

    if (setgid(0) != -1) {
        /* Should be unreachable. */
        return ERR_PRIV;
    }

    if (setuid(0) != -1) {
        /* Should be unreachable. */
        return ERR_PRIV;
    }

    return OK;
}

Error
privsuspend(void)
{
    const uid_t uid = getuid();
    const gid_t gid = getgid();

    if (geteuid() == 0) {
        errno = 0;
        if (setgroups(1, (gid_t [1]) {gid}) != 0) {
            /* Should be unreachable. */
            return ERR_SYS;
        }
    }

    errno = 0;
    if (setegid(gid) != 0) {
        /* Should be unreachable. */
        return ERR_SYS;
    }

    errno = 0;
    if (seteuid(uid) != 0) {
        /* Should be unreachable. */
        return ERR_SYS;
    }

    if (geteuid() != uid) {
        /* Should be unreachable. */
        return ERR_PRIV;
    }

    if (getegid() != gid) {
        /* Should be unreachable. */
        return ERR_PRIV;
    }

    return OK;
}
