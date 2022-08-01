/*
 * Test path_check_len.
 */

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../err.h"
#include "../path.h"
#include "../str.h"


/* Run path_check_len for a path that is longer than len. */
static error
test_longer_than(size_t len)
{
	size_t n = len + 1;
	/* Flawfinder: ignore */
	char path[n + 1];

	memset(path, 'c', n);
	path[n] = '\0';
	assert(strnlen(path, n) > len);

	return path_check_len(path);
}

int
main (void) {
	/* Flawfinder: ignore */
	char cwd[STR_MAX - 1];
	
	assert(test_longer_than(STR_MAX - 1) == ERR_STR_MAX);

#if PATH_MAX > -1
	assert(test_longer_than(PATH_MAX) == ERR_STR_MAX);
#endif

#if NAME_MAX > -1
	assert(test_longer_than(NAME_MAX) == ERR_FNAME_LEN);
#endif

	assert(getcwd(cwd, STR_MAX - 1));
	assert(cwd);
	assert(path_check_len(cwd) == OK);

	return EXIT_SUCCESS;
}
