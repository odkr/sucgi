/*
 * Test str_cpn.
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

	(void) memset(huge, 'x', STR_MAX);
	(void) memset(large, 'x', STR_MAX - 1);

	/* Test overly long string. */
	memset(huge, 'x', STR_MAX);
	assert(strnlen(huge, STR_MAX + 1) == STR_MAX);
	assert(str_cpn(STR_MAX, huge, &s) == ERR_STR_MAX);
	assert(str_cpn(STR_MAX - 1, huge, &s) == OK);
	assert(str_eq(s, large));

	/* Test long string. */
	memset(large, 'x', STR_MAX - 1);
	assert(strnlen(large, STR_MAX) == STR_MAX - 1);
	assert(str_cpn(STR_MAX - 1, large, &s) == OK);
	assert(str_eq(s, large));

	/* Simple test. */
	assert(str_cpn(STR_MAX - 1, "foo", &s) == OK);
	assert(str_eq(s, "foo"));

	/* Test empty string. */
	assert(str_cpn(STR_MAX - 1, "", &s) == OK);
	assert(str_eq(s, ""));

	/* Test truncation */
	assert(str_cpn(3, "abcd", &s) == OK);
	assert(str_eq(s, "abc"));

	/* All good. */
	return EXIT_SUCCESS;
}
