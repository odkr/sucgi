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
#define _XOPEN_SOURCE 700

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <libgen.h>
#include <string.h>

#include "script.h"
#include "str.h"
#include "sysconf.h"
#include "types.h"


enum retval
script_get_int(const struct pair db[], const char *const script,
               char inter[PATH_MAX_LEN])
{
	/* RATS: ignore; writes to path are bounds-checked. */
	char path[PATH_MAX_LEN];	/* Copy of script for basename(3). */
	const char *fname;		/* Filename portion of scpt. */
	const char *suffix;		/* Filename suffix. */
	enum retval rc;			/* Return code. */

	assert(*script != '\0');

	/* basename may alter the path it is given. */
	rc = str_cp(PATH_MAX_LEN - 1U, script, path);
	if (rc != OK)
		return rc;
	fname = basename(path);

	suffix = strrchr(fname, '.');
	if (!suffix || suffix == fname)
		return ERR_ILL;

	for (int i = 0; db[i].key; i++) {
		const struct pair ent = db[i];

		if (strncmp(suffix, ent.key, PATH_MAX_LEN) == 0) {
			if (!ent.value || *ent.value == '\0')
				return FAIL;

			return str_cp(PATH_MAX_LEN - 1U, ent.value, inter);
		}
	}

	return FAIL;
}
