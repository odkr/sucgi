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
 * Functions
 */

/*
 * Copy a string from src to dest.
 *
 * dest is allocated just enough memory to contain src,
 * null-terminated, and should be freed by the caller.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_LEN  src is longer than STR_MAX_LEN.
 *      ERR_SYS      System error. errno(2) should be set.
 */
__attribute__((RO(1)))
error str_cp(const char *const src, char **dest);

/* Return true if s1 and s2 are equal and false otherwise. */
__attribute__((RO(1), RO(2)))
bool str_eq(const char *const s1, const char *const s2);

/*
 * Calculate the length of a string.
 *
 * Return code:
 *      OK           Success.
 *      ERR_STR_LEN  The string is longer than STR_MAX_LEN.
 */
__attribute__((RO(1)))
error str_len(const char *const s, size_t *len);

/*
 * Split s at the first max occurrences of any character in sep and
 * store the substrings in the array pointed to by subs and the
 * number of substrings in the variable pointed to by n.
 *
 * If n is NULL, the number of substrings is discarded.
 * 
 * Return code:
 *      OK           Success.
 *      ERR_STR_LEN  s is longer than STR_MAX_LEN.
 *      ERR_SYS      System error. errno(2) should be set.
 */
__attribute__((RO(1), RO(2)))
error str_split(const char *const s, const char *sep,
                const int max, char ***subs, int *n);

/*
 * Split s at the first n - 1 occurrences of any character in sep
 * and store the substrings in the given variadic arguments.
 *
 * If s is split into fewer than n substrings, the surplus variadic
 * arguments up to the nth argument are set to NULL. If less than
 * n arguments are given the behaviour is undefined.
 *
 * Otherwise the same as str_split.
 */
__attribute__((RO(1), RO(2)))
error str_vsplit(const char *const s, const char *sep, const int n, ...);


#endif /* !defined(SRC_STR_H) */
