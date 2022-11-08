/*
 * Test env_clear.
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

#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../max.h"
#include "../str.h"
#include "testdefs.h"


/*
 * Main
 */

int
main (void) {
	/* RATS: ignore */
	const char *env[MAX_NVARS];	/* Backup of environ(2). */
	const char **var;		/* An environment variable. */
	int nvars;			/* The number of variables. */

	nvars = 0;

	/* Start with a clean environmenet, hopefully. */
	warnx("clearing the environment ...");

	if (env_clear(NULL) != OK)
		errx(T_FAIL, "failed to clear the environment");

	/* Is the environment cleared? */
	warnx("checking cleanup ...");

	errno = 0;
	if (setenv("foo", "bar", true) != 0)
		err(T_FAIL, "setenv foo=bar");
	if (env_clear(env) != OK)
		errx(T_FAIL, "env_clear failed");
	/* RATS: ignore */
	if (getenv("foo"))
		errx(T_FAIL, "$foo: present after clean-up");

	while (environ[nvars])
		nvars++;
	
	if (nvars > 0)
		errx(T_FAIL, "environment not empty after clean-up");

	/* Is the environment backed-up? */
	warnx("checking backup ...");

	nvars = 0;
	for (var = env; *var; var++) {
		char name[MAX_FNAME];	/* RATS: ignore */
		char *value;

		if (str_split(MAX_FNAME, *var, "=", name, &value) != OK)
			errx(T_ERR, "str_split: did not return OK");
		
		if (strcmp(name, "foo") != 0)
			continue;
		if (strcmp(value, "bar") != 0)
			continue;

		nvars++;
	}

	if (nvars < 1)
		errx(T_FAIL, "failed to store all variables.");
	if (nvars > 1)
		errx(T_FAIL, "stored too many variables.");

	/* Does env_clear error out if there are too many variables? */
	warnx("checking errors ...");

	if (env_clear(NULL) != OK)
		errx(T_ERR, "failed to clear the environment");

	for (size_t i = 0; i <= MAX_NVARS; i++) {
		char name[MAX_FNAME];	/* RATS: ignore */

		if (snprintf(name, MAX_FNAME - 1U, "foo%zu", i) < 1)
			errx(T_ERR, "snprintf: returned < 1 bytes");

		errno = 0;
		if (setenv(name, "foo", true) != 0)
			err(T_ERR, "setenv %s=%s", name, "foo");
	}

	if (env_clear(env) != ERR_LEN)
		errx(T_FAIL, "accepted > MAX_NVARS variables.");

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
