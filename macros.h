/*
 * Macros.
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

#if !defined(MACROS_H)
#define MACROS_H

#include <limits.h>

/* Trigger a compile-time error if the given condition is false. */
#define ASSERT(cond) ((void) sizeof(char[(cond) ? 1 : -1]))

/* Log the given message as an error and exit with status EXIT_FAILURE. */
#define BUG(msg, ...) error("%s:%d: " msg, __FILE__, __LINE__, __VA_ARGS__);

/* Size of a type in bits. */
#define BITS(type) (sizeof(type) * (size_t) CHAR_BIT)

/* Check whether a type is signed. */
#define ISSIGNED(type) ((type) -1 < (type) 1)

/* Get the number of elements in the given array. */
#define NELEMS(array) (sizeof((array)) / sizeof(*(array)))

/* Get the maximum value a type could hold if it were signed and unpadded. */
#define MAXSVAL(type) (((uintmax_t) 1 << (BITS(type) - 1U)) - 1U)

#endif /* !defined(MACROS_H) */
