/* Utilities for the test suite. 
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
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../str.h"
#include "utils.h"

void
die (const char *const message, ...)
{
	va_list ap;

	va_start(ap, message);
	// flawfinder: ignore
	vfprintf(stderr, message, ap);
	va_end(ap);
	fputs("\n", stderr);

	exit(EXIT_FAILURE);
}

error
str_to_ulong (const char *const s, unsigned long *n)
{
	long long m = 0;
	char *end = NULL;

	assert(s);

	m = strtoll(s, &end, 10);
	if (*end != '\0') return ERR;
	if (m == 0 && errno != 0) return ERR_SYS;

	*n = (unsigned long) m;
	return OK;
}

error
str_words (const char *const restrict s, char ***subs)
{
	int n = 0;

	assert(s);
	assert(subs);

	reraise(str_split(s, " \f\n\r\t\v", STR_MAX_SUBS, subs, &n));
	if (n > STR_MAX_SUBS) return ERR;
	return OK;
}
