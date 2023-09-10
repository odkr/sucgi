/*
 * Header for groups.c.
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

#if !defined(GROUPS_H)
#define GROUPS_H

#include <sys/types.h>
#include <stdbool.h>

#include "attr.h"

/*
 * Check whether only the given secondary groups are set.
 *
 * FIXME: Not unit-tested.
 */
_read_only(2, 1) _nonnull(2) _nodiscard _unused
bool supgroups_is_subset(size_t ngroups, const gid_t *groups);

/*
 * Check whether two group IDs are the same.
 */
_read_only(1) _read_only(2) _nonnull(1, 2) _nodiscard _unused
int groups_comp(const gid_t *group1, const gid_t *group2);

/*
 * Search for a group in an array of groups and return either a pointer
 * to the first array element that matches the given group or NULL if no
 * matching group was found.
 *
 * FIXME: Not unit-tested.
 */
_read_only(3, 2) _nonnull(3) _nodiscard _unused
gid_t *group_find(gid_t group, size_t ngroups, const gid_t *groups);

/*
 * Check whether one set of groups is a subset of another set of groups.
 *
 * FIXME: Not unit-tested.
 */
_read_only(2, 1) _read_only(4, 3) _nonnull(2, 4) _nodiscard _unused
bool groups_are_subset(size_t nsub, const gid_t *sub,
                       size_t nsuper, const gid_t *super);

#endif /* !defined(GROUPS_H) */
