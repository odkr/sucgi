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
#include <grp.h>
#include <search.h>
#include <stdbool.h>
#include <stdint.h>
#include <syslog.h>
#include <unistd.h>

#include "groups.h"
#include "macros.h"
#include "params.h"
#include "types.h"

int
groups_comp(const gid_t *const group1, const gid_t *const group2)
{
    assert(group1 != NULL);
    assert(group2 != NULL);

    if (*group1 < *group2) {
        return -1;
    }

    if (*group1 > *group2) {
        return 1;
    }

    return 0;
}

gid_t *
groups_find(const gid_t group, size_t ngroups, const gid_t *const groups)
{
    /* cppcheck-suppress misra-c2012-11.5; cast is safe. */
    return (gid_t *) lfind(&group, groups, &ngroups, sizeof(*groups),
                           (int (*)(const void *, const void *)) groups_comp);
}

bool
groups_are_subset(const size_t nsub, const gid_t *const sub,
                  const size_t nsuper, const gid_t *const super)
{
    for (size_t i = 0; i < nsub; ++i) {
        if (groups_find(sub[i], nsuper, super) == NULL) {
            return false;
        }
    }

    return true;
}
