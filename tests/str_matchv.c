/*
 * Test str_matchv.
 */

#include <fnmatch.h>
#include <stdlib.h>

#include "../str.h"
#include "../tools/lib.h"


/* Test case. */
struct tcase {
	const char *s;
	const char *const patv[8];
	const int flags;
	bool ret;
};


/* Tests. */
const struct tcase tests[] = {
	{"foo", {"foo", NULL}, 0, true},
	{"foo", {"f*", NULL}, 0, true},
	{"foo", {"b", "f*", NULL}, 0, true},
	{"foo", {"f*", "b", NULL}, 0, true},
	{"foo", {"f*", "", NULL}, 0, true},
	{"foo", {"", "f*", NULL}, 0, true},
	{".", {".", "*", NULL}, 0, true},
	{"foo", {"bar", NULL}, 0, false},
	{"foo", {"b*", NULL}, 0, false},
	{"foo", {"b*", "z*", NULL}, 0, false},
	{"foo", {"b*", "", NULL}, 0, false},
	{"foo", {"", NULL}, 0, false},
	{".", {"*", NULL}, FNM_PERIOD, false},

	/* Terminator. */
	{NULL, {NULL}, 0, false}
};


int
main (void) {
	for (int i = 0; tests[i].s; i++) {
		struct tcase t = tests[i];
		bool ret;

		ret = str_matchv(t.s, t.patv, t.flags);

		if (ret != t.ret) { 
			croak("test %d: str_matchv '%s' returned %s.",
			      t.s, i, ret ? "true" : "false");
		}
	}

	/* All good. */
	return EXIT_SUCCESS;
}
