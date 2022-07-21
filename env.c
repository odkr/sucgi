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

#include <errno.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "err.h"
#include "file.h"
#include "path.h"
#include "str.h"


/*
 * Globals
 */

/* The environment. */
extern char **environ;

#include <stdio.h>
/*
 * Functions
 */

enum code
env_clear (char ***vars)
{
	char **env = environ;	/* Backup of the environment. */
	size_t n = 0;

	/* FIXME: Test if this works with the glibc. */
	environ = NULL;

	environ = calloc(1, sizeof(char *));
	if (!environ) return ERR_SYS;
	*environ = NULL;
	if (!vars) return OK;

	*vars = calloc(ENV_MAX, sizeof(char *));
	if (!vars) return ERR_SYS;
	for (n = 0; env[n]; n++) {
		/* FIXME: Neither tested in env_clear nor main.sh. */
		if (n == ENV_MAX) return ERR_ENV_MAX;
		(*vars)[n] = env[n];
	}
	return OK;
}

enum code
env_get_fname (const char *const name, char **fname, struct stat **fstatus)
{
	char *value = NULL;

	// The value is checked below, extensively.
	// flawfinder: ignore
	value = getenv(name);
	if (!value) return ERR_VAR_UNDEF;
	if (*value == '\0') return ERR_VAR_EMPTY;
	reraise(path_check_len(value));
	reraise(file_safe_stat(value, fstatus));

	*fname = value;
	return OK;
}

enum code
env_restore (const char *const *vars,
             char *const *const keep,
	     char *const *const toss)
{
	for (; *vars; vars++) {
		char *name, *value;	/* Variable name and value. */

		reraise(str_vsplit(*vars, "=", 2, &name, &value));
		if (name[0] == '\0' || !value) return ERR_VAR_INVALID;

		for (char *const *kp = keep; *kp; kp++) {
			if (fnmatch(*kp, name, FNM_PERIOD) != 0)
				continue;
			for (char *const *tp = toss; *tp; tp++) {
				if (fnmatch(*tp, name, FNM_PERIOD) == 0)
					goto next;
			}

			if (setenv(name, value, true) != 0) return ERR_SYS;
			break;
		}
		next:;
	}

	return OK;
}
