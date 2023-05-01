/*
 * Header for str.c.
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

#if !defined(TESTS_LIB_STR_H)
#define TESTS_LIB_STR_H

#include <stdbool.h>

#include "../../attr.h"


/*
 * Convert the given ID into a string and return a pointer to that string.
 * The memory pointed to should be freed by the caller.
 *
 * Return value:
 *     NULL      Conversion failed. errno should be set.
 *     Non-NULL  A pointer to the converted ID.
 */
__attribute__((warn_unused_result))
char *idtostr(id_t id);

/*
 * Append at most SIZE bytes from SRC to DEST and
 * return a pointer to the terminating NUL.
 *
 * Return value:
 *     Non-NULL  Success.
 *     NULL      String was truncated.
 */
__attribute__((nonnull(2, 3)))
char *catstrs(size_t size, char *dest, const char *src);


/*
 * Join the first N strings in STRS using the separator SEP and store the
 * result in DEST, which must be large enough to hold SIZE bytes. If STRS
 * contains a NULL pointer, processing stops at that pointer. STRS must
 * either be NULL-terminated or have at least N elements.
 *
 * Return value:
 *      0  Success.
 *     -1  SIZE is too small to hold the joined string.
 */
__attribute__((nonnull(2, 3, 5), warn_unused_result))
int joinstrs(size_t nstrs, const char *const *strs,
             const char *sep, size_t size, char *dest);


#endif /* !defined(TESTS_LIB_STR_H) */
