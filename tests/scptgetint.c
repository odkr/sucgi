/*
 * Test script_get_handler.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../max.h"
#include "../script.h"
#include "../str.h"
#include "testdefs.h"


/* Test case. */
struct args {
	const char *script;
	const char *handler;
	const enum retval rc;
};

/* A string just within limits. */
static char long_str[MAX_FNAME] = {0};

/* A string that exceeds MAX_FNAME. */
static char huge_str[MAX_FNAME + 1U] = {0};

/* Tests. */
static const struct args tests[] = {
	/* Simple errors. */
	{"file", NULL, ERR_ILL},
	{".", NULL, ERR_ILL},
	{".sh", NULL, ERR_ILL},
	{".py", NULL, ERR_ILL},
	{"file.null", NULL, FAIL},
	{"file.empty", NULL, FAIL},
	{"file.py", NULL, FAIL},
	{"file.post", NULL, FAIL},
	{huge_str, NULL, ERR_LEN},

	/* Empty string shenanigans. */
	{" ", NULL, ERR_ILL},
	{". ", NULL, ERR_ILL},
	{".sh ", NULL, ERR_ILL},
	{".py ",NULL, ERR_ILL},
	{" .null", NULL, FAIL},
	{" .empty", NULL, FAIL},
	{" .py", NULL, FAIL},
	{" .post", NULL, FAIL},
	{" . ", NULL, FAIL},

	/* Simple test. */
	{"file.sh", "sh", OK},
	{"file.", "dot", OK},
	{long_str, "sh", OK},

	/* Terminator. */
	{NULL, NULL, OK}
};

/* Script handler database for testing. */
static const struct pair db[] = {
	{"", "unreachable"},
	{".", "dot"},
	{".sh", "sh"},
	{".null", NULL},
	{".empty", ""},
	{".pre", "pre"},
	{NULL, NULL},
	{".post", "post"},
};


int
main (void)
{
	(void) memset(huge_str, 'x', sizeof(huge_str) - 1);
	(void) memset(long_str, 'x', sizeof(long_str) - 1);
	/* RATS: ignore; there is enough space for NUL-termination. */
	(void) strncpy(&long_str[sizeof(long_str) - 4], ".sh", 4);

	for (int i = 0; tests[i].script; i++) {
		const struct args t = tests[i];
		char handler[MAX_FNAME];	/* RATS: ignore */
		char script[MAX_FNAME];		/* RATS: ignore */
		enum retval rc;	

		*script = '\0';
		(void) memset(handler, 0, MAX_FNAME);

		warnx("checking (db, %s, -> %s) -> %u ...",
		      t.script, t.handler, t.rc);

		rc = script_get_handler(db, t.script, handler);

		if (t.rc != rc)
			errx(T_FAIL, "returned %u", rc);
		if (t.handler && strcmp(t.handler, handler) != 0)
			errx(T_FAIL, "got handler %s", handler);
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
