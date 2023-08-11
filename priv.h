/*
 * Header file for priv.c.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * SuCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * SuCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PRIV_H)
#define PRIV_H

#include <sys/types.h>
#include <pwd.h>

#include "attr.h"
#include "compat.h"
#include "types.h"

/*
 * Set the real and the effective user, group, and supplementary groups to the
 * given user, group, and supplementary groups respectively. At least "ngroups"
 * supplementary groups must be given; supernumery groups are ignored.
 *
 * Return value:
 *      OK         Success.
 *      ERR_SYS    setuid/setgid/setgroups failed.
 *      ERR_PRIV*  Superuser privileges could be resumed.
 *
 *      * This error should be unreachable.
 *
 * Caveats:
 *      Calls getgrgid unless NDEBUG is true.
 */
_read_only(4, 3) _nonnull(4) _nodiscard
/* cppcheck-suppress misra-c2012-8.4; this *is* the declaration. */
Error priv_drop(uid_t uid, gid_t gid, NGRPS_T ngroups, const gid_t *groups);

/*
 * Set the effective user, group, and supplementary groups
 * to the real user and group respectively.
 *
 * Return value:
 *      OK         Success.
 *      ERR_SYS*   seteuid/setegid/setgroups failed.
 *      ERR_PRIV*  The effective user/group ID was not changed.
 *
 *      * These errors should be unreachable.
 *
 * Caveats:
 *      Calls getgrgid unless NDEBUG is true.
 */
_nodiscard
Error priv_suspend(void);

#endif /* !defined(PRIV_H) */
