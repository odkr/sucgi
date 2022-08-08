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
	/* RATS: ignore */
	char large[STR_MAX] = {0};	/* A string just within limits. */
	/* RATS: ignore */
	char huge[STR_MAX + 1U] = {0};	/* A string that exceeds STR_MAX. */
	/* RATS: ignore */
	char s[STR_MAX] = {0};		/* A string. */

	/* Test overly long string. */
	(void) memset(huge, 'x', STR_MAX);
	assert(strnlen(huge, STR_MAX + 1U) == STR_MAX);
	assert(str_cp(huge, &s) == ERR_STR_MAX);

	/* Test long string. */
	(void) memset(large, 'x', STR_MAX - 1U);
	assert(strnlen(large, STR_MAX) == STR_MAX - 1U);
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
