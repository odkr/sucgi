/*
 * Header file for handler.c.
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

#if !defined(PAIR_H)
#define PAIR_H

#include <sys/types.h>

#include "attr.h"
#include "types.h"

/*
 * Search for the value of the given key in the given array of key-value pairs
 * and return that value in "value". The array of key-value pairs must contain
 * at least "npairs" elements; supernumery elements are ignored.
 *
 * Return value:
 *     OK          Success.
 *     ERR_SEARCH  There is no pair with the given key.
 */
_read_only(2, 1) _read_only(4, 3) _write_only(5) _nonnull(2, 4, 5) _nodiscard
Error pair_find(size_t npairs, const Pair *pairs,
                size_t keylen, const char *key, const char **value);

#endif /* !defined(PAIR_H) */
