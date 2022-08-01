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
str_cp(const char *const src,
       /* Flawfinder: ignore (str_cp copies at most STR_MAX bytes). */
       char (*dest)[STR_MAX])
{
	/* 
	 * FIXME: Using stpncpy causes str_cp and env_sanitise to break in
	 * mysterious ways if suCGI is compiled with instrumentation (e.g.,
	 * -flto, -fstack-protector, or --coverage) using GCC. ¯\_(ツ)_/¯
	 */
	assert(src && dest);
	for (size_t n = 0; n < STR_MAX; n++) {
		(*dest)[n] = src[n];
		if ('\0' == src[n]) return OK;
	}
	return ERR_STR_MAX;
}

error
str_cpn(const size_t n, const char *const src,
        /* Flawfinder: ignore (str_cpn copies at most STR_MAX bytes). */
	char (*dest)[STR_MAX])
{
	assert(src && dest);
	if (n + 1 > STR_MAX) return ERR_STR_MAX;
	char *term = stpncpy(*dest, src, n);
	*term = '\0';
	return OK;
}

bool
str_eq(const char *const s1, const char *const s2)
{
	assert(s1 && s2);
	return (strcmp(s1, s2) == 0);
}

bool
str_fnmatchn(const char *const s, 
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
	size_t n = strnlen(s, STR_MAX - 1 + 2);
	if (n > STR_MAX - 1) return ERR_STR_MAX;

	if (len) *len = n;
	return OK;
}

error
str_split(const char *const s, const char *const sep,
          /* Flawfinder: ignore (str_split writes at most STR_MAX bytes). */
          char (*head)[STR_MAX],
	  char **tail)
{
	*tail = strpbrk(s, sep);
	if (*tail) {
		size_t len = (size_t) (*tail - s);
		reraise(str_cpn(len, s, head));
		(*tail)++;
	} else {
		reraise(str_cp(s, head));
	}

	return OK;
}
