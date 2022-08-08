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
 * System
 */

#if !defined(PATH_MAX)
/* Flawfinder: ignore; this is not a call to system(3). */
#error suCGI requires a POSIX.1-2008-compliant operating system.
#endif


/*
 * Constants
 */

/* Maximum length of strings in bytes, including the terminating nullbyte. */
#if PATH_MAX > 4096
#define STR_MAX ((size_t) PATH_MAX)
#else
#define STR_MAX ((size_t) 4096)
#endif


/*
 * Functions
 */

/*
 * Copy a string from src to dest.
 * dest must be large enough to hold a string that is STR_MAX - 1 bytes long,
 * excluding the terminating null byte.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_MAX  src is longer than STR_MAX - 1 bytes.
 */
error str_cp(const char *const src,
             /* Flawfinder: ignore; strncpy copies at most STR_MAX bytes. */
             char (*dest)[STR_MAX]);

/*
 * Copy n bytes from string src to dest.
 * dest must be large enough to hold a string that is STR_MAX - 1 bytes long,
 * excluding the terminating null byte.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_MAX  n is larger than STR_MAX - 1 bytes.
 */
error str_cpn(const size_t n, const char *const src,
              /* Flawfinder: ignore; stpncpy copies at most STR_MAX bytes. */
	      char (*dest)[STR_MAX]);

/* Return true if s1 and s2 are equal and false otherwise. */
bool str_eq(const char *const s1, const char *const s2);

/*
 * Return true if s matches any shell wildcard pattern in pats.
 * pats must be NULL-terminated. See fnmatch(3) for pattern syntax and flags.
 */
bool str_matchv(const char *const s, const char *const *const pats,
                  const int flags);

/*
 * Calculate the length of a string.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_MAX  The string is longer than STR_MAX - 1 bytes.
 */
error str_len(const char *const s, size_t *len);

/*
 * Split s at the first occurence of any character in sep and store a copy of
 * the substring up to, but not including, that character in head and a
 * pointer to the first character after that character in tail.
 *
 * head must be large enough to hold a string that is STR_MAX - 1 bytes long,
 * excluding the terminating null byte.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_MAX  s is longer than STR_MAX - 1 bytes.
 */
error str_split(const char *const s, const char *const sep,
                /* Flawfinder: ignore; str_cp respects STR_MAX. */
		char (*head)[STR_MAX], char **tail);


#endif /* !defined(SRC_STR_H) */
