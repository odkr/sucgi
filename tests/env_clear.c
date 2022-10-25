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
#include "../str.h"


int
main (void) {
	/* RATS: ignore */
	const char *env[MAX_ENV];	/* Backup of environ(2). */
	const char **var;		/* An environment variable. */
	int nvars;			/* The number of variables. */

	/* Start with a clean environmenet, hopefully. */
	warnx("clearing the environment ...");

	if (env_clear(NULL) != OK) {
		errx(EXIT_FAILURE, "failed to clear the environment");
	}

	/* Is the environment cleared? */
	warnx("checking cleanup ...");

	errno = 0;
	if (setenv("foo", "bar", true) != 0) {
		err(EXIT_FAILURE, "setenv foo=bar");
	}
	if (env_clear(&env) != OK) {
		errx(EXIT_FAILURE, "env_clear: did not return OK");
	}
        
	/* RATS: ignore */
	if (getenv("foo")) {
		errx(EXIT_FAILURE, "$foo: present after clean-up");
	}
	
	nvars = 0;
	for (var = (const char**) environ; *var; var++) {
		nvars++;
	}
	
	if (nvars > 0) {
		errx(EXIT_FAILURE, "environment not empty after clean-up");
	}

	/* Is the environment backed-up? */
	warnx("checking backup ...");

	nvars = 0;
	for (var = env; *var; var++) {
		char name[MAX_STR];	/* RATS: ignore */
		char *value;

		if (str_split(*var, "=", &name, &value) != OK) {
			errx(EXIT_FAILURE, "str_split: did not return OK");
		}
		
		if (strcmp(name, "foo") != 0) {
			continue;
		}

		if (strcmp(value, "bar") != 0) {
			continue;
		}
		
		nvars++;
	}

	if (nvars < 1) {
		errx(EXIT_FAILURE, "failed to store all variables.");
	}
	if (nvars > 1) {
		errx(EXIT_FAILURE, "stored too many variables.");
	}

	/* Does env_clear error out if there are too many variables? */
	warnx("checking errors ...");

	if (env_clear(NULL) != OK) {
		errx(EXIT_FAILURE, "failed to clear the environment");
	}

	for (int i = 0; i <= MAX_ENV; i++) {
		char name[MAX_STR];	/* RATS: ignore */

		/* RATS: ignore */
		if (snprintf(name, MAX_STR - 1U, "foo%d", i) < 1) {
			errx(EXIT_FAILURE, "snprintf: returned < 1 bytes");
		}

		errno = 0;
		if (setenv(name, "foo", true) != 0) {
			err(EXIT_FAILURE, "setenv %s=%s", name, "foo");
		}
	}

	if (env_clear(&env) != ERR_LEN) {
		errx(EXIT_FAILURE, "accepted > MAX_ENV variables.");
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
