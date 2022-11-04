/*
 * Test str_cp.
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

#include "../str.h"
#include "../sysdefs.h"
#include "testdefs.h"
#include "testdefs.h"


/* Test case. */
struct args {
	size_t n;
	const char *src;
	const char *dest;
	const enum retcode rc;
};

/* Tests. */
static const struct args tests[] = {
	/* Simple test. */
	{PATH_SIZE - 1U, "foo", "foo", OK},

	/* Almost out of bounds. */
	{1, "x", "x", OK},
	
	/* Truncation. */
	{3, "abcd", "abc", ERR_LEN},

	/* Truncate to 0. */
	{0, "foo", "", ERR_LEN},

	/* Empty strings. */
	{PATH_SIZE - 1U, "", "", OK},
	{1, "", "", OK},
	{0, "", "", OK},

	/* Terminator. */
	{0, NULL, NULL, OK}
};


int
main (void) {
	for (int i = 0; tests[i].src; i++) {
		struct args t = tests[i];
		char dest[PATH_SIZE];	/* RATS: ignore */
		enum retcode rc; 
		
		*dest = '\0';

		warnx("checking (%zu, %s, -> %s) -> %u ...",
		      t.n, t.src, t.dest, t.rc);

		rc = str_cp(t.n, t.src, dest);

		if (rc != t.rc)
			errx(T_FAIL, "returned %u", rc);
		if (!(t.dest == dest || strcmp(t.dest, dest) == 0))
			errx(T_FAIL, "got copy '%s'", dest);
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
