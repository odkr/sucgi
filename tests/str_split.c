/*
 * Test str_split.
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
#include <stdlib.h>
#include <string.h>

#include "../error.h"
#include "../str.h"


/* Test case. */
struct args {
	const char *s;
	const char *sep;
	const char *head;
	const char *tail;
	enum error rc;
};

/* A string just within limits. */
char large[STR_MAX] = {0};	/* RATS: ignore */

/* A string that exceeds STR_MAX. */
char huge[STR_MAX + 1U] = {0};	/* RATS: ignore */

/* Tests. */
const struct args tests[] = {
	/* Overly long string. */
	{huge, ",", large, NULL, ERR_STR_LEN},

	/* Barely fitting string. */
	{large, ",", large, NULL, OK},

	/* Simple test. */
	{"a,b", ",", "a", "b", OK},

	/* Empty strings. */
	{",b", ",", "", "b", OK},
	{"a,", ",", "a", "", OK},
	{"a,b", "", "a,b", NULL, OK},

	/* Environment-like tests. */
	{"foo=bar", "=", "foo", "bar", OK},
	{"foo=", "=", "foo", "", OK},
	{"foo==bar", "=", "foo", "=bar", OK},
	{"=bar", "=", "", "bar", OK},
	{"foo", "=", "foo", NULL, OK},

	/* Terminator. */
	{NULL, NULL, NULL, NULL, OK}
};



int
main(void)
{
	(void) memset(huge, 'x', STR_MAX);
	(void) memset(large, 'x', STR_MAX - 1U);

	for (int i = 0; tests[i].s; i++) {
		const struct args t = tests[i];
		char head[STR_MAX];	/* RATS: ignore */
		char *tail;
		enum error rc;

		*head = '\0';

		warnx("checking (%s, %s, -> %s, -> %s) -> %u ...",
		      t.s, t.sep, t.head, t.tail, t.rc);

		rc = str_split(t.s, t.sep, &head, &tail);

		if (rc != t.rc) {
			errx(EXIT_FAILURE, "unexpected return code %u", rc);
		}
		if (!(t.head == head || strcmp(t.head, head) == 0)) {
			errx(EXIT_FAILURE, "unexpected head '%s'", head);
		}
		if (!(t.tail == tail || strcmp(t.tail, tail) == 0)) {
			errx(EXIT_FAILURE, "unexpected tail '%s'", tail);
		}
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
