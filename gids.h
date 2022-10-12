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

#include "error.h"


/*
 * Fetch all groups that the user named LOGNAME is a member of and store
 * BASEGID as well the IDs of those groups in GIDS and the total number
 * of group IDs, including BASEGID, in N. GIDS must be large enough to
 * hold NGROUPS_MAX entries.
 *
 * Return code:
 *      OK            Success.
 *      ERR_GIDS_MAX  LOGNAME belongs to more than NGROUPS_MAX groups.
 *      ERR_SYS       getgrent failed. errno(2) should be set.
 */
__attribute__((nonnull(3, 4), warn_unused_result))
enum error gids_get_list(const char *const logname, const gid_t gid,
                         /* RATS: ignore; gids is bounds-checked. */
                         gid_t (*const gids)[NGROUPS_MAX], int *const n);


#endif /* !defined(GIDS_H) */
