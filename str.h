/*
 * Headers for str.c.
 *
 * Copyright 2022 Odin Kroeger
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

#include <stdbool.h>

#include "sysconf.h"
#include "types.h"


/*
 * Copy N bytes, excluding the terminating NUL, from string SRC to DEST, which
 * will be NUL-terminated. DEST must be large enough to hold N + 1 bytes.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  SRC was truncated.
 */
__attribute((nonnull(2, 3)))
enum retval str_cp(const size_t n, const char *const src, char dest[n + 1U]);

/*
 * Split S at the first occurence of any character in SEP and store a copy of
 * the substring up to, but not including, that character in HEAD and a
 * pointer to the substring starting after that character in TAIL.
 *
 * If the substring up to SEP is longer than MAX - 1 characters, an error is
 * returned. HEAD must be large enough to hold MAX characters, including the
 * terminating NUL. 
 *
 * HEAD and TAIL are meaningless if an error occurs.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  HEAD is longer than MAX.
 */
__attribute__((nonnull(2, 3, 4, 5), warn_unused_result))
enum retval str_split(size_t max, const char *const s, const char *const sep,
                      char head[max], char **const tail);


#endif /* !defined(STR_H) */
