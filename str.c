/*
 * String handling for suCGI.
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

#define _ISOC99_SOURCE
#define _POSIX_C_SOURCE 200809L

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif /* !defined(_FORTIFY_SOURCE) */

#include <fnmatch.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "err.h"
#include "str.h"

enum error
str_cp(const size_t n, const char *const src, char dest[n + 1])
{
	char *end;	/* Position of last byte of src. */
	size_t len;	/* Length of src. */

	end = stpncpy(dest, src, n);
	len = (size_t) (end - dest);
	dest[len] = '\0';

	if (src[len] == '\0') return SC_OK;
	return SC_ERR_STR_LEN;
}

bool
str_matchv(const char *const s, const char *const patv[], const int flags)
{
	for (int i = 0; patv[i]; i++) {
		if (fnmatch(patv[i], s, flags) == 0) return true;
	}
	return false;
}

enum error
str_split(const char *const s, const char *const sep,
          /* RATS: ignore; str_split respects STR_MAX. */
          char (*const head)[STR_MAX], char **const tail)
{
	*tail = strpbrk(s, sep);
	if (*tail) {
		size_t len = (size_t) (*tail - s);
		if (len >= STR_MAX) return SC_ERR_STR_LEN;
		(void) (str_cp(len, s, *head));
		(*tail)++;
	} else {
		try(str_cp(STR_MAX - 1, s, *head));
	}

	return SC_OK;
}
