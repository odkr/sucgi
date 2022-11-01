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
#include "../str.h"


/*
 * Constants
 */

/* Shorthand for an empty list. */
#define EMPTY {NULL}

/* Shorthad for a list of patterns that matches any variable. */
#define ANY {"*", NULL}

/* Exit status for failures. */
#define T_FAIL 2


/*
 * Data types
 */

/* Test case. */
struct args {
	char *env[MAX_ENV];			/* RATS: ignore */
	const char *patterns[MAX_ENV];		/* RATS: ignore */
	const char *clean[MAX_ENV];		/* RATS: ignore */
	const enum error rc;
};


/*
 * Globals
 */

/* String that is as long as MAX_STR. */
char long_var[MAX_STR] = {0}; 		/*RATS: ignore */

/* String that is longer than MAX_STR. */
char huge_var[MAX_STR + 1U] = {0};	/* RATS: ignore */

/* Tests. */
struct args tests[] = {
	/* Errors. */
	{{huge_var, NULL}, ANY, EMPTY, ERR_LEN},
	{{"", NULL}, ANY, EMPTY, ERR_ILL},
	{{"foo", NULL}, ANY, EMPTY, ERR_ILL},
	{{"=foo", NULL}, ANY, EMPTY, ERR_ILL},

	/* Other illegal names. */
	{{" foo=foo", NULL}, ANY, EMPTY, ERR_ILL},
	{{"1foo=foo", NULL}, ANY, EMPTY, ERR_ILL},
	{{"*=foo", NULL}, ANY, EMPTY, ERR_ILL},
	{{"FOO =foo", NULL}, ANY, EMPTY, ERR_ILL},
	{{"$(foo)=foo", NULL}, ANY, EMPTY, ERR_ILL},
	{{"`foo`=foo", NULL}, ANY, EMPTY, ERR_ILL},

	/* Simple tests. */
	{{"foo=bar", NULL}, {"foo", NULL}, {"foo=bar", NULL}, OK},
	{{"foo=bar", NULL}, EMPTY, EMPTY, OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"*", NULL},
	 {"foo=foo", "bar=bar", "baz=baz", NULL}, OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"f*", "b*", NULL},
	 {"foo=foo", "bar=bar", "baz=baz", NULL}, OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"f*", NULL},
	 {"foo=foo", "bar", "baz", NULL}, OK},
	{{long_var, NULL}, {"foo", NULL}, {long_var, NULL}, OK},

	/* Odd but legal values. */
	{{"empty=", "assign==", "space= ", "tab=\t", "lf=\n", NULL}, ANY,
	 {"empty=", "assign==", "space= ", "tab=\t", "lf=\n", NULL}, OK},

	/* Terminator. */
	{EMPTY, EMPTY, EMPTY, OK}
};


/*
 * Functions
 */

static enum error
env_init(/* RATS: ignore */
	 char *const vars[MAX_ENV])
{
	size_t n = 0;

	while (vars[n]) {
		n++;
	}

	environ = calloc(n + 1U, sizeof(char *));
	if (!environ) {
		return ERR_CALLOC;
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
	(void) memset((void *) long_var, 'x', MAX_STR - 1U);
	(void) str_cp(4, "foo=", long_var);
	(void) memset((void *) huge_var, 'x', MAX_STR);

	for (int i = 0; tests[i].env[0]; i++) {
		const struct args t = tests[i];
		const char *vars[MAX_ENV];	/* RATS: ignore */
		enum error rc;

		warnx("performing test # %d ...", i + 1);

		if (env_init(t.env) != OK) {
			errx(T_FAIL, "env_init did not return OK");
		}
		if (env_clear(&vars) != OK) {
			errx(T_FAIL, "env_clear did not return OK");
		}

		rc = env_restore(vars, t.patterns);

		if (rc != t.rc) {
			errx(T_FAIL, "returned %u, not %u",
			     rc, t.rc);
		}

		for (int j = 0; t.clean[j]; j++) {
			const char *var = t.clean[j];
			const char *val;
			char name[MAX_STR];	/* RATS: ignore */
			char *exp;

			if (str_split(var, "=", &name, &exp) != OK) {
				errx(T_FAIL,
				     "str_split %s: did not return OK", var);
			}

                        /* RATS: ignore */
			val = getenv(name);
			
			if (val && !exp) {
				errx(T_FAIL, "$%s is set", name);
			}
			if (!val && exp) {
				errx(T_FAIL, "$%s is unset", name);
			}
			if (val && exp && strcmp(val, exp) != 0) {
				errx(T_FAIL, "$%s is '%s'", name, val);
			}
		}
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
