/*
 * String functions for suCGI.
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

#include <assert.h>
#include <fnmatch.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "err.h"
#include "str.h"


error
str_cp(const char *const src, char *dest)
{
	assert(src && dest);
	for (size_t i = 0; i <= STR_MAX_LEN; i++) {
		dest[i] = src[i];
		if ('\0' == src[i]) return OK;
	}

	return ERR_STR_LEN;
}

bool
str_eq(const char *const s1, const char *const s2)
{
	assert(s1 && s2);
	return (strcmp(s1, s2) == 0);
}

bool
str_matchn(const char *const s, 
           const char *const *const pats, const int flags)
{
	for (size_t i = 0; pats[i]; i++) {
		if (fnmatch(pats[i], s, flags) == 0) return true;
	}
	return false;
}

error
str_len(const char *const s, size_t *len)
{
	size_t n = strnlen(s, STR_MAX_LEN + 2);
	if (n > STR_MAX_LEN) return ERR_STR_LEN;

	if (len) *len = n;
	return OK;
}

/* FIXME: Untested. */
error
str_split(const char *const s, const char *const sep,
          char *head, char **tail)
{
	*tail = strpbrk(s, sep);
	if (*tail) {
		size_t len = *tail - s;
		reraise(str_cp(s, head));
		head[len] = '\0';
		(*tail)++;
	} else {
		reraise(str_cp(s, head));
	}

	return OK;
}