/*
 * Test path_contains.
 */

#include <stdio.h>
#include <string.h>

#include "../path.h"
#include "../tools/lib.h"


/* Test case. */
struct tcase {
	const char *parent;
	const char *fname;
	const bool ret;
};


/* Tests. */
const struct tcase tests[] = {
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
		const struct tcase t = tests[i];
		bool ret;

		ret = path_contains(t.parent, t.fname);
		if (ret != t.ret) {
			die("path_contains %s %s returned %s.\n",
			    t.parent, t.fname, ret ? "true" : "false");
		}
	}

	return EXIT_SUCCESS;
}
