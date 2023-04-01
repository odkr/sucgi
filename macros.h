/*
 * Macros.
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

#if !defined(MACROS_H)
#define MACROS_H

#include <limits.h>


/* Trigger a compile-time error if COND is false. */
#define ERRORIF(cond) (void) sizeof(char[(cond) ? -1 : 1])

/* Check whether a given integer TYPE is signed. */
#define ISSIGNED(type) ((type) -1 < (type) 1)

/* Return the lesser of two integer values A and B. */
//#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* Calculate the number of elements in ARRAY. */
#define NELEMS(array) (sizeof((array)) / sizeof(*(array)))

/* Calculate the maximum signed value that a given integer TYPE can hold. */
#define SIGNEDMAX(type) ((1UL << ((size_t) CHAR_BIT * sizeof(type) - 1UL)) - 1UL)

#endif /* !defined(MACROS_H) */
