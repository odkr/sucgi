/*
 * Test str_cp.
 */

#include <stdlib.h>
#include <string.h>

#include "../err.h"
#include "../str.h"
#include "../tools/lib.h"


/* Test case. */
struct tcase {
	size_t n;
	const char *src;
	const char *dest;
	const enum error ret;
};

/* Tests. */
const struct tcase tests[] = {
	/* Simple test. */
	{STR_MAX - 1U, "foo", "foo", SC_OK},

	/* Almost out of bounds. */
	{1, "x", "x", SC_OK},
	
	/* Truncation. */
	{3, "abcd", "abc", SC_ERR_STR_LEN},

	/* Truncate to 0. */
	{0, "foo", "", SC_ERR_STR_LEN},

	/* Empty strings. */
	{STR_MAX - 1U, "", "", SC_OK},
	{1, "", "", SC_OK},
	{0, "", "", SC_OK},

	/* Terminator. */
	{0, NULL, NULL, SC_OK}
};


int
main (void) {
	for (int i = 0; tests[i].src; i++) {
		struct tcase t = tests[i];
		char dest[STR_MAX] = {0};	/* RATS: ignore */
		enum error ret; 
		
		ret = str_cp(t.n, t.src, dest);

		if (ret != t.ret) {
			die("str_cp %zu '%s' returned %u.",
			    t.n, t.src, ret);
		}
		if (!(t.dest == dest || strcmp(t.dest, dest) == 0)) {
			die("str_cp %zu '%s' copied '%s'.",
			    t.n, t.src, dest);
		}
	}

	/* All good. */
	return EXIT_SUCCESS;
}
