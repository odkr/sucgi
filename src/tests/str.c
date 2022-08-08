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
#include "str.h"


error
str_to_id (const char *const s, id_t *id)
{
	unsigned long n = 0;
	char *end = NULL;

	assert(s);
	errno = 0;
	n = strtoul(s, &end, 10);
	if (errno != 0) return ERR_SYS;
	if (*end != '\0') return ERR;
#if defined(UID_MAX)
	if (n > UID_MAX) return ERR_CONV;
#else
	if (n > (unsigned long) UINT_MAX) return ERR_CONV;
#endif /* defined(UID_MAX) */

	*id = (id_t) n;
	return OK;
}

error
str_splitn(const char *const s, const char *sep, const size_t max,
           char **subs, size_t *n)
{
	error rc = ERR_SYS;		/* Return code. */
	char *pivot = NULL;		/* Current start of string. */
	size_t i = 0;			/* Iterator. */

	pivot = strndup(s, STR_MAX);
	if (!pivot) return ERR_SYS;

	assert(subs);
	for (; (i <= max) && (pivot != NULL); i++) {
		/* Flawfinder: ignore; str_split respects STR_MAX. */
		char sub[STR_MAX] = {0};
		rc = str_split(pivot, sep, &sub, &pivot);
		if (rc != OK) goto err;
		subs[i] = strndup(sub, STR_MAX);
		if (!subs[i]) goto err;
	}
	i++;
	subs[i] = NULL;

	if (n != NULL) *n = i;
	return OK;

	err:
		for (size_t j = 0U; j < i; j++) free(subs[j]);
		free(pivot);
		return rc;
}
