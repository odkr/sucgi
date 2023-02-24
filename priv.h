/*
 * Header file for priv.c.
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

#if !defined(PRIV_H)
#define PRIV_H

#include <pwd.h>

#include "cattr.h"
#include "compat.h"
#include "types.h"


/*
 * Set the real and effective user and group IDs to the given UID and GID and
 * the supplementary groups to the first N group IDs in GROUPS respectively.
 *
 * Return value:
 *     OK                 Success.
 *     ERR_SYS_SETUID     setuid failed.
 *     ERR_SYS_SETGID     setgid failed.
 *     ERR_SYS_SETGROUPS  setgroups failed.
 *     ERR_PRIV_RESUME    Superuser privileges could be resumed.
 */
__attribute__((nonnull(4), warn_unused_result))
/* cppcheck-suppress misra-c2012-8.2; declaration is in prototype form. */
Error priv_drop(uid_t uid, gid_t gid, SETGRPNUM_T n, const gid_t groups[n]);

/*
 * Set the effective user, group, and supplementary groups IDs
 * to the real user and group IDs respectively.
 *
 * Return value:
 *     OK                  Success.
 *     ERR_SYS_SETGROUPS*  setgroups failed.
 *     ERR_SYS_SETEUID*    seteuid failed.
 *     ERR_SYS_SETEGID*    setegid failed.
 *
 *     * These errors should be unreachable.
 */
__attribute__((warn_unused_result))
Error priv_suspend(void);


#endif /* !defined(PRIV_H) */
