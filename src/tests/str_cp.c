/*
 * Test str_cp.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "../err.h"
#include "../str.h"


int
main (void) {
	/* Flawfinder: ignore */
	char large[STR_MAX] = "";	/* A string just within limits. */
	/* Flawfinder: ignore */
	char huge[STR_MAX + 1] = "";	/* A string that exceeds STR_MAX. */
	/* Flawfinder: ignore */
	char s[STR_MAX] = "";		/* A string. */

	/* Test overly long string. */
	memset(huge, 'x', STR_MAX);
	assert(strnlen(huge, STR_MAX + 1) == STR_MAX);
	assert(str_cp(huge, &s) == ERR_STR_MAX);

	/* Test long string. */
	memset(large, 'x', STR_MAX - 1);
	assert(strnlen(large, STR_MAX) == STR_MAX - 1);
	assert(str_cp(large, &s) == OK);
	assert(str_eq(s, large));

	/* Simple test. */
	assert(str_cp("foo", &s) == OK);
	assert(str_eq(s, "foo"));

	/* Test empty string. */
	assert(str_cp("", &s) == OK);
	assert(str_eq(s, ""));

	/* All good. */
	return EXIT_SUCCESS;
}
