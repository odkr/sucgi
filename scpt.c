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

#include "err.h"
#include "scpt.h"
#include "str.h"


enum error
scpt_get_handler(const struct scpt_ent handlerdb[], const char *const scpt,
                 const char **const handler)
{
	char path[STR_MAX];	/* Copy of scpt for basename(3). */
	const char *fname;	/* Filename portion of scpt. */
	const char *suffix;	/* Filename suffix. */

	assert(*scpt != '\0');

	try(str_cp(STR_MAX, scpt, path));
	fname = basename(path);
	suffix = strrchr(fname, '.');
	if (!suffix) return ERR_SCPT_NO_SFX;
	if (suffix == fname) return ERR_SCPT_ONLY_SFX;

	for (int i = 0; handlerdb[i].suffix; i++) {
		const struct scpt_ent ent = handlerdb[i];

		if (strcmp(suffix, ent.suffix) == 0) {
			if (!ent.handler || *(ent.handler) == '\0') {
				return ERR_SCPT_NO_HDL;
			}

			*handler = ent.handler;
			return OK;
		}
	}

	return ERR_SCPT_NO_HDL;
}
