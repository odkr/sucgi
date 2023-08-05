/*
 * Process group verification.
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

#define _XOPEN_SOURCE 700

#if defined(__OPTIMIZE__) && !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <search.h>
#include <stdbool.h>
#include <stdint.h>
#include <syslog.h>
#include <unistd.h>

#include "groups.h"
#include "macros.h"
#include "params.h"


bool
groups_match(size_t ngroups, const gid_t *const groups)
{
    gid_t pgroups[MAX_NGROUPS];
    int npgroups;

    assert(ngroups <= MAX_NGROUPS);

    npgroups = getgroups(MAX_NGROUPS, pgroups);
    if (npgroups < 0) {
        /* RATS: ignore; format string is short and a literal. */
        syslog(LOG_ERR, "number of process groups exceeds MAX_NGROUPS.");
        return false;
    }

    if ((size_t) npgroups > ngroups) {
        /* RATS: ignore; format string is short and a literal. */
        syslog(LOG_ERR, "process groups not a subset of expected groups.");
        return false;
    }

    for (int i = 0; i < npgroups; ++i) {
        void *ptr;

        ptr = lfind(&pgroups[i], groups, &ngroups, sizeof(gid_t),
                    (int (*)(const void *, const void *)) groups_compare);
        if (ptr == NULL) {
            if (ISSIGNED(gid_t)) {
                /* RATS: ignore; format string is short and a literal. */
                syslog(LOG_ERR, "process belongs to group %lld.",
                       (long long) pgroups[i]);
            } else {
                /* RATS: ignore; format string is short and a literal. */
                syslog(LOG_ERR, "process belongs to group %llu.",
                       (unsigned long long) pgroups[i]);
            }

            return false;
        }
    }

    return true;
}

int
groups_compare(const gid_t *const gid1, const gid_t *const gid2)
{
    assert(gid1 != NULL);
    assert(gid2 != NULL);

    if (*gid1 < *gid2) {
        return -1;
    }

    if (*gid1 > *gid2) {
        return 1;
    }

    return 0;
}
