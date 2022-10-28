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
#include "scpt.h"
#include "str.h"


enum error
scpt_get_handler(const struct scpt_ent handlerdb[], const char *const scpt,
                 char (*const handler)[MAX_STR])
{
	/* RATS: ignore; path is bounds-checked. */
	char path[MAX_STR];	/* Copy of scpt for basename(3). */
	const char *fname;	/* Filename portion of scpt. */
	const char *suffix;	/* Filename suffix. */

	assert(*scpt != '\0');

	/* basename may alter the path it is given. */
	try(str_cp(MAX_STR, scpt, path));
	/* RATS: ignore; no matching on path is performed. */
	fname = basename(path);
	suffix = strrchr(fname, '.');
	if (!suffix || suffix == fname) {
		return ERR_ILL;
	}

	for (int i = 0; handlerdb[i].suffix; i++) {
		const struct scpt_ent ent = handlerdb[i];

		if (strncmp(suffix, ent.suffix, MAX_STR) == 0) {
			if (!ent.handler || *(ent.handler) == '\0') {
				return FAIL;
			}

			try(str_cp(MAX_STR, ent.handler, *handler));
			return OK;
		}
	}

	return FAIL;
}
