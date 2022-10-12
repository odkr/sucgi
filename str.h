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

#include <limits.h>
#include <stdbool.h>

#include "defs.h"
#include "error.h"


/*
 * Constants
 */

/*  Maximum length of strings in bytes, including the terminating NUL. */
#if defined(PATH_MAX) && PATH_MAX > -1
#if PATH_MAX <= 2048
#define STR_MAX PATH_MAX
#else /* PATH_MAX <= 2048 */
#define STR_MAX 2048U
#endif /* PATH_MAX <= 2048 */
#else /* defined(PATH_MAX) && PATH_MAX > -1 */
#define STR_MAX 1024U
#endif /* defined(PATH_MAX) && PATH_MAX > -1 */


/*
 * Functions
 */

/*
 * Copy N bytes from string SRC to DEST, which will always be NUL-terminated.
 * DEST must be large enough to hold N + 1 bytes.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_LEN  SRC was truncated.
 */
__attribute((nonnull(2, 3)))
enum error str_cp(const size_t n, const char *const src,
                  /* RATS: ignore; must be checked by developers. */
                  char dest[n + 1U]);

/*
 * Split S at the first occurence of any character in SEP and store a copy of
 * the substring up to, but not including, that character in HEAD and a
 * pointer to the substring starting after that character in TAIL.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_LEN  S is too long.
 */
__attribute__((nonnull(1, 2, 3, 4), warn_unused_result))
enum error str_split(const char *const s, const char *const sep,
                     /* RATS: ignore; str_split respects STR_MAX. */
                     char (*const head)[STR_MAX], char **const tail);


#endif /* !defined(STR_H) */
