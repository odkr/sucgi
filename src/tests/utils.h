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
 * Globals
 */

/* The environment. */
extern char **environ;


/*
 * Functions
 */

/* Abort the programme with an error message. */
/* flawfinder: ignore (not a call to printf(3)). */
__attribute__((noreturn, READ_ONLY(1), format(printf, 1, 2)))
void die(const char *const message, ...);

/* 
 * Create a new environment with space for at least n elements
 * and initialise with the given variadic arguments,
 * which must be strings.
 */
__attribute__((READ_ONLY(1)))
error env_init(const size_t n, ...);

/*
 * Covert str to an unsigned long and
 * store its value in the variable pointed to by n.
 *
 * Return code:
 *      OK            Success.
 *      ERR           Trailing nun-numeric characters.
 *      ERR_SYS       System error. errno(2) should be set.
 */
__attribute__((READ_ONLY(1)))
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
__attribute__((READ_ONLY(1), READ_ONLY(2)))
error str_splitn(const char *const s, const char *sep,
                 const size_t max, char ***subs, size_t *n);

/*
 * Split s at the first n - 1 occurrences of any character in sep and store
 * the substrings in the given variadic arguments. The variadic arguments must
 * be pointers to strings and large enough to store a string that is
 * STR_MAX_LEN characters long, excluding the terminating null byte.
 *
 * If s is split into fewer than n substrings, the surplus variadic
 * arguments are left alone. If fewer than n arguments are given the
 * behaviour is undefined.
 *
 * Otherwise the same as str_splitn.
 */
__attribute__((READ_ONLY(1), READ_ONLY(2)))
error str_vsplit(const char *const s, const char *sep, const size_t n, ...);

#endif /* !defined(SRC_TESTS_UTILS) */
