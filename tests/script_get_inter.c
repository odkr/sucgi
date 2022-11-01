/*
 * Test script_get_inter.
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

#include "../script.h"
#include "../str.h"

/* Exit status for failures. */
#define T_FAIL 2

/* Exit status for errors. */
#define T_ERR 3

/* Test case. */
struct args {
	const char *script;
	const char *inter;
	const enum error rc;
};


/* Tests. */
const struct args tests[] = {
	/* Simple errors. */
	{"file", NULL, ERR_ILL},
	{".", NULL, ERR_ILL},
	{".sh", NULL, ERR_ILL},
	{".py", NULL, ERR_ILL},
	{"file.null", NULL, FAIL},
	{"file.empty", NULL, FAIL},
	{"file.py", NULL, FAIL},
	{"file.post", NULL, FAIL},

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

	/* Terminator. */
	{NULL, NULL, OK}
};

/* Script inter database for testing. */
const struct pair db[] = {
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
	for (int i = 0; tests[i].script; i++) {
		const struct args t = tests[i];
		char inter[MAX_STR];
		char script[MAX_STR];
		enum error rc;	

		*script = '\0';
		(void) memset(inter, 0, MAX_STR);

		warnx("checking (db, %s, -> %s) -> %u ...",
		      t.script, t.inter, t.rc);

		rc = script_get_inter(db, t.script, &inter);

		if (t.rc != rc)
			errx(T_FAIL, "returned %u", rc);
		if (t.inter && strcmp(t.inter, inter) != 0)
			errx(T_FAIL, "got interpreter %s", inter);
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
