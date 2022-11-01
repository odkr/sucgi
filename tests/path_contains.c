/*
 * Test path_contains.
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

#include "../path.h"

/* Exit status for failures. */
#define T_FAIL 2

/* Test case. */
struct args {
	const char *parent;
	const char *fname;
	const bool ret;
};


/* Tests. */
const struct args tests[] = {
	/* Absolute paths. */
	{"/", "/foo", true},
	{"/foo", "/foo/bar", true},
	{"/foo", "/bar", false},
	{"/bar", "/foo", false},
	{"/foo", "/foobar", false},
	{"/", "foo", false},
	{"/foo", "/", false},
	{"/foo", "/foo", false},
	{"/", "/", false},

	/* Relative paths. */
	{"foo", "foo/bar", true},
	{"foo", "foo", false},
	{"bar", "foo", false},

	/* Leading dot. */
	{".", "./foo", true},
	{"./foo", "./foo/bar", true},
	{".", ".foo", true},
	{"./bar", "./foo", false},
	{"./foo", ".", false},
	{"./foo", "./", false},
	{"./foo", "./foo", false},
	{".", ".", false},
	{".f", ".foo", false},
	{".foo", ".foo", false},

	/* Terminator. */
	{NULL, NULL, false}
};


int
main (void)
{
	for (int i = 0; tests[i].parent; i++) {
		const struct args t = tests[i];
		bool ret;

		warnx("checking (%s, %s) -> %s ...",
		      t.parent, t.fname, (t.ret) ? "true" : "false");

		ret = path_contains(t.parent, t.fname);
		if (ret != t.ret) {
			errx(T_FAIL, "path_contains returned %s",
			     (ret) ? "true" : "false");
		}
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
