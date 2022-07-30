/* String functions for suCGI's test suite.
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


error
str_to_ulong (const char *const s, unsigned long *n)
{
	long long m = 0;
	char *end = NULL;

	assert(s);
	m = strtoll(s, &end, 10);
	if (*end != '\0') return ERR;
	if (0 == m && 0 != errno) return ERR_SYS;

	*n = (unsigned long) m;
	return OK;
}

error
str_splitn(const char *const s, const char *sep, const size_t max,
           str4096 *subs[], size_t *n)
{
	char *pivot = (char *) s;	/* Current start of string. */
	size_t i = 0;			/* Iterator. */
	error rc = ERR_SYS;		/* Return code. */

	assert(subs);
	for (; i <= max && pivot; i++) {
		subs[i] = calloc(STR_MAX_LEN + 1, sizeof(char));
		if (!subs[i]) goto err;
		rc = str_split(pivot, sep, subs[i], &pivot);
		if (rc != OK) goto err;
	}
	subs[++i] = NULL;

	if (n) *n = i;
	return OK;

	err:
		for (size_t j = 0; j < i; j++) free(subs[j]);
		return rc;
}
