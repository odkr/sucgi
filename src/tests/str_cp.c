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
	/* flawfinder: ignore */
	char huge[STR_MAX_LEN + 2] = {0};	/* A huge string. */
	/* flawfinder: ignore */
	char str[STR_MAX_LEN + 2] = {0};	/* A string. */

	/* Test overly long string. */
	memset(huge, 'x', STR_MAX_LEN + 1);
	assert(strnlen(huge, STR_MAX_LEN + 2) == STR_MAX_LEN + 1);
	assert(str_cp(huge, str) == ERR_STR_LEN);

	/* Simple test. */
	assert(str_cp("foo", str) == OK);
	assert(str_eq(str, "foo"));

	/* Test empty string. */
	assert(str_cp("", str) == OK);
	assert(str_eq(str, ""));

	/* All good. */
	return EXIT_SUCCESS;
}
