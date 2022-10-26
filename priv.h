/*
 * Headers for priv.c.
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

#if !defined(PRIV_H)
#define PRIV_H

#include <limits.h>
#include <pwd.h>

#include "macros.h"
#include "error.h"


/*
 * Set the process' real and effective UID and GID to UID and GID
 * and the supplementary groups to GIDS respectively, where NGIDS
 * is the number of group IDs in GIDS.
 *
 * Return code:
 *      OK             Success.
 *      ERR_SETUID     setuid(2) failed.
 *      ERR_SETGID     setgid(2) failed.
 *      ERR_SETGROUPS  setgroups(2) failed.
 *      FAIL           Superuser privileges could be resumed.
 */
__attribute__((nonnull(4), warn_unused_result))
enum error priv_drop(const uid_t uid, const gid_t gid,
                     const int ngids, const gid_t gids[ngids]);


#endif /* !defined(PRIV_H) */
