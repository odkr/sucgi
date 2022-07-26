/*
 * Test env_is_name.
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

#include "../env.h"
#include "testdefs.h"


/* Test case. */
struct args {
	const char *name;
	const bool ret;
};


/* Tests. */
static const struct args tests[] = {
	/* Invalid names. */
	{"", false},
	{" foo", false},
	{"1foo", false},
	{"=foo", false},
	{"*", false},
	{"FOO ", false},
	{"$(foo)", false},
	{"`foo`", false},

	/* Valid names. */
	{"_", true},
	{"_f", true},
	{"_F", true},
	{"f", true},
	{"F", true},
	{"F_", true},
	{"f0", true},
	{"F0", true},

	/* Terminator. */
	{NULL, false}
};


int
main (void)
{
	for (int i = 0; tests[i].name; i++) {
		const struct args t = tests[i];
		bool ret;

		warnx("checking '%s' -> %s ...",
		      t.name, (t.ret) ? "valid" : "invalid");

		ret = env_is_name(t.name);
		if (ret != t.ret) {
			char *what = (ret) ? "valid" : "invalid";
			errx(T_FAIL, "mistaken for %s", what);
		}
	}

	warnx("all tests passed");
	return EXIT_SUCCESS;
}
