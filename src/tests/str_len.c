/*
 * Test str_len.
 */

#include <assert.h>
#include <string.h>

#include "../err.h"
#include "../str.h"

int
main (void) {
	size_t len = 0;
	/* flawfinder: ignore */
	char huge[STR_MAX_LEN + 2];
	memset(huge, 'c', STR_MAX_LEN + 1);
	huge[STR_MAX_LEN + 1] = '\0';
	assert(strnlen(huge, STR_MAX_LEN + 1) > STR_MAX_LEN);
	assert(str_len(huge, &len) == ERR_STR_LEN);
	assert(len == 0);

	assert(str_len("", &len) == OK);
	assert(len == 0);

	assert(str_len("a", &len) == OK);
	assert(len == 1);

	return EXIT_SUCCESS;
}
