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
#include "path.h"
#include "str.h"
#include "sysconf.h"
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
env_clear(size_t max, const char *vars[max])
{
	static char *var;	/* First environment variable. */
	char **env;		/* Copy of the environ pointer. */

	env = environ;
	var = NULL;
	environ = &var;

	if (!vars)
		return OK;

	for (size_t n = 0; n < max; n++) {
		vars[n] = env[n];
		if (!env[n])
			return OK;
	}

	return ERR_LEN;
}

enum retval
env_fopen(const char *const jail, const char *const var,
	  const int flags, const char **const fname, int *const fd)
{
	const char *value;	/* Unchecked variable value. */
	const char *resolved;	/* Resolved filename. */
	char *unresolved;	/* Unresolved filename. */
	enum retval rc;		/* Return code. */

	assert(*jail != '\0');
	assert(*var != '\0');
	assert(strnlen(jail, PATH_MAX_LEN) < PATH_MAX_LEN);
	/* RATS: ignore; not a permission check. */
	assert(access(jail, F_OK) == 0);
	/* RATS: ignore; length of jail is checked above. */
	assert(strncmp(jail, realpath(jail, NULL), PATH_MAX_LEN) == 0);
	assert(fname);
	assert(fd);

	errno = 0;
	*fname = NULL;

	errno = 0;
	/* RATS: ignore; value is sanitised below. */
	value = getenv(var);
	if (!value || !*value) {
		if (errno == 0)
			return ERR_NIL;
		else
			return ERR_ENV;
	}

	errno = 0;
	unresolved = (char *) calloc(PATH_MAX_LEN, sizeof(*unresolved));
	if (!unresolved)
		return ERR_MEM;

	rc = str_cp(PATH_MAX_LEN - 1U, value, unresolved);
	if (rc != OK) {
		free(unresolved);
		return rc;
	}
	*fname = unresolved;
	
	errno = 0;
	/* RATS: ignore; length of fname is checked above. */
	resolved = realpath(*fname, NULL);
	if (!resolved)
		return ERR_RES;

	*fname = resolved;
	free(unresolved);

	if (strnlen(*fname, PATH_MAX_LEN) >= PATH_MAX_LEN)
		return ERR_LEN;
	if (!path_is_subdir(*fname, jail))
		return ERR_ILL;
	
	return file_sopen(*fname, flags, fd);
}

bool
env_is_name(const char *const name)
{
	return *name && !isdigit(*name) &&
	       name[strspn(name, ENV_NAME_CHARS)] == '\0';
}

enum retval
env_restore(const char **vars, const char *const *patterns,
            char name[ENV_MAX_NAME])
{
	assert(*vars);

	for (const char **var = vars; *var; var++) {
		char *value;

		if (str_split(ENV_MAX_NAME, *var, "=", name, &value) != OK)
			return ERR_CNV;
		if (!value || *name == '\0')
			return ERR_CNV;

		for (const char *const *pat = patterns; *pat; pat++) {
			if (fnmatch(*pat, name, 0) == 0) {
				/*
				 * patterns may contain wildcards,
				 * so the name has to be checked.
				 */
				if (!env_is_name(name))
					return ERR_ILL;
				if (strnlen(value, PATH_MAX_LEN) >= PATH_MAX_LEN)
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
