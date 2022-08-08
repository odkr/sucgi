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
	/* RATS: ignore */
	char huge[STR_MAX + 1U];

	(void) memset(huge, 'c', STR_MAX);
	huge[STR_MAX] = '\0';
	assert(strnlen(huge, STR_MAX) > STR_MAX - 1U);
	assert(str_len(huge, &len) == ERR_STR_MAX);
	assert(0U == len);

	assert(str_len("", &len) == OK);
	assert(0U == len);

	assert(str_len("a", &len) == OK);
	assert(1U == len);

	return EXIT_SUCCESS;
}
