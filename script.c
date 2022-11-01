/*
 * Script handling for suCGI.
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

#include <assert.h>
#include <libgen.h>
#include <string.h>

#include "error.h"
#include "script.h"
#include "str.h"


enum error
script_get_inter(const struct pair db[], const char *const script,
                 char (*const inter)[MAX_STR])
{
	char path[MAX_STR];	/* Copy of script for basename(3). */
	const char *fname;	/* Filename portion of scpt. */
	const char *suffix;	/* Filename suffix. */

	assert(*script != '\0');

	/* basename may alter the path it is given. */
	try(str_cp(MAX_STR, script, path));
	fname = basename(path);
	suffix = strrchr(fname, '.');
	if (!suffix || suffix == fname)
		return ERR_ILL;

	for (int i = 0; db[i].key; i++) {
		const struct pair ent = db[i];

		if (strncmp(suffix, ent.key, MAX_STR) == 0) {
			if (!ent.value || !*(ent.value))
				return FAIL;

			try(str_cp(MAX_STR, ent.value, *inter));
			return OK;
		}
	}

	return FAIL;
}
