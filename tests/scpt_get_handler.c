/*
 * Test scpt_get_handler.
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

#include "../scpt.h"
#include "../str.h"


/* Test case. */
struct args {
	const char *scpt;
	const char *handler;
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

/* Prefixes should make no difference. */
const char *prefixes[] = {
	"", "/", "./", "dir/", "/dir/", " /", NULL
};

/* Script handler database for testing. */
const struct scpt_ent hdb[] = {
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
	for (int i = 0; tests[i].scpt; i++) {
 		for (int j = 0; prefixes[j]; j++) {
			const struct args t = tests[i];
			const char *prefix = prefixes[j];
			const char *handler;
			char scpt[MAX_STR];	/* RATS: ignore */
			enum error rc;	
			int n;

			*scpt = '\0';
			handler = NULL;

			warnx("checking (hdb, %s%s, -> %s) -> %u ...",
			      prefix, t.scpt, t.handler, t.rc);

			/* RATS: ignore */
			n = snprintf(scpt, MAX_STR, "%s%s", prefix, t.scpt);
			if (n >= (long long) MAX_STR) {
				errx(EXIT_FAILURE, "input too long");
			}

			rc = scpt_get_handler(hdb, scpt, &handler);

			if (t.rc != rc) {
				errx(EXIT_FAILURE, "returned %u", rc);
			}
			if (!(t.handler == handler ||
			      strcmp(t.handler, handler) == 0))
			{
				errx(EXIT_FAILURE, "got handler %s", handler);
			}
		}
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
