/*
 * Header for array.c.
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

#if !defined(TESTS_UTIL_ARRAY_H)
#define TESTS_UTIL_ARRAY_H

#include <sys/types.h>
#include <stdbool.h>

#include "../../attr.h"
#include "types.h"


/*
 * Search the given haystack for the given needle and return a pointer
 * to the first matching element or NULL if no element matches.
 *
 * The haystack must have at least as many elements as the given length,
 * and each element must be exactly as wide as the given width;
 * supernumery elements are ignored.
 *
 * Elements are compared by calling the given comparison function.
 */
_read_only(1, 4) _read_only(2, 3) _nonnull(1, 2, 5) _nodiscard
const void *array_find(const void *needle, const void *haystack,
                       size_t len, size_t wd, CompFn cmp);

/*
 * Check whether one array is a subset of another.
 *
 * Each array must have at least as many elements as the given length,
 * and each element must be exactly as wide as the given width;
 * supernumery elements are ignored.
 *
 * Elements are compared by calling the given comparison function.
 */
_read_only(1, 2) _read_only(4, 5) _nonnull(1, 4, 7) _nodiscard
bool array_is_sub(const void *sub, size_t sublen, size_t subwd,
                  const void *super, size_t superlen, size_t superwd,
                  CompFn cmp);

/*
 * Check whether two arrays are non-strict subsets of each other.
 *
 * Otherwise the same as array_is_one.
 */
_read_only(1, 2) _read_only(4, 5) _nonnull(1, 4, 7) _nodiscard
bool array_eq(const void *one, size_t onelen, size_t onewd,
              const void *two, size_t twolen, size_t twowd,
              CompFn cmp);

#endif /* !defined(TESTS_UTIL_ARRAY_H) */
