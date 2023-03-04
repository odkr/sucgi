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
 * Copy at most NBYTES bytes, excluding the terminating NUL, from SRC
 * to DEST and terminate the string at DEST with NUL. DEST must be large
 * enough to hold NBYTES + 1 bytes.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  Source is longer than LEN bytes.
 */
__attribute__((nonnull(2, 3)))
Error str_cp(size_t nbytes, const char *src, char *dest);

/*
 * Split STR at the first occurence of any byte in SEP, copy the substring
 * up to, but not including, that byte, to HEAD, terminate it with NUL, and
 * return a pointer to the substring that starts after that byte in TAIL.
 *
 * The substring before the separator must be at most SIZE - 1 bytes long.
 * HEAD must be large enough to hold SIZE bytes, including the terminating NUL.
 *
 * HEAD and TAIL are meaningless if an error occurs.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  Head is too long.
 */
__attribute__((nonnull(1, 2, 4, 5), warn_unused_result))
Error str_split(const char *str, const char *sep,
                size_t size, char *head, const char **tail);


#endif /* !defined(STR_H) */
