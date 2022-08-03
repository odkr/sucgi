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
	/* Flawfinder: ignore */
	char huge[STR_MAX + 1];

	memset(huge, 'c', STR_MAX);
	huge[STR_MAX] = '\0';
	assert(strnlen(huge, STR_MAX) > STR_MAX - 1);
	assert(str_len(huge, &len) == ERR_STR_MAX);
	assert(0 == len);

	assert(str_len("", &len) == OK);
	assert(0 == len);

	assert(str_len("a", &len) == OK);
	assert(1 == len);

	return EXIT_SUCCESS;
}
