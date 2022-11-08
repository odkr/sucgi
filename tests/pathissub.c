/*
 * Test path_is_subdir.
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
#include "testdefs.h"


/* Test case. */
struct args {
	const char *fname;
	const char *parent;
	const bool ret;
};


/* Tests. */
static const struct args tests[] = {
	/* Absolute paths. */
	{"/foo", "/", true},
	{"/foo/bar", "/foo", true},
	{"/bar", "/foo", false},
	{"/foo", "/bar", false},
	{"/foobar", "/foo", false},
	{"foo", "/", false},
	{"/", "/foo", false},
	{"/foo", "/foo", false},
	{"/", "/", false},

	/* Relative paths. */
	{"foo/bar", "foo", true},
	{"foo", "foo", false},
	{"foo", "bar", false},

	/* Leading dot. */
	{"./foo", ".", true},
	{"./foo/bar", "./foo", true},
	{".foo", ".", true},
	{"./foo", "./bar", false},
	{".", "./foo", false},
	{"./", "./foo", false},
	{"./foo", "./foo", false},
	{".", ".", false},
	{".foo", ".f", false},
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
		      t.fname, t.parent, (t.ret) ? "true" : "false");

		ret = path_is_subdir(t.fname, t.parent);
		if (ret != t.ret) {
			char *what = (ret) ? "true" : "false";
			errx(T_FAIL, "path_is_subdir returned %s", what);
		}
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
