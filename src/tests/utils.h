/* Headers for utils.c
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

#if !defined(SRC_TESTS_UTILS_H)
#define SRC_TESTS_UTILS_H

#include "../attr.h"
#include "../err.h"


/*
 * Constants
 */

/* Maximum number of substrings to split up a string into. */
#define STR_MAX_SUBS 512


/*
 * Functions
 */

/* Abort the programme with an error message. */
/* flawfinder: ignore (not a call to printf(3)). */
__attribute__((noreturn, RO(1), format(printf, 1, 2)))
void die(const char *const message, ...);

/*
 * Covert str to an unsigned long and
 * store its value in the variable pointed to by n.
 *
 * Return code:
 *      OK            Success.
 *      ERR           Trailing nun-numeric characters.
 *      ERR_SYS       System error. errno(2) should be set.
 */
__attribute__((RO(1)))
error str_to_ulong (const char *const s, unsigned long *n);

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
error str_nsplit(const char *const s, const char *sep,
                 const int max, char ***subs, int *n);

/*
 * Split s at the first n - 1 occurrences of any character in sep and store
 * the substrings in the given variadic arguments. The variadic arguments must
 * be large enough to store a string that is STR_MAX_LEN characters long,
 * excluding the terminating null byte.
 *
 * If s is split into fewer than n substrings, the surplus variadic
 * arguments up to the nth argument are left alone. If less than
 * n arguments are given the behaviour is undefined.
 *
 * Otherwise the same as str_split.
 */
__attribute__((RO(1), RO(2)))
error str_vsplit(const char *const s, const char *sep, const int n, ...);

/*
 * Split s at each occurrence of a whitespace character and
 * store the substrings in the array pointed to by subs and the
 * number of substrings in the variable pointed to by n.
 *
 * Return code:
 *      OK            Success.
 *      ERR_STR_LEN   s is longer than STR_MAX_LEN.
 *      ERR           s consists of more than STR_MAX_SUBS words.
 *      ERR_SYS       System error. errno(2) should be set.
 */
__attribute__((RO(1)))
error str_words (const char *const restrict s, char ***subs);

#endif /* !defined(SRC_TESTS_UTILS_H) */
