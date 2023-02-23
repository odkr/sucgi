/*
 * Header file for str.c.
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

#if !defined(STR_H)
#define STR_H

#include <stdlib.h>

#include "cattr.h"
#include "types.h"


/*
 * Copy at most N bytes, excluding the terminating NUL, from SRC to DEST and
 * then NUL-terminate DEST, which must be large enough to hold N + 1 bytes.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  The source string is longer than N bytes.
 */
__attribute__((nonnull(2, 3)))
/* cppcheck-suppress misra-c2012-8.2; declaration is in prototype form. */
Error str_cp(const size_t n, const char *const src, char dest[n + 1U]);

/*
 * Split STR at the first occurence of any byte in SEP and store a copy
 * of the substring up to, but not including, that byte, in HEAD and
 * a pointer to the substring that starts after that byte in TAIL.
 *
 * The substring before the first separator must be at most N - 1 bytes long.
 * HEAD must be large enough to hold N bytes, including the terminating NUL.
 *
 * HEAD and TAIL are meaningless if an error occurs.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  There are more than N - 1 bytes before the first separator.
 */
__attribute__((nonnull(1, 2, 4, 5), warn_unused_result))
/* cppcheck-suppress misra-c2012-8.2; declaration is in prototype form. */
Error str_split(const char *const str, const char *const sep,
                size_t n, char head[n], const char **const tail);


#endif /* !defined(STR_H) */
