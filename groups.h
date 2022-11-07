/*
 * Headers for gids.c.
 *
 * Copyright 2022 Odin Kroeger
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

#if !defined(GIDS_H)
#define GIDS_H

#include "types.h"


/*
 * Fetch N groups that the user named LOGNAME is a member of and store
 * the group ID GID as well the group IDs of those groups in GROUPS and
 * the total number of group IDs, including GID, in N.
 *
 * GIDS must be large enough to hold NGIDS supplementary groups.
 *
 * Return value:
 *      OK         Success.
 *      ERR_LEN    LOGNAME belongs to more than NGIDS groups.
 *      ERR_GETGR  getgrent(3) failed.
 */
__attribute__((no_sanitize("alignment"), nonnull(1, 3, 4), warn_unused_result))
enum retval groups_get(const char *const logname, const gid_t basegid,
                       gid_t *const groups, int *const n);

#endif /* !defined(GIDS_H) */
