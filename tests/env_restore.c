/*
 * Test env_restore.
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
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../error.h"
#include "../str.h"


/*
 * Constants
 */

/* Shorthand for an empty list. */
#define EMPTY {NULL}


/*
 * Data types
 */

/* Test case. */
struct args {
	char *env[ENV_MAX];			/* RATS: ignore */
	const char *patterns[ENV_MAX];		/* RATS: ignore */
	const char *clean[ENV_MAX];		/* RATS: ignore */
	const enum error rc;
};


/*
 * Globals
 */

/* String that is longer than STR_MAX. */
char huge[STR_MAX + 1U] = {0};	/* RATS: ignore */

/* Tests. */
struct args tests[] = {
	/* Errors. */
	{{huge, NULL}, EMPTY, EMPTY, ERR_ENV_LEN},
	{{"", NULL}, EMPTY, EMPTY, ERR_ENV_MAL},
	{{"foo", NULL}, EMPTY, EMPTY, ERR_ENV_MAL},
	{{"=foo", NULL}, EMPTY, EMPTY, ERR_ENV_MAL},

	/* Other illegal names. */
	{{" foo=foo", NULL}, EMPTY, EMPTY, ERR_ENV_MAL},
	{{"1foo=foo", NULL}, EMPTY, EMPTY, ERR_ENV_MAL},
	{{"*=foo", NULL}, EMPTY, EMPTY, ERR_ENV_MAL},
	{{"FOO =foo", NULL}, EMPTY, EMPTY, ERR_ENV_MAL},
	{{"$(foo)=foo", NULL}, EMPTY, EMPTY, ERR_ENV_MAL},
	{{"`foo`=foo", NULL}, EMPTY, EMPTY, ERR_ENV_MAL},

	/* Simple tests. */
	{{"foo=bar", NULL}, {"foo", NULL}, {"foo=bar", NULL}, OK},
	{{"foo=bar", NULL}, EMPTY, EMPTY, OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"*", NULL},
	 {"foo=foo", "bar=bar", "baz=baz", NULL}, OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"f*", "b*", NULL},
	 {"foo=foo", "bar=bar", "baz=baz", NULL}, OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"f*", NULL},
	 {"foo=foo", "bar", "baz", NULL}, OK},
	
	/* Odd but legal values. */
	{{"empty=", "assign==", "space= ", "tab=\t", "lf=\n", NULL},
	 {"*", NULL},
	 {"empty=", "assign==", "space= ", "tab=\t", "lf=\n", NULL}, OK},

	/* Terminator. */
	{EMPTY, EMPTY, EMPTY, OK}
};


/*
 * Functions
 */

static enum error
env_init(/* RATS: ignore */
	 char *const vars[ENV_MAX])
{
	size_t n = 0;

	while (vars[n]) {
		n++;
	}

	environ = calloc(n + 1U, sizeof(char *));
	if (!environ) {
		return ERR_SYS;
	}
	
	for (size_t i = 0; i < n; i++) {
		environ[i] = vars[i];
	}

	return OK;
}


/*
 * Main
 */

int
main (void) {
	(void) memset((void *) huge, 'x', STR_MAX);
	huge[STR_MAX] = '0';

	for (int i = 0; tests[i].env[0]; i++) {
		const struct args t = tests[i];
		const char *vars[ENV_MAX];	/* RATS: ignore */
		enum error rc;

		warnx("performing test # %d ...", i + 1);

		if (env_init(t.env) != OK) {
			errx(EXIT_FAILURE, "env_init did not return OK");
		}
		if (env_clear(&vars) != OK) {
			errx(EXIT_FAILURE, "env_clear did not return OK");
		}

		rc = env_restore(vars, t.patterns);

		if (rc != t.rc) {
			errx(EXIT_FAILURE, "env_restore returned %u, not %u",
			     rc, t.rc);
		}

		for (int j = 0; t.clean[j]; j++) {
			const char *var = t.clean[j];
			const char *val;
			char name[STR_MAX];	/* RATS: ignore */
			char *exp;

			if (str_split(var, "=", &name, &exp) != OK) {
				errx(EXIT_FAILURE,
				     "str_split %s: did not return OK", var);
			}

                        /* RATS: ignore */
			val = getenv(name);
			
			if (val && !exp) {
				errx(EXIT_FAILURE, "$%s is set", name);
			}
			if (!val && exp) {
				errx(EXIT_FAILURE, "$%s is unset", name);
			}
			if (val && exp && strcmp(val, exp) != 0) {
				errx(EXIT_FAILURE, "$%s is '%s'", name, val);
			}
		}
	}

	warnx("success");
	return EXIT_SUCCESS;
}
