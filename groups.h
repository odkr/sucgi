/*
 * Header for groups.c.
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

#if !defined(GROUPS_H)
#define GROUPS_H

#include <sys/types.h>
#include <stdbool.h>

#include "attr.h"

/*
 * Check whether only the given secondary groups are set.
 *
 * Side-effects:
 *      Logs groups that should not be set to the system log as error.
 *
 * FIXME: Not unit-tested.
 */
_read_only(2, 1) _nonnull(2) _nodiscard _unused
bool groups_eq(size_t ngroups, const gid_t *groups);

/*
 * Check whether two group IDs are the same.
 *
 * FIXME: Not unit-tested.
 */
_read_only(1) _read_only(2) _nonnull(1, 2) _nodiscard _unused
int groups_comp(const gid_t *gid1, const gid_t *gid2);

#endif /* !defined(GROUPS_H) */
