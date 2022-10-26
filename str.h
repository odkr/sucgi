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

#include "macros.h"
#include "error.h"


/*
 * Constants
 */

#if !defined(MAX_STR)
#if defined(PATH_MAX) && PATH_MAX > -1 && PATH_MAX < 1024
#define MAX_STR PATH_MAX
#else /* defined(PATH_MAX) && PATH_MAX > -1 && PATH_MAX < 1024 */
#define MAX_STR 1024
#endif /* defined(PATH_MAX) && PATH_MAX > -1 && PATH_MAX < 1024 */
#else /* !defined(MAX_STR) */
#if defined(PATH_MAX) && PATH_MAX > -1 && PATH_MAX < MAX_STR
#error MAX_STR is greater than PATH_MAX.
#endif /* defined(PATH_MAX) && PATH_MAX > -1 && PATH_MAX < MAX_STR */
#endif /* !defined(MAX_STR) */


/*
 * Functions
 */

/*
 * Copy LEN bytes from string SRC to DEST, which will be NUL-terminated.
 * DEST must be large enough to hold LEN + 1 bytes.
 *
 * Return code:
 *      OK       Success.
 *      ERR_LEN  SRC was truncated.
 */
__attribute((nonnull(2, 3)))
enum error str_cp(const size_t len, const char *const src,
                  /* RATS: ignore; must be checked by developers. */
                  char dest[len + 1U]);

/*
 * Split S at the first occurence of any character in SEP and store a copy of
 * the substring up to, but not including, that character in HEAD and a
 * pointer to the substring starting after that character in TAIL.
 *
 * HEAD and TAIL are meaningless if an error occurs.
 *
 * Return code:
 *      OK       Success.
 *      ERR_LEN  S is too long.
 */
__attribute__((nonnull(1, 2, 3, 4), warn_unused_result))
enum error str_split(const char *const s, const char *const sep,
                     /* RATS: ignore; str_split respects MAX_STR. */
                     char (*const head)[MAX_STR], char **const tail);


#endif /* !defined(STR_H) */
