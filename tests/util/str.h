/*
 * Header for str.c.
 *
 * Copyright 2023 Odin Kroeger.
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

#if !defined(TESTS_UTIL_STR_H)
#define TESTS_UTIL_STR_H

#include "../../attr.h"

/*
 * Check whether two pointers point to equal strings.
 *
 * Return value:
 *     Zero      Strings are equal.
 *     Non-zero  Otherwise.
 */
_read_only(1) _read_only(2) _nonnull(1, 2) _nodiscard
int str_cmp_ptrs(const char *const *str1, const char *const *str2);

#endif /* !defined(TESTS_UTIL_STR_H) */
