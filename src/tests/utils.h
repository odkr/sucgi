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

/* Abort the programme with an error message. */
// This is not a call to the access() function.
// flawfinder: ignore
__attribute__((noreturn, access(read_only, 1), format(printf, 1, 2)))
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
// This is not a call to the access() function.
// flawfinder: ignore
__attribute__((access(read_only, 1)))
error str_to_ulong (const char *const s, unsigned long *n);

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
// This is not a call to the access() function.
// flawfinder: ignore
__attribute__((access(read_only, 1)))
error str_words (const char *const restrict s, char ***subs);

#endif /* !defined(SRC_TESTS_UTILS_H) */
