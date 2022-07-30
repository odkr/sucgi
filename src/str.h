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

#if !defined(SRC_STR_H)
#define SRC_STR_H

#include <limits.h>
#include <stdbool.h>

#include "attr.h"
#include "err.h"


/*
 * Constants
 */

/* Maximum length of strings in bytes, without the terminating nullbyte. */
#if PATH_MAX > 4096
#define STR_MAX_LEN PATH_MAX
#else
#define STR_MAX_LEN 4096
#endif


/*
 * Data types
 */

/* A str4096 string type. */
/* flawfinder: ignore (this type is defined to facilitate bound checks). */
typedef char str4096[STR_MAX_LEN + 1];


/*
 * Functions
 */

/*
 * Copy a string from src to dest.
 * dest must be large enough to hold a string that is STR_MAX_LEN bytes long,
 * excluding the terminating null byte.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_LEN  src is longer than STR_MAX_LEN.
 */
__attribute__((READ_ONLY(1)))
error str_cp(const char *const src, str4096 *dest);

/* Return true if s1 and s2 are equal and false otherwise. */
__attribute__((READ_ONLY(1), READ_ONLY(2)))
bool str_eq(const char *const s1, const char *const s2);

/*
 * Return true if s matches any shell wildcard pattern in pats.
 * pats must be NULL-terminated. See fnmatch(3) for pattern syntax and flags.
 */
__attribute__((READ_ONLY(1), READ_ONLY(2), READ_ONLY(3)))
bool str_matchn(const char *const s, const char *const *const pats,
                const int flags);

/*
 * Calculate the length of a string.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_LEN  The string is longer than STR_MAX_LEN.
 */
__attribute__((READ_ONLY(1)))
error str_len(const char *const s, size_t *len);

/*
 * Split s at the first occurence of any character in sep and store a copy of
 * the substring up to, but not including, that character in head and a
 * pointer to the first character after that character in tail.
 *
 * head must be large enough to hold a string that is STR_MAX_LEN bytes long,
 * excluding the terminating null byte.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_LEN  s is longer than STR_MAX_LEN.
 */
__attribute__((READ_ONLY(1), READ_ONLY(2)))
error
str_split(const char *const s, const char *const sep,
          str4096 *head, char **tail);


#endif /* !defined(SRC_STR_H) */
