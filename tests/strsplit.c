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
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "../str.h"
#include "../sysconf.h"
#include "testdefs.h"
#include "testdefs.h"


/* Test case. */
struct args {
	const char *s;
	const char *sep;
	const char *head;
	const char *tail;
	enum retval rc;
};

/* A string just within limits. */
static char long_str[PATH_MAX_LEN] = {0};

/* A string w/o a delimiter that exceeds PATH_MAX_LEN. */
static char huge_str[PATH_MAX_LEN + 1U] = {0};

/* A pair w/ a head that exceeds PATH_MAX_LEN. */
static char huge_head[PATH_MAX_LEN + 32U] = {0};

/* Tests. */
static const struct args tests[] = {
	/* Overly long string. */
	{huge_str, ",", long_str, NULL, ERR_LEN},
	
	/* Overly long head. */
	{huge_head, ",", NULL, ",foo", ERR_LEN},

	/* Barely fitting string. */
	{long_str, ",", long_str, NULL, OK},

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
	(void) memset(long_str, 'x', sizeof(long_str) - 1);
	(void) memset(huge_str, 'x', sizeof(huge_str) - 1);
	(void) memset(huge_head, 'x', sizeof(huge_head) - 1);
	/* RATS: ignore; there is enough space for NUL-termination. */
	(void) strncpy(&huge_head[PATH_MAX_LEN], ",foo", 5);

	for (int i = 0; tests[i].s; i++) {
		const struct args t = tests[i];
		char head[PATH_MAX_LEN];	/* RATS: ignore */
		char *tail;
		enum retval rc;

		*head = '\0';

		warnx("checking (%s, %s, -> %s, -> %s) -> %u ...",
		      t.s, t.sep, t.head, t.tail, t.rc);

		rc = str_split(PATH_MAX_LEN, t.s, t.sep, head, &tail);

		if (rc != t.rc)
			errx(T_FAIL, "returned %u", rc);
		if (!(t.head == NULL || strcmp(t.head, head) == 0))
			errx(T_FAIL, "got head '%s'", head);
		if (!(t.tail == tail || strcmp(t.tail, tail) == 0))
			errx(T_FAIL, "got tail '%s'", tail);
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
