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

#include "../error.h"
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
	{"file", NULL, ERR_SCPT_NO_SFX},
	{".", NULL, ERR_SCPT_ONLY_SFX},
	{".sh", NULL, ERR_SCPT_ONLY_SFX},
	{".py", NULL, ERR_SCPT_ONLY_SFX},
	{"file.null", NULL, ERR_SCPT_NO_HDL},
	{"file.empty", NULL, ERR_SCPT_NO_HDL},
	{"file.py", NULL, ERR_SCPT_NO_HDL},
	{"file.post", NULL, ERR_SCPT_NO_HDL},

	/* Empty string shenanigans. */
	{" ", NULL, ERR_SCPT_NO_SFX},
	{". ", NULL, ERR_SCPT_ONLY_SFX},
	{".sh ", NULL, ERR_SCPT_ONLY_SFX},
	{".py ",NULL, ERR_SCPT_ONLY_SFX},
	{" .null", NULL, ERR_SCPT_NO_HDL},
	{" .empty", NULL, ERR_SCPT_NO_HDL},
	{" .py", NULL, ERR_SCPT_NO_HDL},
	{" .post", NULL, ERR_SCPT_NO_HDL},
	{" . ", NULL, ERR_SCPT_NO_HDL},

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
			char scpt[STR_MAX];	/* RATS: ignore */
			enum error rc;	
			int n;

			*scpt = '\0';
			handler = NULL;

			warnx("checking (hdb, %s%s, -> %s) -> %u ...",
			      prefix, t.scpt, t.handler, t.rc);

			/* RATS: ignore */
			n = snprintf(scpt, STR_MAX, "%s%s", prefix, t.scpt);
			if (n >= (long long) STR_MAX) {
				errx(EXIT_FAILURE, "input too long");
			}

			rc = scpt_get_handler(hdb, scpt, &handler);

			if (t.rc != rc) {
				errx(EXIT_FAILURE,
				     "unexpected return code %u",
				     rc);
			}
			if (!(t.handler == handler ||
			      strcmp(t.handler, handler) == 0))
			{
				errx(EXIT_FAILURE,
				     "unexpected handler %s",
				     handler);
			}
		}
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
