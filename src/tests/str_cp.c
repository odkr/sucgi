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
	char huge[STR_MAX_LEN + 2] = "";	/* A huge string. */
	str4096 s = "";				/* A string. */
	
	/* Test overly long string. */
	memset(huge, 'x', STR_MAX_LEN + 1);
	assert(strnlen(huge, STR_MAX_LEN + 2) == STR_MAX_LEN + 1);
	assert(str_cp(huge, &s) == ERR_STR_LEN);

	/* Simple test. */
	assert(str_cp("foo", &s) == OK);
	assert(str_eq(s, "foo"));

	/* Test empty string. */
	assert(str_cp("", &s) == OK);
	assert(str_eq(s, ""));

	/* All good. */
	return EXIT_SUCCESS;
}
