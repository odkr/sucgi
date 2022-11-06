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
#define _XOPEN_SOURCE 700

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <errno.h>
#include <fnmatch.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "str.h"
#include "types.h"

enum retval
str_cp(const size_t n, const char *const src, char dest[n + 1U])
{
	char *end;	/* Position of last byte of src. */
	size_t len;	/* Bytes copied. */

	end = stpncpy(dest, src, n);
	len = (size_t) (end - dest);
	dest[len] = '\0';

	if (src[len] != '\0')
		return ERR_LEN;

	return OK;
}

enum retval
str_dup(const size_t n, const char *const src, char **const dest)
{
	enum retval rc;		/* Return code. */

	assert(n > 0U);

	errno = 0;
	*dest = (char *) malloc(n * sizeof(**dest));
	if (!*dest)
		return ERR_MEM;

	rc = str_cp(n - 1U, src, *dest);
	if (rc == OK)
		return rc;

	free(*dest);
	return ERR_LEN;
}


enum retval
str_split(size_t max, const char *const s, const char *const sep,
          char head[max], char **const tail)
{
	*tail = strpbrk(s, sep);

	if (*tail) {
		size_t len;	/* Length of head. */

		len = (size_t) (*tail - s);
		if (len >= max)
			return ERR_LEN;

		(void) str_cp(len, s, head);
		(*tail)++;

		return OK;
	}

	return str_cp(max - 1U, s, head);
}
