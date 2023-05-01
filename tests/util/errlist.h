/*
 * Header for errlist.c.
 *
 * Copyright 2023 Odin Kroeger.
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

#if !defined(TESTS_UTIL_ERRLIST_H)
#define TESTS_UTIL_ERRLIST_H

#include <stdbool.h>

#include "../../attr.h"
#include "../../types.h"


/*
 * Data types
 */

/* List of errors. */
typedef struct {
    size_t nelems;          /* Number of errors. */
    const Error *elems;     /* Errors. */
} ErrList;


/*
 * Functions
 */

/*
 * Check whether HAYSTACK contains NEEDLE.
 *
 * Return value:
 *     true   HAYSTACK contains NEEDLE.
 *     false  HAYSTACK does not contain NEEDLE.
 */
__attribute__((nonnull(1), warn_unused_result))
bool errlistcontains(const ErrList *haystack, const Error needle);


#endif /* !defined(TESTS_UTIL_ERRLIST_H) */
