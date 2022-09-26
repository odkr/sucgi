/*
 * Test env_name_valid.
 */

#include <stdio.h>
#include <string.h>

#include "../env.h"
#include "../tools/lib.h"


/* Test case. */
struct tcase {
	const char *name;
	const bool ret;
};


/* Tests. */
const struct tcase tests[] = {
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
		const struct tcase t = tests[i];
		bool ret;

		ret = env_name_valid(t.name);
		if (ret != t.ret) {
			die("env_name_valid '%s' returned %s.\n",
			    t.name, ret ? "true" : "false");
		}
	}

	return EXIT_SUCCESS;
}
