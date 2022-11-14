/*
 * Environment handling for suCGI.
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
#include <ctype.h>
#include <errno.h>
#include <fnmatch.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "env.h"
#include "file.h"
#include "max.h"
#include "path.h"
#include "str.h"
#include "types.h"


/*
 * Constants
 */

/* Characters allowed in environment variable names. */
#define ENV_NAME_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789" \
                       "abcdefghijklmnopqrstuvwxyz"


/*
 * Functions
 */

enum retval
env_file_open(const char *const jail, const char *const var, const int flags,
              const char **const fname, int *const fd)
{
	const char *resolved;			/* Resolved filename. */
	const char *value;			/* Variable value. */

	assert(*jail != '\0');
	assert(*var != '\0');
	assert(strnlen(jail, MAX_FNAME) < MAX_FNAME);
	/* RATS: ignore; not a permission check. */
	assert(access(jail, F_OK) == 0);
	/* RATS: ignore; length of jail is checked above. */
	assert(strncmp(jail, realpath(jail, NULL), MAX_FNAME) == 0);

	*fname = NULL;

	errno = 0;
	/* RATS: ignore; value is sanitised below. */
	value = getenv(var);
	if (!value || *value == '\0') {
		if (errno == 0)
			return ERR_NIL;
		else
			return ERR_ENV;
	}

	if (strnlen(value, MAX_FNAME) >= MAX_FNAME)
		return ERR_LEN;
	*fname = value;

	errno = 0;
	/* RATS: ignore; length of value is checked above. */
	resolved = realpath(value, NULL);
	if (resolved == NULL)
		return ERR_RES;
	if (strnlen(resolved, MAX_FNAME) >= MAX_FNAME)
		return ERR_LEN;
	*fname = resolved;

	if (!path_is_subdir(resolved, jail))
		return ERR_ILL;

	return file_sec_open(resolved, flags, fd);
}

bool
env_is_name(const char *const name)
{
	return *name != '\0' && !isdigit(*name) &&
	       name[strspn(name, ENV_NAME_CHARS)] == '\0';
}

enum retval
env_restore(char *const *vars, const char *const *patterns,
            char name[MAX_VARNAME])
{
	assert(*vars);

	(void) memset(name, '\0', MAX_VARNAME);

	for (char *const *var = vars; *var; ++var) {
		char *value;

		if (str_split(MAX_VARNAME, *var, "=", name, &value) != OK)
			return ERR_CNV;
		if (!value || *name == '\0')
			return ERR_CNV;

		for (const char *const *pat = patterns; *pat; ++pat) {
			if (fnmatch(*pat, name, 0) == 0) {
				/*
				 * patterns may contain wildcards,
				 * so the name has to be checked.
				 */
				if (!env_is_name(name))
					return ERR_ILL;
				if (strnlen(value, MAX_FNAME) >= MAX_FNAME)
					return ERR_LEN;

				errno = 0;
				if (setenv(name, value, true) != 0)
					return ERR_ENV;

				break;
			}
		}
	}

	return OK;
}
